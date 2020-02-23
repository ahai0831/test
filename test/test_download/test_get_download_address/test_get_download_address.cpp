#include <iostream>
#include <memory>
#include <string>

#include <Assistant_v3.hpp>

#include <cloud189/apis/get_download_address.h>
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
  std::string fileId = "8151223815745916";
  int32_t dt = 3;
  std::string x_request_id = "";

  std::string params_json =
      Cloud189::Apis::GetDownloadAddress::JsonStringHelper(fileId, dt, "", "",
                                                           false, x_request_id);

  //打印参数
  printf("params_json:\n%s\n", params_json.c_str());

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

  printf("response parse：\n%s", decode_res.c_str());
  return 0;
}
