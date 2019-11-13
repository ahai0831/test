#include "comfirm_corp_upload_file_complete.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>

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
const static std::string method = "POST";
const static std::string client_type = "CORPPC";
const static std::string content_type = "application/x-www-form-urlencoded";

std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
std::string GetContentType() { return content_type; }
}  // namespace

namespace EnterpriseCloud {
namespace Apis {
namespace ComfirmUploadFileComplete {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& fileCommitUrl,
                             const int64_t uploadFileId, const int64_t corpId,
                             const int32_t fileSource, const int32_t opertype,
                             const std::string& coshareId,
                             const int32_t isLog) {
  std::string json_str = "";
  do {
    if (fileCommitUrl.empty()) {
      break;
    }
    json_str = assistant::tools::string::StringFormat(
        "{\"fileCommitUrl\":\"%s\","
        "\"uploadFileId\":%" PRId64
        ","
        "\"corpId\":%" PRId64
        ","
        "\"coshareId\":\"%s\","
        "\"fileSource\":%d,"
        "\"opertype\":%d,"
        "\"isLog\" : %d}",
        fileCommitUrl.c_str(), uploadFileId, corpId, coshareId.c_str(),
        fileSource, opertype, isLog);
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
    if (!char_reader->parse(params_json.c_str(),
                            params_json.c_str() + params_json.length(),
                            &json_str, nullptr)) {
      break;
    }
    std::string file_commit_url =
        restful_common::jsoncpp_helper::GetString(json_str["fileCommitUrl"]);
    std::string coshare_id =
        restful_common::jsoncpp_helper::GetString(json_str["coshareId"]);
    int64_t upload_file_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["uploadFileId"]);
    int64_t corp_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["corpId"]);
    int32_t is_log = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);
    int32_t file_source =
        restful_common::jsoncpp_helper::GetInt(json_str["fileSource"]);
    int32_t oper_type =
        restful_common::jsoncpp_helper::GetInt(json_str["opertype"]);

    request.url = file_commit_url;
    request.method = GetMethod();

    // add session key, signature and date.
    EnterpriseCloud::SessionHelper::AddEnterpriseCloudSignature(request);

    // set url params
    request.url += assistant::tools::string::StringFormat(
        "?clientType=%s&version=%s&rand=%s", GetClientType().c_str(),
        cloud_base::process_version::GetCurrentProcessVersion().c_str(),
        restful_common::rand_helper::GetRandString().c_str());

    // set header
    request.headers.Set("Content-Type", GetContentType());
    request.headers.Set("X-Request-ID", assistant::uuid::generate());

    // set body
    request.body = assistant::tools::string::StringFormat(
        "corpId=%" PRId64 "&uploadFileId=%" PRId64
        "&opertype=%d"
        "&fileSource=%d"
        "&coshareId=%s"
        "&isLog=%d",
        corp_id, upload_file_id, oper_type, file_source, coshare_id.c_str(),
        is_log);

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
      if (response.body.size() == 0) {
        break;
      }
      if (!json_reader.parse(response.body, json_value)) {
        break;
      }
    } else if (0 == curl_code && http_status_code / 100 != 2) {
      if (content_type.find("application/json; charset=UTF-8") == std::string::npos ||
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
}  // namespace ComfirmUploadFileComplete
}  // namespace Apis
}  // namespace EnterpriseCloud
