#include <gtest/gtest.h>

#include <pugixml.hpp>

#include <Assistant_v3.hpp>

#include "cloud189/restful/cloud189_slice_uploader.h"
#include "cloud189/restful/cloud189_uploader.h"
#include "cloud189/session_helper/session_helper.h"
#include "tools/string_convert.hpp"

TEST(cloud189_uploader, init) {
  std::string file_path = "C:\\Users\\TY-PC\\Desktop\\你好.txt";
  std::wstring temp = assistant::tools::string::ansiToWstring(file_path);
  file_path = assistant::tools::string::wstringToUtf8(temp);
  std::string parent_id = "-11";
  std::string md5 = "";  //可有可无，如果要续传则必须传入，否则重新上传
  std::string x_request_id = "";
  int32_t is_log = 0;
  int32_t oper_type = 1;
  std::string upload_file_id = "1384315735897436096";
  //可以无，如果有则优先选择续传，如果失效则重新上传

  /// 获取session信息
  std::string url =
      "https://api.cloud.189.cn/"
      "getSessionForPC.action?appId=8025431004&accessToken="
      "30b3c47b020e4c38bc3817c0c825409c&clientType=TELEPC&version=6.1.1.0&"
      "clientSn=dc628a0f8a816f712579376e922eb2ff&channelId=web_cloud.189.cn&"
      "rand=10141_953109843";
  auto assistant_ptr = std::make_shared<assistant::Assistant_v3>();
  assistant::HttpRequest get_session_info("GET", url, "");
  auto res = assistant_ptr->SyncHttpRequest(get_session_info);
  if (res.status_code != 200) {
    std::cout << "get session info fail." << std::endl;
  }
  std::string reUrlBody = std::move(res.body);
  pugi::xml_document reDoc;
  pugi::xml_parse_result reDocResult = reDoc.load_string(reUrlBody.c_str());
  std::string psk =
      reDoc.child("userSession").child("sessionKey").first_child().value();
  std::string pss =
      reDoc.child("userSession").child("sessionSecret").first_child().value();
  std::string fsk = reDoc.child("userSession")
                        .child("familySessionKey")
                        .first_child()
                        .value();
  std::string fss = reDoc.child("userSession")
                        .child("familySessionSecret")
                        .first_child()
                        .value();
  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);

  /// uploader_info_helper
  std::string test_uploader_info = Cloud189::Restful::uploader_info_helper(
      file_path, md5, upload_file_id, parent_id, x_request_id, oper_type,
      is_log);

  Cloud189::Restful::Uploader up(test_uploader_info, [](const std::string&) {});

  up.AsyncStart();
  up.SyncWait();
  Cloud189::SessionHelper::Cloud189Logout();
  return;
}

TEST(cloud189_slice_uploader, init) {
  std::string file_path = "C:\\Users\\TY-PC\\Desktop\\slice150.rar";
  std::string parent_id = "-11";
  std::string md5 = "";  //可有可无，如果要续传则必须传入，否则重新上传
  std::string x_request_id = "";
  int32_t is_log = 0;
  int32_t oper_type = 1;
  std::string upload_file_id =
      "1384315735897436096";  // 可以无，如果有则优先选择续传，如果失效则重新上传
  int64_t slice_size_6mb = 6 * 1024 * 1024;

  /// 获取session信息
  std::string url =
      "https://api.cloud.189.cn/"
      "getSessionForPC.action?appId=8025431004&accessToken="
      "786aa0751f66415489a468f06ae3ef84&clientType=TELEPC&version=6.1.1.0&"
      "clientSn=dc628a0f8a816f712579376e922eb2ff&channelId=web_cloud.189.cn&"
      "rand=10141_953109843";
  auto assistant_ptr = std::make_shared<assistant::Assistant_v3>();
  assistant::HttpRequest get_session_info("GET", url, "");
  auto res = assistant_ptr->SyncHttpRequest(get_session_info);
  if (res.status_code != 200) {
    std::cout << "get session info fail." << std::endl;
  }
  std::string reUrlBody = std::move(res.body);
  pugi::xml_document reDoc;
  pugi::xml_parse_result reDocResult = reDoc.load_string(reUrlBody.c_str());
  std::string psk =
      reDoc.child("userSession").child("sessionKey").first_child().value();
  std::string pss =
      reDoc.child("userSession").child("sessionSecret").first_child().value();
  std::string fsk = reDoc.child("userSession")
                        .child("familySessionKey")
                        .first_child()
                        .value();
  std::string fss = reDoc.child("userSession")
                        .child("familySessionSecret")
                        .first_child()
                        .value();
  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);

  /// uploader_info_helper
  // std::string test_uploader_info = Cloud189::Restful::uploader_info_helper(
  //	file_path, md5, upload_file_id, parent_id, oper_type, is_log);

  // Cloud189::Restful::Uploader up(test_uploader_info, [](const std::string&)
  // {});

  std::string test_slice_uploader_info =
      Cloud189::Restful::sliceuploader_info_helper(
          file_path, md5, upload_file_id, parent_id, x_request_id,
          slice_size_6mb, oper_type, is_log);
  Cloud189::Restful::SliceUploader sliceup(
      test_slice_uploader_info,
      [](const std::string& info) { printf("%s\n", info.c_str()); });

  sliceup.AsyncStart();
  sliceup.SyncWait();
  Cloud189::SessionHelper::Cloud189Logout();
  return;
}