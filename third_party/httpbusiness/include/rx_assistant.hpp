#pragma once
#ifndef RX_ASSISTANT
#define RX_ASSISTANT
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif
#include <functional>
#include <memory>
#include <mutex>

#include <Assistant_v3.hpp>

#include <rxcpp/rx.hpp>

#include "speed_counter.hpp"
namespace rx_assistant {

struct default_asssitant_v3 {
  static std::weak_ptr<assistant::Assistant_v3> get_assistant() {
    /// 受部分陈旧架构的影响，它们的线程模型不标准，会导致相应的资源分配异常，导致主线程无法退出
    /// 在此处进行兼容处理，会产生设计内的“内存泄露”（可被相应的工具检测到）
    /// 暂时进行临时性处理（类似Googletest方案）
    /// TODO: 增加对依赖的线程模型的底层检测能力，进行适当的兼容
    /// TODO:
    /// 在项目组织架构中，应保证此源文件仅被引用一次，其他模块通过使用DLL来使用能力
    static std::once_flag flag;
    static std::shared_ptr<assistant::Assistant_v3> r;
    std::call_once(flag, []() {
      r = std::shared_ptr<assistant::Assistant_v3>(
          new (std::nothrow) assistant::Assistant_v3(),
          [](assistant::Assistant_v3*) { /* Do nothing.*/ });
    });
    return r;
  }
};

struct HttpResult {
  assistant::HttpRequest req;
  assistant::HttpResponse res;
  HttpResult(assistant::HttpRequest& res_req, assistant::HttpResponse& res_res)
      : req(std::move(res_req)), res(std::move(res_res)) {}
  HttpResult(HttpResult&& httpresult)
      : req(std::move(httpresult.req)), res(std::move(httpresult.res)) {}
  HttpResult(const HttpResult&) = default;
  HttpResult& operator=(const HttpResult&) = default;
};

struct rx_assistant_factory {
 private:
  std::weak_ptr<assistant::Assistant_v3> _assistant_weak;
  typedef std::function<assistant::HttpRequest(const rx_assistant::HttpResult&)>
      NextResultDelegate;
  typedef std::function<bool(const rx_assistant::HttpResult&,
                             assistant::HttpRequest&)>
      IteratorResultDelegate;
  typedef std::function<bool(const rx_assistant::HttpResult&,
                             assistant::HttpRequest&, int32_t&)>
      DelayedIteratorResultDelegate;

 public:
  explicit rx_assistant_factory(
      const std::weak_ptr<assistant::Assistant_v3>& _weak)
      : _assistant_weak(_weak){};
  /// 废弃声明：不要使用此函数，尽量使用其替代品 rx_httpresult::create
  /// create
  /// 参数：const assistant::HttpRequest& 异步请求的Request
  /// 返回值：rxcpp::observable<rx_assistant::HttpResult>
  /// 请求结果（包含原始Request以及Response）的数据源
  /// 特殊情况：当assistant::Assistant_v3的全部强引用失效时，返回的Observable，不会发射任何数据、通知
  rxcpp::observable<rx_assistant::HttpResult> create(
      const assistant::HttpRequest& async_request) {
    auto request_ptr = std::make_shared<assistant::HttpRequest>(async_request);
    auto assistant_weak = _assistant_weak;
    return rxcpp::observable<>::create<rx_assistant::HttpResult>(
        [request_ptr, assistant_weak](
            rxcpp::subscriber<rx_assistant::HttpResult> s) -> void {
          auto assistant_ptr = assistant_weak.lock();
          if (s.is_subscribed() && nullptr != assistant_ptr) {
            /// 必须以值捕获的方式，保持rxcpp::subscriber<rx_assistant::HttpResult>生命周期
            decltype(request_ptr->solve_func) subscriber_func =
                [s](assistant::HttpResponse& res,
                    assistant::HttpRequest& req) -> void {
              /// 同理，由于移动构造后，会导致subscriber引用计数归零，需要临时保持subscriber生命周期
              auto keep_subscriber = req.solve_func;
              keep_subscriber.swap(req.solve_func);
              s.on_next(std::move(rx_assistant::HttpResult(req, res)));
              s.on_completed();
            };
            request_ptr->solve_func.swap(subscriber_func);
            assistant_ptr->AsyncHttpRequest(*request_ptr);
            request_ptr->solve_func.swap(subscriber_func);
          }
        });
  }
  /// 废弃声明：不要使用此函数，尽量使用其替代品
  /// rx_httpresult::create_with_delay create_with_delay 参数：const
  /// assistant::HttpRequest& 异步请求的Request 参数：const int32_t
  /// 延时，单位毫秒数 参数：const rxcpp::observe_on_one_worker 定时器所在线程
  /// 返回值：rxcpp::observable<rx_assistant::HttpResult>
  /// 请求结果（包含原始Request以及Response）的数据源
  /// 特殊情况：当assistant::Assistant_v3的全部强引用失效时，返回的Observable，不会发射任何数据、通知
  rxcpp::observable<rx_assistant::HttpResult> create_with_delay(
      const assistant::HttpRequest& async_request,
      const int32_t delay_milliseconds,
      const rxcpp::observe_on_one_worker timer_worker) {
    auto request_ptr = std::make_shared<assistant::HttpRequest>(async_request);
    auto assistant_weak = _assistant_weak;
    return rxcpp::observable<>::create<rx_assistant::HttpResult>(
        [request_ptr, assistant_weak, delay_milliseconds,
         timer_worker](rxcpp::subscriber<rx_assistant::HttpResult> s) -> void {
          // 		  auto assistant_ptr = assistant_weak.lock();
          if (s.is_subscribed() /* && nullptr != assistant_ptr*/) {
            /// 必须以值捕获的方式，保持rxcpp::subscriber<rx_assistant::HttpResult>生命周期
            decltype(request_ptr->solve_func) subscriber_func =
                [s](assistant::HttpResponse& res,
                    assistant::HttpRequest& req) -> void {
              /// 同理，由于移动构造后，会导致subscriber引用计数归零，需要临时保持subscriber生命周期
              auto keep_subscriber = req.solve_func;
              keep_subscriber.swap(req.solve_func);
              s.on_next(std::move(rx_assistant::HttpResult(req, res)));
              s.on_completed();
            };
            /// 在delay特定时间后的timer的回调中，启动网络请求
            auto time_delay = rxcpp::observable<>::timer(
                std::chrono::milliseconds(delay_milliseconds), timer_worker);
            time_delay.subscribe([request_ptr, assistant_weak,
                                  subscriber_func](int) {
              auto assistant_ptr = assistant_weak.lock();
              if (nullptr != assistant_ptr) {
                decltype(request_ptr->solve_func) tmp_func = subscriber_func;
                request_ptr->solve_func.swap(tmp_func);
                assistant_ptr->AsyncHttpRequest(*request_ptr);
                request_ptr->solve_func.swap(tmp_func);
              }
            });
          }
        });
  }
  /// 废弃声明：使用此函数毫无意义，尽量直接用flat_map
  /// next_result
  /// 参数：const rxcpp::observable<rx_assistant::HttpResult>&
  /// 上一个异步请求的请求结果的数据源
  /// 参数：std::function<assistant::HttpRequest(rx_assistant::HttpResult)>
  /// 传入参数为rx_assistant::HttpResult，返回值为HttpRequest的回调
  /// 返回值：rxcpp::observable<rx_assistant::HttpResult>
  /// 请求结果（包含原始Request以及Response）的数据源
  /// 特殊情况：当assistant::Assistant_v3的全部强引用失效时，返回的Observable，不会发射任何数据、通知
  rxcpp::observable<rx_assistant::HttpResult> next_result(
      const rxcpp::observable<rx_assistant::HttpResult>& obs,
      NextResultDelegate callback) {
    auto assistant_weak = _assistant_weak;
    return obs.flat_map([assistant_weak, callback](rx_assistant::HttpResult res)
                            -> rxcpp::observable<rx_assistant::HttpResult> {
      rx_assistant_factory ast_factory(assistant_weak);
      return ast_factory.create(callback(res));
    });
  }
  /// 废弃声明：不要使用此函数，尽量使用其替代品 rx_httpresult::loop
  /// iterator_result
  /// 参数：const rxcpp::observable<rx_assistant::HttpResult>&
  /// 上一个异步请求的请求结果的数据源
  /// 参数：std::function<bool(const rx_assistant::HttpResult&,
  /// assistant::HttpRequest&)>
  /// 传入参数为rx_assistant::HttpResult，输出HttpRequest，返回值为迭代是否继续的bool值的回调
  /// 利用同一个Delegate将rx_assistant::HttpResult转换为下一个将要调用的HttpRequest，Delegate返回值为true意味着继续迭代
  /// Delegate返回值为false意味着迭代结束
  /// 返回值：rxcpp::observable<rx_assistant::HttpResult>
  /// 请求结果（包含原始Request以及Response）的数据源
  /// 特殊情况：当assistant::Assistant_v3的全部强引用失效时，返回的Observable，不会发射任何数据、通知
  rxcpp::observable<rx_assistant::HttpResult> iterator_result(
      const rxcpp::observable<rx_assistant::HttpResult>& obs,
      IteratorResultDelegate callback) {
    auto assistant_weak = _assistant_weak;
    return obs.flat_map([assistant_weak, callback](rx_assistant::HttpResult res)
                            -> rxcpp::observable<rx_assistant::HttpResult> {
      rx_assistant_factory ast_factory(assistant_weak);
      assistant::HttpRequest iterate_req("");
      return callback(res, iterate_req)
                 ? ast_factory.iterator_result(ast_factory.create(iterate_req),
                                               callback)
                 : static_cast<rxcpp::observable<rx_assistant::HttpResult>>(
                       rxcpp::observable<>::just(res));
    });
  }
  /// 废弃声明：不要使用此函数，尽量使用其替代品 rx_httpresult::loop_with_delay
  /// iterator_with_delay_result
  /// 参数：const rxcpp::observable<rx_assistant::HttpResult>&
  /// 上一个异步请求的请求结果的数据源
  /// 参数：std::function<bool(const rx_assistant::HttpResult&,
  /// assistant::HttpRequest&)>
  /// 传入参数为rx_assistant::HttpResult，输出HttpRequest，返回值为迭代是否继续的bool值的回调
  /// 利用同一个Delegate将rx_assistant::HttpResult转换为下一个将要调用的HttpRequest，Delegate返回值为true意味着继续迭代
  /// Delegate返回值为false意味着迭代结束
  /// 参数：const rxcpp::observe_on_one_worker 定时器所在线程
  /// 返回值：rxcpp::observable<rx_assistant::HttpResult>
  /// 请求结果（包含原始Request以及Response）的数据源
  /// 特殊情况：当assistant::Assistant_v3的全部强引用失效时，返回的Observable，不会发射任何数据、通知
  rxcpp::observable<rx_assistant::HttpResult> iterator_with_delay_result(
      const rxcpp::observable<rx_assistant::HttpResult>& obs,
      DelayedIteratorResultDelegate callback,
      const rxcpp::observe_on_one_worker timer_worker) {
    auto assistant_weak = _assistant_weak;
    return obs.flat_map([assistant_weak, callback,
                         timer_worker](rx_assistant::HttpResult res)
                            -> rxcpp::observable<rx_assistant::HttpResult> {
      rx_assistant_factory ast_factory(assistant_weak);
      assistant::HttpRequest iterate_req("");
      int32_t delay_milliseconds = 0;
      return callback(res, iterate_req, delay_milliseconds)
                 ? ast_factory.iterator_with_delay_result(
                       delay_milliseconds > 0
                           ? ast_factory.create_with_delay(
                                 iterate_req, delay_milliseconds, timer_worker)
                           : ast_factory.create(iterate_req),
                       callback, timer_worker)
                 : static_cast<rxcpp::observable<rx_assistant::HttpResult>>(
                       rxcpp::observable<>::just(res));
    });
  }

 private:
  /// 禁用默认构造方法、移动构造方法、复制构造方法、"="操作符
  rx_assistant_factory() = delete;
  rx_assistant_factory(rx_assistant_factory&&) = delete;
  rx_assistant_factory(rx_assistant_factory const&) = delete;
  rx_assistant_factory& operator=(rx_assistant_factory const&) = delete;
};

namespace rx_httpresult {
typedef rxcpp::observable<rx_assistant::HttpResult> ObsType;

namespace details {
inline static ObsType create(
    std::shared_ptr<assistant::HttpRequest> request_ptr) {
  auto assistant_weak = default_asssitant_v3::get_assistant();
  return rxcpp::observable<>::create<rx_assistant::HttpResult>(
      [request_ptr,
       assistant_weak](rxcpp::subscriber<rx_assistant::HttpResult> s) -> void {
        auto assistant_ptr = assistant_weak.lock();
        if (s.is_subscribed() && nullptr != assistant_ptr) {
          /// 必须以值捕获的方式，保持rxcpp::subscriber<rx_assistant::HttpResult>生命周期
          decltype(request_ptr->solve_func) subscriber_func =
              [s](assistant::HttpResponse& res,
                  assistant::HttpRequest& req) -> void {
            /// 同理，由于移动构造后，会导致subscriber引用计数归零，需要临时保持subscriber生命周期
            auto keep_subscriber = req.solve_func;
            keep_subscriber.swap(req.solve_func);
            s.on_next(std::move(rx_assistant::HttpResult(req, res)));
            s.on_completed();
          };
          request_ptr->solve_func.swap(subscriber_func);
          assistant_ptr->AsyncHttpRequest(*request_ptr);
          request_ptr->solve_func.swap(subscriber_func);
        } else if (s.is_subscribed()) {
          /// TODO：发射一个负面的响应，保证完备
        }
      });
}
}  // namespace details
inline static ObsType create(const assistant::HttpRequest& async_request) {
  return details::create(
      std::make_shared<assistant::HttpRequest>(async_request));
}
inline static ObsType create_with_delay(
    const assistant::HttpRequest& async_request,
    const int32_t delay_milliseconds) {
  auto request_ptr = std::make_shared<assistant::HttpRequest>(async_request);
  return rxcpp::observable<>::timer(
             std::chrono::milliseconds(delay_milliseconds),
             httpbusiness::default_worker::get_worker())
      .flat_map([request_ptr](int) -> ObsType {
        return details::create(request_ptr);
      });
}
/// loop
/// 参数：const rxcpp::observable<rx_assistant::HttpResult>&
/// 上一个异步请求的请求结果的数据源
/// 参数：std::function<bool(const rx_assistant::HttpResult&,
/// assistant::HttpRequest&)>
/// 传入参数为rx_assistant::HttpResult，输出HttpRequest，返回值为迭代是否继续的bool值的回调
/// 利用同一个Delegate将rx_assistant::HttpResult转换为下一个将要调用的HttpRequest，Delegate返回值为true意味着继续迭代
/// Delegate返回值为false意味着迭代结束
/// 返回值：rxcpp::observable<rx_assistant::HttpResult>
/// 请求结果（包含最后一次Request以及Response）的数据源
/// 特殊情况：当assistant::Assistant_v3的全部强引用失效时，返回的Observable，不会发射任何数据、通知
inline static ObsType loop(const ObsType& obs,
                           std::function<bool(const rx_assistant::HttpResult&,
                                              assistant::HttpRequest&)>
                               callback) {
  return obs.flat_map([callback](rx_assistant::HttpResult& res) -> ObsType {
    assistant::HttpRequest iterate_req("");
    return callback(res, iterate_req)
               ? loop(create(iterate_req), callback)
               : static_cast<ObsType>(rxcpp::observable<>::just(res));
  });
}

/// loop_with_delay
/// 参数：const rxcpp::observable<rx_assistant::HttpResult>&
/// 上一个异步请求的请求结果的数据源
/// 参数：std::function<bool(const rx_assistant::HttpResult&,
/// assistant::HttpRequest&, int32_t&)>
/// 传入参数为rx_assistant::HttpResult，输出HttpRequest和延迟的毫秒数，返回值为迭代是否继续的bool值的回调
/// 利用同一个Delegate将rx_assistant::HttpResult转换为下一个将要调用的HttpRequest，Delegate返回值为true意味着继续迭代
/// Delegate返回值为false意味着迭代结束
/// 返回值：rxcpp::observable<rx_assistant::HttpResult>
/// 请求结果（包含最后一次Request以及Response）的数据源
/// 特殊情况：当assistant::Assistant_v3的全部强引用失效时，返回的Observable，不会发射任何数据、通知
inline static ObsType loop_with_delay(
    const ObsType& obs, std::function<bool(const rx_assistant::HttpResult&,
                                           assistant::HttpRequest&, int32_t&)>
                            callback) {
  return obs.flat_map([callback](rx_assistant::HttpResult& res) -> ObsType {
    assistant::HttpRequest iterate_req("");
    int32_t delay_milliseconds = 0;
    return callback(res, iterate_req, delay_milliseconds)
               ? loop_with_delay(
                     delay_milliseconds > 0
                         ? create_with_delay(iterate_req, delay_milliseconds)
                         : create(iterate_req),
                     callback)
               : static_cast<ObsType>(rxcpp::observable<>::just(res));
  });
}

}  // namespace rx_httpresult
}  // namespace rx_assistant
#endif  /// RX_ASSISTANT
