#ifndef CLOUD189_RESTFUL_FILE_DOWNLOADER_HELPER_H__
#define CLOUD189_RESTFUL_FILE_DOWNLOADER_HELPER_H__

#include "file_downloader_common.h"

namespace Cloud189 {
namespace Restful {
/// 将构造函数完全进行模块化分解，避免单个函数承担过多功能
/// 在此将Downloader所需的函数模块抽取出来

namespace file_downloader_helper {

/// 根据传入的字符串，对线程数据进行初始化
std::shared_ptr<Cloud189::Restful::details::downloader_thread_data>
InitThreadData(const std::string& download_info);
/// 生成总控使用的完成回调
const httpbusiness::downloader::rx_downloader::CompleteCallback
GenerateDataCallback(
    const std::weak_ptr<Cloud189::Restful::details::downloader_thread_data>&
        thread_data_weak,
    const std::function<void(const std::string&)>& data_callback);
/// 根据弱指针，初始化各RX指令
httpbusiness::downloader::proof::proof_obs_packages GenerateOrders(
    const std::weak_ptr<Cloud189::Restful::details::downloader_thread_data>&
        thread_data_weak);

}  // namespace file_downloader_helper

}  // namespace Restful
}  // namespace Cloud189
#endif
