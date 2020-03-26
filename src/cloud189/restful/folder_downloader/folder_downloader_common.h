#ifndef CLOUD189_RESTFUL_FOLDER_DOWNLOADER_COMMON_H__
#define CLOUD189_RESTFUL_FOLDER_DOWNLOADER_COMMON_H__

#include <rx_assistant.hpp>
#include <rx_folder_downloader.hpp>
#include <rx_multiworker.hpp>
#include <rx_uv_fs.hpp>

namespace Cloud189 {
namespace Restful {

/// 对上传总控的要求
/// 内部数据支持多线程访问，线程安全
namespace details {

struct folderdownloader_internal_data {
  std::unique_ptr<httpbusiness::folder_downloader::rx_folder_downloader> const
      folderdownload_unique;

  folderdownloader_internal_data(
      const httpbusiness::folder_downloader::proof::proof_obs_packages&
          proof_orders,
      const httpbusiness::folder_downloader::rx_folder_downloader::
          CompleteCallback complete_callback)
      : folderdownload_unique(
            std::make_unique<
                httpbusiness::folder_downloader::rx_folder_downloader>(
                proof_orders, complete_callback)) {}
  ~folderdownloader_internal_data() = default;

  /// TODO: 待禁用的构造函数
};
struct folderdownloader_thread_data {
  std::shared_ptr<rx_uv_fs::uv_loop_with_thread> const uv_thread;
  const std::string folder_id;  /// 本字段不供实际使用，仅作为校验
  const std::string folder_path;
  const std::string download_path;
  const std::string x_request_id;
  const std::function<void(const std::string&)> data_callback;
  folderdownloader_thread_data(
      const std::string& folder_id_, const std::string& folder_path_,
      const std::string& download_path_, const std::string& x_request_id_,
      const std::function<void(const std::string&)> data_callback_)
      : uv_thread(std::make_shared<rx_uv_fs::uv_loop_with_thread>()),
        folder_id(folder_id_),
        folder_path(folder_path_),
        download_path(download_path_),
        x_request_id(x_request_id_),
        data_callback(data_callback_),
        frozen(false),
        int32_error_code(0) {}

  /// 线程安全
  assistant::tools::lockfree_string_closure<std::string>
      remote_root_server_folder_id;
  assistant::tools::lockfree_string_closure<std::string>
      remote_root_server_folder_name;

  /// 保存停止标志位
  std::atomic<bool> frozen;
  std::atomic<int32_t> int32_error_code;  /// 错误码

  /// TODO: 待禁用的构造函数
};
}  // namespace details
}  // namespace Restful
}  // namespace Cloud189
#endif
