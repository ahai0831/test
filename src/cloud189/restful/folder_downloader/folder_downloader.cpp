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

void FolderDownloader::UserCancel() {}
}  // namespace Restful
}  // namespace Cloud189