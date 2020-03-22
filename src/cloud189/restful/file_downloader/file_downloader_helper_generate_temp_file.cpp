#include "file_downloader_helper.h"

#include <tools/string_format.hpp>

#include "file_downloader_download_breakpoint_data.h"

using assistant::tools::string::StringFormat;
using Cloud189::Restful::details::downloader_thread_data;
using httpbusiness::downloader::proof::downloader_proof;
using httpbusiness::downloader::proof::downloader_stage;
using httpbusiness::downloader::proof::ProofObsCallback;
using httpbusiness::downloader::proof::stage_result;

namespace Cloud189 {
namespace Restful {
namespace file_downloader_helper {
namespace details {
ProofObsCallback generate_temp_file(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak) {
  return [thread_data_weak](
             downloader_proof) -> rxcpp::observable<downloader_proof> {
    rxcpp::observable<downloader_proof> result =
        rxcpp::observable<>::just(downloader_proof{
            downloader_stage::GenerateTempFile, stage_result::GiveupRetry});
    /// 暂时不做任何事，除了校验目录是否存在
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      /// 校验路径是否是个合法的目录
      const auto& download_folder_path = thread_data->download_folder_path;
      const auto& uv_thread = thread_data->uv_thread_ptr;
      result =
          rx_uv_fs::rx_uv_fs_factory::Stat(uv_thread, download_folder_path)
              .map([thread_data_weak](int32_t value) -> downloader_proof {
                downloader_proof result{downloader_stage::GenerateTempFile,
                                        stage_result::GiveupRetry};
                do {
                  if (0x4000 != value) {
                    break;
                  }
                  auto thread_data = thread_data_weak.lock();
                  if (nullptr == thread_data) {
                    break;
                  }

                  /// 特殊处理：处理传入的last_download_breakpoint_data
                  /// 要保证只处理一次
                  const auto& last_download_breakpoint_data =
                      thread_data->download_breakpoint_data;
                  const auto useless =
                      thread_data->breakpoint_data_useless.load();
                  bool valid_bp_data = false;
                  if (!useless) {
                    thread_data->breakpoint_data_useless.store(true);
                    breakpoint_data_0_1 bp_data;

                    const auto res_2 = GenerateData_0_1(
                        last_download_breakpoint_data, bp_data);
                    do {
                      if (!res_2) {
                        break;
                      }
                      if (bp_data.download_folder_path !=
                          thread_data->download_folder_path) {
                        break;
                      }
                      if (bp_data.file_id != thread_data->file_id) {
                        break;
                      }
                      if (bp_data.file_name != thread_data->file_name) {
                        break;
                      }
                      if (bp_data.md5 != thread_data->md5) {
                        break;
                      }
                      const auto& offset = std::get<0>(bp_data.to_be_continued);
                      const auto& length = std::get<1>(bp_data.to_be_continued);
                      if (offset + length !=
                          thread_data->remote_file_size.load()) {
                        break;
                      }

                      const auto& temp_path = bp_data.temp_download_file_path;

                      auto fp = assistant::core::readwrite::details::fopen(
                          temp_path.c_str(), "r+");
                      if (nullptr == fp) {
                        break;
                      }
                      /// 至此续传数据验证成功，进行数据恢复
                      thread_data->file_protect.exchange(fp);
                      thread_data->temp_download_file_path.store(temp_path);
                      thread_data->already_download_bytes.store(offset);
                      thread_data->current_download_bytes.store(0);
                      valid_bp_data = true;
                    } while (false);

                    if (valid_bp_data) {
                      result.result = stage_result::Succeeded;
                      break;
                    }
                  }

                  /// 特殊处理：由于外部因素，导致需要对文件进行断点续传下载的情形
                  /// 判据 file_protect 和 temp_download_file_path 都有效
                  if (nullptr != thread_data->file_protect.load() &&
                      !thread_data->temp_download_file_path.empty()) {
                    result.result = stage_result::Succeeded;
                    break;
                  }

                  /// 在此建立临时文件，用于文件下载
                  /// 获取目录名和文件名
                  const auto& download_folder_path =
                      thread_data->download_folder_path;
                  const auto& file_name = thread_data->file_name;

                  int32_t try_number = 0;
                  FILE* tmp_fp = nullptr;
                  std::string tmp_path;
                  do {
                    if (nullptr != tmp_fp) {
                      fclose(tmp_fp);
                      tmp_fp = nullptr;
                    }
                    tmp_path = StringFormat("%s%s_%d.ecdl",
                                            download_folder_path.c_str(),
                                            file_name.c_str(), try_number);
                    tmp_fp = assistant::core::readwrite::details::fopen(
                        tmp_path.c_str(), "r+");

                    /// 设置如此的条件的原因是，"r+"要求目标文件已存在，如果不存在，则会打开失败；
                    /// 此时tmp_path恰好可以用作创建临时文件
                    /// 设置次数限制，是为了避免死循环
                  } while (nullptr != tmp_fp && 1000 >= ++try_number);
                  if (nullptr != tmp_fp) {
                    fclose(tmp_fp);
                    tmp_fp = nullptr;
                    break;
                  } else {
                    auto new_tmp_file =
                        assistant::core::readwrite::details::fopen(
                            tmp_path.c_str(), "w+");
                    thread_data->file_protect.store(new_tmp_file);
                    thread_data->temp_download_file_path.store(tmp_path);
                  }

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