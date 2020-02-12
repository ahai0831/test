#pragma once
#ifndef _CLOUD_BASE_TOOLS_STRING_CONVERT_H__
#define _CLOUD_BASE_TOOLS_STRING_CONVERT_H__

#include <string>
#include <vector>

namespace cloud_base {
namespace tools {
namespace string {

#ifdef _WIN32
std::string wstringToUtf8(const std::wstring& str);

std::wstring utf8ToWstring(const std::string& str);

std::string wstringToAnsi(const std::wstring& str);

std::wstring ansiToWstring(const std::string& str);
#endif

void StringSplit(const std::string& str, const std::string& split_char,
                 std::vector<std::string>& vec);

}  // namespace string
}  // namespace tools
}  // namespace cloud_base

#endif  // _CLOUD_BASE_TOOLS_STRING_CONVERT_H__
