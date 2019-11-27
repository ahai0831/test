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

namespace {
// 这些是请求中一些固定的参数
#define CONTENTTYPEERROR "ContentTypeError"  //70001
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
                             const int64_t uploadFileId, const int64_t corpId,
                             const int32_t fileSource,
                             const int64_t startOffset,
                             const int64_t offsetLength, const int32_t isLog) {
  std::string json_str = "";
  do {
    if (fileUploadUrl.empty() || localPath.empty()) {
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
    Json::FastWriter json_fastwrite;
    json_str = json_fastwrite.write(json_value);
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
    static Json::CharReaderBuilder char_reader_builder;
    std::unique_ptr<Json::CharReader> char_reader(
        char_reader_builder.newCharReader());
    if (nullptr == char_reader) {
      break;
    }
    Json::Value json_str;
    if (false == char_reader->parse(params_json.c_str(),
                                    params_json.c_str() + params_json.length(),
                                    &json_str, nullptr)) {
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
    int64_t corp_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["corpId"]);
    int32_t is_log = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);
    int32_t file_source =
        restful_common::jsoncpp_helper::GetInt(json_str["fileSource"]);

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
    request.headers.Set(
        "CorpId", assistant::tools::string::StringFormat("%" PRId64, corp_id));
    request.headers.Set("UploadFileId", assistant::tools::string::StringFormat(
                                            "%" PRId64, uploadFileId));
    request.headers.Set("FileSize", assistant::tools::string::StringFormat(
                                        "%" PRId64, file_size));
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
  Json::Reader json_reader;
  auto http_status_code = response.status_code;
  auto curl_code = atoi(response.extends.Get("CURLcode").c_str());
  auto content_type = response.headers.Get("Content-Type");
  auto content_length = atoll(response.headers.Get("Content-Length").c_str());
  do {
    if (0 == curl_code && http_status_code / 100 == 2) {
      if (!response.body.empty() &&
          !json_reader.parse(response.body, json_value)) {
        break;
      }
    } else if (0 == curl_code && http_status_code / 100 != 2) {
      if (!content_type.empty() && !response.body.empty() &&
              content_type.find("application/json") == std::string::npos ||
          !json_reader.parse(response.body, json_value)) {
        json_value["errorCode"] = CONTENTTYPEERROR;
        json_value["int32ErrorCode"] = 70001;
        break;
      }
      if (json_value["errorCode"].isString()) {
        json_value["int32ErrorCode"] = EnterpriseCloud::ErrorCode::int32ErrCode(
            restful_common::jsoncpp_helper::GetString(json_value["errorCode"])
                .c_str());
      } else {
        json_value["int32ErrorCode"] =
            restful_common::jsoncpp_helper::GetInt(json_value["errorCode"]);
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
  response_info = json_value.toStyledString();
  return is_success;
}

}  // namespace UploadFileData
}  // namespace Apis
}  // namespace EnterpriseCloud
