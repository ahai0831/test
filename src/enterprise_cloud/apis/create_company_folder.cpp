#include "create_company_folder.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <cinttypes>

#include <json/json.h>

#include <tools/string_format.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

namespace EnterpriseCloud {
namespace Apis {
namespace CreateCompanyFolder {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const int64_t corpId, const int64_t parentId,
                             const std::string& fileName) {
  std::string json_str = "";
  return json_str;
}

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

}  // namespace CreateCompanyFolder
}  // namespace Apis
}  // namespace EnterpriseCloud
