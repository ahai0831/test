#include "upload_slice_data.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>

#include <tools/string_format.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

namespace Cloud189 {
namespace Apis {
namespace UploadSliceData {

// 用于构建一个json字符串，包含创建文件上传需要的参数
std::string JsonStringHelper(const std::string& fileUploadUrl,
                             const int64_t uploadFileId,
                             const int64_t startOffset,
                             const int64_t offsetLength,
                             const int32_t resumePolicy,
                             const int64_t uploadSliceId,
                             const std::string& MD5) {
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

}  // namespace UploadSliceData
}  // namespace Apis
}  // namespace Cloud189
