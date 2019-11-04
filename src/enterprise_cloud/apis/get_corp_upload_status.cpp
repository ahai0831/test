#include "get_corp_upload_status.h"

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

#include "enterprise_cloud/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"

namespace {
// 这些是请求中一些固定的参数
const static std::string host = "https://api-b.cloud.189.cn";
const static std::string uri = "/api/getCorpUploadFileStatus.action";
const static std::string method = "GET";
const static std::string client_type = "CORPPC";

std::string GetHost() { return host; }
std::string GetURI() { return uri; }
std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
}  // namespace

namespace EnterpriseCloud {
namespace Apis {
namespace GetUploadFileStatus {

// 用于构建一个json字符串，包含查询文件上传需要的参数
std::string JsonStringHelper(const int64_t uploadFileId, const int64_t corpId,
                             const int32_t isLog) {
  std::string json_str = "";
  do {
    json_str =
        assistant::tools::string::StringFormat("{\"uploadFileId\" : %" PRId64
                                               ","
                                               "\"coshareId\" : %" PRId64
                                               ","
                                               "\"isLog\" : %d}",
                                               uploadFileId, corpId, isLog);
  } while (false);
  return json_str;
}

// 查询文件上传状态请求
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
    int64_t corp_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["corpId"]);
    int64_t upload_file_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["uploadFileId"]);
    int32_t is_log = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);

    request.url = GetHost() + GetURI();
    request.method = GetMethod();

    // add session key, signature and date.
    EnterpriseCloud::SessionHelper::AddEnterpriseCloudSignature(request);

    // set url params
    request.url += assistant::tools::string::StringFormat(
        "?uploadFileId=%" PRId64 "corpId=%" PRId64
        "&isLog=%d"
        "&version=%s"
        "&rand=%s",
        upload_file_id, corp_id, is_log,
        cloud_base::process_version::GetCurrentProcessVersion().c_str(),
        restful_common::rand_helper::GetRandString().c_str());

    // set header
    request.headers.Set("X-Request-ID", assistant::uuid::generate());

    is_success = true;
  } while (false);

  return is_success;
}

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info) {
  // 请求的响应暂时不用有具体定义，先这样写
  return false;
}

}  // namespace GetUploadFileStatus
}  // namespace Apis
}  // namespace EnterpriseCloud
