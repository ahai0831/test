#ifndef CLOUD189_RESTFUL_FILE_DOWNLOADER_DOWNLOAD_BREAKPOINT_DATA_H__
#define CLOUD189_RESTFUL_FILE_DOWNLOADER_DOWNLOAD_BREAKPOINT_DATA_H__
#include <cinttypes>
#include <string>
#include <tuple>

namespace Cloud189 {
namespace Restful {
namespace file_downloader_helper {
namespace details {
struct breakpoint_data_0_1 {
  std::string file_id;
  std::string file_name;
  std::string md5;
  std::string download_folder_path;
  std::string temp_download_file_path;
  std::tuple<int64_t, int64_t> to_be_continued;
};

bool GenerateBase64String_0_1(const breakpoint_data_0_1& data,
                              std::string& base64_string);
bool GenerateData_0_1(const std::string& base64_string,
                      breakpoint_data_0_1& data);
}  // namespace details
}  // namespace file_downloader_helper
}  // namespace Restful
}  // namespace Cloud189
#endif