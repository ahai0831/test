/**
 * @brief cloud_base::ProcessVersion.h
 * @date 2019-10-18
 *
 * @copyright Copyright 2019
 *
 */
#ifndef PROCESSVERSION_PROCESSVERSION_H__
#define PROCESSVERSION_PROCESSVERSION_H__
#include <string>

namespace cloud_base {
namespace process_version {
// Get current process version
// return the version infomation if success
// return "" if failed
std::string GetCurrentProcessVersion();
}  // namespace process_version
}  // namespace cloud_base
#endif  // PROCESSVERSION_PROCESSVERSION_H__
