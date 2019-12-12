#pragma once
#ifndef _RX_UPLOADER_HH
#define _RX_UPLOADER_HH

#include <cinttypes>
#include <cmath>

#include <rxcpp/rx.hpp>

#include <speed_counter.hpp>

namespace httpbusiness {
/// 注意对外部工具方法的传入，需要有以下关键选项
/// 建立文件句柄放在最外部，无需传入
/// 0: 计算本地文件MD5
/// 1: 建立续传记录
/// 2: 查询续传状态
/// 3: 数据上传
/// 4: 提交续传记录
/// 完成的通知有两种状态：成功，失败（用户手动取消、达到最大重试次数、无需重试的错误、传输时遇到的严重错误）
/// 所有的对一个状态的处理都会有若干个结果，应根据结果的反馈进行恰当的处理，直至最终一步是完成（再次强调成功和失败都作为完成）状态。对于应该进行处理的：
/// 计算MD5，会有两个结果，计算成功或计算失败。计算失败则直接返回“文件不可读取”。建立续传会有三种情况：建立成功可秒传，直接跳4；建立成功需上传数据，跳2；
/// 建立失败（达到最大重试次数限制、无需重试的错误）

/// 分层思想：最外层，总控的范式
/// 第二层，提供能力，生成HttpResult的处理结果，并提供根据处理结果翻译情况的能力

/// 假定相关worker都已经知道怎么获取物料（必需的信息），那么总控（调度处）只需要发起一个AsyncStart()处理的指令，即可开始。
namespace uploader {
namespace proof {
/// 形式：HttpRequest->HttpResult->Proof
enum uploader_stage {
  UploadInitial = -1,  /// 作为上传流程的初态
  CalculateMd5 = 0,
  CreateUpload,
  CheckUpload,
  FileUplaod,
  FileCommit,
  UploadFinal,  /// 作为上传流程的终态
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
struct uploader_proof {
  uploader_stage stage;  /// 当前上传流程所处状态
  stage_result result;  /// 当前阶段所处状态，若未完成则为：Unfinished

  /// 在stage为CalculateMd5且result为Succeeded，指定下一阶段直接CheckUpload或者CheckUpload;
  /// 在stage为CreateUpload且result为Succeeded，指定下一阶段直接FileCommit或者FileUplaod;
  /// 在stage为CheckUpload且result为Succeeded，指定下一阶段直接FileCommit或者FileUplaod;
  /// 同样在stage为CheckUpload且result为RetryTargetStage，指定下一阶段应CreateUpload;
  /// 在stage为FileCommit且result为RetryTargetStage，指定下一阶段应CheckUpload或者CheckUpload;
  /// 其他情况下，为UploadInitial
  uploader_stage next_stage;
  uint32_t
      suggest_waittime;  /// 单位为毫秒ms。在result为RetrySelf，指定的建议重试时间，从外部读取。读不到可为0。
  uint64_t
      transfered_length;  /// 单位为Byte，相对于整个文件，在续传的情况下，要算上之前已经传输的。
                          /// 在stage为FileUplaod时需要传输

  ///

  /// 由于全部为基本类型，指定移动构造、复制构造和=号操作符为默认即可
};
typedef std::function<rxcpp::observable<uploader_proof>(uploader_proof)>
    ProofObsCallback;
struct proof_obs_packages {
  const ProofObsCallback calculate_md5;
  const ProofObsCallback create_upload;
  const ProofObsCallback check_upload;
  const ProofObsCallback file_uplaod;
  const ProofObsCallback file_commit;
};

}  // namespace proof

struct rx_uploader {
 public:
  /// 存在抛到外部线程延时操作的行为，通过将内部数据包裹到弱指针中，保证线程安全性
  struct rx_uploader_data {
    /// 这个变量表明当前处于哪个阶段
    std::atomic<proof::uploader_stage> current_stage;

    /// 这几个变量作为各阶段的错误计数器
    std::atomic<uint32_t> create_upload_total_error_count;
    std::atomic<uint32_t> check_upload_total_error_count;
    /// 区分total和current的原因是，在总上传数据量增长的情况下，会重置此current计数器
    std::atomic<uint32_t> create_upload_current_error_count;
    std::atomic<uint32_t> check_upload_current_error_count;
    /// 作为currernt计数器超过此上限数值时，放弃重试
    const uint32_t error_count_limit;
    /// 保存此对象生命周期内的“最大”总上传数据量，目的是此值一旦增加，则清零所有的current计数器
    std::atomic<uint64_t> max_transfered_length;
    /// 供阻塞式等待
    std::promise<void> wait;
    /// 构造时必须传入
    const proof::proof_obs_packages orders;

   public:
    explicit rx_uploader_data(const proof::proof_obs_packages &proof_orders)
        : orders(proof_orders),
          current_stage({proof::UploadInitial}),
          create_upload_total_error_count({0}),
          create_upload_current_error_count({0}),
          check_upload_total_error_count({0}),
          check_upload_current_error_count({0}),
          error_count_limit(9),
          max_transfered_length({0}) {}

   private:
    /// 禁止默认构造、复制构造、移动构造和=号操作符
    rx_uploader_data() = delete;
    rx_uploader_data(rx_uploader_data &&) = delete;
    rx_uploader_data(const rx_uploader_data &) = delete;
    rx_uploader_data &operator=(const rx_uploader_data &) = delete;
  };
  std::shared_ptr<rx_uploader_data> data;

 private:
  static rxcpp::observable<proof::uploader_proof> GenerateDelayedObservable(
      const uint32_t delay_milliseconeds,
      const proof::ProofObsCallback &callback,
      const proof::uploader_proof &target_proof) {
    return rxcpp::observable<>::timer(
               std::chrono::milliseconds(delay_milliseconeds),
               httpbusiness::default_worker::get_worker())
        .flat_map(
            [callback, target_proof](int) { return callback(target_proof); });
  }

 private:
  static rxcpp::observable<proof::uploader_proof> GenerateFirstObservable() {
    proof::uploader_proof first_proof = {
        proof::UploadInitial, proof::Unfinished, proof::UploadInitial, 0, 0};
    return rxcpp::observable<>::just(first_proof);
  }

  static rxcpp::observable<proof::uploader_proof> GenerateNextObservable(
      rxcpp::observable<proof::uploader_proof> previous_obs,
      std::weak_ptr<rx_uploader_data> data_weak) {
    return previous_obs.flat_map([data_weak](
                                     proof::uploader_proof currernt_proof)
                                     -> rxcpp::observable<
                                         proof::uploader_proof> {
      auto data = data_weak.lock();
      rxcpp::observable<proof::uploader_proof> result;
      do {
        const proof::uploader_proof error_proof = {
            proof::UploadFinal, proof::GiveupRetry, proof::UploadInitial, 0, 0};
        const proof::uploader_proof succeed_proof = {
            proof::UploadFinal, proof::Succeeded, proof::UploadInitial, 0, 0};
        const proof::uploader_proof do_calculate_md5_proof = {
            proof::CalculateMd5, proof::Unfinished, proof::UploadInitial, 0, 0};
        const proof::uploader_proof do_create_upload_proof = {
            proof::CreateUpload, proof::Unfinished, proof::UploadInitial, 0, 0};
        const proof::uploader_proof do_check_upload_proof = {
            proof::CheckUpload, proof::Unfinished, proof::UploadInitial, 0, 0};
        const proof::uploader_proof do_file_upload_proof = {
            proof::FileUplaod, proof::Unfinished, proof::UploadInitial, 0, 0};
        const proof::uploader_proof do_file_commit_proof = {
            proof::FileCommit, proof::Unfinished, proof::UploadInitial, 0, 0};
        const auto wait_internal_base = 1000.0F;
        const auto pow_base = 1.5F;
        if (nullptr == data || proof::UserCanceled == currernt_proof.result) {
          result = rxcpp::observable<>::just(error_proof);
          break;
        }

        auto &orders = data->orders;
        switch (currernt_proof.stage) {
          case proof::UploadInitial:
            /// 处理UploadInitial的场景，无条件启动MD5计算的流程即可
            do {
              /// 需要注意，避免会发射多个数据项，应加上first()，下同
              result = orders.calculate_md5(do_calculate_md5_proof).first();
              data->current_stage = do_calculate_md5_proof.stage;
            } while (false);
            break;
          case proof::CalculateMd5:
            /// TODO:
            /// 确认：在文件无法读取、遇到硬件级异常的情况下，能够判断出MD5计算失败的结果
            /// 处理计算MD5的场景，计算失败，流程直接失败
            /// 计算成功,若结果中指定了应直接CheckUpload，则直接查询续传记录；
            /// 计算成功的其他情况，一律直接CreateUpload，重新创建续传记录
            do {
              if (proof::Succeeded != currernt_proof.result) {
                result = rxcpp::observable<>::just(error_proof);
                data->current_stage = error_proof.stage;
                break;
              }
              if (proof::CheckUpload == currernt_proof.next_stage) {
                result = orders.check_upload(do_check_upload_proof).first();
                data->current_stage = do_check_upload_proof.stage;
                break;
              }
              result = orders.create_upload(do_create_upload_proof).first();
              data->current_stage = do_create_upload_proof.stage;
            } while (false);
            break;
          case proof::CreateUpload:
            /// 成功：可秒传，直接FileCommit；否则，去FileUplaod
            /// 失败：不可重试的错误，或者重试次数达到上限，流程直接失败
            /// 失败：可重试的错误，且重试次数未达到上限，重试CreateUpload自身
            do {
              if (proof::Succeeded == currernt_proof.result &&
                  proof::FileCommit == currernt_proof.next_stage) {
                result = orders.file_commit(do_file_commit_proof).first();
                data->current_stage = do_file_commit_proof.stage;
                break;
              }
              if (proof::Succeeded == currernt_proof.result) {
                result = orders.file_uplaod(do_file_upload_proof).first();
                data->current_stage = do_file_upload_proof.stage;
                break;
              }
              data->create_upload_total_error_count++;
              const auto current_error_count =
                  data->create_upload_current_error_count++;
              if (proof::GiveupRetry == currernt_proof.result ||
                  current_error_count > data->error_count_limit) {
                result = rxcpp::observable<>::just(error_proof);
                data->current_stage = error_proof.stage;
                break;
              }
              /// 对自身进行重试的情况，应该进行指数级等待间隔处理
              /// 计算规则：wait_internal_base*POW(1.5,current_error_count)
              const auto wait_internal_milliseconds = static_cast<uint32_t>(
                  pow(pow_base, current_error_count - 1) * wait_internal_base);
              result = GenerateDelayedObservable(wait_internal_milliseconds,
                                                 orders.create_upload,
                                                 do_create_upload_proof)
                           .first();
              data->current_stage = do_create_upload_proof.stage;
            } while (false);
            break;
          case proof::CheckUpload:
            /// 成功：可秒传，直接FileCommit；否则，去FileUplaod
            /// 失败：不可重试的错误，或者重试次数达到上限，流程直接失败
            /// 失败：可重试的错误，但应重试CreateUpload
            /// 失败：可重试的错误，且重试次数未达到上限，重试CheckUpload自身
            /// 失败：重试CheckUpload自身时，应将suggest_waittime和计算的等待间隔取较大值
            do {
              if (proof::Succeeded == currernt_proof.result &&
                  proof::FileCommit == currernt_proof.next_stage) {
                result = orders.file_commit(do_file_commit_proof).first();
                data->current_stage = do_file_commit_proof.stage;
                break;
              }
              if (proof::Succeeded == currernt_proof.result) {
                result = orders.file_uplaod(do_file_upload_proof).first();
                data->current_stage = do_file_upload_proof.stage;
                break;
              }
              data->check_upload_total_error_count++;
              const auto current_error_count =
                  data->check_upload_current_error_count++;
              if (proof::GiveupRetry == currernt_proof.result ||
                  current_error_count > data->error_count_limit) {
                result = rxcpp::observable<>::just(error_proof);
                data->current_stage = error_proof.stage;
                break;
              }
              if (proof::RetryTargetStage == currernt_proof.result &&
                  proof::CreateUpload == currernt_proof.next_stage) {
                result = orders.create_upload(do_create_upload_proof).first();
                data->current_stage = do_create_upload_proof.stage;
                break;
              }
              /// 对自身进行重试的情况，应该进行指数级等待间隔处理
              /// 应将计算的间隔，与currernt_proof.suggest_waittime比较，取较大值
              const auto wait_internal_milliseconds = static_cast<uint32_t>(
                  pow(pow_base, current_error_count - 1) * wait_internal_base);
              const auto wait_internal =
                  wait_internal_milliseconds > currernt_proof.suggest_waittime
                      ? wait_internal_milliseconds
                      : currernt_proof.suggest_waittime;
              result =
                  GenerateDelayedObservable(wait_internal, orders.check_upload,
                                            do_check_upload_proof)
                      .first();
              data->current_stage = do_check_upload_proof.stage;
            } while (false);
            break;
          case proof::FileUplaod:
            /// 成功：传输成功，下一步去FileCommit
            /// 失败：数据传输失败的场景比较特殊，不应直接结束，而应去尝试检查续传记录状态CheckUpload
            /// 无论成功失败，都应返回传输成功的字节数，注意，包含了本次传输之前的有效字节数
            do {
              if (currernt_proof.transfered_length > 0) {
                const auto previous_max_transfered_length =
                    data->max_transfered_length.load();
                if (previous_max_transfered_length <
                    currernt_proof.transfered_length) {
                  data->max_transfered_length =
                      currernt_proof.transfered_length;
                  data->check_upload_current_error_count = 0;
                  data->create_upload_current_error_count = 0;
                }
              }
              if (proof::Succeeded == currernt_proof.result) {
                result = orders.file_commit(do_file_commit_proof).first();
                data->current_stage = do_file_commit_proof.stage;
                break;
              }
              result = orders.check_upload(do_check_upload_proof).first();
              data->current_stage = do_check_upload_proof.stage;
            } while (false);
            break;
          case proof::FileCommit:
            /// 成功；代表传输成功，下一步直接UploadFinal
            /// 失败：数据提交失败，场景比较特殊。对特定的不可重试的错误，应直接流程失败；
            /// 失败：对特定的失败，已知是续传记录废弃，应根据RetryTargetStage，直接CreateUpload
            /// 除上述情况外的一切情况，提交失败应前往CheckUpload
            /// 和FileUplaod一样，没有自身的错误计数器，所以不应重试自身
            do {
              if (proof::Succeeded == currernt_proof.result) {
                result = rxcpp::observable<>::just(succeed_proof);
                data->current_stage = succeed_proof.stage;
                break;
              }
              if (proof::RetryTargetStage == currernt_proof.result &&
                  proof::CreateUpload == currernt_proof.next_stage) {
                result = orders.create_upload(do_create_upload_proof).first();
                data->current_stage = do_create_upload_proof.stage;
                break;
              }
              result = orders.check_upload(do_check_upload_proof).first();
              data->current_stage = do_check_upload_proof.stage;
            } while (false);
            break;
          default:
            result = rxcpp::observable<>::just(error_proof);
            data->current_stage = error_proof.stage;
            break;
        }
        /// data->current_stage语义是正在进行的stage
      } while (false);
      return result;
    });
  }

  static rxcpp::observable<proof::uploader_proof> GenerateIteratorObservable(
      rxcpp::observable<proof::uploader_proof> previous_obs,
      std::weak_ptr<rx_uploader_data> data_weak) {
    return previous_obs.flat_map(
        [data_weak](proof::uploader_proof currernt_proof)
            -> rxcpp::observable<proof::uploader_proof> {
          if (proof::UploadFinal == currernt_proof.stage) {
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
  typedef std::function<void(const rx_uploader &)> CompleteCallback;

 private:
  const CompleteCallback on_complete_callback;
  /// 定义所需的Observable
  rxcpp::connectable_observable<proof::uploader_proof> obs;

 public:
  explicit rx_uploader(const proof::proof_obs_packages &proof_orders,
                       const CompleteCallback complete_callback)
      : data(std::make_shared<rx_uploader_data>(proof_orders)),
        obs(GenerateIteratorObservable(GenerateFirstObservable(), data)
                .tap([complete_callback, this](proof::uploader_proof proof) {
                  if (nullptr != this->data) {
                    data->current_stage = proof::UploadFinal;
                  }
                  if (nullptr != complete_callback) {
                    complete_callback(*this);
                  }
                  if (nullptr != this->data) {
                    data->wait.set_value();
                  }
                })
                .publish()) {}
  ~rx_uploader() = default;

 private:
  /// 禁用默认构造，禁用复制构造、移动构造和=号操作符
  /// 迫使使用者有对此对象的所有权转移的需求时，包裹在unique_ptr中进行
  rx_uploader() = delete;
  rx_uploader(rx_uploader &&) = delete;
  rx_uploader(const rx_uploader &) = delete;
  rx_uploader &operator=(const rx_uploader &) = delete;

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

const static rx_uploader::CompleteCallback default_complete_callback =
    [](const rx_uploader &) -> void {};

}  // namespace uploader

}  // namespace httpbusiness

#endif  /// _RX_UPLOADER_HH
