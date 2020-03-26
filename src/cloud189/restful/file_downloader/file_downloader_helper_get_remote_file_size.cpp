#include "file_downloader_helper.h"

#include "cloud189/apis/file_data_download.h"
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

using Cloud189::Apis::FileDataDownload::HttpRequestEncode;
using Cloud189::Apis::FileDataDownload::JsonStringHelper;

namespace Cloud189 {
namespace Restful {
namespace file_downloader_helper {
namespace details {
ProofObsCallback get_remote_file_size(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak) {
  return [thread_data_weak](
             downloader_proof) -> rxcpp::observable<downloader_proof> {
    rxcpp::observable<downloader_proof> result =
        rxcpp::observable<>::just(downloader_proof{
            downloader_stage::GetRemoteFileSize, stage_result::GiveupRetry});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      const auto download_url = thread_data->download_url.load();
      const auto& x_request_id = thread_data->x_request_id;

      HttpRequest get_remote_file_size_request("");
      if (!HttpRequestEncode(JsonStringHelper(download_url, x_request_id),
                             get_remote_file_size_request)) {
        break;
      }
      /// 额外的魔改技巧
      /// get_remote_file_size_request.extends.Set("header_only", "true");
      get_remote_file_size_request.extends.Set("follow_location", "true");
      get_remote_file_size_request.extends.Set("range", "0-0");
      /// process, uuid , for potential stop
      std::string unused_uuid = assistant::tools::uuid::generate();
      get_remote_file_size_request.extends.Set("uuid", unused_uuid);
      thread_data->current_request_uuid.store(unused_uuid);

      /// test
      // get_remote_file_size_request.extends.Set("proxy",
      // "http://127.0.0.1:8888");

      result =
          rx_assistant::rx_httpresult::create(get_remote_file_size_request)
              .map([thread_data_weak](
                       rx_assistant::HttpResult value) -> downloader_proof {
                downloader_proof result{downloader_stage::GetRemoteFileSize,
                                        stage_result::RetryTargetStage,
                                        downloader_stage::GetDownloadUrl};
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
                  /// TODO: Solve not 206
                  if (value.res.status_code != 206) {
                    result.result = stage_result::GiveupRetry;
                    break;
                  }
                  const auto content_range_tatal =
                      value.res.extends.Get("content_range_tatal");
                  thread_data->remote_file_size.store(
                      strtoll(content_range_tatal.c_str(), nullptr, 0));
                  const auto& real_remote_url = value.res.effective_url;
                  thread_data->real_remote_url.store(real_remote_url);

                  /// Succeeded
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