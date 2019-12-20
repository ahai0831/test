#include "commit_slice_upload_file.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>
#include <pugixml.hpp>

#include <filesystem_helper/filesystem_helper.h>
#include <process_version/process_version.h>
#include <v2/tools.h>
#include <v2/uuid.h>
#include <tools/string_format.hpp>

#include "cloud189/error_code/error_code.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"

namespace {
// 这些是请求中一些固定的参数
const static std::string method = "POST";
const static std::string client_type = "TELEPC";
const static std::string channel_id = "web_cloud.189.cn";
const static std::string content_type = "application/x-www-form-urlencoded";
const static int flag = 1;
const static int resume_policy = 1;

std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
std::string GetChannelId() { return channel_id; }
std::string GetContentType() { return content_type; }
int GetFlag() { return flag; }
int GetResumePolicy() { return resume_policy; }

}  // namespace

namespace Cloud189 {
namespace Apis {
namespace CommitSliceUploadFile {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& fileCommitUrl,
                             const std::string& uploadFileId,
                             const std::string& x_request_id,
                             const int32_t isLog, const int32_t opertype,
                             const std::string& sliceMD5) {
  std::string json_str = "";
  do {
    if (uploadFileId.empty() || fileCommitUrl.empty() || sliceMD5.empty()) {
      break;
    }
    json_str = assistant::tools::string::StringFormat(
        R"({"fileCommitUrl":"%s","uploadFileId":"%s","X-Request-ID":"%s","isLog": %s,"opertype": %s,"sliceMD5": "%s"})",
        fileCommitUrl.c_str(), uploadFileId.c_str(), x_request_id.c_str(),
        std::to_string(isLog).c_str(), std::to_string(opertype).c_str(),
        sliceMD5.c_str());
  } while (false);
  return json_str;
}

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  bool is_ok = false;
  do {
    Json::Value json_str;
    if (!restful_common::jsoncpp_helper::ReaderHelper(params_json, json_str)) {
      break;
    }
    std::string fileCommitUrl =
        restful_common::jsoncpp_helper::GetString(json_str["fileCommitUrl"]);
    std::string uploadFileId =
        restful_common::jsoncpp_helper::GetString(json_str["uploadFileId"]);
    std::string x_request_id =
        restful_common::jsoncpp_helper::GetString(json_str["X-Request-ID"]);
    int32_t isLog = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);
    int32_t opertype =
        restful_common::jsoncpp_helper::GetInt(json_str["opertype"]);
    std::string sliceMD5 =
        restful_common::jsoncpp_helper::GetString(json_str["sliceMD5"]);

    request.url = fileCommitUrl;
    request.method = GetMethod();

    // add session key, signature and date.
    Cloud189::SessionHelper::AddCloud189Signature(request);

    // set url params
    request.url += assistant::tools::string::StringFormat(
        R"(?clientType=%s&version=%s&channelId=%s&rand=%s)",
        GetClientType().c_str(),
        cloud_base::process_version::GetCurrentProcessVersion().c_str(),
        GetChannelId().c_str(),
        restful_common::rand_helper::GetRandString().c_str());

    // set header
    request.headers.Set("Content-Type", GetContentType());
    request.headers.Set("X-Request-ID", x_request_id);

    // set body
    request.url += assistant::tools::string::StringFormat(
        R"(&uploadFileId=%s&isLog=%s&opertype=%s&resumePolicy=%s&sliceMD5=%s)",
        uploadFileId.c_str(), std::to_string(isLog).c_str(),
        std::to_string(opertype).c_str(),
        std::to_string(GetResumePolicy()).c_str(), sliceMD5.c_str());
    // request.body = assistant::tools::string::StringFormat(
    //	R"(?uploadFileId=%s&isLog=%s&opertype=%s&resumePolicy=%s&sliceMD5=%s)",
    //	std::to_string(uploadFileId).c_str(), std::to_string(isLog).c_str(),
    //	std::to_string(opertype).c_str(), std::to_string(resumePolicy).c_str(),
    //	sliceMD5.c_str());
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
      auto upload_file = result_xml.child("file");
      result_json["id"] = upload_file.child("id").text().as_string();
      result_json["name"] = upload_file.child("name").text().as_string();
      result_json["size"] = upload_file.child("size").text().as_string();
      result_json["md5"] = upload_file.child("md5").text().as_string();
      result_json["createDate"] =
          upload_file.child("createDate").text().as_string();
      result_json["rev"] = upload_file.child("rev").text().as_string();
      result_json["userId"] = upload_file.child("userId").text().as_string();
      result_json["isSafe"] = upload_file.child("isSafe").text().as_string();
      response_info = result_json.toStyledString();
    } else if (0 == curl_code && http_status_code / 100 != 2) {
      if (!response.body.empty() &&
          content_type.find("text/xml; charset=UTF-8") == std::string::npos) {
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

}  // namespace CommitSliceUploadFile
}  // namespace Apis
}  // namespace Cloud189
