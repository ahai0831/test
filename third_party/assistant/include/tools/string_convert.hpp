#pragma once
#ifndef _TOOLS_STRING_CONVERT_H__
#define _TOOLS_STRING_CONVERT_H__

#ifdef _WIN32
#include <codecvt>
#endif

#include <string>
#include <vector>
namespace assistant {
namespace tools {
namespace string {
namespace details {
static const auto kBerr = "";
static const auto kWerr = L"";
#ifdef _WIN32
typedef std::codecvt_utf8<wchar_t> UTF8;
/// 由于wstring_convert的构造函数不支持同时传入指定的_Pcvt_arg与_Berr_arg和_Werr_arg
/// 为了保证异常安全，需通过定义一个类来继承std::codecvt_byname，以支持指定_Berr_arg和_Werr_arg
typedef class chs_codecvt
    : public std::codecvt_byname<wchar_t, char, std::mbstate_t> {
 public:
  /// 注意"CHS"与平台相关，若需要跨平台的实现，需根据平台定义不同的cvt_name
  chs_codecvt() : codecvt_byname("CHS") {}
} CHS;
#endif
}  // namespace details
#ifdef WIN32
static inline std::string wstringToUtf8(const std::wstring& str) {
  std::wstring_convert<details::UTF8> strcvt(details::kBerr, details::kWerr);
  return strcvt.to_bytes(str);
}

static inline std::wstring utf8ToWstring(const std::string& str) {
  std::wstring_convert<details::UTF8> strcvt(details::kBerr, details::kWerr);
  return strcvt.from_bytes(str);
}

static inline std::string wstringToAnsi(const std::wstring& str) {
  std::wstring_convert<details::CHS> strcvt(details::kBerr, details::kWerr);
  return strcvt.to_bytes(str);
}

static inline std::wstring ansiToWstring(const std::string& str) {
  std::wstring_convert<details::CHS> strcvt(details::kBerr, details::kWerr);
  return strcvt.from_bytes(str);
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

}  // namespace string
}  // namespace tools
}  // namespace assistant

#endif  // _TOOLS_STRING_CONVERT_H__
