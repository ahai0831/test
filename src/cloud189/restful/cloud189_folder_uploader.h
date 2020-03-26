
#include <functional>
#include <memory>
#include <string>

namespace Cloud189 {
namespace Restful {

/// 对上传总控的要求
/// 内部数据支持多线程访问，线程安全
namespace details {

struct folderuploader_internal_data;
struct folderuploader_thread_data;
}  // namespace details

struct FolderUploader {
  /// uploader传参必须参数之一，为一个字符串，避免大量字段
  /// 在构造函数中解析json字符串

  FolderUploader(const std::string &upload_info,
                 std::function<void(const std::string &)> data_callback);
  ~FolderUploader();
  void AsyncStart();
  void SyncWait();
  void UserCancel();
  bool Valid();

 private:
  std::shared_ptr<details::folderuploader_thread_data> const thread_data;
  std::unique_ptr<details::folderuploader_internal_data> const data;

 private:
  /// 禁用此uploader的默认构造、复制构造、移动构造和=号操作符
  FolderUploader() = delete;
  FolderUploader(FolderUploader &&) = delete;
  FolderUploader(const FolderUploader &) = delete;
  FolderUploader &operator=(const FolderUploader &) = delete;
};

}  // namespace Restful

}  // namespace Cloud189
