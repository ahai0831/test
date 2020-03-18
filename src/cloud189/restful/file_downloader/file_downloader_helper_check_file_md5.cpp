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
      const auto save_file_path =
          thread_data->download_folder_path + thread_data->file_name;
      const auto& uv_thread = thread_data->uv_thread_ptr;
      result = rx_uv_fs::rx_uv_fs_factory::Stat(uv_thread, save_file_path)
                   .map([](int32_t value) -> downloader_proof {
                     downloader_proof result{downloader_stage::CheckFileMd5,
                                             stage_result::GiveupRetry};
                     if (0x8000 == value) {
                       result.result = stage_result::Succeeded;
                     }
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