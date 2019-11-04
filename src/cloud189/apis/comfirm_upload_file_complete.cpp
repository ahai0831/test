﻿#include "comfirm_upload_file_complete.h"

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

#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"

namespace {
// 这些是请求中一些固定的参数
const static std::string method = "POST";
const static std::string client_type = "TELEPC";
const static std::string channel_id = "web_cloud.189.cn";
const static std::string content_type = "application/x-www-form-urlencoded";
const static int flag = 1;
const static int resume_policy = 1;

std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
std::string GetChannelId() { return channel_id; }
std::string GetContentType() { return content_type; }
int GetFlag() { return flag; }
int GetResumePolicy() { return resume_policy; }

}  // namespace

namespace Cloud189 {
namespace Apis {
namespace ComfirmUploadFileComplete {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& fileCommitUrl,
                             const int64_t uploadFileId, const int32_t opertype,
                             const int32_t isLog) {
  std::string json_str = "";
  do {
    if (fileCommitUrl.empty()) {
      break;
    }
    json_str = assistant::tools::string::StringFormat(
        "{\"fileCommitUrl\" :\"%s\",\"uploadFileId\" : %" PRId64
        ",\"opertype\" : %d,\"isLog\" : %d}",
        fileCommitUrl.c_str(), uploadFileId, opertype, isLog);
  } while (false);
  return json_str;
}

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  bool is_ok = false;
  do {
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
    std::string fileCommitUrl =
        restful_common::jsoncpp_helper::GetString(json_str["fileCommitUrl"]);
    int64_t uploadFileId =
        restful_common::jsoncpp_helper::GetInt64(json_str["uploadFileId"]);
    int32_t opertype =
        restful_common::jsoncpp_helper::GetInt(json_str["opertype"]);
    int32_t isLog = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);

    request.url = fileCommitUrl;
    request.method = method;

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
        "uploadFileId=%" PRId64
        ""
        "&opertype=%d"
        "&isLog=%d"
        "&ResumePolicy=%d",
        uploadFileId, opertype, isLog, GetResumePolicy());

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
    auto upload_file = result_xml.child("file");
    result_json["id"] = upload_file.child("id").text().as_llong();
    result_json["name"] = upload_file.child("name").text().as_string();
    result_json["size"] = upload_file.child("size").text().as_llong();
    result_json["md5"] = upload_file.child("md5").text().as_string();
    result_json["createDate"] =
        upload_file.child("createDate").text().as_string();
    result_json["rev"] = upload_file.child("rev").text().as_string();
    result_json["userId"] = upload_file.child("userId").text().as_llong();
    result_json["requestId"] =
        upload_file.child("requestId").text().as_string();
    result_json["isSafe"] = upload_file.child("isSafe").text().as_int();
    response_info = result_json.toStyledString();
    is_success = true;
  } while (false);
  return is_success;
}

}  // namespace ComfirmUploadFileComplete
}  // namespace Apis
}  // namespace Cloud189
