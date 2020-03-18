﻿
#ifndef _RX_MULTIWORKER_HH_
#define _RX_MULTIWORKER_HH_

#include <atomic>
#include <functional>
#include <memory>

#include <rxcpp/rx.hpp>

#include <tools/safequeue.hpp>
/// 此处对worker的语义：worker会进行一些费时的操作
/// 此rx封装仅将一系列类似的操作封为一个数据源，
/// 但费时操作由worker自身决定由何外部线程执行

/// 范式所需的元数据：
/// 物料Material（M）
/// 报告Report（R）
/// 过程中可能会需要其他物料，应由worker自身进行保存
/// 并确保过程是线程安全的
/// 保存物料的线程安全队列
/// 初始化：传入若干份物料，将保存到Queue<Material>中
/// 设定worker数的上限（先假定worker 数量，初始传入，后面不会变化）
namespace httpbusiness {
template <class M, class R>
struct rx_multi_worker {
  /// 声明所需用到的类型
 public:
  //////////////////////////////////////////////////////////////////////////
  typedef M Material;
  typedef R Report;
  typedef rxcpp::observable<Report> DataSource;
  typedef std::vector<Material> MaterialVector;
  typedef std::unique_ptr<Material> MaterialType;
  typedef std::unique_ptr<rx_multi_worker> MultiWorkerUnique;
  //////////////////////////////////////////////////////////////////////////
  typedef std::function<MaterialType()> GetMaterialCallback;
  typedef std::function<void(int32_t)> GetMaterialResultCallback;
  /// 消费报告方法
  typedef std::function<void(const Material &)> ExtraMaterialCallback;
  typedef std::function<void(const Report &)> ReportCallback;
  /// 用于传给worker方法调用
  /// 用于检查此物料生产是否已停止
  /// 包含外部要求停止的场景，以及内部出现严重错误，“罢工”
  typedef std::function<bool()> CheckStopCallback;
  typedef std::function<void()> SeriousErrorCallback;

 private:
  struct rx_multi_worker_callbacks_package {
    const GetMaterialCallback get_material;
    const GetMaterialResultCallback get_material_result;
    const ExtraMaterialCallback extra_material;
    const ReportCallback send_report;
    const CheckStopCallback check_stop;
    const SeriousErrorCallback serious_error;

   private:
    /// 禁用默认构造方法
    rx_multi_worker_callbacks_package() = delete;

   public:
    /// 显式构造方法
    rx_multi_worker_callbacks_package(
        GetMaterialCallback get_material_cb,
        GetMaterialResultCallback get_material_result_cb,
        ExtraMaterialCallback extra_material_cb, ReportCallback send_report_cb,
        CheckStopCallback check_stop_cb, SeriousErrorCallback serious_error_cb)
        : get_material(get_material_cb),
          get_material_result(get_material_result_cb),
          extra_material(extra_material_cb),
          send_report(send_report_cb),
          check_stop(check_stop_cb),
          serious_error(serious_error_cb) {}
    /// 对析构方法、移动构造、复制构造和=号操作符使用默认
    ~rx_multi_worker_callbacks_package() = default;
    rx_multi_worker_callbacks_package(rx_multi_worker_callbacks_package &&cbs)
        : get_material(cbs.get_material),
          get_material_result(cbs.get_material_result),
          extra_material(cbs.extra_material),
          send_report(cbs.send_report),
          check_stop(cbs.check_stop),
          serious_error(cbs.serious_error){};
    rx_multi_worker_callbacks_package(
        const rx_multi_worker_callbacks_package &) = default;
    rx_multi_worker_callbacks_package &operator=(
        const rx_multi_worker_callbacks_package &) = default;
  };

 public:
  typedef struct rx_multi_worker_callbacks_package CalledCallbacks;
  typedef std::function<void(CalledCallbacks)> WorkerCallback;
  //////////////////////////////////////////////////////////////////////////

 private:
  struct internal_data {
    std::atomic<bool> stop_flag;
    std::atomic<bool> serious_error;
    assistant::tools::safequeue_closure<Material> material_queue;
    std::atomic_int32_t worker_number;
    std::atomic_int32_t worker_limit;
    internal_data()
        : stop_flag(false),
          serious_error(false),
          worker_number(0),
          worker_limit(0) {}
    /// 禁用移动构造、复制构造、=号操作符
    internal_data(internal_data &&) = delete;
    internal_data(const internal_data &) = delete;
    internal_data &operator=(const internal_data &) = delete;
  };
  std::shared_ptr<internal_data> data;
  /// 令默认构造方法保护
 public:
  rxcpp::observable<Report> data_source;

 private:
  /// 传入初始物料
  rx_multi_worker(const MaterialVector &material, WorkerCallback worker,
                  const int32_t worker_limit)
      : data(std::make_shared<internal_data>()) {
    data->worker_limit = worker_limit;
    for (const auto &x : material) {
      auto tempX = std::make_unique<Material>(x);
      data->material_queue.Enqueue(tempX);
    }
    auto data_weak = std::weak_ptr<internal_data>(data);
    ExtraMaterialCallback extra_material =
        [data_weak](const Material &material) -> void {
      auto data = data_weak.lock();
      if (nullptr != data) {
        auto tempMaterial = std::make_unique<Material>(material);
        data->material_queue.Enqueue(tempMaterial);
      }
    };
    /// worker根据语义，应在取物料前调用此标志位方法
    /// 从而尽快从进行状态停止
    /// worker中可自行实现，从而决定是否忽略此标志位方法
    CheckStopCallback check_stop = [data_weak]() -> bool {
      auto data = data_weak.lock();
      return (nullptr == data) || data->stop_flag.load() ||
             data->serious_error.load();
    };
    SeriousErrorCallback serious_error = [data_weak]() -> void {
      auto data = data_weak.lock();
      if (nullptr != data) {
        data->serious_error = true;
      }
    };
    /// 每次worker调用一次ReportCallback，视为发射了一项数据
    /// 直至worker数为0时（由归零的那个worker）发送完成通知
    data_source = rxcpp::observable<>::create<Report>(
        [worker, data_weak, extra_material, check_stop,
         serious_error](rxcpp::subscriber<Report> s) -> void {
          ReportCallback send_report = [s](const Report &report) -> void {
            s.on_next(report);
          };
          /// 这里解释了“获取物料”这一流程 的秘密
          /// 工人每次调用取料器，潜在的语义是从队列中取了一个物料
          /// 如果这个物料不合法，那么工人可以再次调用一次
          /// 需要注意的是，“物料”是用unique指针包裹起来的，这样可以用nullptr的unique指针代表“物料池空空如也”的语义
          /// 但工人也要说明清楚，自己是刚来的（对传入的int_32为1），老员工（对传入的int_32为0），打算休息（对传入的int_32为-1）
          /// 如果工人是刚来的，获取物料时“物料池空空如也”那么对传入的int_32为0
          /// 这个语义可以扩展到正整数、0和负整数
          GetMaterialCallback get_material = [data_weak,
                                              check_stop]() -> MaterialType {
            auto data = data_weak.lock();
            return (nullptr == data || check_stop())
                       ? MaterialType()
                       : data->material_queue.Dequeue();
          };
          GetMaterialResultCallback get_material_result =
              [data_weak, s, get_material, worker, extra_material, send_report,
               check_stop, &get_material_result,
               serious_error](int32_t result) -> void {
            do {
              auto data = data_weak.lock();
              if (nullptr == data) {
                break;
              }
              /// 当result为0，取此时的值；当result大于0，令原子量自加，获取自加后的值；小于0同理
              const auto kCurrentWorkerNumber =
                  0 == result ? data->worker_number.load()
                              : result > 0 ? ++(data->worker_number)
                                           : --(data->worker_number);
              /// 语义是，有worker即将去休息了，在“最后一位”worker即将去休息时，发送完成通知
              if (result < 0 && 0 == kCurrentWorkerNumber) {
                s.on_completed();
              }
              /// 语义是，有worker仍在工作（刚来有活干，老员工仍有活干），此时的worker数量必然大于0
              /// 排除worker刚来没事干的场景（worker数量为0）【能产生这种场景的情况只有worker首次被执行】
              if (result >= 0 && kCurrentWorkerNumber > 0) {
                /// 如果worker数未达到限制，尝试调来一个新worker（保证一定会传一个非空的物料给他）
                if (!check_stop() &&
                    kCurrentWorkerNumber < data->worker_limit &&
                    data->material_queue.Size() > 0) {
                  if (nullptr != worker) {
                    CalledCallbacks callbacks(
                        std::move(get_material), get_material_result,
                        std::move(extra_material), std::move(send_report),
                        std::move(check_stop), std::move(serious_error));
                    worker(callbacks);
                  }
                }
              }
            } while (false);
          };
          auto data = data_weak.lock();
          if (nullptr != data) {
            /// 增加语义：若待完成队列为空，则应立即complete
            if (data->material_queue.Size() == 0) {
              s.on_completed();
            } else if (nullptr != worker) {
              /// worker如何调用get_material？
              /// 在worker的费时操作开始前：
              /// 应该根据get_material，向get_material_result传入0或1；
              /// 在worker的费时操作结束后：
              ///（且在提交额外生产的物料以及提交生产报告之后），再次尝试get_material，
              /// 向get_material_result传入-1或0
              CalledCallbacks callbacks(
                  std::move(get_material), get_material_result,
                  std::move(extra_material), std::move(send_report),
                  std::move(check_stop), std::move(serious_error));
              worker(callbacks);
            }
          }
        });
  }

 public:
  /// 可定义一个Create方法，用于返回此对象的智能指针包裹
  /// 通过工厂方法产生一个unique_ptr对象
  static MultiWorkerUnique Create(const MaterialVector &material,
                                  WorkerCallback worker,
                                  const int32_t worker_limit) {
    return std::unique_ptr<rx_multi_worker>(
        new (std::nothrow) rx_multi_worker(material, worker, worker_limit));
    /// 使用make_unique带来的好处并不多，但需要声明一个极其复杂的友元类型，所以没必要；std::unique_ptr<>直接使用更方便
  }
  void Stop() { data->stop_flag = true; }
  /// 禁用复制构造、默认构造、移动构造和等号操作符
 private:
  rx_multi_worker() = delete;
  rx_multi_worker(rx_multi_worker &&) = delete;
  rx_multi_worker(const rx_multi_worker &) = delete;
  rx_multi_worker &operator=(const rx_multi_worker &) = delete;
};

/// M, R,如上述所示；I代表中间产物(intermediate products)
/// 此模板类的作用是帮助生成生成器，而无需在外部业务逻辑中定义太多类
/// 只需写好相关的lambda表达式即可
template <class M, class R, class I>
struct rx_multi_worker_helper {
  /// 私有定义，根据M, R定义一个rx_multi_worker的类型
 private:
  typedef rx_multi_worker<M, R> MultiWorkerType;

 public:
  /// M
  typedef typename MultiWorkerType::Material Material;
  /// std::vector<M>
  typedef typename MultiWorkerType::MaterialVector MaterialVector;
  /// rxcpp::observable<Report>
  typedef typename MultiWorkerType::DataSource DataSource;
  /// I
  typedef I IntermediateProducts;
  /// rx_multi_worker_callbacks_package
  typedef typename MultiWorkerType::CalledCallbacks CalledCallbacks;
  /// 通过调用此函数，worker函数汇报中间产物
  typedef std::function<void(IntermediateProducts &)>
      IntermediateProductsFunction;
  /// AsyncWork必须保证：一定会触发且仅触发一次IntermediateProductsFunction
  typedef std::function<void(const Material &,
                             const IntermediateProductsFunction)>
      AsyncWork;
  typedef std::function<void(const IntermediateProducts &,
                             const CalledCallbacks &)>
      AfterWork;
  /// 通过工厂方法产生一个unique_ptr对象
  static std::unique_ptr<rx_multi_worker_helper> Create(
      const MaterialVector &materials, const AsyncWork &async_work_,
      const AfterWork &after_work_, const int32_t max_worker_number) {
    return std::unique_ptr<rx_multi_worker_helper>(
        new (std::nothrow) rx_multi_worker_helper(
            materials, async_work_, after_work_, max_worker_number));
    /// 使用make_unique带来的好处并不多，但需要声明一个极其复杂的友元类型，所以没必要；std::unique_ptr<>直接使用更方便
  }
  DataSource GetDataSource() { return multi_worker_unique->data_source; }
  void Stop() { multi_worker_unique->Stop(); }

 private:
  /// 构造函数，其中应做好初始化
  /// multi_worker_unique
  rx_multi_worker_helper(const MaterialVector &materials,
                         const AsyncWork &async_work_,
                         const AfterWork &after_work_,
                         const int32_t max_worker_number)
      : multi_worker_unique(MultiWorkerType::Create(
            materials,
            std::bind(&rx_multi_worker_helper::multi_worker_worker, this,
                      std::placeholders::_1),
            max_worker_number)),
        async_work(async_work_),
        after_work(after_work_) {}

  typename MultiWorkerType::MultiWorkerUnique const multi_worker_unique;
  const AsyncWork async_work;
  const AfterWork after_work;

  /// 定义一个worker函数
  void multi_worker_worker(CalledCallbacks callbacks) {
    typename MultiWorkerType::MaterialType material_unique = nullptr;
    if (!callbacks.check_stop()) {
      material_unique = callbacks.get_material();
    }
    /// 注意这里是新增的worker的对应的选项，要么新增，要么不变（指物料池空）
    callbacks.get_material_result(nullptr != material_unique ? 1 : 0);
    if (nullptr != material_unique) {
      async_worker_function(
          *material_unique,
          std::bind(&rx_multi_worker_helper::after_work_callback, this,
                    std::placeholders::_1, std::placeholders::_2),
          callbacks);
    }
  }

  /// 在此定义好async_worker_function
  void async_worker_function(
      Material material,
      std::function<void(IntermediateProducts &, CalledCallbacks)>
          done_callback,
      CalledCallbacks callbacks) {
    /// 要保证传入的material，一定会触发且仅触发一次on_done
    IntermediateProductsFunction on_done =
        std::bind(done_callback, std::placeholders::_1, callbacks);
    async_work(material, on_done);
  }

  /// 在此定义好after_work_callback
  void after_work_callback(IntermediateProducts &done_result,
                           CalledCallbacks callbacks) {
    /// 产生额外物料，调用extra_material
    /// 进行报告
    /// 需要检查主控是否已经“frozen”，如有需尽快停止整个multi_worker流程
    /// 以上均为AfterWork的职责
    after_work(done_result, callbacks);

    /// 尝试下一轮生产
    typename MultiWorkerType::MaterialType material_after = nullptr;
    if (!callbacks.check_stop()) {
      material_after = callbacks.get_material();
    }
    /// 注意这里是旧worker的对应的选项，要么不变，要么减少（指物料池空）
    callbacks.get_material_result(nullptr != material_after ? 0 : -1);
    if (nullptr != material_after) {
      async_worker_function(
          *material_after,
          std::bind(&rx_multi_worker_helper::after_work_callback, this,
                    std::placeholders::_1, std::placeholders::_2),
          callbacks);
    }
  }

  /// 构造函数禁用掉
  rx_multi_worker_helper() = delete;
  rx_multi_worker_helper(rx_multi_worker_helper &&) = delete;
  rx_multi_worker_helper(const rx_multi_worker_helper &) = delete;
  rx_multi_worker_helper &operator=(const rx_multi_worker_helper &) = delete;
};
}  // namespace httpbusiness

#endif  /// _RX_MULTIWORKER_HH_
