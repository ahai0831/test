#include "filecommon_helper.h"
#include "filecommon_unix.h"
#include "filecommon_win.h"

#include <tools/string_convert.h>

namespace cloud_base {
namespace file_common {
bool GetFileName(const std::string file_path, std::string& file_name) {
#ifdef _WIN32
  std::wstring file_name_wstr;
  bool result = false;
  result = filecommon_win::GetFileName(tools::string::utf8ToWstring(file_path),
                                       file_name_wstr);
  if (result) {
    file_name = tools::string::wstringToUtf8(file_name_wstr);
  }
  return result;
#elif
  return filecommon_unix::GetFileName(file_path, file_name);
#endif
}

bool GetFileLastChange(const char* file_path, std::string& file_modify_date) {
#ifdef _WIN32
  return filecommon_win::GetFileLastChange(
      tools::string::utf8ToWstring(file_path), file_modify_date);
#elif
  return filecommon_unix::get
#endif
  return filecommon_unix::GetFileLastChange(file_path, file_modify_date);
}

bool GetFileSize(const std::string& file_path, uint64_t& file_size) {
#ifdef _WIN32
  return filecommon_win::GetFileSize(tools::string::utf8ToWstring(file_path),
                                     file_size);
#elif
  return filecommon_unix::GetFileSize(file_path, file_size);
#endif
}

bool guarantee_directory_exists(const std::string& dir_path) {
#ifdef _WIN32
  return filecommon_win::guarantee_directory_exists(
      tools::string::utf8ToWstring(dir_path));
#elif
  return filecommon_unix::guarantee_directory_exists(dir_path);

#endif
}

bool get_file_list(const std::string& dirPath, const std::string& suffix,
                   std::vector<std::string>& vec) {
#ifdef _WIN32
  std::vector<std::wstring> wstr_vec;
  bool result = false;
  result = filecommon_win::get_file_list(tools::string::utf8ToWstring(dirPath),
                                         tools::string::utf8ToWstring(suffix),
                                         wstr_vec);
  if (result) {
    for (const auto& ch : wstr_vec) {
      std::string str = tools::string::wstringToUtf8(ch);
      vec.emplace_back(str);
    }
  }
  return result;
#elif
  return filecommon_unix::get_file_list(dirPath, suffix, vec);
#endif
}

}  // namespace file_common
}  // namespace cloud_base
