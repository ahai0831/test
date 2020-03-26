#include "get_download_address.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>
#include <pugixml.hpp>

//#include <filesystem_helper/filesystem_helper.h>
#include <process_common/process_common_helper.h>

#include <tools/string_format.hpp>

#include "cloud189/error_code/error_code.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"
#include "cloud189/params_helper/params_helper.hpp"


using Cloud189::ParamsHelper::GetClientType;
using cloud_base::process_common_helper::GetCurrentApplicationVersion;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetString;

namespace {
// 这些是请求中一些固定的参数
const static std::string host = "https://api.cloud.189.cn";
const static std::string uri = "/getFileDownloadUrl.action";
const static std::string method = "GET";
const static int flag = 1;
const static std::string client_type = "TELEPC";
const static std::string channel_id = "web_cloud.189.cn";

std::string GetHost() { return host; }
std::string GetURI() { return uri; }
std::string GetMethod() { return method; }
//std::string GetClientType() { return client_type; }
std::string GetChannelId() { return channel_id; }
int GetFlag() { return flag; }

}  // namespace
namespace Cloud189 {
namespace Apis {
namespace GetDownloadAddress {

// 用于构建一个json字符串，包含查询文件上传需要的参数
std::string JsonStringHelper(const std::string fileId, int32_t dt,
                             const std::string shareId,
                             const std::string groupSpaceId, bool shorts,
                             const std::string x_request_id) {
  return assistant::tools::string::StringFormat(
      R"({"fileId":"%s","dt":%d,"shareId":"%s","groupSpaceId":"%s","short":%d,"X-Request-ID":"%s"})",
      fileId.c_str(), dt, shareId.c_str(), groupSpaceId.c_str(), shorts,
      x_request_id.c_str());
}

// 查询文件上传状态请求
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  bool is_ok = false;
  do {
    Json::Value json_str;
    if (!restful_common::jsoncpp_helper::ReaderHelper(params_json, json_str)) {
      break;
    }
    request.url = GetHost() + GetURI();
    request.method = GetMethod();

    // add session key, signature and date.
    Cloud189::SessionHelper::AddCloud189Signature(request);

    std::string fileId = GetString(json_str["fileId"]);
    int32_t dt = GetInt(json_str["dt"]);
    std::string shareId = GetString(json_str["shareId"]);
    std::string groupSpaceId = GetString(json_str["groupSpaceId"]);
    int32_t shorts = GetInt(json_str["short"]);
    std::string x_request_id = GetString(json_str["X-Request-ID"]);

    //  set url params
    request.url += assistant::tools::string::StringFormat(
        "?fileId=%s&dt=%d&shareId=%s&groupSpaceId=%s&short=%d&x_request_id=%s"
        "&clientType=%s&version=%s&channelId=%s&rand=%s",
        fileId.c_str(), dt, shareId.c_str(), groupSpaceId.c_str(), shorts,
        x_request_id.c_str(), GetClientType().c_str(),
        GetCurrentApplicationVersion().c_str(), GetChannelId().c_str(),
        restful_common::rand_helper::GetRandString().c_str());

    // set header
    request.headers.Set("X-Request-ID", x_request_id);

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
      if (response.body.size() == 0) {
        break;
      }
      auto prs = result_xml.load_string(response.body.c_str());
      if (prs.status != pugi::xml_parse_status::status_ok) {
        break;
      }
      result_json["fileDownloadUrl"] =
          result_xml.child("fileDownloadUrl").first_child().text().as_string();
    } else if (0 == curl_code && http_status_code / 100 != 2) {
      if (!response.body.empty()) {
        /*****************解析error节点***********************/
        auto prs = result_xml.load_string(response.body.c_str());
        if (prs.status != pugi::xml_parse_status::status_ok) {
          break;
        }
        auto folder = result_xml.child("error");
        std::string errCode = folder.child("code").text().as_string();
        result_json["message"] = folder.child("message").text().as_string();
        /********************************************************/
        result_json["errorCode"] = errCode;
        result_json["int32ErrorCode"] =
            Cloud189::ErrorCode::int32ErrCode(errCode.c_str());
      }
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

}  // namespace GetDownloadAddress
}  // namespace Apis
}  // namespace Cloud189
