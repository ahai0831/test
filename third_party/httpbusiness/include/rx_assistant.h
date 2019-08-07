#pragma once
#ifndef RX_ASSISTANT
#define RX_ASSISTANT
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif
#include <functional>
#include <memory>

#include <Assistant_v2.h>
#include <rxcpp/rx.hpp>
namespace rx_assistant {
struct HttpResult {
  assistant::HttpRequest req;
  assistant::HttpResponse res;
  HttpResult(assistant::HttpRequest& res_req, assistant::HttpResponse& res_res)
      : req(std::move(res_req)), res(std::move(res_res)) {}
  HttpResult(HttpResult&& httpresult)
      : req(std::move(httpresult.req)), res(std::move(httpresult.res)) {}
};

struct rx_assistant_factory {
 private:
  std::weak_ptr<assistant::Assistant_v2> _assistant_weak;
  typedef std::function<assistant::HttpRequest(const rx_assistant::HttpResult&)>
      NextResultDelegate;
  typedef std::function<bool(const rx_assistant::HttpResult&,
                             assistant::HttpRequest&)>
      IteratorResultDelegate;
  /// TODO:
  /// 考虑到潜在的"重试"和幂等重试间隔策略，后续也许可增加IteratorObservableDelegate

 public:
  explicit rx_assistant_factory(
      const std::weak_ptr<assistant::Assistant_v2>& _weak)
      : _assistant_weak(_weak){};
  /// create
  /// 参数：const assistant::HttpRequest& 异步请求的Request
  /// 返回值：rxcpp::observable<rx_assistant::HttpResult>
  /// 请求结果（包含原始Request以及Response）的数据源
  /// 特殊情况：当assistant::Assistant_v2的全部强引用失效时，返回的Observable，不会发射任何数据、通知
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
  /// next_result
  /// 参数：const rxcpp::observable<rx_assistant::HttpResult>&
  /// 上一个异步请求的请求结果的数据源
  /// 参数：std::function<assistant::HttpRequest(rx_assistant::HttpResult)>
  /// 传入参数为rx_assistant::HttpResult，返回值为HttpRequest的回调
  /// 返回值：rxcpp::observable<rx_assistant::HttpResult>
  /// 请求结果（包含原始Request以及Response）的数据源
  /// 特殊情况：当assistant::Assistant_v2的全部强引用失效时，返回的Observable，不会发射任何数据、通知
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
  /// 特殊情况：当assistant::Assistant_v2的全部强引用失效时，返回的Observable，不会发射任何数据、通知
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
                 : rxcpp::observable<>::just(res);
    });
  }

 private:
  /// 禁用默认构造方法、移动构造方法、复制构造方法、"="操作符
  rx_assistant_factory() = delete;
  rx_assistant_factory(rx_assistant_factory&&) = delete;
  rx_assistant_factory(rx_assistant_factory const&) = delete;
  rx_assistant_factory& operator=(rx_assistant_factory const&) = delete;
};
}  // namespace rx_assistant
#endif  /// RX_ASSISTANT
