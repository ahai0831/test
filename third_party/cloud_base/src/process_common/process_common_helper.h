#pragma once
#ifndef PROCESS_COMMON_HELPER_H__
#define PROCESS_COMMON_HELPER_H__

#include <string>

namespace cloud_base {
namespace process_common_helper {
//  get the application temp path and write it to reference
//  parameter(appdata_path)
bool GetCurrentApplicationDataPath(std::string& appdata_path);
// Get current process version
// return the version infomation if success
// return "" if failed
std::string GetCurrentApplicationVersion();
// get appName
std::string GetCurrentApplicationName();
}  // namespace process_common_helper
}  // namespace cloud_base
#endif  // PROCESS_COMMON_HELPER_H__