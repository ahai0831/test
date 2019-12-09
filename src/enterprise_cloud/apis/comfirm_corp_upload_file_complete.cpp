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
using namespace restful_common;

namespace {
// 这些是请求中一些固定的参数
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
                             const std::string& uploadFileId,
                             const std::string& corpId,
                             const int32_t fileSource, const int32_t opertype,
                             const std::string& coshareId,
                             const int32_t isLog) {
  std::string json_str = "";
  do {
    if (fileCommitUrl.empty() || uploadFileId.empty() || corpId.empty()) {
      break;
    }
    Json::Value json_value;
    json_value["fileCommitUrl"] = fileCommitUrl;
    json_value["uploadFileId"] = uploadFileId;
    json_value["corpId"] = corpId;
    json_value["coshareId"] = coshareId;
    json_value["fileSource"] = fileSource;
    json_value["opertype"] = opertype;
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
    std::string file_commit_url =
        jsoncpp_helper::GetString(json_str["fileCommitUrl"]);
    std::string coshare_id = jsoncpp_helper::GetString(json_str["coshareId"]);
    std::string upload_file_id =
        jsoncpp_helper::GetString(json_str["uploadFileId"]);
    std::string corp_id = jsoncpp_helper::GetString(json_str["corpId"]);
    int32_t is_log = jsoncpp_helper::GetInt(json_str["isLog"]);
    int32_t file_source = jsoncpp_helper::GetInt(json_str["fileSource"]);
    int32_t oper_type = jsoncpp_helper::GetInt(json_str["opertype"]);

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
        "corpId=%s&uploadFileId=%s"
        "&opertype=%d"
        "&fileSource=%d"
        "&coshareId=%s"
        "&isLog=%d",
        corp_id.c_str(), upload_file_id.c_str(), oper_type, file_source,
        coshare_id.c_str(), is_log);

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
}  // namespace ComfirmUploadFileComplete
}  // namespace Apis
}  // namespace EnterpriseCloud
