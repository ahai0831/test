#include "folder_downloader.h"

#include "folder_downloader_common.h"
#include "folder_downloader_helper.h"

using Cloud189::Restful::details::folderdownloader_internal_data;
using Cloud189::Restful::folder_downloader_helper::GenerateDataCallback;
using Cloud189::Restful::folder_downloader_helper::GenerateOrders;
using Cloud189::Restful::folder_downloader_helper::InitThreadData;
namespace Cloud189 {
namespace Restful {

FolderDownloader::FolderDownloader(
    const std::string& download_info,
    std::function<void(const std::string&)> data_callback)
    : thread_data(InitThreadData(download_info, data_callback)),
      data(std::make_unique<folderdownloader_internal_data>(
          GenerateOrders(thread_data),
          GenerateDataCallback(thread_data, data_callback))) {}
FolderDownloader::~FolderDownloader() = default;

void FolderDownloader::AsyncStart() {
  data->folderdownload_unique->AsyncStart();
}

void FolderDownloader::SyncWait() {}

void FolderDownloader::UserCancel() { thread_data->frozen.store(true); }
bool FolderDownloader::Valid() {
  bool flag = false;
  do {
    /// folder_id 必传
    if (thread_data->folder_id.empty()) {
      break;
    }

    /// folder_path，必传，必须以‘/’结尾，但不能代表云端根目录"/"
    if (thread_data->folder_path.empty()) {
      break;
    }
    if ('/' != thread_data->folder_path.back()) {
      break;
    }
    if (thread_data->folder_path.compare("/") == 0) {
      break;
    }

    /// download_path必传，且必须以'/'结尾
    if (thread_data->download_path.empty()) {
      break;
    }
    if ('/' != thread_data->download_path.back()) {
      break;
    }
    flag = true;
  } while (false);
  return flag;
}
}  // namespace Restful
}  // namespace Cloud189