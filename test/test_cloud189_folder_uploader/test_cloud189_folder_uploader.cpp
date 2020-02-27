#include <future>
#include <thread>

#include <json/json.h>

#include "cloud189/restful/cloud189_folder_uploader.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

using Cloud189::Restful::FolderUploader;

static std::promise<void> test_for_finished;
static std::future<void> complete_signal;

int main(void) {
  /// 开发者须知： 将此处的sesssion信息换成自己云盘的session信息
  Cloud189::SessionHelper::Cloud189Login(
      "d54d1680-b88d-4795-8146-ba967d5e199c",
      "3A77CF580DC25002024FB9C9104F67B7",
      "dddd3bf5-95c5-4735-a1e2-644a670e617d_family",
      "2D6FBBD2630E71A626614DFFD599696E");

  Json::Value root;
  root["parent_folder_id"] = "-11";
  /// 开发者须知：将此处的路径换成自己本地的路径
  root["local_folder_path"] = "C:/Apache24/";
  root["server_folder_path"] = "/";
  const auto upload_folder_info = WriterHelper(root);

  complete_signal = test_for_finished.get_future();

  std::unique_ptr<FolderUploader> hi_folder;
  auto callback_func = [&hi_folder](const std::string& str) {
    printf("%s\n", str.c_str());
    Json::Value root;
    ReaderHelper(str, root);
    const auto is_complete = GetBool(root["is_complete"]);
    if (is_complete) {
      hi_folder = nullptr;
      test_for_finished.set_value();
    }
  };
  hi_folder = std::make_unique<FolderUploader>(upload_folder_info.c_str(),
                                               callback_func);

  hi_folder->AsyncStart();

  complete_signal.wait();

  return 0;
}
