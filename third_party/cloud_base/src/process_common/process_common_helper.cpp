#include "process_common_helper.h"
#ifdef _WIN32
#include "process_common_win.h"

#include <tools/string_convert.h>
#else
#include "process_common_unix.h"
#endif

namespace cloud_base {
namespace process_common_helper {
bool GetCurrentApplicationDataPath(std::string& appdata_path) {
#ifdef _WIN32
  std::wstring appdata_path_wstr;
  bool result = false;
  result = process_common_win::GetCurrentApplicationDataPath(appdata_path_wstr);
  if (result) {
    appdata_path = tools::string::wstringToUtf8(appdata_path_wstr);
  }
  return result;
#else
  bool result = false;
  std::string app_name =
      cloud_base::process_common_unix::GetCurrentMacOsXApplicationName();
  std::string log_home_path;
  result = cloud_base::process_common_unix::get_log_path(log_home_path);
  if (result && app_name.empty()) {
    std::string process_name =
        cloud_base::process_common_unix::GetCurrentUnixApplicationName();
    appdata_path = log_home_path + '/' + process_name;
  } else if (result) {
    appdata_path = log_home_path;
  }
  return result;
#endif
}
std::string GetCurrentApplicationVersion() {
#ifdef _WIN32
  return cloud_base::process_common_win::GetCurrentApplicationVersion();
#else
  return cloud_base::process_common_unix::GetCurrentApplicationVersion();
#endif
}
std::string GetCurrentApplicationName() {
#ifdef _WIN32
  std::wstring appdata_name_wstr =
      cloud_base::process_common_win::GetCurrentApplicationName();
  std::string appdata_name = tools::string::wstringToUtf8(appdata_name_wstr);
  return appdata_name;
#else
  std::string app_name =
      cloud_base::process_common_unix::GetCurrentMacOsXApplicationName();
  if (app_name.empty()) {
    std::string process_name =
        cloud_base::process_common_unix::GetCurrentUnixApplicationName();
    return process_name;
  } else {
    return app_name;
  }
#endif
}
}  // namespace process_common_helper
}  // namespace cloud_base
