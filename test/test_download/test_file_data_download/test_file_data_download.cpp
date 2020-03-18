#include <iostream>
#include <memory>
#include <string>

#include <Assistant_v3.hpp>

#include <cloud189/apis/file_data_download.h>
#include <cloud189/apis/get_download_address.h>
#include <cloud189/session_helper/session_helper.h>
#include <json/json.h>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;

int main() {
  // 测试步骤：
  // 1.根据抓包 更换登录session参数
  // 2.根据抓包 更换http请求参数
  // 3.运行

  // 设置登录session参数
  std::string psk = "47ecb433-b90b-4610-a74e-1d0f53cfb990";
  std::string pss = "07B241F6DFF1667F860C1AA850D2F52D";
  std::string fsk = "2154dbf9-a9aa-4bcd-b5d7-81a57cbca55f_family";
  std::string fss = "eb2d65dcba894db58ee4e9b7efcf5e52";
  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);  // 登录

  /***********************获取文件下载地址***************************/
  // 设置http请求参数
  std::string fileId = "5150024184557351";
  int32_t dt = 3;
  std::string x_request_id = "";

  std::string params_json =
      Cloud189::Apis::GetDownloadAddress::JsonStringHelper(fileId, dt, "", "",
                                                           false, x_request_id);

  //打印参数
  printf("params_json:\n%s\n\n", params_json.c_str());

  //  http请求初始化
  assistant::HttpRequest get_download_addr_request("");
  Cloud189::Apis::GetDownloadAddress::HttpRequestEncode(
      params_json, get_download_addr_request);

  // 设置代理
  // get_download_addr_request.extends.Set("proxy", "127.0.0.1:8888");

  //  发起请求
  auto assistant_ptr = std::make_unique<assistant::Assistant_v3>();
  auto response = assistant_ptr->SyncHttpRequest(get_download_addr_request);

  //  http响应解析
  std::string decode_res;
  Cloud189::Apis::GetDownloadAddress::HttpResponseDecode(
      response, get_download_addr_request, decode_res);

  printf("download address parse:\n%s\n\n", decode_res.c_str());

  /*************************下载url重定向*****************************/
  std::string filedata_decode_str;
  do {
    Json::Value json_object;
    if (!ReaderHelper(decode_res, json_object)) {
      printf("read json string failed\n");
      break;
    }
    std::string download_url = GetString(json_object["fileDownloadUrl"]);
    std::string download_params_json =
        Cloud189::Apis::FileDataDownload::JsonStringHelper(download_url, "");
    //  http请求初始化
    assistant::HttpRequest filedata_download_request("");
    Cloud189::Apis::FileDataDownload::HttpRequestEncode(
        download_params_json, filedata_download_request);

    // 设置代理
    // filedata_download_request.extends.Set("proxy", "127.0.0.1:8888");

    // 发起请求
    auto filedata_ast_ptr = std::make_unique<assistant::Assistant_v3>();
    auto filedata_res =
        filedata_ast_ptr->SyncHttpRequest(filedata_download_request);

    // 响应解析

    Cloud189::Apis::FileDataDownload::HttpResponseDecode(
        filedata_res, filedata_download_request, filedata_decode_str);
    printf("redirect data parse:\n%s\n\n", filedata_decode_str.c_str());
  } while (false);

  /*************************文件数据下载*****************************/
  do {
    Json::Value json_str;
    if (!ReaderHelper(filedata_decode_str, json_str)) {
      printf("read json string failed\n");
      break;
    }
    std::string redirect_url = GetString(json_str["redirect_url"]);
    if (redirect_url.empty()) {
      printf("redirect_url is empty");
      break;
    }
    std::string redirect_params_json =
        Cloud189::Apis::FileDataDownload::JsonStringHelper(redirect_url, "");
    //  http请求初始化
    assistant::HttpRequest redirect_download_request("");
    Cloud189::Apis::FileDataDownload::HttpRequestEncode(
        redirect_params_json, redirect_download_request);

    // 设置代理
    // redirect_download_request.extends.Set("proxy", "127.0.0.1:8888");

    // 发起请求
    auto redirect_ast_ptr = std::make_unique<assistant::Assistant_v3>();
    auto redirect_res =
        redirect_ast_ptr->SyncHttpRequest(redirect_download_request);

    // 响应解析
    std::string redirect_decode_str;
    Cloud189::Apis::FileDataDownload::HttpResponseDecode(
        redirect_res, redirect_download_request, redirect_decode_str);

    printf("download data parse：\n%s\n", redirect_decode_str.c_str());
  } while (false);

  return 0;
}
