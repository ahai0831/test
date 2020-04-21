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
const char *dll_name = "libgeneral_restful_sdk.dylib";
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

static std::promise<void> test_for_finished[500];
static std::future<void> complete_signal[500];



static std::promise<void> test_for_folder_finished;
static std::future<void> complete_folder_signal;

static int i;
static int32_t size;

void filedownload(std::string file_id,std::string file_name,std::string md5,std::string download_path, int m){
    
  Json::Value save_cloud189_session;
  save_cloud189_session.append("22c8d9bf-5b19-4e80-9c2e-5368fe738feb");
  save_cloud189_session.append("6FA9027AAEABBC6E8234A2A6E31731A5");
  save_cloud189_session.append("dddd3bf5-95c5-4735-a1e2-644a670e617d_family");
  save_cloud189_session.append("2D6FBBD2630E71A626614DFFD599696E");
  Json::Value config;
  config["save_cloud189_session"] = save_cloud189_session;
  config["proxy_info"] = "http://127.0.0.1:8899";
  const auto config_str = WriterHelper(config);
  printf("%s\n", config_str.c_str());
  AstConfig(config_str.c_str(),
            [](const char *on_config) { printf("OnConfig: %s\n", on_config); });

  Json::Value file_json;
  file_json["domain"] = "Cloud189";
  file_json["operation"] = "DoDownload";
  file_json["file_id"] = file_id;
  file_json["file_name"] = file_name;
  file_json["md5"] = md5;
  file_json["download_folder_path"] = download_path;
  i = m;
  const auto test_info = restful_common::jsoncpp_helper::WriterHelper(file_json);
  AstProcess(
      test_info.c_str(),
      [](const char *start_data) {
        Json::Value start_data_json;
        ReaderHelper(start_data, start_data_json);
        const auto start_result = GetInt(start_data_json["start_result"]);
        printf("OnStart: %s%s\n", 0 != start_result ? "failed: " : "",
               start_data);
        if (0 != start_result) {
          test_for_finished[i].set_value();
        }
      },
      [](const char *callback_data) {
        Json::Value callback_data_json;
        ReaderHelper(callback_data, callback_data_json);
        const auto is_complete = GetBool(callback_data_json["is_complete"]);
        printf("%s: %s\n", is_complete ? "OnComplete" : "OnCallback",
               callback_data);
        if (is_complete) {
          test_for_finished[i].set_value();
        }
      });

  complete_signal[i].wait();
}

int main(void) {
  if (nullptr == AstConfig || nullptr == AstProcess) {
    return -11;
  }
  for (int  i = 0; i < 500; i++)
{
  complete_signal[i] = test_for_finished[i].get_future();
}
  complete_folder_signal = test_for_folder_finished.get_future();
  /// 设置登录
  /// 生成Config字符串
  Json::Value save_cloud189_session;
  save_cloud189_session.append("22c8d9bf-5b19-4e80-9c2e-5368fe738feb");
  save_cloud189_session.append("6FA9027AAEABBC6E8234A2A6E31731A5");
  save_cloud189_session.append("dddd3bf5-95c5-4735-a1e2-644a670e617d_family");
  save_cloud189_session.append("2D6FBBD2630E71A626614DFFD599696E");
  Json::Value config;
  config["save_cloud189_session"] = save_cloud189_session;
  config["proxy_info"] = "http://127.0.0.1:8899";
  const auto config_str = WriterHelper(config);
  printf("%s\n", config_str.c_str());
  AstConfig(config_str.c_str(),
            [](const char *on_config) { printf("OnConfig: %s\n", on_config); });

  /*************************测试文件下载*****************************/
  /// 在外部生成测试字符串
  Json::Value root;
  root["domain"] = "Cloud189";
  root["operation"] = "DoFolderDownload";
//  root["folder_id"] = "2142017390720463";
//  root["folder_path"] = "/docs/";
    root["folder_id"] = "9149117437664174";
    root["folder_path"] = "/ceshipywenjian/";
  root["download_path"] = "/Users/zhaozt/Desktop/download1/";
  const auto download_info = restful_common::jsoncpp_helper::WriterHelper(root);
  printf("%s\n", download_info.c_str());

  AstProcess(
      download_info.c_str(),
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

          size = callback_data_json["sub_file_data"].size(); //得到json数组的大小
//            printf("size is %i",size);
          for (static int i = 0; i < size; i++) //逐个获取数组中对象的值
          {
            std::string file_id = callback_data_json["sub_file_data"][i]["file_id"].asString();
//              printf("文件id %s",file_id.c_str());
            std::string file_name = callback_data_json["sub_file_data"][i]["file_name"].asString();
            std::string md5 = callback_data_json["sub_file_data"][i]["md5"].asString();
              printf("download_pathis %s",callback_data_json["download_path"].asString().c_str());
            printf("启动文件下载\n");
            std::thread t(filedownload,file_id.c_str(),file_name.c_str(),md5.c_str(),"/Users/zhaozt/Desktop/download1/",i);
            t.join();
          } 
          
        }
      });

  complete_folder_signal.wait();


  return 0;
}
