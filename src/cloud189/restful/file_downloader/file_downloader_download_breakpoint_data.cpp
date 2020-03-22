#include "file_downloader_download_breakpoint_data.h"

#include <base64/base64.h>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using cloud_base::Base64Decode;
using cloud_base::Base64Encode;

using restful_common::jsoncpp_helper::GetInt64;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

namespace Cloud189 {
namespace Restful {
namespace file_downloader_helper {
namespace details {

bool GenerateBase64String_0_1(const breakpoint_data_0_1& data,
                              std::string& base64_string) {
  bool result = false;
  do {
    if (data.file_id.empty()) {
      break;
    }
    if (data.file_name.empty()) {
      break;
    }
    if (data.md5.empty()) {
      break;
    }
    if (data.download_folder_path.empty()) {
      break;
    }
    if (data.temp_download_file_path.empty()) {
      break;
    }
    const auto& offset = std::get<0>(data.to_be_continued);
    const auto& length = std::get<1>(data.to_be_continued);

    Json::Value root;
    root["ver"] = "0.1";
    root["file_id"] = data.file_id;
    root["file_name"] = data.file_name;
    root["md5"] = data.md5;
    root["download_folder_path"] = data.download_folder_path;
    root["temp_download_file_path"] = data.temp_download_file_path;

    Json::Value to_be_continued;
    to_be_continued["offset"] = offset;
    to_be_continued["length"] = length;
    root["to_be_continued"] = to_be_continued;

    const auto origin_string = WriterHelper(root);

    base64_string = Base64Encode(origin_string);
    result = true;
  } while (false);
  return result;
}

bool GenerateData_0_1(const std::string& base64_string,
                      breakpoint_data_0_1& data) {
  bool result = false;

  do {
    if (base64_string.empty()) {
      break;
    }

    const auto origin_string = Base64Decode(base64_string);
    if (origin_string.empty()) {
      break;
    }

    Json::Value root;
    if (!ReaderHelper(origin_string, root)) {
      break;
    }

    const auto ver = GetString(root["ver"]);
    if (ver.compare("0.1") != 0) {
      break;
    }
    data.file_id = GetString(root["file_id"]);
    if (data.file_id.empty()) {
      break;
    }
    data.file_name = GetString(root["file_name"]);
    if (data.file_name.empty()) {
      break;
    }
    data.md5 = GetString(root["md5"]);
    if (data.md5.empty()) {
      break;
    }
    data.download_folder_path = GetString(root["download_folder_path"]);
    if (data.download_folder_path.empty()) {
      break;
    }
    data.temp_download_file_path = GetString(root["temp_download_file_path"]);
    if (data.temp_download_file_path.empty()) {
      break;
    }

    const auto& to_be_continued = root["to_be_continued"];
    const auto offset = GetInt64(to_be_continued["offset"]);
    const auto length = GetInt64(to_be_continued["length"]);
    data.to_be_continued = std::make_tuple(offset, length);

    result = true;
  } while (false);
  return result;
}

}  // namespace details
}  // namespace file_downloader_helper
}  // namespace Restful
}  // namespace Cloud189
