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
      result = rx_uv_fs::rx_uv_fs_factory::Stat(uv_thread, download_folder_path)
                   .map([](int32_t value) -> downloader_proof {
                     downloader_proof result{downloader_stage::GenerateTempFile,
                                             stage_result::GiveupRetry};
                     if (0x4000 == value) {
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