#pragma once
#ifndef _SPEED_COUNTER_H
#define _SPEED_COUNTER_H
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif

#include <atomic>
#include <cinttypes>
#include <functional>

#include <scopeguard.h>
#include <v2/safecontainer.h>
#include <v2/uuid.h>
#include <rxcpp/rx.hpp>

namespace httpbusiness {
struct default_worker {
  static rxcpp::observe_on_one_worker get_worker() {
    static rxcpp::observe_on_one_worker r(
        rxcpp::schedulers::make_scheduler<rxcpp::schedulers::new_thread>());
    return r;
  }
};
struct speed_counter {
 public:
  typedef uint64_t UintType;
  typedef std::function<void(UintType)> OnnextCb;
  typedef std::function<void(void)> OncompleteCb;
  /// 计速器需要：
  /// 开放给外部使用的原子性传输字节数
  /// 传输实际的字节数（由于允许回滚，可能大于processed_bytes）
  std::atomic<UintType> finished_bytes;
  /// 用于数据源OnComplete 的原子bool标志
  std::atomic_bool finished_flag;

 private:
  /// speedcounter使用的工作线程标识
  rxcpp::observe_on_one_worker worker;
  /// 速度的数据源
  rxcpp::observable<UintType> speed_stream;
  /// 平滑后速度的数据源
  rxcpp::observable<UintType> smoothspeed_stream;
  /// 保障退出时不发生内存泄露，显式地取消所有订阅
  assistant::safemap_closure<std::string, rxcpp::composite_subscription>
      subscription_map;
  scopeguard_internal::ScopeGuard guard;

 public:
  /// 默认构造方法
  speed_counter()
      : worker(default_worker::get_worker()),
        finished_bytes(0),
        finished_flag({false}),
        guard([this]() {
          subscription_map.ForeachDelegate(
              [](std::string, rxcpp::composite_subscription cs) {
                if (cs.is_subscribed()) {
                  cs.unsubscribe();
                }
              });
        }) {
    /// 1000ms 计速间隔，瞬时速率更新频率
    const UintType speed_interval = 1000ULL;
    speed_stream =
        rxcpp::observable<>::interval(std::chrono::milliseconds(speed_interval),
                                      speed_counter_thread())
            .take_while([this](int) -> bool { return !finished_flag; })
            .map([this](int) -> UintType { return finished_bytes.load(); })
            .pairwise()
            .map([](std::tuple<UintType, UintType> v) -> UintType {
              return std::get<1>(v) - std::get<0>(v);
            })
            .publish()
            .ref_count();

    smoothspeed_stream =
        speed_stream.take(1)
            .merge(
                speed_stream.take(2).average().map(
                    [](double ave_oe) -> UintType {
                      return static_cast<UintType>(ave_oe);
                    }),
                speed_stream.pairwise().pairwise().map(
                    [](const std::tuple<const std::tuple<UintType, UintType>&,
                                        const std::tuple<UintType, UintType>&>&
                           v) -> UintType {
                      return static_cast<UintType>(
                          (std::get<0>(std::get<0>(v)) +
                           std::get<1>(std::get<0>(v)) +
                           std::get<1>(std::get<1>(v))) /
                          3);
                    }))
            .take_while([this](UintType) -> bool { return !finished_flag; });
  }
  /// 析构方法使用默认
  ~speed_counter() = default;
  std::string RegSubscription(OnnextCb onnext_cb, OncompleteCb oncomplete_cb) {
    std::string flag = assistant::uuid::generate();
    subscription_map.Put(flag, std::move(smoothspeed_stream.subscribe(
                                   onnext_cb, oncomplete_cb)));
    return flag;
  }
  bool UnregSubscription(const std::string& uuid) {
    bool flag = false;
    rxcpp::composite_subscription subscrition;
    if (flag = subscription_map.Take(uuid, subscrition)) {
      subscrition.unsubscribe();
    }
    return flag;
  }

  rxcpp::observe_on_one_worker speed_counter_thread() {
    return worker;
  }

 private:
  /// 禁用掉移动复制构造，=操作符
  speed_counter(speed_counter&& httpresult) = delete;
  speed_counter(const speed_counter&) = delete;
  speed_counter& operator=(const speed_counter&) = delete;
};

struct progress_notifier {
 public:
  typedef uint64_t UintType;
  typedef float FloatType;
  typedef std::function<void(FloatType)> OnnextCb;
  typedef std::function<void(void)> OncompleteCb;
  /// 进度通知器需要：
  /// 开放给外部使用的总数和已完成数
  std::atomic<UintType> finished_number;
  std::atomic<UintType> total_number;
  /// 用于数据源OnComplete 的原子bool标志
  std::atomic_bool finished_flag;

 private:
  /// speedcounter使用的工作线程标识
  rxcpp::observe_on_one_worker worker;
  /// 通知的数据源
  rxcpp::observable<FloatType> notify_stream;
  /// 保障退出时不发生内存泄露，显式地取消所有订阅
  assistant::safemap_closure<std::string, rxcpp::composite_subscription>
      subscription_map;
  scopeguard_internal::ScopeGuard guard;

 public:
  /// 默认构造方法
  progress_notifier()
      : worker(default_worker::get_worker()),
        finished_number(0),
        total_number(0),
        finished_flag({false}),
        guard([this]() {
          subscription_map.ForeachDelegate(
              [](std::string, rxcpp::composite_subscription cs) {
                if (cs.is_subscribed()) {
                  cs.unsubscribe();
                }
              });
        }) {
    ///// 1000ms 计速间隔，瞬时速率更新频率
    const auto notify_interval = 1000ULL;
    notify_stream =
        rxcpp::observable<>::interval(
            std::chrono::milliseconds(notify_interval), worker)
            .take_while([this](int) -> bool { return !finished_flag; })
            .map([this](int) -> FloatType {
              return 0 != total_number
                         ? static_cast<FloatType>(finished_number) /
                               total_number
                         : 0.F;
            })
            .publish()
            .ref_count();

  }
  /// 析构方法使用默认
  ~progress_notifier() = default;
  std::string RegSubscription(OnnextCb onnext_cb, OncompleteCb oncomplete_cb) {
    std::string flag = assistant::uuid::generate();
    subscription_map.Put(
        flag, std::move(notify_stream.subscribe(onnext_cb, oncomplete_cb)));
    return flag;
  }
  bool UnregSubscription(const std::string& uuid) {
    bool flag = false;
    rxcpp::composite_subscription subscrition;
    if (flag = subscription_map.Take(uuid, subscrition)) {
      subscrition.unsubscribe();
    }
    return flag;
  }

 private:
  /// 禁用掉移动复制构造，=操作符
  progress_notifier(progress_notifier&& httpresult) = delete;
  progress_notifier(const progress_notifier&) = delete;
  progress_notifier& operator=(const progress_notifier&) = delete;
};

}  // namespace httpbusiness

#endif  /// _SPEED_COUNTER_H
