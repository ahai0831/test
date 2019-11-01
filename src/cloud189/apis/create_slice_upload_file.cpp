#include "create_slice_upload_file.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <cinttypes>

#include <json/json.h>

#include <tools/string_format.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

namespace Cloud189 {
namespace Apis {
namespace CreateSliceUploadFile {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& localPath,
                             const int64_t parentFolderId,
                             const std::string& md5, const int32_t isLog,
                             const int32_t opertype) {
  std::string json_str = "";
  return json_str;
}

// 创建文件上传请求
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

}  // namespace CreateSliceUploadFile
}  // namespace Apis
}  // namespace Cloud189
