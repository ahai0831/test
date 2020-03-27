#include "file_data_download.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>
#include <pugixml.hpp>

#include <tools/string_format.hpp>

#include "cloud189/error_code/error_code.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using restful_common::jsoncpp_helper::GetString;

namespace Cloud189 {
namespace Apis {
namespace FileDataDownload {

// 用于构建一个json字符串，包含查询文件上传需要的参数
std::string JsonStringHelper(const std::string downloadUrl,
                             const std::string x_request_id) {
  return assistant::tools::string::StringFormat(
      R"({"downloadUrl":"%s","X-Request-ID":"%s"})", downloadUrl.c_str(),
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

    // add session key, signature and date.
    Cloud189::SessionHelper::AddCloud189Signature(request);

    //  set url params
    request.url = GetString(json_str["downloadUrl"]);

    // set header
    std::string x_request_id = GetString(json_str["X-Request-ID"]);
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
  do {
    if (0 == curl_code && http_status_code == 302) {
      result_json["redirect_url"] = response.extends.Get("redirect_url");
    } else if (0 == curl_code && http_status_code == 200) {
      result_json["content_length_download"] =
          response.extends.Get("content_length_download");
      result_json["size_download"] = response.extends.Get("size_download");
      result_json["speed_download"] = response.extends.Get("speed_download");
    } else if (0 == curl_code) {
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

}  // namespace FileDataDownload
}  // namespace Apis
}  // namespace Cloud189
