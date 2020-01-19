#include <cinttypes>
#include <cstdio>
#include <future>
#include <memory>

#include "upload_dll.h"

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

static std::promise<void> test_for_finished;
static std::future<void> complete_signal;

int main(void) {
  complete_signal = test_for_finished.get_future();

  /// 设置登录
  /// 生成Config字符串
  Json::Value save_cloud189_session;
  save_cloud189_session.append("2b2d56b9-92ec-4214-ae61-cbe79b99e600");
  save_cloud189_session.append("8A01789EC67579348460701AD565EAAD");
  save_cloud189_session.append("76c7be65-0367-46c7-844a-7c86655d29ef_family");
  save_cloud189_session.append("8A01789EC67579348460701AD565EAAD");
  Json::Value config;
  config["save_cloud189_session"] = save_cloud189_session;
  const auto config_str = WriterHelper(config);
  printf("%s\n", config_str.c_str());
  AstConfig(config_str.c_str(),
            [](const char* on_config) { printf("OnConfig: %s\n", on_config); });

  /// 在外部生成测试字符串
  Json::Value test_info_json;
  test_info_json["domain"] = "Cloud189";
  test_info_json["operation"] = "DoUpload";
  /// 设置必传的业务字段
  test_info_json["local_path"] = "D:\\1.apk";
  test_info_json["parent_folder_id"] = "-11";
  test_info_json["oper_type"] = int32_t(1);
  test_info_json["is_log"] = int32_t(0);

  auto test_info = WriterHelper(test_info_json);

  AstProcess(
      test_info.c_str(),
      [](const char* start_data) {
        Json::Value start_data_json;
        ReaderHelper(start_data, start_data_json);
        const auto start_result = GetInt(start_data_json["start_result"]);
        if (0 != start_result) {
          printf("OnStart, failed: %s\n", start_data);
          test_for_finished.set_value();
        } else {
          printf("OnStart: %s\n", start_data);
        }
      },
      [](const char* callback_data) {
        Json::Value callback_data_json;
        ReaderHelper(callback_data, callback_data_json);
        const auto is_complete = GetBool(callback_data_json["is_complete"]);
        if (is_complete) {
          printf("OnComplete: %s\n", callback_data);
          test_for_finished.set_value();
        } else {
          printf("OnCallback: %s\n", callback_data);
        }
      });

  complete_signal.wait();
  return 0;
}
