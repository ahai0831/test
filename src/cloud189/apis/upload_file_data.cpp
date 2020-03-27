#include "upload_file_data.h"

#include <cstdio>

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>
#include <pugixml.hpp>

#include <filesystem_helper/filesystem_helper.h>
#include <process_common/process_common_helper.h>
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
using Cloud189::ParamsHelper::GetChannelId;
using cloud_base::process_common_helper::GetCurrentApplicationVersion;

namespace {
// 这些是请求中一些固定的参数
const static std::string method = "PUT";
const static std::string content_type = "application/octet-stream";
const static int resume_policy = 1;

std::string GetMethod() { return method; }
std::string GetContentType() { return content_type; }
int GetResumePolicy() { return resume_policy; }
}  // namespace

namespace Cloud189 {
namespace Apis {
namespace UploadFileData {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& fileUploadUrl,
                             const std::string& localPath,
                             const std::string& uploadFileId,
                             const std::string& x_request_id,
                             const int64_t startOffset,
                             const int64_t offsetLength) {
  Json::Value json_value;
  do {
    if (fileUploadUrl.empty() || localPath.empty() || uploadFileId.empty()) {
      break;
    }
    json_value["fileUploadUrl"] = fileUploadUrl;
    json_value["localPath"] = localPath;
    json_value["uploadFileId"] = uploadFileId;
    json_value["X-Request-ID"] = x_request_id;
    json_value["startOffset"] = startOffset;
    json_value["offsetLength"] = offsetLength;
  } while (false);
  return restful_common::jsoncpp_helper::WriterHelper(json_value);
}

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  bool is_ok = false;
  do {
    Json::Value json_str;
    if (!restful_common::jsoncpp_helper::ReaderHelper(params_json, json_str)) {
      break;
    }
    std::string fileUploadUrl =
        restful_common::jsoncpp_helper::GetString(json_str["fileUploadUrl"]);
    std::string localPath =
        restful_common::jsoncpp_helper::GetString(json_str["localPath"]);
    std::string uploadFileId =
        restful_common::jsoncpp_helper::GetString(json_str["uploadFileId"]);
    std::string x_request_id =
        restful_common::jsoncpp_helper::GetString(json_str["X-Request-ID"]);
    int64_t startOffset =
        restful_common::jsoncpp_helper::GetInt64(json_str["startOffset"]);
    int64_t offsetLength =
        restful_common::jsoncpp_helper::GetInt64(json_str["offsetLength"]);

    uint64_t file_size;
    if (!cloud_base::file_common::GetFileSize(localPath, file_size)) {
      break;
    }

    request.url = fileUploadUrl;
    request.method = GetMethod();

    // add session key, signature and date.
    Cloud189::SessionHelper::AddCloud189Signature(request);

    // set url params
    request.url += assistant::tools::string::StringFormat(
        "?clientType=%s&version=%s&channelId=%s&rand=%s",
        GetClientType().c_str(), GetCurrentApplicationVersion().c_str(),
        GetChannelId().c_str(),
        restful_common::rand_helper::GetRandString().c_str());

    // set header
    request.headers.Set("Content-Type", GetContentType());
    request.headers.Set("X-Request-ID", x_request_id);
    request.headers.Set("ResumePolicy", std::to_string(GetResumePolicy()));
    request.headers.Set("Edrive-UploadFileId", uploadFileId);
    request.headers.Set("Edrive-UploadFileRange",
                        assistant::tools::string::StringFormat(
                            "bytes=%" PRId64 "-%" PRId64, startOffset,
                            file_size - startOffset));

    // set upload data
    request.extends.Set("upload_filepath", localPath.c_str());
    request.extends.Set("upload_filesize", std::to_string(file_size));
    request.extends.Set("upload_offset", std::to_string(startOffset));
    request.extends.Set("upload_length",
                        std::to_string(file_size - startOffset));

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
  response_info = restful_common::jsoncpp_helper::WriterHelper(result_json);
  return is_success;
}

}  // namespace UploadFileData
}  // namespace Apis
}  // namespace Cloud189
