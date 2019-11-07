/// Date: 2019.10.30
/// 重构为Assistant_v3版本。新增特性：放弃编译为静态lib特性，保证多cpp引用可编译成功；
/// 开放全局插桩方法，便于进行网络调试；新增：使用无锁实现，优化多线程条件下桩函数执行效率
/// 开放tools目录下组件，tools目录下任意源文件可单独引用或拷贝走，不依赖其他静态lib。
/// Data: 2019.10.31
/// 对线程池的worker方法内调用的对象存在已销毁的风险导致的潜在线程安全问题，进行优化

#pragma once
#ifndef ASSISTANT3_H__
#define ASSISTANT3_H__
#include <future>
#include <memory>

#include "http_primitives.h"

#include "core/curl_easy.hpp"
#include "core/curl_global.hpp"
#include "core/curl_multi.hpp"
#include "core/curl_share.hpp"
#include "core/uv_async.hpp"
#include "core/uv_loop.hpp"
#include "core/uv_thread.hpp"
#include "tools/safecontainer.hpp"
#include "tools/safequeue.hpp"
#include "tools/scopeguard.hpp"
namespace assistant {

struct Assistant_v3 {
 private:
  /// 定义为私有类型
  struct assistant_v3_thread_closure {
   public:
    assistant_v3_thread_closure()
        : thread_model_guard(
              std::bind(&assistant_v3_thread_closure::assistant_guard, this)),
          request_notify(
              data.thread_loop,
              std::bind(&assistant_v3_thread_closure::request_notify_cb, this)),
          threadpool_data(
              std::make_shared<assistant_v3_thread_closure_threadpool_data>()),
          multi_socket(multi.multi, &data.thread_loop,
                       std::bind(&assistant_v3_thread_closure::solve_curlmsg_cb,
                                 this, std::placeholders::_1)),
          thread_pool(data.thread_loop) {
      /// 延迟初始化 threadpool_data 内的 ready_notify
      threadpool_data->ready_notify.Init(
          data.thread_loop,
          std::bind(&assistant_v3_thread_closure::ready_notify_cb, this));
      thread_model.ThreadCreate(assistant_worker, static_cast<void *>(&data));
    }

   private:
    /// curl库需进行全局的初始化和反初始化，此成员构造应尽可能早
    assistant::core::libcurl_global_closure init_curl_global;
    /// 使用uv_loop_t的闭包封装
    assistant::core::thread_loop_closure data;
    /// 执行事件循环的线程模型
    assistant::core::libuv_threadmodel thread_model;

   public:
    /// 保存来自外部线程的 assistant::HttpRequest , （无锁实现）线程安全
    assistant::tools::safequeue_closure<assistant::HttpRequest> request_queue;
    /// request_queue 一旦有数据入列，应通知事件循环；异步唤醒闭包
    assistant::core::libuv_async_closure request_notify;

   private:
    /// 用于保存共享句柄的容器
    assistant::core::libcurl_share_container share;
    // 	/// 以下对象会被线程池内运行的函数用到，应保证线程安全性2019.10.31
    /// 定义一个结构体，并以share_ptr进行包裹
    struct assistant_v3_thread_closure_threadpool_data {
      /// 用于保存可直接重用的简单句柄
      assistant::core::libcurl_easy_safequeue reuse_easy;
      /// 用于保存已经就绪（已完成SetInfo，即将与multi绑定）的简单句柄
      assistant::core::libcurl_easy_safequeue ready_easy;
      /// 用于通知工作线程从 ready_easy 中取出 easyHandle 进行处理
      assistant::core::libuv_async_closure ready_notify;
      /// assistant_request_response 闭包（弱指针）的映射关系
      assistant::tools::safemap_closure<
          std::string,
          std::weak_ptr<assistant::core::curl_easy::assistant_request_response>>
          uuid_map;
      /// 默认构造方法、默认析构方法
      assistant_v3_thread_closure_threadpool_data() = default;
      ~assistant_v3_thread_closure_threadpool_data() = default;
      /// 禁用移动构造、复制构造和=号操作符
      assistant_v3_thread_closure_threadpool_data(
          assistant_v3_thread_closure_threadpool_data &&) = delete;
      assistant_v3_thread_closure_threadpool_data(
          const assistant_v3_thread_closure_threadpool_data &) = delete;
      assistant_v3_thread_closure_threadpool_data &operator=(
          const assistant_v3_thread_closure_threadpool_data &) = delete;
    };
    std::shared_ptr<assistant_v3_thread_closure_threadpool_data>
        threadpool_data;
    /// 用于保存 assistant_request_response  闭包的映射关系
    assistant::core::easy_mapping_assistantclosure request_map;
    /// 用于保存multi句柄
    assistant::core::libcurl_multi_closure multi;
    /// 用于处理multi socket的相关闭包
    assistant::core::libcurl_multi_socket_closure multi_socket;
    /// 线程池
    assistant::core::libuv_threadpool_closure thread_pool;
    /// 线程模型的守护对象，析构时一定会执行的保护性回调
    assistant::tools::scope_guard thread_model_guard;

   public:
    typedef std::function<void(assistant::HttpRequest &)>
        HttprequestPileDelegate;
    typedef std::function<void(assistant::HttpResponse &)>
        HttpresponsePileDelegate;
    /// 优化为无锁实现，进一步提升性能 2019.10.30
    static assistant::tools::lockfree_vector_closure<HttprequestPileDelegate>
        &HttprequestPile() {
      static assistant::tools::lockfree_vector_closure<HttprequestPileDelegate>
          pile;
      return pile;
    }
    static assistant::tools::lockfree_vector_closure<HttpresponsePileDelegate>
        &HttpresponsePile() {
      static assistant::tools::lockfree_vector_closure<HttpresponsePileDelegate>
          pile;
      return pile;
    }

   private:
    /// 供线程模型执行，作为事件循环所在线程的函数
    static void assistant_worker(void *data) {
      return static_cast<assistant::core::thread_loop_closure *>(data)
          ->LoopRun();
    }
    /// Assistant对象析构时被执行
    void assistant_guard() {
      /// 打开冻结标志位，即使事件循环中接收到需要处理的句柄。
      /// 根据冻结标志位，不再将闭包丢到线程池中去处理。
      /// TODO: 添加冻结标志位和相关的逻辑处理，避免潜在的内存泄露
      /// 做好对curl muiti的清理，保证在事件循环所在线程执行
      auto opt = std::make_unique<assistant::core::libcurl_easy_closure>(
          assistant::core::libcurl_easy_closure::SpecialOpt::ClearMultiStack);
      threadpool_data->ready_easy.Enqueue(opt);
      threadpool_data->ready_notify.Notify();

      /// 利用future阻塞, 保证全部的poll req都被处理完毕
      /// 待捕获的loop句柄
      auto &loop = data.thread_loop;
      /// 跨线程通知信号
      std::promise<void> signal;
      auto wait_signal = signal.get_future();
      /// 检查的定时器闭包
      assistant::core::libuv_timer_closure poll_check;
      /// 回调方法：检查特定的loop句柄上是否有poll类型的句柄；
      /// 直至没有poll类型的句柄后，停止定时器，并发送特定信号
      auto check_cb = [&loop, &signal, &poll_check]() -> void {
        /// 注意是在生命周期即将结束时进行保障
        /// 遍历即可，不用进行更多优化
        int32_t poll_num = 0;
        auto walk_cb = [](uv_handle_t *handle, void *arg) -> void {
          auto &poll_num = *static_cast<int32_t *>(arg);
          if (UV_POLL == handle->type) {
            ++poll_num;
          }
        };
        uv_walk(&loop, walk_cb, static_cast<void *>(&poll_num));
        if (0 == poll_num) {
          poll_check.Stop();
          signal.set_value();
        }
      };
      poll_check.Init(loop, check_cb);
      /// 1ms的间隔启动定时器
      poll_check.Start(1, 1);
      /// 若使用同步Http请求，事件循环线程此时休眠中
      /// 若不唤醒将死锁；ready_notify实际上不会生效，可复用
      threadpool_data->ready_notify.Notify();
      /// 阻塞性等待，直到检查完毕
      wait_signal.wait();

      /// 之后通知loop的异步回调进行stop
      data.LoopStop();
      /// 阻塞，直到事件循环所在线程返回
      thread_model.ThreadJoin();
    }
    /// 有assistant::HttpRequest入列通知对应的回调：取出进行处理
    /// 事件循环所在线程上执行
    void request_notify_cb() {
      /// 不断取出assistant::HttpRequest
      decltype(request_queue.Dequeue()) request;
      for (; nullptr != (request = request_queue.Dequeue());) {
        SolveHttpRequest(request);
      }
    }
    /// 有easyHandle入列通知对应的回调：取出进行处理
    /// 事件循环所在线程上执行
    void ready_notify_cb() {
      /// 不断取出easyHandle
      decltype(threadpool_data->ready_easy.Dequeue()) easy;
      for (; nullptr != (easy = threadpool_data->ready_easy.Dequeue());) {
        switch (easy->operation) {
          case assistant::core::libcurl_easy_closure::SpecialOpt::None:
            assistant::core::curl_multi::BindEasy(easy, multi);
            break;
          case assistant::core::libcurl_easy_closure::SpecialOpt::
              ClearMultiStack:
            assistant::core::curl_multi::ClearMultiStack(multi);
            break;
          case assistant::core::libcurl_easy_closure::SpecialOpt::
              LimitDownloadSpeed:
          case assistant::core::libcurl_easy_closure::SpecialOpt::
              LimitUploadSpeed:
            assistant::core::curl_multi::SolveSpeedLimit(easy, multi);
            break;
          default:
            break;
        }
      }
    }
    /// 有easy 句柄被处理完毕，对应的回调
    /// 间接被check_multi_info()调用；
    /// 实质还是在事件循环所在线程上执行
    void solve_curlmsg_cb(CURLMsg *message) {
      auto result = message->data.result;
      auto easy =
          assistant::core::curl_multi::UnbindEasy(message->easy_handle, multi);
      easy->result = result;
      SolveEasyHandle(easy);
    }
    /// 处理从外部线程传入的 assistant::HttpRequest
    /// 此函数应在事件循环所在线程被调用
    void SolveHttpRequest(std::unique_ptr<assistant::HttpRequest> &request) {
      const auto kOpts = assistant::HttpRequest::StringToSpcecialOperators(
          request->extends.Get("SpcecialOperators"));
      switch (kOpts) {
        case assistant::HttpRequest_v1::SPCECIALOPERATORS_NORMAL:
          do {
            /// 处理为闭包
            auto closure = std::make_shared<
                assistant::core::curl_easy::assistant_request_response>();
            closure->easy = threadpool_data->reuse_easy.Dequeue();
            if (nullptr == closure->easy) {
              closure->easy =
                  std::make_unique<assistant::core::libcurl_easy_closure>();
            }
            closure->easy->share_handle = share.get_first_weak();
            closure->easy_handle = closure->easy->get_easy();
            closure->request = std::move(request);
            closure->response = std::make_unique<assistant::HttpResponse>();
            auto weak = std::weak_ptr<
                assistant::core::curl_easy::assistant_request_response>(
                closure);

            /// 添加到闭包的映射
            request_map.Put(closure->easy->get_easy(), closure);

            /// 调用线程池进行处理
            /// 由于此对象析构时，无法阻塞式等待线程池执行全部方法
            /// 2019.10.31 存在潜在的线程安全问题，需要进行优化；
            /// 通过将线程池方法中，需要用到的对象抽取出来，以弱指针进行封装
            /// 可避免此对象析构后，才调用相应的方法的问题（this指针指向的内存是悬空的）。
            thread_pool.Run(
                std::bind(assistant_v3_thread_closure::solve_httprequest_worker,
                          threadpool_data, weak),
                nullptr);
          } while (false);
          break;
        case assistant::HttpRequest_v1::SPCECIALOPERATORS_CLEARCACHE:
          do {
            /// TODO: 待实现清理共享缓存相关逻辑
          } while (false);
          break;
        case assistant::HttpRequest_v1::SPCECIALOPERATORS_STOPCONNECT:
          do {
            /// 没有需要在当前事件循环线程同步进行
            /// 此流程放到线程池中进行
            std::shared_ptr<assistant::HttpRequest> req(std::move(request));
            thread_pool.Run(
                std::bind(assistant_v3_thread_closure::solve_stopconnect_worker,
                          threadpool_data, std::move(req)),
                nullptr);
          } while (false);
          break;
        case assistant::HttpRequest_v1::SPCECIALOPERATORS_LIMITDOWNLOADSPEED:
          do {
            std::shared_ptr<assistant::HttpRequest> req(std::move(request));
            thread_pool.Run(
                std::bind(assistant_v3_thread_closure::solve_limitspeed_worker,
                          threadpool_data, std::move(req), false),
                nullptr);

          } while (false);
          break;
        case assistant::HttpRequest_v1::SPCECIALOPERATORS_LIMITUPLOADSPEED:
          do {
            std::shared_ptr<assistant::HttpRequest> req(std::move(request));
            thread_pool.Run(
                std::bind(assistant_v3_thread_closure::solve_limitspeed_worker,
                          threadpool_data, std::move(req), true),
                nullptr);

          } while (false);
          break;
        default:
          break;
      }
    }
    /// 处理传输完毕的 EasyHandle
    /// 此函数应在事件循环所在线程被调用
    void SolveEasyHandle(
        std::unique_ptr<assistant::core::libcurl_easy_closure> &easy) {
      /// 取出 assistant_request_response 闭包并进行处理
      auto closuer = request_map.Get(easy->get_easy());
      closuer->easy = std::move(easy);

      /// 调用线程池进行处理
      thread_pool.Run(
          std::bind(assistant_v3_thread_closure::solve_easyhandle_worker,
                    threadpool_data, std::move(closuer)),
          nullptr);
    }
    /// 异步地利用assistant::HttpRequest对EasyHandle进行SetInfo
    /// 并将就绪的EasyHandle加入 ready_easy 队列
    /// 应在外部线程（比如线程池）调用此函数
    /// 涉及到的对象：ready_easy、ready_notify、uuid_map
    static void solve_httprequest_worker(
        std::weak_ptr<assistant_v3_thread_closure_threadpool_data>
            threadpool_data_weak,
        std::weak_ptr<assistant::core::curl_easy::assistant_request_response>
            &weak) {
      auto threadpool_data = threadpool_data_weak.lock();
      auto closure = weak.lock();
      if (nullptr != closure && nullptr != threadpool_data) {
        /// 调用桩内的代理方法，对assistant::HttpRequest进行处理
        assistant::HttpRequest &the_req = *closure->request;
        auto vec_ptr = HttprequestPile().WeakPtr().lock();
        if (nullptr != vec_ptr) {
          for (const auto &func : *vec_ptr) {
            if (nullptr != func) {
              func(the_req);
            }
          }
        }
        /// 关联uuid
        std::string uuid;
        if (closure->request->extends.Get("uuid", uuid)) {
          threadpool_data->uuid_map.Put(uuid, weak);
        }

        assistant::core::curl_easy::ConfigEasyHandle(
            *closure->request, *closure->response, *closure->easy);
        threadpool_data->ready_easy.Enqueue(closure->easy);
        threadpool_data->ready_notify.Notify();
      }
    }
    /// 在外部线程（如线程池）中运行
    /// 涉及到的对象：uuid_map
    static void solve_stopconnect_worker(
        std::weak_ptr<assistant_v3_thread_closure_threadpool_data>
            threadpool_data_weak,
        std::shared_ptr<assistant::HttpRequest> &request) {
      auto threadpool_data = threadpool_data_weak.lock();
      if (nullptr != request && nullptr != threadpool_data) {
        const auto uuids = request->extends.Get("uuids");
        std::vector<std::string> vec;
        assistant::tools::string::StringSplit(uuids, ";", vec);
        /// 对特定的UUID，取到其中的weak指针，对其中特定字段加true
        auto SolveStopFlag =
            [](const std::weak_ptr<
                assistant::core::curl_easy::assistant_request_response> &weak) {
              auto closure = weak.lock();
              if (nullptr != closure) {
                closure->response->stop_flag = true;
              }
            };
        for (const auto &x : vec) {
          threadpool_data->uuid_map.FindDelegate(x, SolveStopFlag);
        }
      }
    }
    /// 用于限制特定连接的下载速度
    /// 在外部线程（如线程池）中运行
    /// 涉及到的对象：uuid_map,ready_easy,ready_notify
    static void solve_limitspeed_worker(
        std::weak_ptr<assistant_v3_thread_closure_threadpool_data>
            threadpool_data_weak,
        std::shared_ptr<assistant::HttpRequest> &request,
        const bool upload_transfer) {
      auto threadpool_data = threadpool_data_weak.lock();
      if (nullptr != request && nullptr != threadpool_data) {
        const auto uuids = request->extends.Get("uuids");
        const auto kSpeedLimit =
            strtoll(request->extends.Get("speed_limit").c_str(), nullptr, 0);
        std::vector<std::string> vec;
        assistant::tools::string::StringSplit(uuids, ";", vec);
        auto LimitDownloadSpeed =
            [=](const std::weak_ptr<
                assistant::core::curl_easy::assistant_request_response> &weak) {
              auto closure = weak.lock();
              if (nullptr != closure) {
                /// TODO: to be continued.
                auto opt =
                    std::make_unique<assistant::core::libcurl_easy_closure>(
                        upload_transfer
                            ? assistant::core::libcurl_easy_closure::
                                  SpecialOpt::LimitUploadSpeed
                            : assistant::core::libcurl_easy_closure::
                                  SpecialOpt::LimitDownloadSpeed,
                        closure->easy_handle, kSpeedLimit);
                threadpool_data->ready_easy.Enqueue(std::move(opt));
              }
            };
        for (const auto &x : vec) {
          threadpool_data->uuid_map.FindDelegate(x, LimitDownloadSpeed);
        }
        if (!vec.empty()) {
          threadpool_data->ready_notify.Notify();
        }
      }
    }
    /// 异步地利用传输已完毕的EasyHandle进行GetInfo，处理 HttpResponse
    /// 处理完毕后，对EasyHandle重置，加入 reuse_easy 队列
    /// 之后应调用 assistant::HttpRequest 内指定的回调
    /// 应在外部线程（比如线程池）调用此函数
    /// 涉及到的对象：reuse_easy、uuid_map
    static void solve_easyhandle_worker(
        std::weak_ptr<assistant_v3_thread_closure_threadpool_data>
            threadpool_data_weak,
        std::shared_ptr<assistant::core::curl_easy::assistant_request_response>
            &closure) {
      auto threadpool_data = threadpool_data_weak.lock();
      if (nullptr != threadpool_data) {
        assistant::core::curl_easy::ResolveEasyHandle(*closure->easy,
                                                      *closure->response);
        /// 尽快使EasyHandle可被重用 2019.8.28
        closure->easy->reset();
        threadpool_data->reuse_easy.Enqueue(closure->easy);
        /// 取消uuid关联
        std::string uuid;
        if (closure->request->extends.Get("uuid", uuid)) {
          threadpool_data->uuid_map.Delete(uuid);
        }
        /// 调用桩内的代理方法，对assistant::HttpResponse进行处理
        assistant::HttpResponse &the_res = *closure->response;
        auto vec_ptr = HttpresponsePile().WeakPtr().lock();
        if (nullptr != vec_ptr) {
          for (const auto &func : *vec_ptr) {
            if (nullptr != func) {
              func(the_res);
            }
          }
        }

        /// 调用 assistant::HttpRequest 内指定的回调
        if (nullptr != closure->request->solve_func) {
          closure->request->solve_func(*closure->response, *closure->request);
        }
      }
    }

   private:
    assistant_v3_thread_closure(assistant_v3_thread_closure &&) = delete;
    assistant_v3_thread_closure(const assistant_v3_thread_closure &) = delete;
    assistant_v3_thread_closure &operator=(
        const assistant_v3_thread_closure &) = delete;
  };
  std::unique_ptr<assistant_v3_thread_closure> thread_closure;

 public:
  Assistant_v3()
      : thread_closure(std::make_unique<assistant_v3_thread_closure>()) {}
  ~Assistant_v3() = default;
  /// 调用网络库发起异步请求
  void Assistant_v3::AsyncHttpRequest(const assistant::HttpRequest &request) {
    auto i = std::make_unique<assistant::HttpRequest>(request);
    thread_closure->request_queue.Enqueue(i);
    thread_closure->request_notify.Notify();
  }
  /// 调用网络库发起同步请求
  /// 由于HttpResponse在此处返回时，必须被复制构造，应尽可能调用异步方法减少消耗
  assistant::HttpResponse Assistant_v3::SyncHttpRequest(
      const assistant::HttpRequest &request) {
    /// 优化掉一次HttpRequest的深度拷贝
    auto &cast_request = const_cast<assistant::HttpRequest &>(request);
    std::promise<assistant::HttpResponse> httpresponse_promise;
    auto httpresponse_future = httpresponse_promise.get_future();
    decltype(request.solve_func) sync_func =
        [&httpresponse_promise](assistant::HttpResponse &reqsponse,
                                assistant::HttpRequest &request) {
          httpresponse_promise.set_value(reqsponse);
        };
    cast_request.solve_func.swap(sync_func);
    this->AsyncHttpRequest(request);
    cast_request.solve_func.swap(sync_func);
    httpresponse_future.wait();
    auto sync_response = httpresponse_future.get();
    if (nullptr != request.solve_func) {
      request.solve_func(sync_response, cast_request);
    }
    return sync_response;
  }
  /// 增加插桩方法
  static decltype(assistant_v3_thread_closure::HttprequestPile())
  HttprequestPile() {
    return assistant_v3_thread_closure::HttprequestPile();
  }
  static decltype(assistant_v3_thread_closure::HttpresponsePile())
  HttpresponsePile() {
    return assistant_v3_thread_closure::HttpresponsePile();
  }

  Assistant_v3(Assistant_v3 &&) = delete;
  Assistant_v3(const Assistant_v3 &) = delete;
  Assistant_v3 &operator=(const Assistant_v3 &) = delete;
};
}  // namespace assistant

#endif  // ASSISTANT3_H__
