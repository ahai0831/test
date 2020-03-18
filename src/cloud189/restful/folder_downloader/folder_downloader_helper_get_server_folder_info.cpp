#include "folder_downloader_helper.h"

#include "cloud189/apis/get_folder_info.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using assistant::HttpRequest;
using Cloud189::Restful::details::folderdownloader_thread_data;
using httpbusiness::folder_downloader::proof::folder_downloader_proof;
using httpbusiness::folder_downloader::proof::folder_downloader_stage;
using httpbusiness::folder_downloader::proof::ProofObsCallback;
using httpbusiness::folder_downloader::proof::stage_result;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using rx_assistant::HttpResult;

using Cloud189::Apis::GetFolderInfo::HttpRequestEncode;
using Cloud189::Apis::GetFolderInfo::HttpResponseDecode;
using Cloud189::Apis::GetFolderInfo::JsonStringHelper;
namespace Cloud189 {
namespace Restful {
namespace folder_downloader_helper {
namespace details {
ProofObsCallback get_server_folder_info(
    const std::weak_ptr<folderdownloader_thread_data>& thread_data_weak) {
  return [thread_data_weak](folder_downloader_proof)
             -> rxcpp::observable<folder_downloader_proof> {
    rxcpp::observable<folder_downloader_proof> result =
        rxcpp::observable<>::just(folder_downloader_proof{
            folder_downloader_stage::GetServerFolderInfo,
            stage_result::GiveupRetry});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      const auto& folder_path = thread_data->folder_path;
      const auto& x_request_id = thread_data->x_request_id;
      JsonStringHelper("", folder_path, 0, 3, "", "", x_request_id);
      HttpRequest get_server_folder_info_request("");

      if (!HttpRequestEncode(
              JsonStringHelper("", folder_path, 0, 3, "", "", x_request_id),
              get_server_folder_info_request)) {
        break;
      }

      result =
          rx_assistant::rx_httpresult::create(get_server_folder_info_request)
              .map([thread_data_weak](
                       HttpResult value) -> folder_downloader_proof {
                folder_downloader_proof result{
                    folder_downloader_stage::GetServerFolderInfo,
                    stage_result::RetrySelf};
                do {
                  auto thread_data = thread_data_weak.lock();
                  if (nullptr == thread_data) {
                    break;
                  }
                  std::string response_info;
                  HttpResponseDecode(value.res, value.req, response_info);

                  Json::Value response_json;
                  ReaderHelper(response_info, response_json);

                  const auto id = GetString(response_json["id"]);
                  const auto name = GetString(response_json["name"]);
                  if (id.empty() || name.empty()) {
                    break;
                  }
                  thread_data->remote_root_server_folder_id.store(id);
                  thread_data->remote_root_server_folder_name.store(name);
                  /// Succeeded
                  result.result = stage_result::Succeeded;

                } while (false);
                return result;
              });
    } while (false);
    return result;
  };
}
}  // namespace details
}  // namespace folder_downloader_helper
}  // namespace Restful
}  // namespace Cloud189
