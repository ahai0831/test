#include <iostream>
#include <memory>
#include <string>

#include <Assistant_v3.hpp>
#ifdef _WIN32
#include <tools/string_convert.hpp>
#endif
#include <cloud189/apis/get_download_address.h>
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
  std::string psk = "984a6228-e544-4fd3-9181-9ea53833ca07";
  std::string pss = "6D3CE734D0D3EDAB6E0D4E20AE270217";
  std::string fsk = "af5ab774-0d06-470a-9e55-ddec88568611_family";
  std::string fss = "2E7E0D81BF0C679482A57E783AFFFD62";
  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);  // 登录


//https://api.cloud.189.cn/getFileDownloadUrl.action?fileId=2150027468151523&dt=3&shareId=&groupSpaceId=&short=0&x_request_id=75D68834-0104-F478-2ED3-E591222A50E&clientType=TELEPC&version=&channelId=web_cloud.189.cn&rand=5504_98464583
std::string fileId = "2150027468151523";
int32_t dt = 3;
std::string shareId = "";
std::string groupSpaceId = "";
bool shorts = 0;
std::string x_request_id = "75D68834-0104-F478-2ED3-E591222A50E";

std::string params_json = Cloud189::Apis::GetDownloadAddress::JsonStringHelper(fileId,dt,shareId,groupSpaceId,shorts,x_request_id);

  //打印参数
#ifdef _WIN32
  auto params_json_wstr = utf8ToWstring(params_json);
  auto params_json_ansi = wstringToAnsi(params_json_wstr);
  printf("params_json:\n%s\n", params_json_ansi.c_str());
#else
  printf("params_json:\n%s\n", params_json.c_str());
#endif
  //  http请求初始化
  assistant::HttpRequest get_download_address_request("");
  Cloud189::Apis::GetDownloadAddress::HttpRequestEncode(params_json,
                                                  get_download_address_request);

  // 设置代理
   get_download_address_request.extends.Set("proxy", "127.0.0.1:8899");
    for (int i = 0; i<300; i++) {
         //  发起请求
          auto assistant_ptr = std::make_unique<assistant::Assistant_v3>();
          auto response = assistant_ptr->SyncHttpRequest(get_download_address_request);

          //  http响应解析
          std::string decode_res;
          Cloud189::Apis::GetDownloadAddress::HttpResponseDecode(
              response, get_download_address_request, decode_res);

        #ifdef _WIN32
          auto decode_res_wstr = utf8ToWstring(decode_res);
          auto decode_res_ansi = wstringToAnsi(decode_res_wstr);
          printf("response parse:\n%s", decode_res_ansi.c_str());
        #else
          printf("response parse:\n%s", decode_res.c_str());
        #endif
    }
 
  return 0;
}
