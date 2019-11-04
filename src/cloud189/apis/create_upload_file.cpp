#include "create_upload_file.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <cinttypes>

#include <json/json.h>
#include <pugixml.hpp>

#include <filesystem_helper/filesystem_helper.h>
#include <process_version/process_version.h>
#include <v2/tools.h>
#include <v2/uuid.h>
#include <tools/string_format.hpp>

#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"

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
std::string GetClientType() { return client_type; }
std::string GetChannelId() { return channel_id; }
std::string GetContentType() { return content_type; }
int GetFlag() { return flag; }
int GetResumePolicy() { return resume_policy; }

}  // namespace

namespace Cloud189 {
namespace Apis {
namespace CreateUploadFile {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const int64_t parent_folder_id,
                             const std::string& local_path,
                             const std::string& md5, const int32_t oper_type,
                             const int32_t is_log) {
  std::string json_str = "";
  do {
    if (local_path.empty() || md5.empty()) {
      break;
    }
    json_str = assistant::tools::string::StringFormat(
        "{\"parentFolderId\" : %" PRId64
        ","
        "\"localPath\" : \"%s\","
        "\"md5\" : \"%s\","
        "\"opertype\" : %d,"
        "\"isLog\" : %d}",
        parent_folder_id, local_path.c_str(), md5.c_str(), oper_type, is_log);

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

    int64_t parent_folder_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["parentFolderId"]);
    std::string local_path =
        restful_common::jsoncpp_helper::GetString(json_str["localPath"]);
    std::string md5 =
        restful_common::jsoncpp_helper::GetString(json_str["md5"]);
    int32_t oper_type =
        restful_common::jsoncpp_helper::GetInt(json_str["opertype"]);
    int32_t is_log = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);

    if (local_path.empty() || md5.empty()) {
      break;
    }

    uint64_t file_size;
    std::wstring file_name;
    std::string file_last_change;
    if (!cloud_base::filesystem_helper::GetFileSize(
            assistant::tools::utf8ToWstring(local_path), file_size) ||
        !cloud_base::filesystem_helper::GetFileName(
            assistant::tools::utf8ToWstring(local_path), file_name) ||
        !cloud_base::filesystem_helper::GetFileLastChange(
            assistant::tools::utf8ToWstring(local_path), file_last_change)) {
      break;
    }

    request.url = GetHost() + GetURI();
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
    // set body
    request.body = assistant::tools::string::StringFormat(
        "parentFolderId=%" PRId64
        ""
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
        parent_folder_id, assistant::tools::wstringToUtf8(file_name).c_str(),
        file_size, md5.c_str(), file_last_change.c_str(), local_path.c_str(),
        oper_type, GetFlag(), GetResumePolicy(), is_log);
    is_ok = true;
  } while (false);

  return is_ok;
}

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info) {
  bool is_success = false;
  do {
    if (200 != response.status_code) {
      break;
    }
    pugi::xml_document result_xml;
    auto prs = result_xml.load_string(response.body.c_str());
    if (prs.status != pugi::xml_parse_status::status_ok) {
      break;
    }
    Json::Value result_json;
    auto upload_file = result_xml.child("uploadFile");
    result_json["uploadFileId"] =
        upload_file.child("uploadFileId").text().as_llong();
    result_json["fileUploadUrl"] =
        upload_file.child("fileUploadUrl").text().as_string();
    result_json["fileCommitUrl"] =
        upload_file.child("fileCommitUrl").text().as_string();
    result_json["fileDataExists"] =
        upload_file.child("fileDataExists").text().as_int();
    response_info = result_json.toStyledString();
    is_success = true;
  } while (false);
  return is_success;
}

}  // namespace CreateUploadFile
}  // namespace Apis
}  // namespace Cloud189
