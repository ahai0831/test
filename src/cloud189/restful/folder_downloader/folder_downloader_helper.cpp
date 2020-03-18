#include "folder_downloader_helper.h"

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetInt64;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

using Cloud189::Restful::details::folderdownloader_thread_data;
using httpbusiness::folder_downloader::rx_folder_downloader;
using httpbusiness::folder_downloader::proof::proof_obs_packages;
using httpbusiness::folder_downloader::proof::ProofObsCallback;

namespace Cloud189 {
namespace Restful {

namespace folder_downloader_helper {

std::shared_ptr<Cloud189::Restful::details::folderdownloader_thread_data>
InitThreadData(const std::string& download_info,
               const std::function<void(const std::string&)>& data_callback) {
  Json::Value download_json;
  ReaderHelper(download_info, download_json);
  const auto folder_id = GetString(download_json["folder_id"]);
  const auto folder_path = GetString(download_json["folder_path"]);
  const auto download_path = GetString(download_json["download_path"]);
  auto x_request_id = GetString(download_json["x_request_id"]);
  if (x_request_id.empty()) {
    x_request_id = assistant::tools::uuid::generate();
  }

  return std::make_shared<
      Cloud189::Restful::details::folderdownloader_thread_data>(
      folder_id, folder_path, download_path, x_request_id, data_callback);
}

const rx_folder_downloader::CompleteCallback GenerateDataCallback(
    const std::weak_ptr<
        Cloud189::Restful::details::folderdownloader_thread_data>&
        thread_data_weak,
    const std::function<void(const std::string&)>& data_callback) {
  /// 生成总控使用的完成时回调
  return [data_callback](const rx_folder_downloader&) -> void {
    Json::Value root;
    root["is_complete"] = bool(true);
    root["download_folder_path"];
    root["sub_file_data"];
    const auto root_str = WriterHelper(root);
    data_callback(root_str);
  };
}

/// 仅供内部解耦无需外部调用，因此无需单独一个头文件
namespace details {
ProofObsCallback get_server_folder_info(
    const std::weak_ptr<folderdownloader_thread_data>& thread_data_weak);
ProofObsCallback check_local_folder(
    const std::weak_ptr<folderdownloader_thread_data>& thread_data_weak);
ProofObsCallback resolve_each_sub_folder(
    const std::weak_ptr<folderdownloader_thread_data>& thread_data_weak);
}  // namespace details

proof_obs_packages GenerateOrders(
    const std::weak_ptr<folderdownloader_thread_data>& thread_data_weak) {
  return proof_obs_packages{details::get_server_folder_info(thread_data_weak),
                            details::check_local_folder(thread_data_weak),
                            details::resolve_each_sub_folder(thread_data_weak)};
}
}  // namespace folder_downloader_helper
}  // namespace Restful
}  // namespace Cloud189
