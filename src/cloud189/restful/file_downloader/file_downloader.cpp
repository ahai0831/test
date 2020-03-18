#include "file_downloader.h"

#include <rx_assistant.hpp>

#include "file_downloader_common.h"
#include "file_downloader_helper.h"

// using Cloud189::Restful::details::downloader_thread_data;
using Cloud189::Restful::file_downloader_helper::GenerateDataCallback;
using Cloud189::Restful::file_downloader_helper::GenerateOrders;
using Cloud189::Restful::file_downloader_helper::InitThreadData;

namespace Cloud189 {
namespace Restful {
Downloader::Downloader(const std::string& download_info,
                       std::function<void(const std::string&)> data_callback)
    : thread_data(InitThreadData(download_info)),
      data(std::make_unique<details::downloader_internal_data>(
          GenerateOrders(thread_data),
          GenerateDataCallback(thread_data, data_callback))) {
  thread_data->master_control_data = data->master_control->data;
  if (!data->Valid()) {
    /// thread_data->int32_error_code =
    /// Cloud189::ErrorCode::nderr_file_access_error;
    data->null_file_callback(*data->master_control);
  } else {
    /// thread_data->init_success = true;
  }
}

void Downloader::AsyncStart() {
  if (data->Valid()) {
    data->master_control->AsyncStart();
  }
}

void Downloader::SyncWait() {
  if (data->Valid()) {
    data->master_control->SyncWait();
  }
}
/// 在用户手动点击“暂停”或取消，或“退出登录”等需要迫使流程立即失败的场景
/// 非阻塞调用，函数返回不代表立即“取消成功”
/// 将迫使流程尽可能早结束，可能存在延迟
/// 在流程即将完成时，未必能“取消成功”，仍有可能“成功完成”
void Downloader::UserCancel() {
  thread_data->frozen.store(true);
  /// MD5计算已做处理，无需另行处理

  /// 使正在进行的费时操作（如网络请求、数据传输等）中断
  const auto current_request_uuid = thread_data->current_request_uuid.load();
  if (!current_request_uuid.empty()) {
    assistant::HttpRequest stop_req(
        assistant::HttpRequest::Opts::SPCECIALOPERATORS_STOPCONNECT);
    stop_req.extends.Set("uuids", current_request_uuid);
    rx_assistant::rx_httpresult::create(stop_req).publish().connect();
  }
}

Downloader::~Downloader() = default;

}  // namespace Restful
}  // namespace Cloud189
