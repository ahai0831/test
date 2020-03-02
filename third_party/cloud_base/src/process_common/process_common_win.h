/**
 * @brief cloud_base::process_common_win.h
 * @date 2019-02-18
 *
 * @copyright Copyright 2020
 *
 */
#ifndef PROCESS_COMMON_WIN__H__
#define PROCESS_COMNON_WIN__H__
#include <string>

namespace cloud_base {
namespace process_common_win {
//  get the application temp path and write it to reference
//  parameter(appdata_path)
bool GetCurrentApplicationDataPath(std::wstring& appdata_path);
// Get current process version
// return the version infomation if success
// return "" if failed
std::string GetCurrentApplicationVersion();
// get appName
std::wstring GetCurrentApplicationName();
}  // namespace process_common_win
}  // namespace cloud_base
#endif  // PROCESS_COMMON_WIN__H__
