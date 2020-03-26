#include <future>

#include "ffi_simulation.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

namespace {
std::promise<void> test_for_finished;
std::future<void> complete_signal;
std::string breaked_uuid;
}  // namespace
void test_ast_process_cloud189_usercancel_folder_upload() {
  /// 初始化promise和future
  do {
    auto temp = decltype(test_for_finished)();
    test_for_finished.swap(temp);
    complete_signal = test_for_finished.get_future();
  } while (false);

  /// 在外部生成测试字符串
  Json::Value test_info_json;
  test_info_json["domain"] = "Cloud189";
  test_info_json["operation"] = "DoFolderUpload";
  /// 设置必传的业务字段
  test_info_json["local_folder_path"] = "/test_spencer/";
  test_info_json["server_folder_path"] = "/";
  test_info_json["parent_folder_id"] = "-11";

  auto test_info = WriterHelper(test_info_json);
  printf("%s\n", test_info.c_str());

  AstProcess(
      test_info.c_str(),
      [](const char *start_data) {
        Json::Value start_data_json;
        ReaderHelper(start_data, start_data_json);
        const auto start_result = GetInt(start_data_json["start_result"]);
        printf("OnStart: %s%s\n", 0 != start_result ? "failed: " : "",
               start_data);
        /// 保存预备被暂停的上传任务的uuid
        breaked_uuid = GetString(start_data_json["uuid"]);
        if (0 != start_result) {
          test_for_finished.set_value();
        }
      },
      [](const char *callback_data) {
        Json::Value callback_data_json;
        ReaderHelper(callback_data, callback_data_json);
        const auto is_complete = GetBool(callback_data_json["is_complete"]);
        printf("%s: %s\n", is_complete ? "OnComplete" : "OnCallback",
               callback_data);
        if (is_complete) {
          test_for_finished.set_value();
        }
      });

  /// 延时2.25秒，进行任务暂停
  std::this_thread::sleep_for(std::chrono::milliseconds(2250));
  Json::Value test_usercancel_json;
  test_usercancel_json["domain"] = "Cloud189";
  test_usercancel_json["operation"] = "UserCancelFolderUpload";
  test_usercancel_json["uuid"] = breaked_uuid;
  auto test_usercancel_json_info = WriterHelper(test_usercancel_json);
  AstProcess(
      test_usercancel_json_info.c_str(),
      [](const char *start_data) {
        Json::Value start_data_json;
        ReaderHelper(start_data, start_data_json);
        const auto start_result = GetInt(start_data_json["start_result"]);
        printf("OnStart: %s%s\n",
               0 != start_result ? "UserCancelFolderUpload failed: " : "",
               start_data);
      },
      nullptr);

  /// 原本的任务在此阻塞式等待结束
  complete_signal.wait();
}
