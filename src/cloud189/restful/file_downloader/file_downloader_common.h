#ifndef CLOUD189_RESTFUL_FILE_DOWNLOADER_COMMON_H__
#define CLOUD189_RESTFUL_FILE_DOWNLOADER_COMMON_H__

#include <rx_assistant.hpp>
#include <rx_downloader.hpp>

namespace Cloud189 {
namespace Restful {

namespace details {

struct downloader_internal_data {
 private:
  /// 在此占据某个文件的相应句柄
  /// TODO: 改为原子量访问
  FILE* file_protect;
  /// 以ScopeGuard机制在析构时释放此句柄
  assistant::tools::scope_guard guard_file_protect;

 public:
  std::unique_ptr<httpbusiness::downloader::rx_downloader> const master_control;
  /// 以file_protect为标志，如果此指针已为空，则什么都不用做
  bool Valid() { return nullptr != file_protect; }
  /// 保存此回调，以供调用
  const httpbusiness::downloader::rx_downloader::CompleteCallback
      null_file_callback;

  explicit downloader_internal_data(
      const httpbusiness::downloader::proof::proof_obs_packages& proof_orders,
      const httpbusiness::downloader::rx_downloader::CompleteCallback
          data_callback)
      : file_protect(nullptr),
        guard_file_protect([this]() {
          /// TODO: 在file_protect被修改为原子量后，要注意访问方式
          if (nullptr != file_protect) {
            fclose(file_protect);
            file_protect = nullptr;
          }
        }),
        null_file_callback(data_callback),
        master_control(
            std::make_unique<httpbusiness::downloader::rx_downloader>(
                proof_orders, data_callback)) {}
  ~downloader_internal_data() = default;
  /// 由于scope_guard的存在，无需再显式地禁用另外四个构造函数
};

struct downloader_thread_data {
 public:
  /// const的数据成员，必须在构造函数中进行显式的初始化
  const std::string file_id;
  const std::string file_name;
  const std::string md5;  /// 保存文件下载完毕后的md5期待值，用于和实际值校验
  const std::string download_folder_path;
  const std::string x_request_id;

  /// 禁用移动复制默认构造和=号操作符，必须通过显式指定所有必须字段进行初始化
  downloader_thread_data(const std::string& file_id_,
                         const std::string& file_name_, const std::string& md5_,
                         const std::string& download_folder_path_,
                         const std::string& x_request_id_)
      : file_id(file_id_),
        file_name(file_name_),
        md5(md5_),
        download_folder_path(download_folder_path_),
        x_request_id(x_request_id_),
        remote_file_size(0),
        transferred_size(0),
        already_download_bytes(0),
        current_download_bytes(0),
        frozen(false),
        speed_count(std::make_unique<httpbusiness::speed_counter_with_stop>()) {
  }
  /// 线程安全的数据成员
  assistant::tools::lockfree_string_closure<std::string> download_url;
  assistant::tools::lockfree_string_closure<std::string> real_remote_url;
  /// TODO: 原子量需要进行初始化
  std::atomic<int64_t>
      remote_file_size;  /// 从远端资源的Headers中解析到的资源实体字节数，若为Trunk传输则此值为-1
  std::atomic<int64_t> transferred_size;
  std::atomic<int64_t> already_download_bytes;
  std::atomic<int64_t> current_download_bytes;

  /// 以下为确认文件上传完成后解析到的字段

  /// 保存计速器，以进行平滑速度计算，以及1S一次的数据推送
  std::unique_ptr<httpbusiness::speed_counter_with_stop> speed_count;
  std::weak_ptr<httpbusiness::downloader::rx_downloader::rx_downloader_data>
      master_control_data;

  /// 保存停止标志位
  std::atomic<bool> frozen;
  /// 保存当前请求的UUID，以供停止请求
  assistant::tools::lockfree_string_closure<std::string> current_request_uuid;

 private:
  downloader_thread_data() = delete;
  downloader_thread_data(downloader_thread_data const&) = delete;
  downloader_thread_data& operator=(downloader_thread_data const&) = delete;
  downloader_thread_data(downloader_thread_data&&) = delete;
};

}  // namespace details
}  // namespace Restful
}  // namespace Cloud189

#endif
