#include <future>
#include <thread>

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

/// 注意，这两个变量要保证线程安全
/// 这里没有做任何限制，原因是他们在此场景下不会发生访问竞争
std::string breaked_uuid;
std::string breakpoint_data;
}  // namespace
void test_ast_process_cloud189_file_download_resume_from_breakpoint() {
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

        /// 保存预备被暂停的下载任务的uuid
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

        /// 保存用来续传的下载任务的续传数据
        breakpoint_data =
            GetString(callback_data_json["download_breakpoint_data"]);
        if (is_complete) {
          test_for_finished.set_value();
        }
      });

  /// 延时2.25秒，进行任务暂停
  std::this_thread::sleep_for(std::chrono::milliseconds(2250));
  Json::Value test_usercancel_json;
  test_usercancel_json["domain"] = "Cloud189";
  test_usercancel_json["operation"] = "UserCancelDownload";
  test_usercancel_json["uuid"] = breaked_uuid;
  auto test_usercancel_json_info = WriterHelper(test_usercancel_json);
  AstProcess(
      test_usercancel_json_info.c_str(),
      [](const char *start_data) {
        Json::Value start_data_json;
        ReaderHelper(start_data, start_data_json);
        const auto start_result = GetInt(start_data_json["start_result"]);
        printf("OnStart: %s%s\n",
               0 != start_result ? "UserCancelDownload failed: " : "",
               start_data);
      },
      nullptr);

  /// 原本的任务在此阻塞式等待结束
  complete_signal.wait();

  /// 再次启动任务
  /// 初始化promise和future
  do {
    auto temp = decltype(test_for_finished)();
    test_for_finished.swap(temp);
    complete_signal = test_for_finished.get_future();
  } while (false);

  test_info_json["last_download_breakpoint_data"] = breakpoint_data;
  auto test_info_2 = WriterHelper(test_info_json);
  printf("%s\n", test_info_2.c_str());
  AstProcess(
      test_info_2.c_str(),
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
