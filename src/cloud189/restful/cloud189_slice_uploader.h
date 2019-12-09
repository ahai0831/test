
#include <functional>
#include <memory>
#include <string>

namespace Cloud189 {
namespace Restful {

/// 对上传总控的要求
/// 内部数据支持多线程访问，线程安全
namespace details {

struct sliceuploader_internal_data;
struct sliceuploader_thread_data;
}  // namespace details

struct SliceUploader {
  /// uploader传参必须参数之一，为一个字符串，避免大量字段
  /// 在构造函数中解析json字符串

  SliceUploader(const std::string &upload_info,
                std::function<void(const std::string &)> complete_callback);
  ~SliceUploader();
  void AsyncStart();
  void SyncWait();

 private:
  std::shared_ptr<details::sliceuploader_thread_data> thread_data;
  std::unique_ptr<details::sliceuploader_internal_data> data;

 private:
  /// 禁用此uploader的默认构造、复制构造、移动构造和=号操作符
  SliceUploader() = delete;
  SliceUploader(SliceUploader &&) = delete;
  SliceUploader(const SliceUploader &) = delete;
  SliceUploader &operator=(const SliceUploader &) = delete;
};

/// 为此Uploader提供一个Helper函数，用于生成合规的json字符串
std::string sliceuploader_info_helper(
    const std::string &local_path, const std::string &md5,
    /*const std::string slice_md5, const std::string slice_md5_list,*/
    const std::string &last_upload_id, const std::string &parent_folder_id,
    /*const int32_t upload_slice_id,*/ const int64_t per_slice_size,
    const int32_t resume_policy, const int32_t oper_type, const int32_t is_log);

}  // namespace Restful

}  // namespace Cloud189
