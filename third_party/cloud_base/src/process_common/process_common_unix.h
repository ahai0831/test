/**
 * @brief cloud_base::process_common_unix.h
 * @date 2020-2-17
 *
 * @copyright Copyright 2020
 *
 */
#ifndef PROCESS_COMMON_UNIX__H__
#define PROCESS_COMMON_UNIX__H__
#include <string>

namespace cloud_base {
namespace process_common_unix {
// appdata路径的获取
// 需拼接appName到此路径中
bool GetCurrentApplicationDataPath(std::string &appdata_path);

// Get current process version
// return the version infomation if success
// return "" if failed
std::string GetCurrentApplicationVersion();

/// log存放路径
bool get_log_path(std::string &log_path);

/// get current application name
std::string GetCurrentMacOsXApplicationName();

// get current unix的进程名
std::string GetCurrentUnixApplicationName();

}  // namespace process_common_unix
}  // namespace cloud_base
#endif  // PROCESS_COMMON_UNIX__H__
