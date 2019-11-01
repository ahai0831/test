#include "get_file_download_url.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>

#include <tools/string_format.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

namespace EnterpriseCloud {
namespace Apis {
namespace GetFileDownloadUrl {

// 用于构建一个json字符串，包含查询文件上传需要的参数
std::string JsonStringHelper(const int64_t fileId, const int64_t coshareId,
                             const int64_t corpId, const int32_t dt,
                             const int32_t flag) {
  std::string json_str = "";
  return json_str;
}

// 查询文件上传状态请求
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  return false;
}

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info) {
  // 请求的响应暂时不用有具体定义，先这样写
  return false;
}

}  // namespace GetFileDownloadUrl
}  // namespace Apis
}  // namespace EnterpriseCloud
