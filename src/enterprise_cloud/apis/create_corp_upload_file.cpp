#include "create_corp_upload_file.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <cinttypes>

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
const static std::string uri = "/api/createCorpUploadFile.action";
const static std::string method = "POST";
const static std::string client_type = "CORPPC";
const static std::string content_type = "application/x-www-form-urlencoded";

std::string GetHost() { return host; }
std::string GetURI() { return uri; }
std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
std::string GetContentType() { return content_type; }
}  // namespace

namespace EnterpriseCloud {
namespace Apis {
namespace CreateUploadFile {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& localPath, const int64_t corpId,
                             const int64_t parentId, const std::string& md5,
                             const int32_t fileSource,
                             const std::string coshareId, const int32_t isLog) {
  std::string json_str = "";
  do {
    if (localPath.empty() || md5.empty()) {
      break;
    }
    json_str = assistant::tools::string::StringFormat(
        "{\"localPath\" : \"%s\","
        "\"corpId\" : %" PRId64
        ","
        "\"parentId\" : %" PRId64
        ","
        "\"md5\" : \"%s\","
        "\"fileSource\" : %d,"
        "\"coshareId\" :  \"%s\","
        "\"isLog\" : %d}",
        localPath.c_str(), corpId, parentId, md5.c_str(), fileSource,
        coshareId.c_str(), isLog);
  } while (false);
  return json_str;
}

// 创建文件上传请求
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
    std::string local_path =
        restful_common::jsoncpp_helper::GetString(json_str["localPath"]);
    std::string md5 =
        restful_common::jsoncpp_helper::GetString(json_str["md5"]);
    std::string coshare_id =
        restful_common::jsoncpp_helper::GetString(json_str["coshareId"]);
    int64_t corp_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["corpId"]);
    int64_t parent_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["parentId"]);
    int32_t file_source =
        restful_common::jsoncpp_helper::GetInt(json_str["fileSource"]);
    int32_t is_log = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);
    if (local_path.empty() || md5.empty()) {
      break;
    }

    uint64_t file_size;
    std::wstring file_name;
    if (!cloud_base::filesystem_helper::GetFileSize(
            assistant::tools::ansiToWstring(local_path), file_size) ||
        !cloud_base::filesystem_helper::GetFileName(
            assistant::tools::ansiToWstring(local_path), file_name)) {
      break;
    }
    std::string file_name_temp = assistant::tools::wstringToUtf8(file_name);

    request.url = GetHost() + GetURI();
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
        "corpId=%" PRId64 "&parentId=%" PRId64
        "&baseFileId="
        "&fileName=%s"
        "&fileSize=%" PRIu64
        "&md5=%s"
        "&fileSource=%d"
        "&coshareId=%s"
        "&isLog=%d",
        corp_id, parent_id,
        cloud_base::url_encode::http_post_form::url_encode(file_name_temp)
            .c_str(),
        file_size, md5.c_str(), file_source, coshare_id.c_str(), is_log);

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

}  // namespace CreateUploadFile
}  // namespace Apis
}  // namespace EnterpriseCloud
