#include "file_downloader_helper.h"

#include "cloud189/apis/get_download_address.h"
#include "cloud189/error_code/error_code.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using assistant::HttpRequest;
using Cloud189::Restful::details::downloader_thread_data;
using httpbusiness::downloader::proof::downloader_proof;
using httpbusiness::downloader::proof::downloader_stage;
using httpbusiness::downloader::proof::ProofObsCallback;
using httpbusiness::downloader::proof::stage_result;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;

using Cloud189::Apis::GetDownloadAddress::HttpRequestEncode;
using Cloud189::Apis::GetDownloadAddress::HttpResponseDecode;
using Cloud189::Apis::GetDownloadAddress::JsonStringHelper;

namespace Cloud189 {
namespace Restful {
namespace file_downloader_helper {
namespace details {
ProofObsCallback get_download_url(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak) {
  return [thread_data_weak](
             downloader_proof) -> rxcpp::observable<downloader_proof> {
    rxcpp::observable<downloader_proof> result =
        rxcpp::observable<>::just(downloader_proof{
            downloader_stage::GetDownloadUrl, stage_result::GiveupRetry});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      const auto& file_id = thread_data->file_id;
      const auto& x_request_id = thread_data->x_request_id;

      HttpRequest get_download_url_request("");
      if (!HttpRequestEncode(
              JsonStringHelper(file_id, 3, "", "", false, x_request_id),
              get_download_url_request)) {
        break;
      }
      /// process, uuid , for potential stop
      std::string unused_uuid = assistant::tools::uuid::generate();
      get_download_url_request.extends.Set("uuid", unused_uuid);
      thread_data->current_request_uuid.store(unused_uuid);

      /// test
      // get_download_url_request.extends.Set("proxy", "http://127.0.0.1:8888");

      result =
          rx_assistant::rx_httpresult::create(get_download_url_request)
              .map([thread_data_weak](
                       rx_assistant::HttpResult value) -> downloader_proof {
                downloader_proof result{downloader_stage::GetDownloadUrl,
                                        stage_result::GiveupRetry};
                do {
                  auto thread_data = thread_data_weak.lock();
                  if (nullptr == thread_data) {
                    break;
                  }
                  /// 请求到此已完成，无需再记录UUID
                  thread_data->current_request_uuid.Clear();

                  /// 增加对用户手动取消的处理
                  if (thread_data->frozen.load()) {
                    result.result = stage_result::UserCanceled;
                    thread_data->int32_error_code =
                        Cloud189::ErrorCode::nderr_usercanceled;
                    break;
                  }
                  std::string response_info;
                  if (!HttpResponseDecode(value.res, value.req,
                                          response_info)) {
                    break;
                  }
                  Json::Value root;
                  if (!ReaderHelper(response_info, root)) {
                    break;
                  }
                  /// TODO: 此处需要增加网络抖动导致的重试策略
                  const auto download_url = GetString(root["fileDownloadUrl"]);
                  if (download_url.empty()) {
                    break;
                  }
                  thread_data->download_url.store(download_url);
                  result.result = stage_result::Succeeded;
                } while (false);
                return result;
              });

    } while (false);

    /// TODO: 必须完善
    return result;
  };
}
}  // namespace details
}  // namespace file_downloader_helper
}  // namespace Restful
}  // namespace Cloud189