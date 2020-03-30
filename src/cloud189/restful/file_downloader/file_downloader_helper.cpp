#include "file_downloader_helper.h"

#include "cloud189/error_code/error_code.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using assistant::HttpRequest;
using assistant::tools::lockfree_string_closure;
using Cloud189::Restful::details::downloader_thread_data;
using httpbusiness::downloader::rx_downloader;
using httpbusiness::downloader::proof::downloader_proof;
using httpbusiness::downloader::proof::downloader_stage;
using httpbusiness::downloader::proof::proof_obs_packages;
using httpbusiness::downloader::proof::ProofObsCallback;
using httpbusiness::downloader::proof::stage_result;
using rx_assistant::HttpResult;

using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetInt64;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

using Cloud189::Restful::details::downloader_thread_data;

namespace Cloud189 {
namespace Restful {
/// 将构造函数完全进行模块化分解，避免单个函数承担过多功能
/// 在此将Downloader所需的函数模块抽取出来

namespace file_downloader_helper {

/// 根据传入的字符串，对线程数据进行初始化
std::shared_ptr<downloader_thread_data> InitThreadData(
    const std::string& download_info) {
  /// 根据传入信息，初始化线程信息结构体
  Json::Value download_json;
  ReaderHelper(download_info, download_json);
  const auto file_id = GetString(download_json["file_id"]);
  const auto file_name = GetString(download_json["file_name"]);
  const auto md5 = GetString(download_json["md5"]);
  const auto download_folder_path =
      GetString(download_json["download_folder_path"]);
  auto x_request_id = GetString(download_json["x_request_id"]);
  const auto download_breakpoint_data =
      GetString(download_json["last_download_breakpoint_data"]);
  if (x_request_id.empty()) {
    x_request_id = assistant::tools::uuid::generate();
  }

  return std::make_shared<downloader_thread_data>(
      file_id, file_name, md5, download_folder_path, download_breakpoint_data,
      x_request_id);
}

/// 生成总控使用的完成回调
const rx_downloader::CompleteCallback GenerateDataCallback(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak,
    const std::function<void(const std::string&)>& data_callback) {
  /// 为计速器提供每秒一次的回调
  auto thread_data = thread_data_weak.lock();
  if (nullptr != thread_data) {
    thread_data->speed_count->RegSubscription(
        [thread_data_weak, data_callback](uint64_t smooth_speed) {
          /// 每秒一次的回调信息字段
          /// 每秒一次的回调信息字段
          Json::Value info;
          info["speed"] = int64_t(smooth_speed);
          auto thread_data = thread_data_weak.lock();
          if (nullptr != thread_data) {
            const auto remote_file_size = thread_data->remote_file_size.load();
            info["file_size"] = remote_file_size;
            info["x_request_id"] = thread_data->x_request_id;
            /// 已传输数据量
            const auto already_download_bytes =
                thread_data->already_download_bytes.load();
            const auto current_download_bytes =
                thread_data->current_download_bytes.load();
            auto transferred_size =
                already_download_bytes + current_download_bytes;
            if (transferred_size > thread_data->remote_file_size.load()) {
              transferred_size = thread_data->remote_file_size.load();
            }
            info["transferred_size"] = transferred_size;

            auto mc_data = thread_data->master_control_data.lock();
            int32_t current_stage = -1;
            if (nullptr != mc_data) {
              current_stage = int32_t(mc_data->current_stage.load());
            }
            info["stage"] = current_stage;
            /// 仅在stage==3的情况下，加入`average_speed`字段
            if (3 == current_stage) {
              const auto seconds = ++thread_data->seconds_in_stage3;
              if (seconds > 0 && current_download_bytes > 0) {
                const auto average_speed =
                    static_cast<int64_t>(current_download_bytes / seconds);
                info["average_speed"] = average_speed;
              }
            }

            if (current_stage >= 3) {
            }
            /// 在errorcode机制完善后，需要在ec为0的情况下，才加相应业务字段
            if (current_stage >= 5) {
            }
          }
          /// TODO: 有待完善
          data_callback(WriterHelper(info));
        },
        []() {});
  }

  /// 提供流程完成（成功和失败均包括在内）时的回调
  return [thread_data_weak, data_callback](const rx_downloader&) -> void {
    /// TODO: 在此处取出相应的信息，传递给回调
    Json::Value info;
    info["is_complete"] = bool(true);
    info["stage"] = int32_t(downloader_stage::DownloadFinal);

    auto thread_data = thread_data_weak.lock();
    if (nullptr != thread_data) {
      /// 需保证计速器回调不再发送
      thread_data->speed_count->Stop();
      /// 为MT的CRT的线程模型不一致问题进行变通处理，
      /// 在此处强行迫使相应的订阅尽早提前取消
      thread_data->speed_count = nullptr;

      const auto remote_file_size = thread_data->remote_file_size.load();
      info["file_size"] = remote_file_size;
      info["x_request_id"] = thread_data->x_request_id;
      info["int32_error_code"] = thread_data->int32_error_code.load();
      /// 增加MD5校验流程中的结果
      const auto stat_result = thread_data->stat_result.load();
      if (0x8000 != stat_result) {
        info["stat_result"] = stat_result;
      }
      const auto md5_result = thread_data->md5_result.load();
      if (md5_result != thread_data->md5) {
        info["md5_result"] = md5_result;
      }
      const auto rename_result = thread_data->rename_result.load();
      if (!rename_result) {
        info["rename_result"] = rename_result;
      }
      const auto download_file_path = thread_data->download_file_path.load();
      if (!download_file_path.empty()) {
        info["download_file_path"] = download_file_path;
      }
      const auto download_breakpoint_data =
          thread_data->current_download_breakpoint_data.load();
      if (!download_breakpoint_data.empty()) {
        info["download_breakpoint_data"] = download_breakpoint_data;
      }

      /// 已传输数据量
      auto transferred_size = thread_data->already_download_bytes.load() +
                              thread_data->current_download_bytes.load();
      if (transferred_size > remote_file_size) {
        transferred_size = remote_file_size;
      }
      info["transferred_size"] = transferred_size;
      /// 在errorcode机制完善后，需要在ec为0的情况下，才加相应业务字段
    }
    /// TODO: 有待完善
    data_callback(WriterHelper(info));
  };
}

/// 在此对各order的生成函数进行声明
/// 仅供内部解耦无需外部调用，因此无需单独一个头文件
namespace details {
ProofObsCallback get_download_url(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak);
ProofObsCallback get_remote_file_size(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak);
ProofObsCallback generate_temp_file(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak);
ProofObsCallback file_download(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak);
ProofObsCallback check_file_md5(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak);
}  // namespace details

/// 根据弱指针，初始化各RX指令
proof_obs_packages GenerateOrders(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak) {
  return proof_obs_packages{details::get_download_url(thread_data_weak),
                            details::get_remote_file_size(thread_data_weak),
                            details::generate_temp_file(thread_data_weak),
                            details::file_download(thread_data_weak),
                            details::check_file_md5(thread_data_weak)};
}

}  // namespace file_downloader_helper
}  // namespace Restful
}  // namespace Cloud189
