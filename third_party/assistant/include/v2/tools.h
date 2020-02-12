#pragma once
#ifndef ASSISTANT_TOOLS_H__
#define ASSISTANT_TOOLS_H__

#ifdef _WIN32
#include <codecvt>
#endif

#include <string>
#include <vector>
namespace assistant {
namespace tools {

  #ifdef _WIN32
static std::string wstringToUtf8(const std::wstring& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> strcvt(std::string(""),
                                                          std::wstring(L""));
  return strcvt.to_bytes(str);
}

static std::wstring utf8ToWstring(const std::string& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> strcvt(std::string(""),
                                                          std::wstring(L""));
  return strcvt.from_bytes(str);
}

static std::string wstringToAnsi(const std::wstring& str) {
  std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>>
  gbkStringConverter(
      new std::codecvt_byname<wchar_t, char, std::mbstate_t>("CHS"));
  return gbkStringConverter.to_bytes(str);
}

static std::wstring ansiToWstring(const std::string& str) {
  std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>>
  gbkStringConverter(
      new std::codecvt_byname<wchar_t, char, std::mbstate_t>("CHS"));
  return gbkStringConverter.from_bytes(str);
}
#endif

static void StringSplit(const std::string& str, const std::string& split_char,
                 std::vector<std::string>& vec) {
  char splitchar = ';';
  if (!split_char.empty()) {
    splitchar = split_char[0];
  }
  size_t start = 0;
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    if (str[i] == splitchar && i == 0)  //第一个就遇到分割符
    {
      start += 1;
    } else if (str[i] == splitchar) {
      std::string idStr = str.substr(start, i - start);
      vec.push_back(idStr);
      start = i + 1;
    } else if (i == length - 1)  //到达尾部
    {
      std::string idStr = str.substr(start, i + 1 - start);
      vec.push_back(idStr);
    }
  }
}
}  // namespace tools
}  // namespace assistant

#endif  // ASSISTANT_TOOLS_H__
