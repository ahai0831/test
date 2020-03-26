#include "get_folder_info.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>
#include <pugixml.hpp>

//#include <filesystem_helper/filesystem_helper.h>
#include <UrlEncode/UrlEncode.h>
#include <process_common/process_common_helper.h>
#include <tools/string_format.hpp>

#include "cloud189/error_code/error_code.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"
#include "cloud189/params_helper/params_helper.hpp"


using Cloud189::ParamsHelper::GetClientType;
using Cloud189::ParamsHelper::GetChannelId;
using Cloud189::ParamsHelper::GetHost;
using cloud_base::process_common_helper::GetCurrentApplicationVersion;
using cloud_base::url_encode::http_post_form::url_encode;

namespace {
// 这些是请求中一些固定的参数

const static std::string uri = "/getFolderInfo.action";
const static std::string method = "GET";

std::string GetURI() { return uri; }
std::string GetMethod() { return method; }



}  // namespace
namespace Cloud189 {
namespace Apis {

namespace GetFolderInfo {

// 用于构建一个json字符串，包含查询文件上传需要的参数
// 注：原folderId类型为int32_t，由于表示范围不够会被截断，此处改为string类型
std::string JsonStringHelper(const std::string folderId,
                             const std::string folderPath,
                             const int32_t pathList, const int32_t dt,
                             const std::string shareId,
                             const std::string groupSpaceId,
                             const std::string x_request_id) {
  return assistant::tools::string::StringFormat(
      R"({"folderId":"%s","folderPath":"%s","pathList":%d,"dt":%d,"shareId":"%s","groupSpaceId":"%s","X-Request-ID":"%s"})",
      folderId.c_str(), folderPath.c_str(), pathList, dt, shareId.c_str(),
      groupSpaceId.c_str(), x_request_id.c_str());
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
    std::string folderId =
        restful_common::jsoncpp_helper::GetString(json_str["folderId"]);
    int32_t pathList =
        restful_common::jsoncpp_helper::GetInt(json_str["pathList"]);
    int32_t dt = restful_common::jsoncpp_helper::GetInt(json_str["dt"]);
    std::string shareId =
        restful_common::jsoncpp_helper::GetString(json_str["shareId"]);
    std::string groupSpaceId =
        restful_common::jsoncpp_helper::GetString(json_str["groupSpaceId"]);
    /*********************解决中文参数乱码问题**********************/
    std::string folderPath_temp =
        restful_common::jsoncpp_helper::GetString(json_str["folderPath"]);
    std::string folderPath = url_encode(folderPath_temp);
    /**************************************************************/
    std::string x_request_id =
        restful_common::jsoncpp_helper::GetString(json_str["X-Request-ID"]);

    //  set url params
    request.url += assistant::tools::string::StringFormat(
        "?folderId=%s&folderPath=%s&pathList=%d&dt=%d&shareId=%s&groupSpaceId=%"
        "s"
        "&clientType=%s&version=%s&channelId=%s&rand=%s",
        folderId.c_str(), folderPath.c_str(), pathList, dt, shareId.c_str(),
        groupSpaceId.c_str(), GetClientType().c_str(),
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
      auto folderInfo = result_xml.child("folderInfo");
      result_json["id"] = folderInfo.child("id").text().as_string();
      result_json["parentFolderId"] =
          folderInfo.child("parentFolderId").text().as_int();
      result_json["groupSpaceId"] =
          folderInfo.child("groupSpaceId").text().as_string();
      result_json["path"] = folderInfo.child("path").text().as_string();
      result_json["name"] = folderInfo.child("name").text().as_string();
      result_json["starLabel"] = folderInfo.child("starLabel").text().as_int();
      result_json["createDate"] =
          folderInfo.child("createDate").text().as_string();
      result_json["lastOpTime"] =
          folderInfo.child("lastOpTime").text().as_string();
      result_json["rev"] = folderInfo.child("rev").text().as_string();
      response_info = restful_common::jsoncpp_helper::WriterHelper(result_json);
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

}  // namespace GetFolderInfo
}  // namespace Apis
}  // namespace Cloud189
