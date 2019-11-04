#pragma once
#ifndef _TOOLS_STRINGFORMAT_H__
#define _TOOLS_STRINGFORMAT_H__

#include <cstdarg>
#include <string>

namespace assistant {
namespace tools {
namespace string {

// Depending on the format string, the function may require a series of
// additional parameters, each of which contains a value to be inserted,
// replacing each % tag specified in the format parameter. The number of
// parameters should be the same as the number of % tags

static std::string StringFormat(const char *format, ...) {
  std::string result;
  va_list args = nullptr;
  va_start(args, format);
  auto size_1 = vsnprintf(nullptr, 0, format, args);
  result.resize(size_1 + 1);
  // Notice that only when this returned value is non - negative and less than
  // n, the string has been completely written.
  auto size_2 = vsnprintf((char *)result.data(), size_1 + 1, format, args);
  result.resize(size_2);
  va_end(args);
  return (size_1 == size_2 ? result : std::string(""));
}

}  // namespace string
}  // namespace tools
}  // namespace assistant

#endif  // _TOOLS_STRINGFORMAT_H__
