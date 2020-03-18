#pragma once
#ifndef _RX_FOLDER_DOWNLOADER_HH
#define _RX_FOLDER_DOWNLOADER_HH
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif

#include <atomic>
#include <cinttypes>
#include <functional>

#include <rxcpp/rx.hpp>

//#include <tools/safecontainer.hpp>
//#include <tools/scopeguard.hpp>
//#include <tools/uuid.hpp>

#include <speed_counter.hpp>

namespace httpbusiness {

namespace folder_downloader {
namespace proof {
/// 形式：HttpRequest->HttpResult->Proof
enum folder_downloader_stage {
  FolderDownloadInitial = -1,  /// 作为上传流程的初态
  GetServerFolderInfo = 0,
  CheckLocalFolder,
  // GenerateTempFile,
  ResolveEachSubFolder,
  // CheckFileMd5,
  FolderDownloadFinal,  /// 作为上传流程的终态
};
enum stage_result {
  Unfinished = -1,
  Succeeded = 0,
  GiveupRetry,
  RetrySelf,
  RetryTargetStage,
  UserCanceled,  /// 在用户手动点击“暂停”或取消，或“退出登录”等需要迫使流程立即失败的场景
                 /// 避免GiveupRetry承担过多的语义，并且被调用者误用
};
struct folder_downloader_proof {
  folder_downloader_stage stage;  /// 当前流程所处状态
  stage_result result;  /// 当前阶段所处状态，若未完成则为：Unfinished
  folder_downloader_stage next_stage;
  /// 单位为毫秒ms。在result为RetrySelf，指定的建议重试时间，从外部读取。读不到可为0。
  uint32_t suggest_waittime;
  // uint64_t      transfered_length;

  ///

  /// 由于全部为基本类型，指定移动构造、复制构造和=号操作符为默认即可
};
typedef std::function<rxcpp::observable<folder_downloader_proof>(
    folder_downloader_proof)>
    ProofObsCallback;
struct proof_obs_packages {
  const ProofObsCallback get_server_folder_info;
  const ProofObsCallback check_local_folder;
  const ProofObsCallback resolve_each_sub_folder;
};

}  // namespace proof

struct rx_folder_downloader {
 public:
  /// 存在抛到外部线程延时操作的行为，通过将内部数据包裹到弱指针中，保证线程安全性
  struct rx_folder_downloader_data {
    /// 这个变量表明当前处于哪个阶段
    std::atomic<proof::folder_downloader_stage> current_stage;

    /// 这几个变量作为各阶段的错误计数器
    std::atomic<uint32_t> get_server_folder_info_total_error_count;
    /// 区分total和current的原因是，在总上传数据量增长的情况下，会重置此current计数器
    std::atomic<uint32_t> get_server_folder_info_current_error_count;
    /// 作为currernt计数器超过此上限数值时，放弃重试
    const uint32_t error_count_limit;
    /// 保存此对象生命周期内的“最大”总上传数据量，目的是此值一旦增加，则清零所有的current计数器
    // std::atomic<uint64_t> max_transfered_length;
    /// 供阻塞式等待
    std::promise<void> wait;
    /// 构造时必须传入
    const proof::proof_obs_packages orders;

   public:
    explicit rx_folder_downloader_data(
        const proof::proof_obs_packages &proof_orders)
        : orders(proof_orders),
          current_stage(proof::FolderDownloadInitial),
          get_server_folder_info_total_error_count(0),
          get_server_folder_info_current_error_count(0),
          error_count_limit(9)
    //		,
    //          max_transfered_length(0)
    {}

   private:
    /// 禁止默认构造、复制构造、移动构造和=号操作符
    rx_folder_downloader_data() = delete;
    rx_folder_downloader_data(rx_folder_downloader_data &&) = delete;
    rx_folder_downloader_data(const rx_folder_downloader_data &) = delete;
    rx_folder_downloader_data &operator=(const rx_folder_downloader_data &) =
        delete;
  };
  std::shared_ptr<rx_folder_downloader_data> data;

 private:
  static rxcpp::observable<proof::folder_downloader_proof>
  GenerateDelayedObservable(
      const uint32_t delay_milliseconeds,
      const proof::ProofObsCallback &callback,
      const proof::folder_downloader_proof &target_proof) {
    return rxcpp::observable<>::timer(
               std::chrono::milliseconds(delay_milliseconeds),
               httpbusiness::default_worker::get_worker())
        .flat_map(
            [callback, target_proof](int) { return callback(target_proof); });
  }

 private:
  static rxcpp::observable<proof::folder_downloader_proof>
  GenerateFirstObservable() {
    proof::folder_downloader_proof first_proof = {
        proof::FolderDownloadInitial, proof::Unfinished,
        proof::FolderDownloadFinal, 0};
    return rxcpp::observable<>::just(first_proof);
  }

  static rxcpp::observable<proof::folder_downloader_proof>
  GenerateNextObservable(
      rxcpp::observable<proof::folder_downloader_proof> previous_obs,
      std::weak_ptr<rx_folder_downloader_data> data_weak) {
    return previous_obs.flat_map(
        [data_weak](proof::folder_downloader_proof currernt_proof)
            -> rxcpp::observable<proof::folder_downloader_proof> {
          auto data = data_weak.lock();
          rxcpp::observable<proof::folder_downloader_proof> result =
              rxcpp::observable<>::just(proof::folder_downloader_proof{
                  proof::FolderDownloadFinal, proof::GiveupRetry,
                  proof::FolderDownloadInitial, 0});
          do {
            const auto wait_internal_base = 1000.0F;
            const auto pow_base = 1.5F;
            if (nullptr == data) {
              break;
            }

            /// 以此Lambda对result和current_stage进行统一赋值，避免潜在的疏忽导致bug
            auto &current_stage = data->current_stage;
            auto &orders = data->orders;
            auto ResolveNextStage =
                [&result, &current_stage, &orders](
                    const proof::folder_downloader_stage next_stage,
                    const int32_t wait_internal_milliseconds,
                    const bool giveup_retry) -> void {
              switch (next_stage) {
                case proof::GetServerFolderInfo:
                  do {
                    const proof::folder_downloader_proof
                        do_get_server_folder_info_proof = {
                            proof::GetServerFolderInfo, proof::Unfinished,
                            proof::FolderDownloadInitial, 0};
                    result = orders
                                 .get_server_folder_info(
                                     do_get_server_folder_info_proof)
                                 .first();
                    current_stage = do_get_server_folder_info_proof.stage;
                  } while (false);
                  break;
                case proof::CheckLocalFolder:
                  do {
                    const proof::folder_downloader_proof
                        do_check_local_folder_proof = {
                            proof::CheckLocalFolder, proof::Unfinished,
                            proof::FolderDownloadInitial, 0};
                    result =
                        orders.check_local_folder(do_check_local_folder_proof)
                            .first();
                    current_stage = do_check_local_folder_proof.stage;
                  } while (false);
                  break;
                case proof::ResolveEachSubFolder:
                  do {
                    const proof::folder_downloader_proof
                        do_resolve_each_sub_folder_proof = {
                            proof::ResolveEachSubFolder, proof::Unfinished,
                            proof::FolderDownloadInitial, 0};
                    result = orders
                                 .resolve_each_sub_folder(
                                     do_resolve_each_sub_folder_proof)
                                 .first();
                    current_stage = do_resolve_each_sub_folder_proof.stage;
                  } while (false);
                  break;
                case proof::FolderDownloadFinal:
                default:
                  do {
                    const proof::folder_downloader_proof error_proof = {
                        proof::FolderDownloadFinal, proof::GiveupRetry,
                        proof::FolderDownloadInitial, 0};
                    const proof::folder_downloader_proof succeed_proof = {
                        proof::FolderDownloadFinal, proof::Succeeded,
                        proof::FolderDownloadInitial, 0};
                    if (giveup_retry) {
                      result = rxcpp::observable<>::just(error_proof);
                    } else {
                      result = rxcpp::observable<>::just(succeed_proof);
                    }
                    current_stage.store(proof::FolderDownloadFinal);
                  } while (false);
                  break;
              }
            };

            if (proof::UserCanceled == currernt_proof.result) {
              /// 应走错误
              ResolveNextStage(proof::FolderDownloadFinal, 0, true);
              break;
            }
            switch (currernt_proof.stage) {
              case proof::FolderDownloadInitial:
                /// 处理FolderDownloadInitial的场景，无条件启动获取远端文件夹信息的流程即可
                do {
                  ResolveNextStage(proof::GetServerFolderInfo, 0, false);
                } while (false);
                break;
              case proof::GetServerFolderInfo:
                /// 成功：进一步获取文件的远端大小（content-size）
                /// 失败：无需重试的错误，或是重试此处达到上限，则直接失败
                /// 重试：可进行重试的错误，或未知情形
                do {
                  if (proof::Succeeded == currernt_proof.result) {
                    ResolveNextStage(proof::CheckLocalFolder, 0, true);
                    break;
                  }
                  /// 错误计数器
                  data->get_server_folder_info_total_error_count++;
                  const auto current_error_count =
                      data->get_server_folder_info_current_error_count++;
                  /// 失败：
                  if (proof::GiveupRetry == currernt_proof.result ||
                      current_error_count > data->error_count_limit) {
                    ResolveNextStage(proof::FolderDownloadFinal, 0, true);
                    break;
                  }
                  /// 对GetServerFolderInfo进行重试的情况，应该进行指数级等待间隔处理
                  /// 计算规则：wait_internal_base*POW(1.5,current_error_count)
                  const auto wait_internal_milliseconds = static_cast<uint32_t>(
                      pow(pow_base, current_error_count - 1) *
                      wait_internal_base);
                  ResolveNextStage(proof::GetServerFolderInfo,
                                   wait_internal_milliseconds, false);
                } while (false);
                break;
              case proof::CheckLocalFolder:
                /// 成功：进一步去开始对云端目录进行扫描
                /// 失败：流程直接失败
                do {
                  if (proof::Succeeded == currernt_proof.result) {
                    ResolveNextStage(proof::ResolveEachSubFolder, 0, false);
                    break;
                  }

                  /// 由于result有一默认值，与error_proof等价
                  /// 如果遇到需要流程直接失败的场景，这里什么也不用做
                } while (false);
                break;
              case proof::ResolveEachSubFolder:
                /// 成功：代表对云端所有子目录的成功遍历，流程可结束
                /// 失败：遍历云端目录的场景比较特殊，假定此子数据源已做好了重试的工作
                /// 那么此子数据源的失败就已经意味着整个流程的失败，无需再重试了
                do {
                  if (proof::Succeeded == currernt_proof.result) {
                    ResolveNextStage(proof::FolderDownloadFinal, 0, false);
                    break;
                  }

                  /// 由于result有一默认值，与error_proof等价
                  /// 如果遇到需要流程直接失败的场景，这里什么也不用做
                } while (false);
                break;
              default:
                ResolveNextStage(proof::FolderDownloadFinal, 0, true);
                break;
            }
            /// data->current_stage语义是正在进行的stage
          } while (false);
          return result;
        });
  }

  static rxcpp::observable<proof::folder_downloader_proof>
  GenerateIteratorObservable(
      rxcpp::observable<proof::folder_downloader_proof> previous_obs,
      std::weak_ptr<rx_folder_downloader_data> data_weak) {
    return previous_obs.flat_map(
        [data_weak](proof::folder_downloader_proof currernt_proof)
            -> rxcpp::observable<proof::folder_downloader_proof> {
          if (proof::FolderDownloadFinal == currernt_proof.stage) {
            return rxcpp::observable<>::just(currernt_proof);
          } else {
            auto next_obs = GenerateNextObservable(
                rxcpp::observable<>::just(currernt_proof), data_weak);
            return GenerateIteratorObservable(next_obs, data_weak);
          }
        });
  }

 public:
  /// 定义一个回调返回自身
  typedef std::function<void(const rx_folder_downloader &)> CompleteCallback;

 private:
  const CompleteCallback on_complete_callback;
  /// 定义所需的Observable
  rxcpp::connectable_observable<proof::folder_downloader_proof> obs;

 public:
  explicit rx_folder_downloader(const proof::proof_obs_packages &proof_orders,
                                const CompleteCallback complete_callback)
      : data(std::make_shared<rx_folder_downloader_data>(proof_orders)),
        obs(GenerateIteratorObservable(GenerateFirstObservable(), data)
                .tap([complete_callback,
                      this](proof::folder_downloader_proof proof) {
                  if (nullptr != this->data) {
                    data->current_stage = proof::FolderDownloadFinal;
                  }
                  std::promise<void> wait_move;
                  bool move_success = false;
                  if (nullptr != this->data) {
                    wait_move = std::move(data->wait);
                    move_success = true;
                  }
                  if (nullptr != complete_callback) {
                    complete_callback(*this);
                  }
                  if (move_success) {
                    wait_move.set_value();
                  }
                })
                .publish()) {}
  ~rx_folder_downloader() = default;

 private:
  /// 禁用默认构造，禁用复制构造、移动构造和=号操作符
  /// 迫使使用者有对此对象的所有权转移的需求时，包裹在unique_ptr中进行
  rx_folder_downloader() = delete;
  rx_folder_downloader(rx_folder_downloader &&) = delete;
  rx_folder_downloader(const rx_folder_downloader &) = delete;
  rx_folder_downloader &operator=(const rx_folder_downloader &) = delete;

 private:
  /// 生成Observable
 public:
  void AsyncStart() {
    /// 进行connect即可
    obs.connect();
  }
  /// 这个方法定义出来几乎不会在实际的场景中被用到。仅仅便于进行测试而已
  void SyncWait() {
    auto signal = data->wait.get_future();
    signal.wait();
  }
};

/// TODO: 此处的实现有问题，应改为inline或者static函数。
/// 不应使用functional对象
const static rx_folder_downloader::CompleteCallback default_complete_callback =
    [](const rx_folder_downloader &) -> void {};

}  // namespace folder_downloader

}  // namespace httpbusiness

#endif  /// _RX_FOLDER_DOWNLOADER_HH
