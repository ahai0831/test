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
  return cloud_base::process_common_unix::GetCurrentApplicationDataPath(
      appdata_path);
#endif
}
std::string GetCurrentApplicationVersion() {
#ifdef _WIN32
  return cloud_base::process_common_win::GetCurrentApplicationVersion();
#else
  return cloud_base::process_common_unix::GetCurrentApplicationVersion();
#endif
}
}  // namespace process_common_helper
}  // namespace cloud_base
