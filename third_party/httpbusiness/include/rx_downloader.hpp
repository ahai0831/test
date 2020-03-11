#pragma once
#ifndef _RX_DOWNLOADER_HH
#define _RX_DOWNLOADER_HH
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

namespace downloader {
namespace proof {
/// 形式：HttpRequest->HttpResult->Proof
enum downloader_stage {
  DownloadInitial = -1,  /// 作为上传流程的初态
  GetDownloadUrl = 0,
  GetRemoteFileSize,
  GenerateTempFile,
  FileDownload,
  CheckFileMd5,
  DownloadFinal,  /// 作为上传流程的终态
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
struct downloader_proof {
  downloader_stage stage;  /// 当前上传流程所处状态
  stage_result result;  /// 当前阶段所处状态，若未完成则为：Unfinished

  /// 在stage为CalculateMd5且result为Succeeded，指定下一阶段直接CheckUpload或者CheckUpload;
  /// 在stage为CreateUpload且result为Succeeded，指定下一阶段直接FileCommit或者FileUplaod;
  /// 在stage为CheckUpload且result为Succeeded，指定下一阶段直接FileCommit或者FileUplaod;
  /// 同样在stage为CheckUpload且result为RetryTargetStage，指定下一阶段应CreateUpload;
  /// 在stage为FileCommit且result为RetryTargetStage，指定下一阶段应CheckUpload或者CheckUpload;
  /// 其他情况下，为UploadInitial
  downloader_stage next_stage;
  uint32_t
      suggest_waittime;  /// 单位为毫秒ms。在result为RetrySelf，指定的建议重试时间，从外部读取。读不到可为0。
  uint64_t
      transfered_length;  /// 单位为Byte，相对于整个文件，在续传的情况下，要算上之前已经传输的。
                          /// 在stage为FileDownload时需要传输

  ///

  /// 由于全部为基本类型，指定移动构造、复制构造和=号操作符为默认即可
};
typedef std::function<rxcpp::observable<downloader_proof>(downloader_proof)>
    ProofObsCallback;
struct proof_obs_packages {
  const ProofObsCallback get_download_url;
  const ProofObsCallback get_remote_file_size;
  const ProofObsCallback generate_temp_file;
  const ProofObsCallback file_download;
  const ProofObsCallback check_file_md5;
};

}  // namespace proof

struct rx_downloader {
 public:
  /// 存在抛到外部线程延时操作的行为，通过将内部数据包裹到弱指针中，保证线程安全性
  struct rx_downloader_data {
    /// 这个变量表明当前处于哪个阶段
    std::atomic<proof::downloader_stage> current_stage;

    /// 这几个变量作为各阶段的错误计数器
    std::atomic<uint32_t> get_download_url_total_error_count;
    /// 区分total和current的原因是，在总上传数据量增长的情况下，会重置此current计数器
    std::atomic<uint32_t> get_download_url_current_error_count;
    /// 作为currernt计数器超过此上限数值时，放弃重试
    const uint32_t error_count_limit;
    /// 保存此对象生命周期内的“最大”总上传数据量，目的是此值一旦增加，则清零所有的current计数器
    std::atomic<uint64_t> max_transfered_length;
    /// 供阻塞式等待
    std::promise<void> wait;
    /// 构造时必须传入
    const proof::proof_obs_packages orders;

   public:
    explicit rx_downloader_data(const proof::proof_obs_packages &proof_orders)
        : orders(proof_orders),
          current_stage(proof::DownloadInitial),
          get_download_url_total_error_count(0),
          get_download_url_current_error_count(0),
          error_count_limit(9),
          max_transfered_length(0) {}

   private:
    /// 禁止默认构造、复制构造、移动构造和=号操作符
    rx_downloader_data() = delete;
    rx_downloader_data(rx_downloader_data &&) = delete;
    rx_downloader_data(const rx_downloader_data &) = delete;
    rx_downloader_data &operator=(const rx_downloader_data &) = delete;
  };
  std::shared_ptr<rx_downloader_data> data;

 private:
  static rxcpp::observable<proof::downloader_proof> GenerateDelayedObservable(
      const uint32_t delay_milliseconeds,
      const proof::ProofObsCallback &callback,
      const proof::downloader_proof &target_proof) {
    return rxcpp::observable<>::timer(
               std::chrono::milliseconds(delay_milliseconeds),
               httpbusiness::default_worker::get_worker())
        .flat_map(
            [callback, target_proof](int) { return callback(target_proof); });
  }

 private:
  static rxcpp::observable<proof::downloader_proof> GenerateFirstObservable() {
    proof::downloader_proof first_proof = {
        proof::DownloadInitial, proof::Unfinished, proof::DownloadFinal, 0, 0};
    return rxcpp::observable<>::just(first_proof);
  }

  static rxcpp::observable<proof::downloader_proof> GenerateNextObservable(
      rxcpp::observable<proof::downloader_proof> previous_obs,
      std::weak_ptr<rx_downloader_data> data_weak) {
    return previous_obs.flat_map([data_weak](
                                     proof::downloader_proof currernt_proof)
                                     -> rxcpp::observable<
                                         proof::downloader_proof> {
      auto data = data_weak.lock();
      rxcpp::observable<proof::downloader_proof> result =
          rxcpp::observable<>::just(
              proof::downloader_proof{proof::DownloadFinal, proof::GiveupRetry,
                                      proof::DownloadInitial, 0, 0});
      do {
        const auto wait_internal_base = 1000.0F;
        const auto pow_base = 1.5F;
        if (nullptr == data) {
          break;
        }

        /// 以此Lambda对result和current_stage进行统一赋值，避免潜在的疏忽导致bug
        auto &current_stage = data->current_stage;
        auto &orders = data->orders;
        auto ResolveNextStage = [&result, &current_stage, &orders](
                                    const proof::downloader_stage next_stage,
                                    const int32_t wait_internal_milliseconds,
                                    const bool giveup_retry) -> void {
          switch (next_stage) {
            case proof::GetDownloadUrl:
              do {
                const proof::downloader_proof do_get_download_url_proof = {
                    proof::GetDownloadUrl, proof::Unfinished,
                    proof::DownloadInitial, 0, 0};
                result =
                    orders.get_download_url(do_get_download_url_proof).first();
                current_stage = do_get_download_url_proof.stage;
              } while (false);
              break;
            case proof::GetRemoteFileSize:
              do {
                const proof::downloader_proof do_get_remote_file_size_proof = {
                    proof::GetRemoteFileSize, proof::Unfinished,
                    proof::DownloadInitial, 0, 0};
                result =
                    orders.get_remote_file_size(do_get_remote_file_size_proof)
                        .first();
                current_stage = do_get_remote_file_size_proof.stage;
              } while (false);
              break;
            case proof::GenerateTempFile:
              do {
                const proof::downloader_proof do_generate_temp_file_proof = {
                    proof::GenerateTempFile, proof::Unfinished,
                    proof::DownloadInitial, 0, 0};
                result = orders.generate_temp_file(do_generate_temp_file_proof)
                             .first();
                current_stage = do_generate_temp_file_proof.stage;
              } while (false);
              break;
            case proof::FileDownload:
              do {
                const proof::downloader_proof do_file_download_proof = {
                    proof::FileDownload, proof::Unfinished,
                    proof::DownloadInitial, 0, 0};
                result = orders.file_download(do_file_download_proof).first();
                current_stage = do_file_download_proof.stage;
              } while (false);
              break;
            case proof::CheckFileMd5:
              do {
                const proof::downloader_proof do_check_file_md5_proof = {
                    proof::CheckFileMd5, proof::Unfinished,
                    proof::DownloadInitial, 0, 0};
                result = orders.check_file_md5(do_check_file_md5_proof).first();
                current_stage = do_check_file_md5_proof.stage;
              } while (false);
              break;
            case proof::DownloadFinal:
            default:
              do {
                const proof::downloader_proof error_proof = {
                    proof::DownloadFinal, proof::GiveupRetry,
                    proof::DownloadInitial, 0, 0};
                const proof::downloader_proof succeed_proof = {
                    proof::DownloadFinal, proof::Succeeded,
                    proof::DownloadInitial, 0, 0};
                if (giveup_retry) {
                  result = rxcpp::observable<>::just(error_proof);
                } else {
                  result = rxcpp::observable<>::just(succeed_proof);
                }
                current_stage.store(proof::DownloadFinal);
              } while (false);
              break;
          }
        };

        if (proof::UserCanceled == currernt_proof.result) {
          /// 应走错误
          ResolveNextStage(proof::DownloadFinal, 0, true);
          break;
        }
        switch (currernt_proof.stage) {
          case proof::DownloadInitial:
            /// 处理DownloadInitial的场景，无条件启动获取下载地址的流程即可
            do {
              ResolveNextStage(proof::GetDownloadUrl, 0, false);
            } while (false);
            break;
          case proof::GetDownloadUrl:
            /// 成功：进一步获取文件的远端大小（content-size）
            /// 失败：无需重试的错误，或是重试此处达到上限，则直接失败
            /// 重试：可进行重试的错误，或未知情形
            do {
              if (proof::Succeeded != currernt_proof.result) {
                ResolveNextStage(proof::GetRemoteFileSize, 0, true);
                break;
              }
              /// 错误计数器
              data->get_download_url_total_error_count++;
              const auto current_error_count =
                  data->get_download_url_current_error_count++;
              /// 失败：
              if (proof::GiveupRetry == currernt_proof.result ||
                  current_error_count > data->error_count_limit) {
                ResolveNextStage(proof::DownloadFinal, 0, true);
                break;
              }
              /// 对GetDownloadUrl进行重试的情况，应该进行指数级等待间隔处理
              /// 计算规则：wait_internal_base*POW(1.5,current_error_count)
              const auto wait_internal_milliseconds = static_cast<uint32_t>(
                  pow(pow_base, current_error_count - 1) * wait_internal_base);
              ResolveNextStage(proof::GetDownloadUrl,
                               wait_internal_milliseconds, false);
            } while (false);
            break;
          case proof::GetRemoteFileSize:
            /// 成功：进一步去创建本地临时文件
            /// 失败：重试GetDownloadUrl
            do {
              if (proof::Succeeded == currernt_proof.result) {
                ResolveNextStage(proof::GenerateTempFile, 0, false);
                break;
              }

              /// 由于出错导致需要重新GetDownloadUrl，注意要避免触发无限GetDownloadUrl；
              /// 应给此计数器+1，且同样应进行次数检测
              data->get_download_url_total_error_count++;
              const auto current_error_count =
                  data->get_download_url_current_error_count++;
              if (current_error_count <= data->error_count_limit) {
                ResolveNextStage(proof::GetDownloadUrl, 1000, false);
              }

            } while (false);
            break;
          case proof::GenerateTempFile:
            /// 成功：开始下载流程。
            /// 失败：流程直接失败。因为没有生成什么东西，所以无需额外清理什么东西。
            do {
              if (proof::Succeeded == currernt_proof.result) {
                ResolveNextStage(proof::FileDownload, 0, false);
                break;
              }
              /// 由于result有一默认值，与error_proof等价
              /// 如果遇到需要流程直接失败的场景，这里什么也不用做
            } while (false);
            break;
          case proof::FileDownload:
            /// 成功：传输成功，下一步去CheckFileMd5
            /// 失败：数据传输失败的场景比较特殊，不应直接结束，而应去尝试重新获取下载地址GetDownlaodUrl
            /// 无论成功失败，都应返回传输成功的字节数，注意，包含了本次传输之前的有效字节数
            do {
              if (currernt_proof.transfered_length > 0) {
                const auto previous_max_transfered_length =
                    data->max_transfered_length.load();
                if (previous_max_transfered_length <
                    currernt_proof.transfered_length) {
                  data->max_transfered_length =
                      currernt_proof.transfered_length;
                  data->get_download_url_current_error_count = 0;
                }
              }
              if (proof::Succeeded == currernt_proof.result) {
                ResolveNextStage(proof::CheckFileMd5, 0, false);
                break;
              }
              /// 由于出错导致需要重新GetDownloadUrl，注意要避免触发无限GetDownloadUrl；
              /// 应给此计数器+1，且同样应进行次数检测
              data->get_download_url_total_error_count++;
              const auto current_error_count =
                  data->get_download_url_current_error_count++;
              if (current_error_count <= data->error_count_limit) {
                ResolveNextStage(proof::GetDownloadUrl, 1000, false);
              }
              /// 由于result有一默认值，与error_proof等价
              /// 如果遇到需要流程直接失败的场景，这里什么也不用做
            } while (false);
            break;
          case proof::CheckFileMd5:
            /// 成功；代表传输成功，下一步直接DownloadFinal
            /// 失败：校验失败，应直接流程失败；
            /// TODO:
            /// 由于校验失败，特定场合下应考虑自动从头开始下载，但应有次数限制。
            do {
              if (proof::Succeeded == currernt_proof.result) {
                /// 应走成功
                ResolveNextStage(proof::DownloadFinal, 0, false);
                break;
              }

              /// 由于result有一默认值，与error_proof等价
              /// 如果遇到需要流程直接失败的场景，这里什么也不用做
            } while (false);
            break;
          default:
            ResolveNextStage(proof::DownloadFinal, 0, true);
            break;
        }
        /// data->current_stage语义是正在进行的stage
      } while (false);
      return result;
    });
  }

  static rxcpp::observable<proof::downloader_proof> GenerateIteratorObservable(
      rxcpp::observable<proof::downloader_proof> previous_obs,
      std::weak_ptr<rx_downloader_data> data_weak) {
    return previous_obs.flat_map(
        [data_weak](proof::downloader_proof currernt_proof)
            -> rxcpp::observable<proof::downloader_proof> {
          if (proof::DownloadFinal == currernt_proof.stage) {
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
  typedef std::function<void(const rx_downloader &)> CompleteCallback;

 private:
  const CompleteCallback on_complete_callback;
  /// 定义所需的Observable
  rxcpp::connectable_observable<proof::downloader_proof> obs;

 public:
  explicit rx_downloader(const proof::proof_obs_packages &proof_orders,
                         const CompleteCallback complete_callback)
      : data(std::make_shared<rx_downloader_data>(proof_orders)),
        obs(GenerateIteratorObservable(GenerateFirstObservable(), data)
                .tap([complete_callback, this](proof::downloader_proof proof) {
                  if (nullptr != this->data) {
                    data->current_stage = proof::DownloadFinal;
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
  ~rx_downloader() = default;

 private:
  /// 禁用默认构造，禁用复制构造、移动构造和=号操作符
  /// 迫使使用者有对此对象的所有权转移的需求时，包裹在unique_ptr中进行
  rx_downloader() = delete;
  rx_downloader(rx_downloader &&) = delete;
  rx_downloader(const rx_downloader &) = delete;
  rx_downloader &operator=(const rx_downloader &) = delete;

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
const static rx_downloader::CompleteCallback default_complete_callback =
    [](const rx_downloader &) -> void {};

}  // namespace downloader

}  // namespace httpbusiness

#endif  /// _RX_DOWNLOADER_HH
