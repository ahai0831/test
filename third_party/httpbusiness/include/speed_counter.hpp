#pragma once
#ifndef _SPEED_COUNTER_H
#define _SPEED_COUNTER_H
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif

#include <atomic>
#include <cinttypes>
#include <functional>

#include <rxcpp/rx.hpp>

#include <tools/safecontainer.hpp>
#include <tools/scopeguard.hpp>
#include <tools/uuid.hpp>

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
  assistant::tools::safemap_closure<std::string, rxcpp::composite_subscription>
      subscription_map;
  assistant::tools::scope_guard guard;

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
    std::string flag = assistant::tools::uuid::generate();
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

  rxcpp::observe_on_one_worker speed_counter_thread() { return worker; }

 private:
  /// 禁用掉移动复制构造，=操作符
  speed_counter(speed_counter&& httpresult) = delete;
  speed_counter(const speed_counter&) = delete;
  speed_counter& operator=(const speed_counter&) = delete;
};

/// 需要处理好如何保证线程安全性
struct speed_counter_with_stop {
 public:
  typedef uint64_t UintType;
  typedef std::function<void(UintType)> OnnextCb;
  typedef std::function<void(void)> OncompleteCb;

 private:
  /// 用于数据源OnComplete 的原子bool标志
  /// 计速器需要：
  /// 传输实际的字节数（由于允许回滚，可能大于processed_bytes）
  std::atomic<UintType> finished_bytes;
  /// 平滑后速度的数据源
  std::function<void()> subscriber_operator;
  const rxcpp::observable<UintType> smoothspeed_stream;

 public:
  const std::function<void(UintType)> Add;
  const std::function<void()> Stop;

 private:
  rxcpp::observable<UintType> InitDataSource() {
    /// 1000ms 计速间隔，瞬时速率更新频率
    const UintType speed_interval = 1000ULL;
    const int StopMagicNumber = 0;
    /// speedcounter使用的工作线程标识
    const auto worker = default_worker::get_worker();

    /// 定制一个Observable，随时可被手动OnComplete
    auto& so = subscriber_operator;
    rxcpp::connectable_observable<int> stop_observable =
        rxcpp::observable<>::create<int>(
            [&so, StopMagicNumber](rxcpp::subscriber<int> s) -> void {
              so = [s, StopMagicNumber]() -> void {
                s.on_next(StopMagicNumber - 1);
              };
            })
            .publish();
    /// TODO: 由于speed_counter用法，基本都会在构造函数中直接被订阅
    /// 所以无需在此处进行connect()
    /// TODO: 需要搞清楚为什么VLD会因此行代码检测出内存泄露
    stop_observable.connect();

    rxcpp::observable<int> interval_with_stop =
        stop_observable
            .merge(worker, rxcpp::observable<>::interval(
                               std::chrono::milliseconds(speed_interval))
                               .map([StopMagicNumber](int) -> int {
                                 return StopMagicNumber;
                               }))
            .take_while([StopMagicNumber](int v) -> bool {
              return StopMagicNumber == v;
            });

    auto& fb = finished_bytes;
    typedef std::tuple<UintType, UintType> Tuple;
    rxcpp::observable<UintType> speed_stream =
        interval_with_stop.map([&fb](int) -> UintType { return fb.load(); })
            .pairwise()
            .map([](Tuple v) -> UintType {
              return std::get<1>(v) - std::get<0>(v);
            })
            .publish()
            .ref_count();

    return speed_stream.take(1).merge(
        speed_stream.take(2).average().map([](double ave_oe) -> UintType {
          return static_cast<UintType>(ave_oe);
        }),
        speed_stream.pairwise().pairwise().map(
            [](const std::tuple<const Tuple&, const Tuple&>& v) -> UintType {
              return static_cast<UintType>((std::get<0>(std::get<0>(v)) +
                                            std::get<1>(std::get<0>(v)) +
                                            std::get<1>(std::get<1>(v))) /
                                           3);
            }));
  }
  /// 保障退出时不发生内存泄露，显式地取消所有订阅
  assistant::tools::safemap_closure<std::string, rxcpp::composite_subscription>
      subscription_map;
  assistant::tools::scope_guard guard;

 public:
  /// 默认构造方法
  speed_counter_with_stop()
      : finished_bytes(0),
        smoothspeed_stream(InitDataSource()),
        Add(std::move(std::function<void(UintType)>(
            [this](UintType v) -> void { finished_bytes += v; }))),
        Stop(std::move(subscriber_operator)),
        guard([this]() {
          subscription_map.ForeachDelegate(
              [](std::string, rxcpp::composite_subscription cs) {
                if (cs.is_subscribed()) {
                  cs.unsubscribe();
                }
              });
        }) {}
  std::string RegSubscription(OnnextCb onnext_cb, OncompleteCb oncomplete_cb) {
    std::string flag = assistant::tools::uuid::generate();
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

 private:
  /// 禁用掉移动复制构造，=操作符
  speed_counter_with_stop(speed_counter_with_stop&& httpresult) = delete;
  speed_counter_with_stop(const speed_counter_with_stop&) = delete;
  speed_counter_with_stop& operator=(const speed_counter_with_stop&) = delete;
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
  assistant::tools::safemap_closure<std::string, rxcpp::composite_subscription>
      subscription_map;
  assistant::tools::scope_guard guard;

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
    std::string flag = assistant::tools::uuid::generate();
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
