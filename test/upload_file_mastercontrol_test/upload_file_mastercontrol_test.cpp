#include <cinttypes>

#include <iostream>

#include <json/json.h>

#include "enterprise_cloud/session_helper/session_helper.h"
using EnterpriseCloud::SessionHelper::EnterpriseCloudLogin;
using EnterpriseCloud::SessionHelper::EnterpriseCloudLogout;

#include "enterprise_cloud/upload_file_mastercontrol/upload_file_mastercontrol.h"
using EnterpriseCloud::UploadFileMasterControl::UploadFileMasterControl;

int main() {
  // 获取session信息
  std::string url =
      "https://api-b.cloud.189.cn/"
      "loginByOpen189AccessToken.action?appId=8138111606&accessToken="
      "29f36d40a8b0400ba8dcc68756866864&clientType=CORPPC";
  auto assistant_ptr = std::make_unique<assistant::Assistant_v3>();
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

  std::string file_path = "E:\\Desktop\\Desktop222.rar";
  int64_t corp_id = 114131189491;
  int64_t parent_id = -683279323;
  std::string md5 = "123FFEBB67E8F2F0E0B1E2CA593DBCBB";  //可有可无
  int32_t file_source = 3;
  std::string coshare_id;
  int32_t is_log = 0;
  int32_t oper_type = 1;
  std::string upload_file_id = "";  // 如果设置那么最好有效
  // 上传
  UploadFileMasterControl uf_master_control(file_path, corp_id, parent_id, md5,
                                            file_source, coshare_id, is_log,
                                            oper_type, upload_file_id);
  uf_master_control.StatusCallback(
      [](std::string msg) { std::cout << msg << std::endl; });
  if (!uf_master_control.Start()) {
    std::cout << "master control start fail." << std::endl;
  }

  int stop_flag;
  std::cin >> stop_flag;
  uf_master_control.Stop();

  while (true) {
  }

  // 登出，清除session信息
  EnterpriseCloudLogout();

  return 0;
}
