#pragma once
#ifndef _TOOLS_STRING_CONVERT_H__
#define _TOOLS_STRING_CONVERT_H__

#include <codecvt>
#include <string>
#include <vector>
namespace assistant {
namespace tools {
namespace string {
namespace details {
static const auto kBerr = "";
static const auto kWerr = L"";
typedef std::codecvt_utf8<wchar_t> UTF8;
/// ����wstring_convert�Ĺ��캯����֧��ͬʱ����ָ����_Pcvt_arg��_Berr_arg��_Werr_arg
/// Ϊ�˱�֤�쳣��ȫ����ͨ������һ�������̳�std::codecvt_byname����֧��ָ��_Berr_arg��_Werr_arg
typedef class chs_codecvt
    : public std::codecvt_byname<wchar_t, char, std::mbstate_t> {
 public:
  /// ע��"CHS"��ƽ̨��أ�����Ҫ��ƽ̨��ʵ�֣������ƽ̨���岻ͬ��cvt_name
  chs_codecvt() : codecvt_byname("CHS") {}
} CHS;

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
    if (str[i] == splitchar && i == 0)  //��һ���������ָ��
    {
      start += 1;
    } else if (str[i] == splitchar) {
      std::string idStr = str.substr(start, i - start);
      vec.push_back(idStr);
      start = i + 1;
    } else if (i == length - 1)  //����β��
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
