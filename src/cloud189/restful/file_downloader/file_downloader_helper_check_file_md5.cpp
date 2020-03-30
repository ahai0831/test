#include "cloud189/error_code/error_code.h"
#include "file_downloader_helper.h"
#include "tools/string_format.hpp"

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
namespace {
int strCmp(const std::string& s1, const std::string& s2) {
#ifdef _WIN32
  return stricmp(s1.c_str(), s2.c_str());
#else
  return strcasecmp(s1.c_str(), s2.c_str());
#endif
}
}  // namespace
ProofObsCallback check_file_md5(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak) {
  return [thread_data_weak](
             downloader_proof) -> rxcpp::observable<downloader_proof> {
    /// TODO: 必须完善
    /// 暂时校验文件已存在
    rxcpp::observable<downloader_proof> result =
        rxcpp::observable<>::just(downloader_proof{
            downloader_stage::CheckFileMd5, stage_result::GiveupRetry});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      /// 校验路径是否是个合法的文件 0x8000
      const auto save_file_path = thread_data->temp_download_file_path.load();
      const auto& uv_thread = thread_data->uv_thread_ptr;
      /// 处理流程：1. 保证需要被校验的临时文件，是个“文件”
      /// 2. 计算此文件MD5是否匹配；
      /// 3. 对此文件MD5进行rename；
      /// 另外，为了保证排查问题，需要在thread_data中保存这三个结果
      /// 一旦有异常，应插入回调的字段中，以便排查问题
      result =
          rx_uv_fs::rx_uv_fs_factory::Stat(uv_thread, save_file_path)
              .flat_map([thread_data_weak](
                            int32_t value) -> rxcpp::observable<std::string> {
                rxcpp::observable<std::string> result =
                    rxcpp::observable<>::just(std::string());
                do {
                  auto thread_data = thread_data_weak.lock();
                  if (nullptr == thread_data) {
                    break;
                  }
                  /// 此处补充到线程信息中保存
                  thread_data->stat_result.store(value);
                  if (0x8000 != value) {
                    break;
                  }
                  /// 先解除文件锁定保护，准备进行文件MD5计算
                  auto protect_fp = thread_data->file_protect.exchange(nullptr);
                  if (nullptr != protect_fp) {
                    fclose(protect_fp);
                    protect_fp = nullptr;
                  }

                  const auto save_file_path =
                      thread_data->temp_download_file_path.load();
                  const auto& uv_thread = thread_data->uv_thread_ptr;
                  const auto& frozen = thread_data->frozen;
                  rx_uv_fs::uv_sync_md5::CheckStopCallback stop_cb =
                      [&frozen]() -> bool { return !frozen.load(); };
                  result = rx_uv_fs::rx_uv_fs_factory::Md5(
                      uv_thread, save_file_path, stop_cb);

                } while (false);
                return result;
              })
              .flat_map([thread_data_weak](
                            std::string value) -> rxcpp::observable<bool> {
                rxcpp::observable<bool> result =
                    rxcpp::observable<>::just(false);
                do {
                  auto thread_data = thread_data_weak.lock();
                  if (nullptr == thread_data) {
                    break;
                  }

                  /// 此处补充到线程信息中保存
                  thread_data->md5_result.store(value);
                  if (value.empty()) {
                    const auto frozen = thread_data->frozen.load();
                    if (frozen) {
                      thread_data->int32_error_code =
                          Cloud189::ErrorCode::nderr_usercanceled;
                    }
                    break;
                  } else {
                    /// 在value非空的情况下不进行frozen的判断
                  }
                  const auto& md5 = thread_data->md5;
                  if (0 != strCmp(md5, value)) {
                    break;
                  }
                  int32_t try_number = 0;
                  FILE* tmp_fp = nullptr;
                  std::string tmp_path;

                  /// 在此生成目标文件名，用于将临时文件名重命名为目标文件名
                  /// 获取目录名和文件名
                  const auto& download_folder_path =
                      thread_data->download_folder_path;
                  const auto& file_name = thread_data->file_name;
                  const auto temp_download_file_path =
                      thread_data->temp_download_file_path.load();
                  std::string just_name, name_extension;
                  auto iter = file_name.rfind(".");
                  if (iter != std::string::npos) {
                    just_name = file_name.substr(0, iter);
                    name_extension =
                        file_name.substr(iter, file_name.size() - iter);
                  } else {
                    just_name = file_name;
                    name_extension.clear();
                  }

                  do {
                    if (nullptr != tmp_fp) {
                      fclose(tmp_fp);
                      tmp_fp = nullptr;
                    }
                    tmp_path =
                        0 == try_number
                            ? StringFormat("%s%s", download_folder_path.c_str(),
                                           file_name.c_str())
                            : StringFormat("%s%s (%d)%s",
                                           download_folder_path.c_str(),
                                           just_name.c_str(), try_number,
                                           name_extension.c_str());
                    tmp_fp = assistant::core::readwrite::details::fopen(
                        tmp_path.c_str(), "r+");

                    /// 设置如此的条件的原因是，"r+"要求目标文件已存在，如果不存在，则会打开失败；
                    /// 此时tmp_path恰好可以用作创建临时文件
                    /// 设置次数限制，是为了避免死循环
                  } while (nullptr != tmp_fp && 1000 >= ++try_number);
                  if (nullptr != tmp_fp) {
                    fclose(tmp_fp);
                    tmp_fp = nullptr;
                    /// 意味着没有搜索到符合条件的目标名，不用再试
                    break;
                  }

                  /// 保存重命名后的文件路径
                  thread_data->download_file_path.store(tmp_path);
                  const auto& uv_thread_ptr = thread_data->uv_thread_ptr;
                  result = rx_uv_fs::rx_uv_fs_factory::Rename(
                      uv_thread_ptr, temp_download_file_path, tmp_path);
                } while (false);

                return result;
              })
              .map([thread_data_weak](bool rename_result) -> downloader_proof {
                downloader_proof result{downloader_stage::CheckFileMd5,
                                        stage_result::GiveupRetry};
                do {
                  auto thread_data = thread_data_weak.lock();
                  if (nullptr == thread_data) {
                    break;
                  }

                  /// 此处补充到线程信息中保存
                  thread_data->rename_result.store(rename_result);
                  if (!rename_result) {
                    break;
                  }
                  /// 这种情况下，校验MD5、重命名都成功了，可以认为无需保存续传数据了
                  thread_data->current_download_breakpoint_data.Clear();

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