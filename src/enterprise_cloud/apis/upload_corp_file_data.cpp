#include "upload_corp_file_data.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>

#include <tools/string_format.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

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
const static std::string method = "PUT";
const static std::string client_type = "CORPPC";
const static std::string content_type = "application/octet-stream";
const static std::string expect = "100-continue";

std::string GetMethod() { return method; }
std::string GetClientType() { return client_type; }
std::string GetContentType() { return content_type; }
std::string GetExpect() { return expect; }

int64_t GetFileData(const int64_t startOffset, const int64_t offsetLength,
                    const std::string file_path, std::string& data_buff) {
  int64_t data_length = -1;
  do {
    if (file_path.empty() || 0 > startOffset || 0 > offsetLength) {
      break;
    }
    auto file_ptr = _fsopen(file_path.c_str(), "rb", _SH_DENYRW);
    if (nullptr == file_ptr) {
      break;
    }
    if (0 != _fseeki64(file_ptr, startOffset, SEEK_SET)) {
      break;
    }

    // read data
    data_buff.reserve(offsetLength + 1);
    data_buff.resize(offsetLength);
    data_length = fread((void*)data_buff.data(), 1, offsetLength, file_ptr);
  } while (false);
  return data_length;
}
}  // namespace

namespace EnterpriseCloud {
namespace Apis {
namespace UploadFileData {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& fileUploadUrl,
                             const std::string& localPath,
                             const int64_t uploadFileId, const int64_t corpId,
                             const int32_t fileSource,
                             const int64_t startOffset,
                             const int64_t offsetLength, const int32_t isLog) {
  std::string json_str = "";
  do {
    if (fileUploadUrl.empty() || localPath.empty()) {
      break;
    }
    json_str = assistant::tools::string::StringFormat(
        "{\"fileUploadUrl\":\"%s\","
        "\"localPath\":\"%s\","
        "\"uploadFileId\":%" PRId64
        ","
        "\"corpId\":%" PRId64
        ","
        "\"fileSource\":%d,"
        "\"startOffset\":%" PRId64
        ","
        "\"offsetLength\":%" PRId64
        ","
        "\"isLog\" : %d}",
        fileUploadUrl.c_str(), localPath.c_str(), uploadFileId, corpId,
        fileSource, startOffset, offsetLength, isLog);
  } while (false);
  return json_str;
}

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
    if (false == char_reader->parse(params_json.c_str(),
                                    params_json.c_str() + params_json.length(),
                                    &json_str, nullptr)) {
      break;
    }
    std::string fileUploadUrl =
        restful_common::jsoncpp_helper::GetString(json_str["fileUploadUrl"]);
    std::string localPath =
        restful_common::jsoncpp_helper::GetString(json_str["localPath"]);
    int64_t uploadFileId =
        restful_common::jsoncpp_helper::GetInt64(json_str["uploadFileId"]);
    int64_t startOffset =
        restful_common::jsoncpp_helper::GetInt64(json_str["startOffset"]);
    int64_t offsetLength =
        restful_common::jsoncpp_helper::GetInt64(json_str["offsetLength"]);
    int64_t corp_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["corpId"]);
    int32_t is_log = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);
    int32_t file_source =
        restful_common::jsoncpp_helper::GetInt(json_str["fileSource"]);

    uint64_t file_size;
    if (!cloud_base::filesystem_helper::GetFileSize(
            assistant::tools::ansiToWstring(localPath), file_size)) {
      break;
    }

    request.url = fileUploadUrl;
    request.method = GetMethod();

    // add session key, signature and date.
    EnterpriseCloud::SessionHelper::AddEnterpriseCloudSignature(request);

    // set url params
    request.url += assistant::tools::string::StringFormat(
        "?clientType=%s&version=%s&rand=%s", GetClientType().c_str(),
        cloud_base::process_version::GetCurrentProcessVersion().c_str(),
        restful_common::rand_helper::GetRandString().c_str());

    // set headers
    request.headers.Set("Content-Type", GetContentType());
    request.headers.Set("X-Request-ID", assistant::uuid::generate());
    request.headers.Set(
        "corpId", assistant::tools::string::StringFormat("%" PRId64, corp_id));
    request.headers.Set("uploadFileId", assistant::tools::string::StringFormat(
                                            "%" PRId64, uploadFileId));
    request.headers.Set("fileSize", assistant::tools::string::StringFormat(
                                        "%" PRId64, file_size));
    request.headers.Set("fileSource", std::to_string(file_source));
    request.headers.Set("isLog", std::to_string(is_log));
    request.headers.Set("Expect", GetExpect());

    // set upload data
    int64_t content_length =
        GetFileData(startOffset, offsetLength, localPath, request.body);
    if (-1 == content_length) {
      break;
    }
    request.headers.Set(
        "Content-Length",
        assistant::tools::string::StringFormat("%" PRId64, content_length));

    request.headers.Set("UploadFileRange",
                        assistant::tools::string::StringFormat(
                            "bytes=%" PRId64 "-%" PRId64, startOffset,
                            startOffset + content_length));

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

}  // namespace UploadFileData
}  // namespace Apis
}  // namespace EnterpriseCloud
