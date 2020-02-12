#include "string_convert.h"

#ifdef _WIN32
#include <codecvt>
#endif

namespace cloud_base {
namespace tools {
namespace string {
namespace details {
static const auto kBerr = "";
static const auto kWerr = L"";
#ifdef _WIN32
typedef std::codecvt_utf8<wchar_t> UTF8;
/// ����wstring_convert�Ĺ��캯����֧��ͬʱ����ָ����_Pcvt_arg��_Berr_arg��_Werr_arg
/// Ϊ�˱�֤�쳣��ȫ����ͨ������һ�������̳�std::codecvt_byname����֧��ָ��_Berr_arg��_Werr_arg
typedef class chs_codecvt
    : public std::codecvt_byname<wchar_t, char, std::mbstate_t> {
 public:
  /// ע��"CHS"��ƽ̨��أ�����Ҫ��ƽ̨��ʵ�֣������ƽ̨���岻ͬ��cvt_name
  chs_codecvt() : codecvt_byname("CHS") {}
} CHS;
#endif
}  // namespace details

#ifdef _WIN32
std::string wstringToUtf8(const std::wstring& str) {
  std::wstring_convert<details::UTF8> strcvt(details::kBerr, details::kWerr);
  return strcvt.to_bytes(str);
}

std::wstring utf8ToWstring(const std::string& str) {
  std::wstring_convert<details::UTF8> strcvt(details::kBerr, details::kWerr);
  return strcvt.from_bytes(str);
}

std::string wstringToAnsi(const std::wstring& str) {
  std::wstring_convert<details::CHS> strcvt(details::kBerr, details::kWerr);
  return strcvt.to_bytes(str);
}

std::wstring ansiToWstring(const std::string& str) {
  std::wstring_convert<details::CHS> strcvt(details::kBerr, details::kWerr);
  return strcvt.from_bytes(str);
}
#endif

void StringSplit(const std::string& str, const std::string& split_char,
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
}  // namespace cloud_base
