﻿#include <future>

#include "ffi_simulation.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;
namespace {
std::promise<void> test_for_finished;
std::future<void> complete_signal;
}  // namespace
void test_ast_process_cloud189_file_download() {
  /// 初始化promise和future
  do {
    auto temp = decltype(test_for_finished)();
    test_for_finished.swap(temp);
    complete_signal = test_for_finished.get_future();
  } while (false);

  /// 在外部生成测试字符串
  Json::Value test_info_json;
  test_info_json["domain"] = "Cloud189";
  test_info_json["operation"] = "DoDownload";
  /// 设置必传的业务字段
  test_info_json["file_id"] = "7147022658216724";
  test_info_json["file_name"] = "19MB_500files.part0006.rar";
  test_info_json["md5"] = "F1EB7945EF543D6A6E9C8B71A435AAD0";
  test_info_json["download_folder_path"] = "D:/test/";

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

  complete_signal.wait();
}
