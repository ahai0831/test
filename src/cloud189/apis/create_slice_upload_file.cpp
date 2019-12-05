#include "create_slice_upload_file.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>
#include <pugixml.hpp>

#include <UrlEncode/UrlEncode.h>
#include <filesystem_helper/filesystem_helper.h>
#include <process_version/process_version.h>
#include <v2/tools.h>
#include <v2/uuid.h>
#include <tools/string_format.hpp>

#include "cloud189/error_code/nderror.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"

namespace {
// 这些是请求中一些固定的参数
const static std::string host = "https://api.cloud.189.cn";
const static std::string uri = "/multitask/createSliceUploadFile.action";
const static std::string method = "POST";
const static std::string client_type = "TELEPC";
const static std::string channel_id = "web_cloud.189.cn";
const static std::string content_type = "application/x-www-form-urlencoded";
const static int flag = 1;
const static int resume_policy = 1;

std::string GetHost() { return host; }
std::string GetURI() { return uri; }
std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
std::string GetChannelId() { return channel_id; }
std::string GetContentType() { return content_type; }
int GetFlag() { return flag; }
int GetResumePolicy() { return resume_policy; }

}  // namespace

namespace Cloud189 {
namespace Apis {
namespace CreateSliceUploadFile {

// 用于构建一个json字符串，包含创建分片文件上传需要的参数
std::string JsonStringHelper(const std::string& localPath,
                             const int64_t parentFolderId,
                             const std::string& md5, const int32_t isLog,
                             const int32_t opertype) {
  std::string json_str = "";
  do {
    if (localPath.empty() || md5.empty()) {
      break;
    }
    Json::Value json_value;
    json_value["localPath"] = localPath;
    json_value["parentFolderId"] = parentFolderId;
    json_value["md5"] = md5;
    json_value["isLog"] = isLog;
    json_value["opertype"] = opertype;
    Json::FastWriter json_fastwrite;
    json_str = json_fastwrite.write(json_value);
  } while (false);
  return json_str;
}

// 创建文件上传请求
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  bool is_ok = false;
  do {
    if (params_json.empty()) {
      break;
    }
    Json::Value json_str;
    Json::CharReaderBuilder reader_builder;
    Json::CharReaderBuilder::strictMode(&reader_builder.settings_);
    std::unique_ptr<Json::CharReader> const reader(
        reader_builder.newCharReader());
    if (nullptr == reader) {
      break;
    }
    if (!reader->parse(params_json.c_str(),
                       params_json.c_str() + params_json.size(), &json_str,
                       nullptr)) {
      break;
    }
    std::string localPath =
        restful_common::jsoncpp_helper::GetString(json_str["localPath"]);
    int64_t parentFolderId =
        restful_common::jsoncpp_helper::GetInt64(json_str["parentFolderId"]);
    std::string md5 =
        restful_common::jsoncpp_helper::GetString(json_str["md5"]);
    int32_t isLog = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);
    int32_t opertype =
        restful_common::jsoncpp_helper::GetInt(json_str["opertype"]);

    if (localPath.empty() || md5.empty()) {
      break;
    }

    uint64_t size;
    std::wstring file_name;
    // 此处默认为6MB,6291456Bytes,合法值应该大于0且为3的倍数,单位为MB
    uint64_t sliceSize = 6291456;

    std::string file_last_change;
    if (!cloud_base::filesystem_helper::GetFileSize(
            assistant::tools::utf8ToWstring(localPath), size) ||
        !cloud_base::filesystem_helper::GetFileName(
            assistant::tools::utf8ToWstring(localPath), file_name)) {
      break;
    }
    std::string file_name_temp = assistant::tools::wstringToUtf8(file_name);

    request.url = GetHost() + GetURI();
    request.method = GetMethod();

    // add session key, signature and date.
    Cloud189::SessionHelper::AddCloud189Signature(request);

    // set url params
    request.url += assistant::tools::string::StringFormat(
        "?parentFolderId=%s&filename=%s&md5=%s&size=%s&sliceSize=%s&isLog=%s&"
        "opertype=%s&clientType=%s&version=%s&channelId=%s&rand=%s",
        std::to_string(parentFolderId).c_str(), file_name_temp.c_str(),
        md5.c_str(), std::to_string(size).c_str(),
        std::to_string(sliceSize).c_str(), std::to_string(isLog).c_str(),
        std::to_string(opertype).c_str(), GetClientType().c_str(),
        cloud_base::process_version::GetCurrentProcessVersion().c_str(),
        GetChannelId().c_str(),
        restful_common::rand_helper::GetRandString().c_str());
    // set header
    request.headers.Set("Content-Type", GetContentType());
    request.headers.Set("X-Request-ID", assistant::uuid::generate());
    // set body
    request.body = assistant::tools::string::StringFormat(
        R"({"parentFolderId":%s,"filename":"%s","md5": "%s","size": %s,"sliceSize": %s,"isLog": %s,"opertype": %s})",
        std::to_string(parentFolderId).c_str(), file_name_temp.c_str(),
        md5.c_str(), std::to_string(size).c_str(),
        std::to_string(sliceSize).c_str(), std::to_string(isLog).c_str(),
        std::to_string(opertype).c_str());
    is_ok = true;
  } while (false);

  return is_ok;
}

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info) {
  bool is_success = false;
  Json::Value result_json;
  Json::StreamWriterBuilder wbuilder;
  wbuilder.settings_["indentation"] = "";
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
          upload_file.child("uploadFileId").text().as_llong();
      result_json["fileUploadUrl"] =
          upload_file.child("fileUploadUrl").text().as_string();
      result_json["fileCommitUrl"] =
          upload_file.child("fileCommitUrl").text().as_string();
      result_json["uploadStatus"] =
          upload_file.child("uploadStatus").text().as_int();
      result_json["waitingTime"] =
          upload_file.child("waitingTime").text().as_int();
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
  response_info = Json::writeString(wbuilder, result_json);
  return is_success;
}

}  // namespace CreateSliceUploadFile
}  // namespace Apis
}  // namespace Cloud189
