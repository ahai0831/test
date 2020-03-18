#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <Assistant_v3.hpp>
#ifdef _WIN32
#include <tools/string_convert.hpp>
#endif
#include <cloud189/apis/get_folder_info.h>
#include <cloud189/session_helper/session_helper.h>

#ifdef _WIN32
using assistant::tools::string::utf8ToWstring;
using assistant::tools::string::wstringToAnsi;
using assistant::tools::string::wstringToUtf8;
#endif

int main() {
  // 设置登录所需参数
  std::string psk = "2ae7df42-bc9d-4c1e-ae87-538b971bbe1f";
  std::string pss = "F881E9F9B0CB9F96926EC3F359CCD1A1";
  std::string fsk = "edfbe3d9-ae8f-442e-918a-7d453a083048_family";
  std::string fss = "F881E9F9B0CB9F96926EC3F359CCD1A1";
  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);  // 登录

  /**********************测试一(普通测试)***********************/
  // 设置http请求所需参数
  std::string folderId = "";
#ifdef _WIN32
  std::string folderPath = wstringToUtf8(L"test/abc/ast");
#else
  std::string folderPath = "XYZ/abc";
#endif
  std::string x_request_id = "x_request_id";
  int pathList = 0;
  int dt = 3;
  std::string params_json = Cloud189::Apis::GetFolderInfo::JsonStringHelper(
      folderId, folderPath, pathList, dt, "", "", x_request_id);

  //打印参数
#ifdef _WIN32
  auto params_json_wstr = utf8ToWstring(params_json);
  auto params_json_ansi = wstringToAnsi(params_json_wstr);
  printf("params_json:\n%s\n", params_json_ansi.c_str());
#else
  printf("params_json:\n%s\n", params_json.c_str());
#endif

  // 初始化http请求
  assistant::HttpRequest get_folder_info_request("");
  Cloud189::Apis::GetFolderInfo::HttpRequestEncode(params_json,
                                                   get_folder_info_request);

  // 设置代理
  // get_folder_info_request.extends.Set("proxy", "127.0.0.1:8888");

  //  发起http请求
  auto assistant_ptr = std::make_unique<assistant::Assistant_v3>();
  auto response = assistant_ptr->SyncHttpRequest(get_folder_info_request);

  // http响应解析
  std::string decode_res;
  Cloud189::Apis::GetFolderInfo::HttpResponseDecode(
      response, get_folder_info_request, decode_res);
  printf("response parse:\n%s\n\n", decode_res.c_str());
  return 0;
}
