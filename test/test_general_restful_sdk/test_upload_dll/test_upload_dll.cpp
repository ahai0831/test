#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <cinttypes>
#include <cstdio>
#include <future>
#include <memory>

// #include "upload_dll.h"

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

/// defines some symbol for dll dynamic loading.
namespace {
typedef void (*OnProcessStart)(const char *);
typedef void (*OnProcessCallback)(const char *);
typedef void (*OnConfigFinished)(const char *);

typedef void (*pAstConfig)(const char *, OnConfigFinished);
typedef void (*pAstProcess)(const char *, OnProcessStart, OnProcessCallback);

pAstConfig AstConfig = nullptr;
pAstProcess AstProcess = nullptr;

#ifdef _WIN32
const char *dll_name = "upload_dll";
#else
const char *dll_name = "libupload_dll.dylib";
#endif

/// dynamic loading dll or dylib here
static struct DynamicLoading {
#ifdef _WIN32
  HINSTANCE instance = nullptr;
#else
  void *instance = nullptr;
#endif
  DynamicLoading() {
#ifdef _WIN32
    instance = LoadLibraryA(dll_name);
    AstConfig = (pAstConfig)GetProcAddress(instance, "AstConfig");
    AstProcess = (pAstProcess)GetProcAddress(instance, "AstProcess");
#else
    char *error_info = nullptr;
    instance = dlopen(dll_name, RTLD_NOW);
    error_info = dlerror();
    if (nullptr != error_info) {
      fprintf(stderr, "Error when loading pAstConfig: %s\n", error_info);
      error_info = nullptr;
    }
    AstConfig = (pAstConfig)dlsym(instance, "AstConfig");
    error_info = dlerror();
    if (nullptr != error_info) {
      fprintf(stderr, "Error when loading pAstConfig: %s\n", error_info);
      error_info = nullptr;
    }
    AstProcess = (pAstProcess)dlsym(instance, "AstProcess");
    error_info = dlerror();
    if (nullptr != error_info) {
      fprintf(stderr, "Error when loading pAstProcess: %s\n", error_info);
      error_info = nullptr;
    }
#endif
  }
  ~DynamicLoading() {
    AstProcess = nullptr;
    AstConfig = nullptr;
#ifdef _WIN32
    FreeLibrary(instance);
#else
    dlclose(instance);
#endif
  }
} _dynamic_loading;
}  // namespace

static std::promise<void> test_for_finished;
static std::future<void> complete_signal;

static std::promise<void> test_for_folder_finished;
static std::future<void> complete_folder_signal;

static std::promise<void> test_for_download_finished;
static std::future<void> complete_download_signal;

static std::promise<void> test_for_folder_download_finished;
static std::future<void> complete_folder_download_signal;

int main(void) {
  if (nullptr == AstConfig || nullptr == AstProcess) {
    return -11;
  }
  complete_signal = test_for_finished.get_future();
  complete_folder_signal = test_for_folder_finished.get_future();
  complete_download_signal = test_for_download_finished.get_future();
  complete_folder_download_signal =
      test_for_folder_download_finished.get_future();
  /// 设置登录
  /// 生成Config字符串
  Json::Value save_cloud189_session;
  save_cloud189_session.append("5ed26416-0c6f-4005-9e7c-1beddd529a47");
  save_cloud189_session.append("B94DF5E701085B74B515F5D87AC7DE55");
  save_cloud189_session.append("dddd3bf5-95c5-4735-a1e2-644a670e617d_family");
  save_cloud189_session.append("2D6FBBD2630E71A626614DFFD599696E");
  Json::Value config;
  config["save_cloud189_session"] = save_cloud189_session;
  // config["proxy_info"] = "http://127.0.0.1:8888";
  const auto config_str = WriterHelper(config);
  printf("%s\n", config_str.c_str());
  AstConfig(config_str.c_str(),
            [](const char *on_config) { printf("OnConfig: %s\n", on_config); });

  /*************************测试文件夹下载*****************************/
  /// 在外部生成测试字符串
  Json::Value test_info_folder_download_json;
  test_info_folder_download_json["domain"] = "Cloud189";
  test_info_folder_download_json["operation"] = "DoFolderDownload";
  /// 设置必传的业务字段

  test_info_folder_download_json["folder_id"] = "7149115489233549";
  test_info_folder_download_json["folder_path"] = "/tools/";
  test_info_folder_download_json["download_path"] = "D:/test/";

  auto test_info_download = WriterHelper(test_info_folder_download_json);
  printf("%s\n", test_info_download.c_str());

  AstProcess(
      test_info_download.c_str(),
      [](const char *start_data) {
        Json::Value start_data_json;
        ReaderHelper(start_data, start_data_json);
        const auto start_result = GetInt(start_data_json["start_result"]);
        if (0 != start_result) {
          printf("OnStart, failed: %s\n", start_data);
          test_for_folder_download_finished.set_value();
        } else {
          printf("OnStart: %s\n", start_data);
        }
      },
      [](const char *callback_data) {
        Json::Value callback_data_json;
        ReaderHelper(callback_data, callback_data_json);
        const auto is_complete = GetBool(callback_data_json["is_complete"]);
        if (is_complete) {
          printf("OnComplete: %s\n", callback_data);
          test_for_folder_download_finished.set_value();
        } else {
          printf("OnCallback: %s\n", callback_data);
        }
      });

  complete_folder_download_signal.wait();

  return 0;
  /*************************测试文件下载*****************************/
  /// 在外部生成测试字符串
  Json::Value test_info_download_json;
  test_info_download_json["domain"] = "Cloud189";
  test_info_download_json["operation"] = "DoDownload";
  /// 设置必传的业务字段
  test_info_download_json["file_id"] = "7147022658216724";
  test_info_download_json["file_name"] = "19MB_500files.part0006.rar";
  test_info_download_json["md5"] = "F1EB7945EF543D6A6E9C8B71A435AAD0";
  test_info_download_json["download_folder_path"] = "D:/test/";

  auto test_info_download = WriterHelper(test_info_download_json);
  printf("%s\n", test_info_download.c_str());

  AstProcess(
      test_info_download.c_str(),
      [](const char *start_data) {
        Json::Value start_data_json;
        ReaderHelper(start_data, start_data_json);
        const auto start_result = GetInt(start_data_json["start_result"]);
        if (0 != start_result) {
          printf("OnStart, failed: %s\n", start_data);
          test_for_download_finished.set_value();
        } else {
          printf("OnStart: %s\n", start_data);
        }
      },
      [](const char *callback_data) {
        Json::Value callback_data_json;
        ReaderHelper(callback_data, callback_data_json);
        const auto is_complete = GetBool(callback_data_json["is_complete"]);
        if (is_complete) {
          printf("OnComplete: %s\n", callback_data);
          test_for_download_finished.set_value();
        } else {
          printf("OnCallback: %s\n", callback_data);
        }
      });

  complete_download_signal.wait();

  return 0;
  /*************************测试文件上传*****************************/
  /// 在外部生成测试字符串
  Json::Value test_info_json;
  test_info_json["domain"] = "Cloud189";
  test_info_json["operation"] = "DoUpload";
  /// 设置必传的业务字段
  test_info_json["local_path"] = "D:/SVN/test/test.z01";
  test_info_json["parent_folder_id"] = "-11";
  test_info_json["oper_type"] = int32_t(1);
  test_info_json["is_log"] = int32_t(0);

  auto test_info = WriterHelper(test_info_json);
  printf("%s\n", test_info.c_str());

  AstProcess(
      test_info.c_str(),
      [](const char *start_data) {
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
      [](const char *callback_data) {
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

  /*************************测试文件夹上传*****************************/
  /// 在外部生成测试字符串
  Json::Value test_info_json_folder;
  test_info_json_folder["domain"] = "Cloud189";
  test_info_json_folder["operation"] = "DoFolderUpload";
  /// 设置必传的业务字段
  test_info_json_folder["local_folder_path"] = "D:/tools/";
  test_info_json_folder["server_folder_path"] = "/";
  test_info_json_folder["parent_folder_id"] = "-11";
  test_info_json_folder["oper_type"] = int32_t(1);
  test_info_json_folder["is_log"] = int32_t(0);

  auto test_info_folder = WriterHelper(test_info_json_folder);
  printf("test_info:%s\n", test_info_folder.c_str());

  AstProcess(
      test_info_folder.c_str(),
      [](const char *start_data) {
        Json::Value start_data_json;
        ReaderHelper(start_data, start_data_json);
        const auto start_result = GetInt(start_data_json["start_result"]);
        if (0 != start_result) {
          printf("OnStart, failed: %s\n", start_data);
          test_for_folder_finished.set_value();
        } else {
          printf("OnStart: %s\n", start_data);
        }
      },
      [](const char *callback_data) {
        Json::Value callback_data_json;
        ReaderHelper(callback_data, callback_data_json);
        const auto is_complete = GetBool(callback_data_json["is_complete"]);
        if (is_complete) {
          printf("OnComplete: %s\n", callback_data);
          test_for_folder_finished.set_value();
        } else {
          printf("OnCallback: %s\n", callback_data);
        }
      });
  complete_folder_signal.wait();
  return 0;
}
