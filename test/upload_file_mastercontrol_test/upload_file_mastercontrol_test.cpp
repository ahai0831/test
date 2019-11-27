#include <cinttypes>

#include <iostream>

#include <json/json.h>

#include "enterprise_cloud/session_helper/session_helper.h"
using EnterpriseCloud::SessionHelper::EnterpriseCloudLogin;
using EnterpriseCloud::SessionHelper::EnterpriseCloudLogout;

#include "enterprise_cloud/upload_file_mastercontrol/upload_file_mastercontrol.h"
using EnterpriseCloud::UploadFileMasterControl::UploadFileMasterControl;

std::string file_md5;
std::string file_upload_id;

int main() {
  // 获取session信息
  std::string url =
      "https://api-b.cloud.189.cn/"
      "loginByOpen189AccessToken.action?appId=8138111606&accessToken="
      "29f36d40a8b0400ba8dcc68756866864&clientType=CORPPC";
  auto assistant_ptr = std::make_shared<assistant::Assistant_v3>();
  assistant::HttpRequest get_session_info("GET", url, "");
  auto res = assistant_ptr->SyncHttpRequest(get_session_info);
  Json::Value session_value;
  Json::Reader session_reader;
  if (!session_reader.parse(res.body, session_value)) {
    std::cout << "get session info fail." << std::endl;
    return 0;
  }
  // 登陆设置session信息
  EnterpriseCloudLogin(session_value["sessionKey"].asString(),
                       session_value["sessionSecret"].asString());
  // 参数设置
  std::string file_path = "E:\\Desktop\\Desktop.rar";
  auto tt = assistant::tools::string::ansiToWstring(file_path);
  file_path = assistant::tools::string::wstringToUtf8(tt);
  std::string corp_id = "114131189491";
  std::string parent_id = "-683279323";
  std::string md5 = "";  //可有可无，如果要续传则必须传入，否则重新上传
  int32_t file_source = 3;
  std::string coshare_id;
  int32_t is_log = 0;
  int32_t oper_type = 1;
  std::string upload_file_id =
      "1384315735897436096";  // 可以无，如果有则优先选择续传，如果失效则重新上传
  // 上传
  UploadFileMasterControl uf_master_control(
      assistant_ptr, file_path, corp_id, parent_id, md5, file_source,
      coshare_id, is_log, oper_type, upload_file_id);
  uf_master_control.StatusCallback([](std::string msg) {
    Json::Value json_value;
    Json::Reader json_reader;
    json_reader.parse(msg, json_value);
    if (json_value["stage"].asInt() == 0) {
      file_md5 = json_value["md5"].asString();
    } else if (json_value["stage"].asInt() == 300) {
      file_upload_id = json_value["upload_file_id"].asString();
    }
    std::cout << msg << std::endl;
  });
  if (!uf_master_control.Start()) {
    std::cout << "master control start fail." << std::endl;
  }

  // 输入数字来暂停
  int stop_flag;
  std::cin >> stop_flag;
  uf_master_control.Stop();
  // 沉睡3秒
  Sleep(3000);
  std::cout << file_md5 << std::endl << file_upload_id << std::endl;
  // 测试续传
  UploadFileMasterControl uf_master_control_2(
      assistant_ptr, file_path, corp_id, parent_id, file_md5, file_source,
      coshare_id, is_log, oper_type, file_upload_id);
  uf_master_control_2.StatusCallback(
      [](std::string msg) { std::cout << msg << std::endl; });
  uf_master_control_2.Start();

  std::cin >> stop_flag;
  uf_master_control_2.Stop();

  while (true) {
  }

  // 登出，清除session信息
  EnterpriseCloudLogout();

  return 0;
}
