#include "create_folder.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>
#include <pugixml.hpp>

#include <UrlEncode/UrlEncode.h>
#include <process_common/process_common_helper.h>

#include <tools/string_format.hpp>

#include "cloud189/error_code/error_code.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"
#include "cloud189/params_helper/params_helper.hpp"

using cloud_base::process_common_helper::GetCurrentApplicationVersion;
using cloud_base::url_encode::http_post_form::url_encode;
using restful_common::jsoncpp_helper::GetString;
using Cloud189::ParamsHelper::GetClientType;
using Cloud189::ParamsHelper::GetChannelId;
using Cloud189::ParamsHelper::GetHost;

namespace {
// 这些是请求中一些固定的参数

const static std::string uri = "/createFolder.action";
const static std::string method = "POST";

std::string GetURI() { return uri; }
std::string GetMethod() { return method; }


}  // namespace
namespace Cloud189 {
namespace Apis {
namespace CreateFolder {

// 用于构建一个json字符串，包含查询文件上传需要的参数
std::string JsonStringHelper(const std::string parentFolderId,
                             const std::string relativePath,
                             const std::string folderName,
                             const std::string x_request_id) {
  return assistant::tools::string::StringFormat(
      R"({"parentFolderId":"%s","relativePath":"%s","folderName":"%s","X-Request-ID":"%s"})",
      parentFolderId.c_str(), relativePath.c_str(), folderName.c_str(),
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

    std::string parentFolderId = GetString(json_str["parentFolderId"]);
    /*********************解决中文参数乱码问题**********************/
    std::string relativePath_temp = GetString(json_str["relativePath"]);
    std::string folderName_temp = GetString(json_str["folderName"]);
    std::string relativePath = url_encode(relativePath_temp);
    std::string folderName = url_encode(folderName_temp);
    /**************************************************************/
    std::string x_request_id = GetString(json_str["X-Request-ID"]);

    //  set url params
    request.url += assistant::tools::string::StringFormat(
        "?parentFolderId=%s&relativePath=%s&folderName=%s"
        "&clientType=%s&version=%s&channelId=%s&rand=%s",
        parentFolderId.c_str(), relativePath.c_str(), folderName.c_str(),
        GetClientType().c_str(), GetCurrentApplicationVersion().c_str(),
        GetChannelId().c_str(),
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
  // auto content_length =
  // atoll(response.headers.Get("Content-Length").c_str());
  do {
    if (0 == curl_code && http_status_code / 100 == 2) {
      if (response.body.size() == 0) {
        break;
      }
      auto prs = result_xml.load_string(response.body.c_str());
      if (prs.status != pugi::xml_parse_status::status_ok) {
        break;
      }
      auto folder = result_xml.child("folder");

      result_json["id"] = folder.child("id").text().as_string();
      // 注：原id类型为int32_t，由于表示范围不够会被截断，此处改为string类型

      result_json["parentId"] = folder.child("parentId").text().as_string();
      result_json["name"] = folder.child("name").text().as_string();
      result_json["createDate"] = folder.child("createDate").text().as_string();
      result_json["lastOpTime"] = folder.child("lastOpTime").text().as_string();
      result_json["rev"] = folder.child("rev").text().as_string();
      // 注：原rev类型为int32_t，由于表示范围不够会被截断，此处改为string类型
      result_json["fileCata"] = folder.child("fileCata").text().as_int();

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

}  // namespace CreateFolder
}  // namespace Apis
}  // namespace Cloud189
