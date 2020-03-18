#ifndef CLOUD189_RESTFUL_FOLDER_DOWNLOADER_HELPER_H__
#define CLOUD189_RESTFUL_FOLDER_DOWNLOADER_HELPER_H__

#include "folder_downloader_common.h"

namespace Cloud189 {
namespace Restful {

namespace folder_downloader_helper {
/// 根据传入的字符串，对线程数据进行初始化
std::shared_ptr<Cloud189::Restful::details::folderdownloader_thread_data>
InitThreadData(const std::string& download_info,
               const std::function<void(const std::string&)>& data_callback);
/// 生成总控使用的完成回调
const httpbusiness::folder_downloader::rx_folder_downloader::CompleteCallback
GenerateDataCallback(
    const std::weak_ptr<
        Cloud189::Restful::details::folderdownloader_thread_data>&
        thread_data_weak,
    const std::function<void(const std::string&)>& data_callback);
/// 根据弱指针，初始化各RX指令
httpbusiness::folder_downloader::proof::proof_obs_packages GenerateOrders(
    const std::weak_ptr<
        Cloud189::Restful::details::folderdownloader_thread_data>&
        thread_data_weak);

}  // namespace folder_downloader_helper
}  // namespace Restful
}  // namespace Cloud189
#endif
