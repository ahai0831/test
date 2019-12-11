
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
                std::function<void(const std::string &)> data_callback);
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
/// local_path,[std::string]
/// [表明上传文件的本地全路径]
/// last_md5,[std::string]
/// [表明上传文件的md5，如果为续传则必须传入，如果为创建新的上传可为空]
/// last_upload_id,[std::string]
/// [表明上传文件的id，如果为续传则必须传入且有效，如果为创建新的上传可为空]
/// parent_folder_id,[std::string]
/// [表明上传文件的父文件夹id]
/// per_slice_size,[int64_t]
/// [表明分片的大小，为3M的倍数，如6MB可表示为6*1024*1024]
/// per_slice_size,[int32_t]
/// [表明是否支持断点续传控制，1-断点续传控制策略版本1，空-不支持]
/// oper_type,[int32_t]
/// [表明上传后操作方式，
/// 1-遇到相同文件名(只检查文件名)，执行重命名操作，
/// 3-遇到相同文件名（只检查文件名），执行覆盖原文件]
/// is_log,[int32_t]
/// [表明是否为客户端日志上传，
/// 1–客户端日志文件上传至指定账户，
/// 0-非客户端日志文件上传]
std::string sliceuploader_info_helper(
    const std::string &local_path, const std::string &last_md5,
    const std::string &last_upload_id, const std::string &parent_folder_id,
    const std::string &x_request_id, const int64_t per_slice_size,
    const int32_t resume_policy, const int32_t oper_type, const int32_t is_log);

}  // namespace Restful

}  // namespace Cloud189
