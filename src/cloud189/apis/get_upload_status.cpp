#include "get_upload_status.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>
#include <pugixml.hpp>

#include <filesystem_helper/filesystem_helper.h>
#include <process_version/process_version.h>
#include <v2/tools.h>
//#include <v2/uuid.h>
#include <tools/string_format.hpp>

#include "cloud189/error_code/error_code.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"

namespace {
// 这些是请求中一些固定的参数
const static std::string host = "https://api.cloud.189.cn";
const static std::string uri = "/getUploadFileStatus.action";
const static std::string method = "GET";
const static int flag = 1;
const static int resume_policy = 1;
const static std::string client_type = "TELEPC";
const static std::string channel_id = "web_cloud.189.cn";

std::string GetHost() { return host; }
std::string GetURI() { return uri; }
std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
std::string GetChannelId() { return channel_id; }
int GetFlag() { return flag; }
int GetResumePolicy() { return resume_policy; }

}  // namespace
namespace Cloud189 {
namespace Apis {
namespace GetUploadFileStatus {

// 用于构建一个json字符串，包含查询文件上传需要的参数
std::string JsonStringHelper(const std::string uploadFileId,
                             const std::string x_request_id) {
  return assistant::tools::string::StringFormat(
      R"({"uploadFileId":"%s","X-Request-ID":"%s"})", uploadFileId.c_str(),
      x_request_id.c_str());
}

// 查询文件上传状态请求
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  bool is_ok = false;
  do {
    Json::Value json_str;
    if (!restful_common::jsoncpp_helper::ReaderHelper(params_json, json_str)) {
      break;
    }
    request.url = GetHost() + GetURI();
    request.method = GetMethod();

    // add session key, signature and date.
    Cloud189::SessionHelper::AddCloud189Signature(request);

    std::string uploadFileId =
        restful_common::jsoncpp_helper::GetString(json_str["uploadFileId"]);
    std::string x_request_id =
        restful_common::jsoncpp_helper::GetString(json_str["X-Request-ID"]);

    // set url params
    // request.url += assistant::tools::string::StringFormat(
    //     "?uploadFileId=%s"
    //     "&ResumePolicy=%d"
    //     "&clientType=%s&version=%s&channelId=%s&rand=%s",
    //     uploadFileId.c_str(), GetResumePolicy(), GetClientType().c_str(),
    //     cloud_base::process_version::GetCurrentProcessVersion().c_str(),
    //     GetChannelId().c_str(),
    //     restful_common::rand_helper::GetRandString().c_str());

    // set header
    request.headers.Set("X-Request-ID", x_request_id);

    is_ok = true;
  } while (false);

  return is_ok;
}

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info) {
  bool is_success = false;
  Json::Value result_json;
  pugi::xml_document result_xml;
  auto http_status_code = response.status_code;
  auto curl_code = atoi(response.extends.Get("CURLcode").c_str());
  auto content_type = response.headers.Get("Content-Type");
  auto content_length = atoll(response.headers.Get("Content-Length").c_str());
  do {
    if (0 == curl_code && http_status_code / 100 == 2) {
      if (response.body.size() == 0) {
        break;
      }
      auto prs = result_xml.load_string(response.body.c_str());
      if (prs.status != pugi::xml_parse_status::status_ok) {
        break;
      }
      auto upload_file = result_xml.child("uploadFile");
      result_json["uploadFileId"] =
          upload_file.child("uploadFileId").text().as_string();
      result_json["size"] = upload_file.child("size").text().as_llong();
      result_json["fileUploadUrl"] =
          upload_file.child("fileUploadUrl").text().as_string();
      result_json["fileCommitUrl"] =
          upload_file.child("fileCommitUrl").text().as_string();
      result_json["fileDataExists"] =
          upload_file.child("fileDataExists").text().as_int();
      auto message = result_xml.child("message");
      result_json["waitingTime"] = message.child("waitingTime").text().as_int();
      response_info = result_json.toStyledString();
    } else if (0 == curl_code && http_status_code / 100 != 2) {
      if (!response.body.empty() &&
          content_type.find("application/xml; charset=utf-8") ==
              std::string::npos) {
        result_json["errorCode"] = Cloud189::ErrorCode::strErrCode(
            ErrorCode::nderr_content_type_error);
        result_json["int32ErrorCode"] = ErrorCode::nderr_content_type_error;
        break;
      }
      if (result_json["errorCode"].isString()) {
        result_json["int32ErrorCode"] = Cloud189::ErrorCode::int32ErrCode(
            restful_common::jsoncpp_helper::GetString(result_json["errorCode"])
                .c_str());
      } else {
        result_json["int32ErrorCode"] =
            restful_common::jsoncpp_helper::GetInt(result_json["errorCode"]);
      }
      break;
    } else if (0 >= http_status_code) {
      break;
    } else {
      break;
    }
    is_success = true;
  } while (false);
  result_json["isSuccess"] = is_success;
  result_json["httpStatusCode"] = http_status_code;
  result_json["curlCode"] = curl_code;
  response_info = restful_common::jsoncpp_helper::WriterHelper(result_json);
  return is_success;
}

}  // namespace GetUploadFileStatus
}  // namespace Apis
}  // namespace Cloud189
