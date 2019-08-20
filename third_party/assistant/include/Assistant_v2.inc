#include "Assistant_v2.h"

#include <future>

#include <scopeguard.h>
#include <uv.h>

#include "http_primitives.h"
#include "v2/curl_global.h"
#include "v2/curl_multi.h"
#include "v2/curl_share.h"
#include "v2/safecontainer.h"
#include "v2/thread.h"
#include "v2/uv_async.h"

namespace assistant {
/// 特定线程循环的基本事件循环闭包
struct a1_2 {
  uv_loop_t assist_loop;
  scopeguard_internal::ScopeGuard assist_loop_guard;
  /// 停止通知对应的回调：使事件循环停止
  void stop_notify_cb() {
    uv_walk(&assist_loop, walker, nullptr);
    uv_stop(&assist_loop);
  }
  libuv_async_closure stop_notify;
  void LoopRun() { return (void)uv_run(&assist_loop, UV_RUN_DEFAULT); }
  void LoopStop() { stop_notify.Notify(); }
  a1_2()
      : assist_loop({0}),
        assist_loop_guard([this]() { uv_loop_close(&assist_loop); }) {
    /// 必须在构造方法中执行loop的初始化和退出循环句柄的初始化；
    /// 否则外部的初始化和对析构的调用时序是不确定的，
    /// 潜在地会导致对象即使析构，loop也不会停止（进程无法退出）
    uv_loop_init(&assist_loop);
    stop_notify.Init(assist_loop, std::bind(&a1_2::stop_notify_cb, this));
  }
  static void walker(uv_handle_t *handle, void *arg) {
    uv_close(handle, nullptr);
  }
};

/// 用于存取assistant::HttpRequest(Unique_ptr)的闭包
typedef safequeue_closure<assistant::HttpRequest> a4_1;

/// 特定线程循环的闭包
/// 注意，必须使此闭包的内存分配于堆上
/// 建议利用智能指针持有此闭包的实例
struct a1_0 {
 public:
  a1_0()
      : thread_model_guard(std::bind(&a1_0::assistant_guard, this)),
        request_notify(data.assist_loop,
                       std::bind(&a1_0::request_notify_cb, this)),
        ready_notify(data.assist_loop, std::bind(&a1_0::ready_notify_cb, this)),
        multi_socket(
            multi.multi, &data.assist_loop,
            std::bind(&a1_0::solve_curlmsg_cb, this, std::placeholders::_1)),
        thread_pool(data.assist_loop) {
    thread_model.ThreadCreate(assistant_worker, static_cast<void *>(&data));
  }

 private:
  /// curl库需进行全局的初始化和反初始化，此成员构造应尽可能早
  CurlGlobal init_curl_global;
  /// 使用uv_loop_t的闭包封装
  a1_2 data;
  /// 执行事件循环的线程模型
  a1_1_uv thread_model;

 public:
  /// 保存来自外部线程的 assistant::HttpRequest , （无锁实现）线程安全
  a4_1 request_queue;
  /// request_queue 一旦有数据入列，应通知事件循环；异步唤醒闭包
  libuv_async_closure request_notify;

 private:
  /// 用于保存共享句柄的容器
  a7_3 share;
  /// 用于保存可直接重用的简单句柄
  a3_5 reuse_easy;
  /// 用于保存已经就绪（已完成SetInfo，即将与multi绑定）的简单句柄
  a3_6 ready_easy;
  /// 保存uuid与a3_2闭包（弱指针）的映射关系
  assistant::safemap_closure<std::string, std::weak_ptr<a3_2>> uuid_map;
  /// 用于保存 a3_2 闭包的映射关系
  a3_7 request_map;
  /// 用于通知工作线程从 ready_easy 中取出 easyHandle 进行处理
  libuv_async_closure ready_notify;
  /// 用于保存multi句柄
  a6_1 multi;
  /// 用于处理multi socket的相关闭包
  a6_3 multi_socket;
  /// 线程池
  libuv_threadpool_closure thread_pool;
  /// 线程模型的守护对象，析构时一定会执行的保护性回调
  scopeguard_internal::ScopeGuard thread_model_guard;

 private:
  /// 供线程模型执行，作为事件循环所在线程的函数
  static void assistant_worker(void *data) {
    return static_cast<a1_2 *>(data)->LoopRun();
  }
  /// 外部线程上Assistant对象析构时被执行
  void assistant_guard() {
    /// 打开冻结标志位，即使事件循环中接收到需要处理的句柄。
    /// 根据冻结标志位，不再将闭包丢到线程池中去处理。
    /// TODO: 添加冻结标志位和相关的逻辑处理，避免潜在的内存泄露
    /// 做好对curl muiti的清理，保证在事件循环所在线程执行
    auto opt = std::make_unique<a3_1>(SpecialOpt::ClearMultiStack);
    ready_easy.Enqueue(opt);
    ready_notify.Notify();

    /// 利用future阻塞, 保证全部的poll req都被处理完毕
    /// 待捕获的loop句柄
    auto &loop = data.assist_loop;
    /// 跨线程通知信号
    std::promise<void> signal;
    auto wait_signal = signal.get_future();
    /// 检查的定时器闭包
    libuv_timer_closure poll_check;
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
    ready_notify.Notify();
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
    decltype(ready_easy.Dequeue()) easy;
    for (; nullptr != (easy = ready_easy.Dequeue());) {
      /// TODO: 利用 a6-1 的闭包进行处理
      switch (easy->operation) {
        case SpecialOpt::None:
          a6_1::BindEasy(easy, multi);
          break;
        case SpecialOpt::ClearMultiStack:
          a6_1::ClearMultiStack(multi);
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
    auto easy = a6_1::UnbindEasy(message->easy_handle, multi);
    easy->result = result;
    SolveEasyHandle(easy);
  }
  /// 处理从外部线程传入的 assistant::HttpRequest
  /// 此函数应在事件循环所在线程被调用
  void SolveHttpRequest(std::unique_ptr<assistant::HttpRequest> &request) {
    auto SpcecialOperators = request->extends.Get("SpcecialOperators");
    auto opts = assistant::HttpRequest::SPCECIALOPERATORS_NORMAL;
    if (!SpcecialOperators.empty()) {
      if (SpcecialOperators == "StopConnect") {
        opts = assistant::HttpRequest::SPCECIALOPERATORS_STOPCONNECT;
      } else if (SpcecialOperators == "ClearCache") {
        opts = assistant::HttpRequest::SPCECIALOPERATORS_CLEARCACHE;
      }
    }
    switch (opts) {
      case assistant::HttpRequest_v1::SPCECIALOPERATORS_NORMAL: {
        /// 处理为闭包
        auto closure = std::make_shared<a3_2>();
        closure->easy = reuse_easy.Dequeue();
        if (nullptr == closure->easy) {
          closure->easy = std::make_unique<a3_1>();
        }
        closure->easy->share_handle = share.get_first_weak();
        closure->request = std::move(request);
        closure->response = std::make_unique<assistant::HttpResponse>();
        auto weak = std::weak_ptr<a3_2>(closure);

        /// 添加到闭包的映射
        request_map.Put(closure->easy->get_easy(), closure);

        /// 调用线程池进行处理
        thread_pool.Run(std::bind(&a1_0::solve_httprequest_worker, this, weak),
                        nullptr);
      } break;
      case assistant::HttpRequest_v1::SPCECIALOPERATORS_CLEARCACHE:
        break;
      case assistant::HttpRequest_v1::SPCECIALOPERATORS_STOPCONNECT: {
        /// 没有需要在当前事件循环线程同步进行
        /// 此流程放到线程池中进行
        std::shared_ptr<assistant::HttpRequest> req(std::move(request));
        thread_pool.Run(
            std::bind(&a1_0::solve_stopconnect_worker, this, std::move(req)),
            nullptr);
      } break;
      default:
        break;
    }
  }
  /// 处理传输完毕的 EasyHandle
  /// 此函数应在事件循环所在线程被调用
  void SolveEasyHandle(std::unique_ptr<a3_1> &easy) {
    /// 取出 a3_2 闭包并进行处理
    auto closuer = request_map.Get(easy->get_easy());
    closuer->easy = std::move(easy);

    /// 调用线程池进行处理
    thread_pool.Run(
        std::bind(&a1_0::solve_easyhandle_worker, this, std::move(closuer)),
        nullptr);
  }
  /// 异步地利用assistant::HttpRequest对EasyHandle进行SetInfo
  /// 并将就绪的EasyHandle加入 ready_easy 队列
  /// 应在外部线程（比如线程池）调用此函数
  void solve_httprequest_worker(std::weak_ptr<a3_2> &weak) {
    auto closure = weak.lock();
    if (nullptr != closure) {
      /// 关联uuid
      std::string uuid;
      if (closure->request->extends.Get("uuid", uuid)) {
        uuid_map.Put(uuid, weak);
      }
      a3_2::SetEasyInfo(closure->request, closure->response, closure->easy);
      ready_easy.Enqueue(closure->easy);
      ready_notify.Notify();
    }
  }
  void solve_stopconnect_worker(
      std::shared_ptr<assistant::HttpRequest> &request) {
    if (nullptr != request) {
      auto uuids = request->extends.Get("uuids");
      std::vector<std::string> vec;
      assistant::tools::StringSplit(uuids, ";", vec);
      /// 对特定的UUID，取到其中的weak指针，对其中特定字段加true
      auto SolveStopFlag = [](const std::weak_ptr<a3_2> &weak) {
        auto closure = weak.lock();
        if (nullptr != closure) {
          closure->response->stop_flag = true;
        }
      };
      for (const auto &x : vec) {
        uuid_map.FindDelegate(x, SolveStopFlag);
      }
    }
  }
  /// 异步地利用传输已完毕的EasyHandle进行GetInfo，处理 HttpResponse
  /// 处理完毕后，对EasyHandle重置，加入 reuse_easy 队列
  /// 之后应调用 assistant::HttpRequest 内指定的回调
  /// 应在外部线程（比如线程池）调用此函数
  void solve_easyhandle_worker(std::shared_ptr<a3_2> &closure) {
    a3_2::GetEasyInfo(closure->easy, closure->response);
    /// 取消uuid关联
    std::string uuid;
    if (closure->request->extends.Get("uuid", uuid)) {
      uuid_map.Delete(uuid);
    }
    if (nullptr != closure->request->solve_func) {
      closure->request->solve_func(*closure->response, *closure->request);
    }
    closure->easy->reset();
    reuse_easy.Enqueue(closure->easy);
  }
};

Assistant_v2::Assistant_v2() : thread_closure(std::make_unique<a1_0>()) {}
Assistant_v2::~Assistant_v2() = default;
/// 调用网络库发起异步请求
void Assistant_v2::AsyncHttpRequest(const assistant::HttpRequest &request) {
  auto i = std::make_unique<assistant::HttpRequest>(request);
  thread_closure->request_queue.Enqueue(i);
  thread_closure->request_notify.Notify();
}
/// 调用网络库发起同步请求
/// 由于HttpResponse在此处返回时，必须被复制构造，应尽可能调用异步方法减少消耗
assistant::HttpResponse Assistant_v2::SyncHttpRequest(
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

}  // namespace assistant
