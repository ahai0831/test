#include "get_upload_status.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>

#include <tools/string_format.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

namespace {
// 这些是请求中一些固定的参数
const static char* host = "https://api.cloud.189.cn";
const static char* uri = "/getUploadFileStatus.action";
const static char* method = "GET";
const static int flag = 1;
const static int resume_policy = 1;
const static char* client_type = "TELEPC";
const static char* channel_id = "web_cloud.189.cn";

}  // namespace
namespace Cloud189 {
namespace Apis {
namespace GetUploadFileStatus {

// 用于构建一个json字符串，包含查询文件上传需要的参数
std::string JsonStringHelper(const int64_t uploadFileId) {
  return assistant::tools::string::StringFormat(
      "{\"uploadFileId\": %" PRId64 "}", uploadFileId);
}

// 查询文件上传状态请求
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
    int64_t uploadFileId =
        restful_common::jsoncpp_helper::GetInt64(json_str["uploadFileId"]);

    // TODO(tiany): GET version

    // TODO(tiany): GET rand

    request.url = std::string(host) + std::string(uri);
    request.method = method;

    // TODO(tiany): set header(use SessionHelper)

    // TODO(tiany): set url(params)

    // TODO(tiany): set body

    is_ok = true;
  } while (false);

  return is_ok;
}

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info) {
  // 请求的响应暂时不用有具体定义，先这样写
  return false;
}

}  // namespace GetUploadFileStatus
}  // namespace Apis
}  // namespace Cloud189
