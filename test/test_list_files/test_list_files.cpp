#include <iostream>
#include <memory>
#include <string>

#include <Assistant_v3.hpp>

#include <cloud189/apis/list_files.h>
#include <cloud189/session_helper/session_helper.h>

int main() {
  // 测试步骤：
  // 1.根据抓包 更换登录session参数
  // 2.根据抓包 更换http请求参数
  // 3.运行

  // 设置登录session参数
  std::string psk = "c2a87325-651c-4a2e-99eb-59fc8d0c0fab";
  std::string pss = "55FC07F65B0FA9835210E2786D4229E6";
  std::string fsk = "a0ee924a-2e18-405c-b3bc-eba880985c6b_family";
  std::string fss = "55FC07F65B0FA9835210E2786D4229E6";
  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);  // 登录

  // 设置http请求参数
  std::string folderId = "9138315275102370";
  int32_t recursive = 0;
  int32_t fileType = 0;
  int32_t mediaType = 0;
  int32_t mediaAttr = 0;
  int32_t iconOption = 10;
  std::string orderBy = "filename";
  bool descending = true;
  int64_t pageNum = 1;
  int64_t pageSize = 130;
  std::string x_request_id = "";

  std::string params_json = Cloud189::Apis::ListFiles::JsonStringHelper(
      folderId, recursive, fileType, mediaType, mediaAttr, iconOption, orderBy,
      descending, pageNum, pageSize, x_request_id);

  //打印参数
  printf("params_json:\n%s\n", params_json.c_str());

  //  http请求初始化
  assistant::HttpRequest list_files_request("");
  Cloud189::Apis::ListFiles::HttpRequestEncode(params_json, list_files_request);

  // 设置代理
  // list_files_request.extends.Set("proxy", "127.0.0.1:8888");

  //  发起请求
  auto assistant_ptr = std::make_unique<assistant::Assistant_v3>();
  auto response = assistant_ptr->SyncHttpRequest(list_files_request);

  //  http响应解析
  std::string decode_res;
  Cloud189::Apis::ListFiles::HttpResponseDecode(response, list_files_request,
                                                decode_res);

  printf("response parse：\n%s", decode_res.c_str());
  return 0;
}
