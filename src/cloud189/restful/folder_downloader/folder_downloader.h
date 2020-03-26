#ifndef CLOUD189_RESTFUL_FOLDER_DOWNLOADER_H__
#define CLOUD189_RESTFUL_FOLDER_DOWNLOADER_H__

#include <functional>
#include <memory>
#include <string>

namespace Cloud189 {
namespace Restful {

/// 对上传总控的要求
/// 内部数据支持多线程访问，线程安全
namespace details {

struct folderdownloader_internal_data;
struct folderdownloader_thread_data;
}  // namespace details

struct FolderDownloader {
  /// uploader传参必须参数之一，为一个字符串，避免大量字段
  /// 在构造函数中解析json字符串

  FolderDownloader(const std::string &download_info,
                   std::function<void(const std::string &)> data_callback);
  ~FolderDownloader();
  void AsyncStart();
  void SyncWait();
  void UserCancel();
  bool Valid();

 private:
  std::shared_ptr<details::folderdownloader_thread_data> const thread_data;
  std::unique_ptr<details::folderdownloader_internal_data> const data;

 private:
  /// 禁用此uploader的默认构造、复制构造、移动构造和=号操作符
  FolderDownloader() = delete;
  FolderDownloader(FolderDownloader &&) = delete;
  FolderDownloader(const FolderDownloader &) = delete;
  FolderDownloader &operator=(const FolderDownloader &) = delete;
};

}  // namespace Restful

}  // namespace Cloud189

#endif
