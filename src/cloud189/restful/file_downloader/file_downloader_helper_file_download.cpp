#include "file_downloader_helper.h"

using Cloud189::Restful::details::downloader_thread_data;
using httpbusiness::downloader::proof::downloader_proof;
using httpbusiness::downloader::proof::downloader_stage;
using httpbusiness::downloader::proof::ProofObsCallback;
using httpbusiness::downloader::proof::stage_result;

namespace Cloud189 {
namespace Restful {
namespace file_downloader_helper {
namespace details {
ProofObsCallback file_download(
    const std::weak_ptr<downloader_thread_data> &thread_data_weak) {
  return [thread_data_weak](
             downloader_proof) -> rxcpp::observable<downloader_proof> {
    /// TODO: 必须完善
    /// 暂时将文件完整下载到本地即可
    rxcpp::observable<downloader_proof> result =
        rxcpp::observable<>::just(downloader_proof{
            downloader_stage::FileDownload, stage_result::RetryTargetStage,
            downloader_stage::GetDownloadUrl});

    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }

      /// 将文件下载到本地。完全下载。必须校验最终接收到的二进制流长度，是否与期望一致
      ;
      const auto real_remote_url = thread_data->real_remote_url.load();
      const auto save_file_path =
          thread_data->download_folder_path + thread_data->file_name;
      const auto remote_file_size = thread_data->remote_file_size.load();

      /// 开始下载前，需要清理已经下载的数据
      thread_data->already_download_bytes.store(0);
      thread_data->current_download_bytes.store(0);

      /// Download File
      assistant::HttpRequest file_download_request(real_remote_url);
      file_download_request.extends.Set(
          "range",
          std::to_string(0) + "-" + std::to_string(remote_file_size - 1));
      file_download_request.extends.Set("download_filepath", save_file_path);
      file_download_request.extends.Set("download_filesize",
                                        std::to_string(remote_file_size));
      file_download_request.extends.Set("download_offset", std::to_string(0));
      file_download_request.extends.Set("download_length",
                                        std::to_string(remote_file_size));
      /// 对下载类传输http请求，应将传输超时（未收到1Byte）缩短到10S（默认值60S）
      /// 避免过长时间占着worker数但不贡献下载速率 2019.9.22
      file_download_request.extends.Set("transfer_timeout", std::to_string(10));

      /// 增加计速器
      auto &Add = thread_data->speed_count->Add;
      auto &current_download_bytes = thread_data->current_download_bytes;
      file_download_request.retval_func =
          [&Add, &current_download_bytes](uint64_t value) {
            Add(value);
            current_download_bytes += value;
          };

      /// test
      // file_download_request.extends.Set("proxy", "http://127.0.0.1:8888");

      result =
          rx_assistant::rx_httpresult::create(file_download_request)
              .map([thread_data_weak](
                       rx_assistant::HttpResult value) -> downloader_proof {
                downloader_proof result{downloader_stage::FileDownload,
                                        stage_result::RetryTargetStage,
                                        downloader_stage::GetDownloadUrl};

                do {
                  auto thread_data = thread_data_weak.lock();
                  if (nullptr == thread_data) {
                    break;
                  }
                  if (value.res.status_code != 206) {
                    break;
                  }

                  const auto kDownloadSize =
                      strtoll(value.res.extends.Get("size_download").c_str(),
                              nullptr, 0);
                  const auto kFileSize = thread_data->remote_file_size.load();
                  if (kFileSize != kDownloadSize) {
                    break;
                  }

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
}  // namespace file_downloader_helper
}  // namespace Restful
}  // namespace Cloud189