#include <cinttypes>

#include <iostream>
#include <thread>

#include <json/json.h>

#include <rxcpp/rx.hpp>

#include "enterprise_cloud/session_helper/session_helper.h"
using EnterpriseCloud::SessionHelper::EnterpriseCloudLogin;
using EnterpriseCloud::SessionHelper::EnterpriseCloudLogout;

#include "enterprise_cloud/upload_file_mastercontrol/upload_file_mastercontrol.h"
using EnterpriseCloud::UploadFileMasterControl::UploadFileMasterControl;

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
using namespace restful_common;

auto assistant_ptr = std::make_shared<assistant::Assistant_v3>();
// 参数设置
std::string corp_id = "114131189491";
std::string parent_id = "-683279323";
std::string md5 = "";  // 可有可无，如果要续传则必须传入
std::string coshare_id = "";
int32_t file_source = 3;
int32_t is_log = 0;
int32_t oper_type = 1;
std::string upload_file_id =
    "1384315735897436096";  // 可以无，如果有则优先选择续传，如果失效则重新上传

void multi_thread_test(int test_num, std::string file_path) {
  std::string file_md5;
  std::string file_upload_id;
  // 生成主控对象
  UploadFileMasterControl uf_master_control(
      assistant_ptr, file_path, corp_id, parent_id, md5, file_source,
      coshare_id, is_log, oper_type, upload_file_id);
  // 注册数据回调函数
  uf_master_control.StatusCallback([&](std::string msg) {
    // 读取数据
    Json::Value json_value;
    jsoncpp_helper::ReaderHelper(msg, json_value);
    // 记录数据
    if (json_value["stage"].asInt() == 0) {
      file_md5 = json_value["md5"].asString();
    } else if (json_value["stage"].asInt() == 300) {
      file_upload_id = json_value["upload_file_id"].asString();
    }

    printf("test%d: %s\n", test_num, msg.c_str());
  });
  // 启动上传
  if (!uf_master_control.Start()) {
    printf("test%d: master control start fail.\n", test_num);
  }

  //// 输入数字来暂停
  // int stop_flag;
  // std::cin >> stop_flag;
  // uf_master_control.Stop();
  //// 沉睡3秒
  // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

  // std::cout << file_md5 << std::endl << file_upload_id << std::endl;
  //// 测试续传
  // UploadFileMasterControl uf_master_control_2(
  //  assistant_ptr, file_path, corp_id, parent_id, file_md5, file_source,
  //  coshare_id, is_log, oper_type, file_upload_id);
  // uf_master_control_2.StatusCallback(
  //  [&](std::string msg) {  printf("test%d: %s\n", test_num, msg.c_str()); });
  // uf_master_control_2.Start();

  // std::cin >> stop_flag;
  // uf_master_control_2.Stop();

  while (1) {
  }
}

void multi_thread_test2(std::string path) {}

int main() {
  // 获取session信息
  std::string url =
      "https://api-b.cloud.189.cn/"
      "loginByOpen189AccessToken.action?appId=8138111606&accessToken="
      "45095ea6a25a42578cf41e06202e60a8&clientType=CORPPC";

  assistant::HttpRequest get_session_info("GET", url, "");
  auto res = assistant_ptr->SyncHttpRequest(get_session_info);
  Json::Value session_value;
  if (!jsoncpp_helper::ReaderHelper(res.body, session_value)) {
    std::cout << "get session info fail." << std::endl;
    return 0;
  }
  // 登陆设置session信息
  EnterpriseCloudLogin(session_value["sessionKey"].asString(),
                       session_value["sessionSecret"].asString());

  // 多线程测试
  std::string file_path1 = "E:\\Desktop\\test1.rar";
  auto tt1 = assistant::tools::string::ansiToWstring(file_path1);
  file_path1 = assistant::tools::string::wstringToUtf8(tt1);

  std::thread test1(multi_thread_test, 1, file_path1);
  test1.detach();

  std::string file_path2 = "E:\\Desktop\\test2.rar";
  auto tt2 = assistant::tools::string::ansiToWstring(file_path2);
  file_path2 = assistant::tools::string::wstringToUtf8(tt2);

  std::thread test2(multi_thread_test, 2, file_path2);
  test2.detach();

  std::string file_path3 = "E:\\Desktop\\test3.rar";
  auto tt3 = assistant::tools::string::ansiToWstring(file_path3);
  file_path3 = assistant::tools::string::wstringToUtf8(tt3);

  std::thread test3(multi_thread_test, 3, file_path3);
  test3.detach();

  std::string file_path4 = "E:\\Desktop\\test4.rar";
  auto tt4 = assistant::tools::string::ansiToWstring(file_path4);
  file_path4 = assistant::tools::string::wstringToUtf8(tt4);

  std::thread test4(multi_thread_test, 4, file_path4);
  test4.detach();

  std::string file_path5 = "E:\\Desktop\\test5.rar";
  auto tt5 = assistant::tools::string::ansiToWstring(file_path5);
  file_path5 = assistant::tools::string::wstringToUtf8(tt5);

  std::thread test5(multi_thread_test, 5, file_path5);
  test5.detach();

  while (true) {
  }

  // 登出，清除session信息
  EnterpriseCloudLogout();

  return 0;
}
