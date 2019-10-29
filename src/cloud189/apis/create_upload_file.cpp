#include "create_upload_file.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <cinttypes>

#include <json/json.h>

#include <tools/string_format.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

namespace {
// 这些是请求中一些固定的参数
const static char* host = "https://api.cloud.189.cn";
const static char* uri = "/createUploadFile.action";
const static char* method = "POST";
const static int flag = 1;
const static int resume_policy = 1;
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

    // TODO(sun): GET file size, file name, file last change

    // TODO(sun): GET version

    // TODO(sun): GET rand

    request.url = std::string(host) + std::string(uri);
    request.method = method;

    // TODO(sun): set header(use SessionHelper)

    // TODO(sun): set url(params)

    // TODO(sun): set body

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

}  // namespace CreateUploadFile
}  // namespace Apis
}  // namespace Cloud189
