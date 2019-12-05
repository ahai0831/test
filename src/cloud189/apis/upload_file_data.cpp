#include "upload_file_data.h"

#include <cstdio>

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

#include "cloud189/error_code/nderror.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"

namespace {
// 这些是请求中一些固定的参数
const static std::string method = "PUT";
const static std::string client_type = "TELEPC";
const static std::string channel_id = "web_cloud.189.cn";
const static std::string content_type = "application/octet-stream";
const static int resume_policy = 1;

std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
std::string GetChannelId() { return channel_id; }
std::string GetContentType() { return content_type; }
int GetResumePolicy() { return resume_policy; }
}  // namespace

namespace Cloud189 {
namespace Apis {
namespace UploadFileData {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& fileUploadUrl,
                             const std::string& localPath,
                             const int64_t uploadFileId,
                             const int64_t startOffset,
                             const int64_t offsetLength) {
  std::string json_str = "";
  do {
    if (fileUploadUrl.empty() || localPath.empty()) {
      break;
    }
    Json::Value json_value;
    json_value["fileUploadUrl"] = fileUploadUrl;
    json_value["localPath"] = localPath;
    json_value["uploadFileId"] = uploadFileId;
    json_value["startOffset"] = startOffset;
    json_value["offsetLength"] = offsetLength;
    Json::FastWriter json_fastwrite;
    json_str = json_fastwrite.write(json_value);
  } while (false);
  return json_str;
}

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
    std::string fileUploadUrl =
        restful_common::jsoncpp_helper::GetString(json_str["fileUploadUrl"]);
    std::string localPath =
        restful_common::jsoncpp_helper::GetString(json_str["localPath"]);
    int64_t uploadFileId =
        restful_common::jsoncpp_helper::GetInt64(json_str["uploadFileId"]);
    int64_t startOffset =
        restful_common::jsoncpp_helper::GetInt64(json_str["startOffset"]);
    int64_t offsetLength =
        restful_common::jsoncpp_helper::GetInt64(json_str["offsetLength"]);

    uint64_t file_size;
    if (!cloud_base::filesystem_helper::GetFileSize(
            assistant::tools::ansiToWstring(localPath), file_size)) {
      break;
    }

    request.url = fileUploadUrl;
    request.method = GetMethod();

    // add session key, signature and date.
    Cloud189::SessionHelper::AddCloud189Signature(request);

    // set url params
    request.url += assistant::tools::string::StringFormat(
        "?clientType=%s&version=%s&channelId=%s&rand=%s",
        GetClientType().c_str(),
        cloud_base::process_version::GetCurrentProcessVersion().c_str(),
        GetChannelId().c_str(),
        restful_common::rand_helper::GetRandString().c_str());

    // set header
    request.headers.Set("Content-Type", GetContentType());
    request.headers.Set("X-Request-ID", assistant::uuid::generate());
    request.headers.Set("ResumePolicy", std::to_string(GetResumePolicy()));
    request.headers.Set(
        "Edrive-UploadFileId",
        assistant::tools::string::StringFormat("%" PRId64, uploadFileId));
    request.headers.Set("Edrive-UploadFileRange",
                        assistant::tools::string::StringFormat(
                            "bytes=%" PRId64 "-%" PRId64, startOffset,
                            startOffset + offsetLength));

    // set upload data
    request.extends.Set("upload_filepath", localPath.c_str());
    request.extends.Set("upload_filesize", std::to_string(file_size));
    request.extends.Set("upload_offset", std::to_string(startOffset));
    request.extends.Set("upload_length", std::to_string(offsetLength));

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
      if (response.body.size() == 0 && response.body.size() == content_length) {
        is_success = true;
        break;
      }
      auto prs = result_xml.load_string(response.body.c_str());
      if (prs.status != pugi::xml_parse_status::status_ok) {
        if (response.body.size() == 0) {
          is_success = true;
        }
        break;
      }
      response_info = result_json.toStyledString();
    } else if (0 == curl_code && http_status_code / 100 != 2) {
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

}  // namespace UploadFileData
}  // namespace Apis
}  // namespace Cloud189
