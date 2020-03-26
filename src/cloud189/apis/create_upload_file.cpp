#include "create_upload_file.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <cinttypes>

#include <json/json.h>
#include <pugixml.hpp>

#include <UrlEncode/UrlEncode.h>
#include <filesystem_helper/filesystem_helper.h>
#include <process_version/process_version.h>
#include <v2/tools.h>
//#include <v2/uuid.h>
#include <filecommon/filecommon_helper.h>
#include <tools/string_format.hpp>

#include "cloud189/error_code/error_code.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"
#include "cloud189/params_helper/params_helper.hpp"


using Cloud189::ParamsHelper::GetClientType;
namespace {
// 这些是请求中一些固定的参数
const static std::string host = "https://api.cloud.189.cn";
const static std::string uri = "/createUploadFile.action";
const static std::string method = "POST";
const static std::string client_type = "TELEPC";
const static std::string channel_id = "web_cloud.189.cn";
const static std::string content_type = "application/x-www-form-urlencoded";
const static int flag = 1;
const static int resume_policy = 1;

std::string GetHost() { return host; }
std::string GetURI() { return uri; }
std::string GetMethod() { return method; }
//std::string GetClientType() { return client_type; }
std::string GetChannelId() { return channel_id; }
std::string GetContentType() { return content_type; }
int GetFlag() { return flag; }
int GetResumePolicy() { return resume_policy; }

}  // namespace

namespace Cloud189 {
namespace Apis {
namespace CreateUploadFile {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& parent_folder_id,
                             const std::string& local_path,
                             const std::string& md5,
                             const std::string& x_request_id,
                             const int32_t oper_type, const int32_t is_log) {
  Json::Value json_value;
  do {
    if (parent_folder_id.empty() || local_path.empty() || md5.empty()) {
      break;
    }
    json_value["parentFolderId"] = parent_folder_id;
    json_value["localPath"] = local_path;
    json_value["md5"] = md5;
    json_value["X-Request-ID"] = x_request_id;
    json_value["opertype"] = oper_type;
    json_value["isLog"] = is_log;
  } while (false);
  return restful_common::jsoncpp_helper::WriterHelper(json_value);
}

// 创建文件上传请求
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  bool is_ok = false;
  do {
    Json::Value json_str;
    if (!restful_common::jsoncpp_helper::ReaderHelper(params_json, json_str)) {
      break;
    }
    std::string parent_folder_id =
        restful_common::jsoncpp_helper::GetString(json_str["parentFolderId"]);
    std::string local_path =
        restful_common::jsoncpp_helper::GetString(json_str["localPath"]);
    std::string md5 =
        restful_common::jsoncpp_helper::GetString(json_str["md5"]);
    std::string x_request_id =
        restful_common::jsoncpp_helper::GetString(json_str["X-Request-ID"]);
    int32_t oper_type =
        restful_common::jsoncpp_helper::GetInt(json_str["opertype"]);
    int32_t is_log = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);

    if (parent_folder_id.empty() || local_path.empty() || md5.empty()) {
      break;
    }

    uint64_t file_size;
    std::string file_name;
    std::string file_last_change;
    if (!cloud_base::file_common::GetFileSize(local_path, file_size) ||
        !cloud_base::file_common::GetFileName(local_path, file_name) ||
        !cloud_base::file_common::GetFileLastChange(local_path.c_str(),
                                                    file_last_change)) {
      break;
    }
    //     std::string file_name_temp /*=
    //     assistant::tools::wstringToUtf8(file_name)*/;

    request.url = GetHost() + GetURI();
    request.method = GetMethod();

    // add session key, signature and date.
    Cloud189::SessionHelper::AddCloud189Signature(request);

    // set url params
    request.url += assistant::tools::string::StringFormat(
        "?clientType=%s&version=%s&channelId=%s&rand=%s",
        GetClientType().c_str(), "1.0.0.0", GetChannelId().c_str(),
        restful_common::rand_helper::GetRandString().c_str());
    // set header
    request.headers.Set("Content-Type", GetContentType());
    request.headers.Set("X-Request-ID", x_request_id);
    // set body
    request.body = assistant::tools::string::StringFormat(
        "parentFolderId=%s"
        "&baseFileId="
        "&fileName=%s"
        "&size=%" PRIu64
        ""
        "&md5=%s"
        "&lastWrite=%s"
        "&localPath=%s"
        "&opertype=%d"
        "&flag=%d"
        "&resumePolicy=%d"
        "&isLog=%d"
        "&fileExt=",
        parent_folder_id.c_str(),
        cloud_base::url_encode::http_post_form::url_encode(file_name).c_str(),
        file_size, md5.c_str(), file_last_change.c_str(),
        cloud_base::url_encode::http_post_form::url_encode(local_path).c_str(),
        oper_type, GetFlag(), GetResumePolicy(), is_log);
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

}  // namespace CreateUploadFile
}  // namespace Apis
}  // namespace Cloud189
