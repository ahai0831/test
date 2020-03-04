#include <iostream>
#include <memory>
#include <string>

#include <Assistant_v3.hpp>
#ifdef _WIN32
#include <tools/string_convert.hpp>
#endif
#include <cloud189/apis/create_folder.h>
#include <cloud189/session_helper/session_helper.h>

#ifdef _WIN32
using assistant::tools::string::utf8ToWstring;
using assistant::tools::string::wstringToAnsi;
using assistant::tools::string::wstringToUtf8;
#endif

int main() {
  // 测试步骤：
  // 1.根据抓包 更换登录session参数
  // 2.根据抓包 更换http请求参数
  // 3.运行

  // 设置登录session参数
  std::string psk = "c48340d3-61db-4339-9266-c5faa6c48379";
  std::string pss = "2E7E0D81BF0C679482A57E783AFFFD62";
  std::string fsk = "af5ab774-0d06-470a-9e55-ddec88568611_family";
  std::string fss = "2E7E0D81BF0C679482A57E783AFFFD62";
  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);  // 登录

  // 设置http请求参数
  std::string parentFolderId = "-11";
  std::string x_request_id = "";

#ifdef _WIN32
  //std::string relativePath = wstringToUtf8(
  //    L"XYZ/abc/a1/b2/c3/d4/e5/f6/g7/h8/i9/j10/k11/l12/m13/n14/o15/p16/q17/r18/"
  //    L"s19/t20/u21/v22/w23/x24/y25");
  std::string relativePath = wstringToUtf8(L"/XYZ/abc");
  std::string folderName = wstringToUtf8(L"spencer");
#else
  std::string relativePath = "/XYZ/雪山妖狐";
  std::string folderName = "太阳";
#endif
  std::string params_json = Cloud189::Apis::CreateFolder::JsonStringHelper(
      parentFolderId, relativePath, folderName, x_request_id);

  //打印参数
#ifdef _WIN32
  auto params_json_wstr = utf8ToWstring(params_json);
  auto params_json_ansi = wstringToAnsi(params_json_wstr);
  printf("params_json:\n%s\n", params_json_ansi.c_str());
#else
  printf("params_json:\n%s\n",params_json.c_str());
#endif
  //  http请求初始化
  assistant::HttpRequest create_folder_request("");
  Cloud189::Apis::CreateFolder::HttpRequestEncode(params_json,
                                                  create_folder_request);

  // 设置代理
  // create_folder_request.extends.Set("proxy", "127.0.0.1:8888");

  //  发起请求
  auto assistant_ptr = std::make_unique<assistant::Assistant_v3>();
  auto response = assistant_ptr->SyncHttpRequest(create_folder_request);

  //  http响应解析
  std::string decode_res;
  Cloud189::Apis::CreateFolder::HttpResponseDecode(
      response, create_folder_request, decode_res);

#ifdef _WIN32
  auto decode_res_wstr = utf8ToWstring(decode_res);
  auto decode_res_ansi = wstringToAnsi(decode_res_wstr);
#else
  printf("响应解析：\n%s", decode_res.c_str());
#endif
  return 0;
}
