#include "upload_corp_file_data.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>

#include <tools/string_format.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

#include <UrlEncode/UrlEncode.h>
#include <filesystem_helper/filesystem_helper.h>
#include <process_version/process_version.h>
#include <v2/tools.h>
#include <v2/uuid.h>
#include <tools/string_format.hpp>

#include "enterprise_cloud/error_code/nderror.h"
#include "enterprise_cloud/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"
using namespace restful_common;

namespace {
// 这些是请求中一些固定的参数
const static std::string method = "PUT";
const static std::string client_type = "CORPPC";
const static std::string content_type = "application/octet-stream";

std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
std::string GetContentType() { return content_type; }

}  // namespace

namespace EnterpriseCloud {
namespace Apis {
namespace UploadFileData {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& fileUploadUrl,
                             const std::string& localPath,
                             const std::string& uploadFileId,
                             const std::string& corpId,
                             const int32_t fileSource,
                             const int64_t startOffset,
                             const int64_t offsetLength, const int32_t isLog) {
  std::string json_str = "";
  do {
    if (fileUploadUrl.empty() || localPath.empty() || uploadFileId.empty() ||
        corpId.empty()) {
      break;
    }
    Json::Value json_value;
    json_value["fileUploadUrl"] = fileUploadUrl;
    json_value["localPath"] = localPath;
    json_value["uploadFileId"] = uploadFileId;
    json_value["corpId"] = corpId;
    json_value["fileSource"] = fileSource;
    json_value["startOffset"] = startOffset;
    json_value["offsetLength"] = offsetLength;
    json_value["isLog"] = isLog;
    json_str = jsoncpp_helper::WriterHelper(json_value);
  } while (false);
  return json_str;
}

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  bool is_success = false;
  do {
    if (params_json.empty()) {
      break;
    }
    // parse json
    Json::Value json_str;
    if (!jsoncpp_helper::ReaderHelper(params_json, json_str)) {
      break;
    }

    std::string fileUploadUrl =
        jsoncpp_helper::GetString(json_str["fileUploadUrl"]);
    std::string localPath = jsoncpp_helper::GetString(json_str["localPath"]);
    std::string uploadFileId =
        jsoncpp_helper::GetString(json_str["uploadFileId"]);
    int64_t startOffset = jsoncpp_helper::GetInt64(json_str["startOffset"]);
    int64_t offsetLength = jsoncpp_helper::GetInt64(json_str["offsetLength"]);
    std::string corp_id = jsoncpp_helper::GetString(json_str["corpId"]);
    int32_t is_log = jsoncpp_helper::GetInt(json_str["isLog"]);
    int32_t file_source = jsoncpp_helper::GetInt(json_str["fileSource"]);

    uint64_t file_size;
    if (!cloud_base::filesystem_helper::GetFileSize(
            assistant::tools::utf8ToWstring(localPath), file_size)) {
      break;
    }

    request.url = fileUploadUrl;
    request.method = GetMethod();

    // add session key, signature and date.
    EnterpriseCloud::SessionHelper::AddEnterpriseCloudSignature(request);

    // set url params
    request.url += assistant::tools::string::StringFormat(
        "?clientType=%s&version=%s&rand=%s", GetClientType().c_str(),
        cloud_base::process_version::GetCurrentProcessVersion().c_str(),
        restful_common::rand_helper::GetRandString().c_str());

    // set headers
    request.headers.Set("Content-Type", GetContentType());
    request.headers.Set("X-Request-ID", assistant::uuid::generate());
    request.headers.Set("CorpId", corp_id);
    request.headers.Set("UploadFileId", uploadFileId);
    request.headers.Set("FileSize", std::to_string(file_size));
    request.headers.Set("FileSource", std::to_string(file_source));
    request.headers.Set("IsLog", std::to_string(is_log));

    // set upload data
    request.extends.Set("upload_filepath", localPath.c_str());
    request.extends.Set("upload_filesize", std::to_string(file_size));
    request.extends.Set("upload_offset", std::to_string(startOffset));
    request.extends.Set("upload_length", std::to_string(offsetLength));

    request.headers.Set("UploadFileRange",
                        assistant::tools::string::StringFormat(
                            "bytes=%" PRId64 "-%" PRId64, startOffset,
                            startOffset + offsetLength));

    is_success = true;
  } while (false);
  return is_success;
}

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info) {
  bool is_success = false;
  Json::Value json_value;
  auto http_status_code = response.status_code;
  auto curl_code = atoi(response.extends.Get("CURLcode").c_str());
  auto content_type = response.headers.Get("Content-Type");
  auto content_length = atoll(response.headers.Get("Content-Length").c_str());
  do {
    if (0 == curl_code && http_status_code / 100 == 2) {
      if (0 == content_length) {
        is_success = true;
        break;
      }
      if (response.body.empty() ||
          !jsoncpp_helper::ReaderHelper(response.body, json_value)) {
        break;
      }
    } else if (0 == curl_code && http_status_code / 100 != 2) {
      if (content_type.find("application/json; charset=UTF-8") ==
          std::string::npos) {
        json_value["int32ErrorCode"] =
            EnterpriseCloud::ErrorCode::nderr_content_type_error;
        json_value["errorCode"] = EnterpriseCloud::ErrorCode::strErrCode(
            EnterpriseCloud::ErrorCode::nderr_content_type_error);
        break;
      }
      if (response.body.empty() ||
          !jsoncpp_helper::ReaderHelper(response.body, json_value)) {
        break;
      }
      if (json_value["errorCode"].isString()) {
        json_value["int32ErrorCode"] = EnterpriseCloud::ErrorCode::int32ErrCode(
            jsoncpp_helper::GetString(json_value["errorCode"]).c_str());
      } else {
        json_value["int32ErrorCode"] =
            jsoncpp_helper::GetInt(json_value["errorCode"]);
      }
      break;
    } else if (0 >= http_status_code) {
      break;
    } else {
      break;
    }
    is_success = true;
  } while (false);
  json_value["isSuccess"] = is_success;
  json_value["httpStatusCode"] = http_status_code;
  json_value["curlCode"] = curl_code;
  response_info = jsoncpp_helper::WriterHelper(json_value);
  return is_success;
}

}  // namespace UploadFileData
}  // namespace Apis
}  // namespace EnterpriseCloud
