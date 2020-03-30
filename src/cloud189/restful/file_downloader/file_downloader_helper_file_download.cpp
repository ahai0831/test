#include "cloud189/error_code/error_code.h"
#include "file_downloader_download_breakpoint_data.h"
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
      const auto real_remote_url = thread_data->real_remote_url.load();
      /// 以临时文件为名，进行写文件
      const auto save_file_path = thread_data->temp_download_file_path.load();
      const auto remote_file_size = thread_data->remote_file_size.load();

      /// 开始下载前，需要处理已经下载的数据

      thread_data->already_download_bytes.fetch_add(
          thread_data->current_download_bytes.exchange(0));
      const auto already_download_bytes =
          thread_data->already_download_bytes.load();
      thread_data->seconds_in_stage3.store(0);
      /// Download File
      assistant::HttpRequest file_download_request(real_remote_url);
      file_download_request.extends.Set(
          "range", std::to_string(already_download_bytes) + "-" +
                       std::to_string(remote_file_size - 1));
      file_download_request.extends.Set("download_filepath", save_file_path);
      file_download_request.extends.Set("download_filesize",
                                        std::to_string(remote_file_size));
      file_download_request.extends.Set("download_offset",
                                        std::to_string(already_download_bytes));
      file_download_request.extends.Set(
          "download_length",
          std::to_string(remote_file_size - already_download_bytes));
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
      /// process, uuid , for potential stop
      std::string unused_uuid = assistant::tools::uuid::generate();
      file_download_request.extends.Set("uuid", unused_uuid);
      thread_data->current_request_uuid.store(unused_uuid);

      /// test
      /// file_download_request.extends.Set("proxy", "http://127.0.0.1:8888");

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
                  /// 请求到此已完成，无需再记录UUID
                  thread_data->current_request_uuid.Clear();

                  /// 对4XX错误码的处理：整个任务直接失败，无需再重试
                  if (4 == static_cast<int32_t>(value.res.status_code / 100)) {
                    result.result = stage_result::GiveupRetry;
                    break;
                  }

                  if (value.res.status_code != 206) {
                    break;
                  }

                  /// 在206请求中特有的校验
                  /// 校验range字段，是否匹配；校验远端实体长度是否匹配
                  const auto range_in_request = value.req.extends.Get("range");
                  const auto range_in_response =
                      value.res.extends.Get("content_range_range");
                  if (range_in_request.compare(range_in_response) != 0) {
                    break;
                  }
                  const auto kFileSize = thread_data->remote_file_size.load();
                  const auto content_range_tatal =
                      value.res.extends.Get("content_range_tatal");
                  const auto cur_remote_file_size =
                      strtoll(content_range_tatal.c_str(), nullptr, 0);
                  if (kFileSize != cur_remote_file_size) {
                    break;
                  }

                  const auto kDownloadContentLength = strtoll(
                      value.res.extends.Get("content_length_download").c_str(),
                      nullptr, 0);
                  const auto kDownloadSize =
                      strtoll(value.res.extends.Get("size_download").c_str(),
                              nullptr, 0);

                  /// 进行到此时，可以认为下载的数据，要么完全下载，要么下载了部分有效数据
                  /// 可以填写 result.transfered_length 的内容了
                  const auto current_already_download_bytes =
                      thread_data->already_download_bytes.load();
                  if (kDownloadSize <= kDownloadContentLength) {
                    thread_data->current_download_bytes.store(kDownloadSize);
                    result.transfered_length =
                        current_already_download_bytes + kDownloadSize;
                  } else {
                    result.transfered_length = current_already_download_bytes;
                  }

                  /// 若已下载数据的总长度恰好等于文件大小，那么可以直接成功
                  if (kFileSize == result.transfered_length) {
                    result.result = stage_result::Succeeded;
                    /// 暂时认为，这种情况，无需对外传出续传数据了
                    /// TODO: 一旦有MD5校验导致的失败流程，要仔细斟酌此处
                    thread_data->current_download_breakpoint_data.Clear();
                    break;
                  }

                  /// 恰好在此时，可更新一次current_download_breakpoint_data
                  /// 输入的信息：file_id, file_name, md5, download_folder_path,
                  /// 上述四项，必须完全匹配
                  /// 需要校验的数据： temp_download_file_path
                  /// 要校验这个路径是否可以fopen（用r+的方式）
                  /// 需要记录的数据：后续还需要下载（留白）的范围，用
                  /// {offset:x, length:y}这种形式保存
                  /// 自说明字段：0.1，以后相应的字符串用相应的版本去处理
                  breakpoint_data_0_1 bp_data;
                  bp_data.download_folder_path =
                      thread_data->download_folder_path;
                  bp_data.file_id = thread_data->file_id;
                  bp_data.file_name = thread_data->file_name;
                  bp_data.md5 = thread_data->md5;
                  bp_data.temp_download_file_path =
                      thread_data->temp_download_file_path.load();
                  const int64_t offset = result.transfered_length;
                  const int64_t length = kFileSize - offset;
                  bp_data.to_be_continued = std::make_tuple(offset, length);

                  std::string base64_str;
                  const auto res_1 =
                      GenerateBase64String_0_1(bp_data, base64_str);

                  if (res_1 && !base64_str.empty()) {
                    thread_data->current_download_breakpoint_data.store(
                        base64_str.c_str());
                  }

                  /// 延迟到此时，文件下载长度与应下载不一致
                  /// 增加对用户手动取消的处理
                  if (thread_data->frozen.load()) {
                    result.result = stage_result::UserCanceled;
                    thread_data->int32_error_code =
                        Cloud189::ErrorCode::nderr_usercanceled;
                    break;
                  }

                  /// Should Retry
                  result.result = stage_result::RetryTargetStage;
                  result.next_stage = downloader_stage::GetDownloadUrl;

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