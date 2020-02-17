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
bool get_appdata_path(std::string &appdata_path);
// Get current process version
// return the version infomation if success
// return "" if failed
std::string GetCurrentProcessVersion();
}  // namespace process_common_unix
}  // namespace cloud_base
#endif  // PROCESS_COMMON_UNIX__H__