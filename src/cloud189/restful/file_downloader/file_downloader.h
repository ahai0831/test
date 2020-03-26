#ifndef CLOUD189_RESTFUL_FILE_DOWNLOADER_H__
#define CLOUD189_RESTFUL_FILE_DOWNLOADER_H__
#include <functional>
#include <memory>
#include <string>

namespace Cloud189 {
namespace Restful {

/// 对上传总控的要求
/// 内部数据支持多线程访问，线程安全
namespace details {

struct downloader_internal_data;
struct downloader_thread_data;
}  // namespace details

struct Downloader {
  /// 传参必须参数之一，为一个字符串，避免大量字段
  /// 在构造函数中解析json字符串

  Downloader(const std::string &download_info,
             std::function<void(const std::string &)> data_callback);
  ~Downloader();
  void AsyncStart();
  void SyncWait();
  void UserCancel();
  bool Valid();

 private:
  std::shared_ptr<details::downloader_thread_data> const thread_data;
  std::unique_ptr<details::downloader_internal_data> const data;

 private:
  /// 禁用此uploader的默认构造、复制构造、移动构造和=号操作符
  Downloader() = delete;
  Downloader(Downloader &&) = delete;
  Downloader(const Downloader &) = delete;
  Downloader &operator=(const Downloader &) = delete;
};

}  // namespace Restful

}  // namespace Cloud189
#endif
