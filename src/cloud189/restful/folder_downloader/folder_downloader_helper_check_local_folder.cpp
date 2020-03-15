#include "folder_downloader_helper.h"

using assistant::HttpRequest;
using Cloud189::Restful::details::folderdownloader_thread_data;
using httpbusiness::folder_downloader::proof::folder_downloader_proof;
using httpbusiness::folder_downloader::proof::folder_downloader_stage;
using httpbusiness::folder_downloader::proof::ProofObsCallback;
using httpbusiness::folder_downloader::proof::stage_result;
using rx_assistant::HttpResult;

namespace Cloud189 {
namespace Restful {
namespace folder_downloader_helper {
namespace details {
ProofObsCallback check_local_folder(
    const std::weak_ptr<folderdownloader_thread_data>& thread_data_weak) {
  return [thread_data_weak](folder_downloader_proof)
             -> rxcpp::observable<folder_downloader_proof> {
    rxcpp::observable<folder_downloader_proof> result =
        rxcpp::observable<>::just(
            folder_downloader_proof{folder_downloader_stage::CheckLocalFolder,
                                    stage_result::GiveupRetry});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      const auto& download_path = thread_data->download_path;
      const auto& uv_thread = thread_data->uv_thread;

      result = rx_uv_fs::rx_uv_fs_factory::Stat(uv_thread, download_path)
                   .map([thread_data_weak](rx_uv_fs::uv_fs_stat::Type value)
                            -> folder_downloader_proof {
                     folder_downloader_proof result{
                         folder_downloader_stage::CheckLocalFolder,
                         stage_result::GiveupRetry};
                     do {
                       auto thread_data = thread_data_weak.lock();
                       if (nullptr == thread_data) {
                         break;
                       }
                       /// 0x4000 代表文件夹
                       if (0x4000 != value) {
                         break;
                       }
                       /// Succeeded
                       result.result = stage_result::Succeeded;
                     } while (false);
                     return result;
                   });

    } while (false);
    return result;
  };
}
}  // namespace details
}  // namespace folder_downloader_helper
}  // namespace Restful
}  // namespace Cloud189
