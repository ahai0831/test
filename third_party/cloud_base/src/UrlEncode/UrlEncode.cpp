#include "UrlEncode.h"

#include <cstdio>

namespace {
unsigned char to_hex(unsigned char x) { return x > 9 ? x + 55 : x + 48; }

unsigned char from_hex(unsigned char x) {
  unsigned char y;
  if (x >= 'A' && x <= 'Z') {
    y = x - 'A' + 10;
  } else if (x >= 'a' && x <= 'z') {
    y = x - 'a' + 10;
  } else if (isdigit(x)) {
    y = x - '0';
  } else {
    y = '\0';
  }
  return y;
}
}  // namespace

namespace cloud_base {
namespace url_encode {

/// rfc3986
namespace rfc3986 {
// url encode,ensure that input parameter is utf8 format
std::string url_encode(const std::string& str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') ||
        (str[i] == '.') || (str[i] == '~')) {
      strTemp += str[i];
    } else {
      strTemp += '%';
      strTemp += to_hex((unsigned char)str[i] >> 4);
      strTemp += to_hex((unsigned char)str[i] % 16);
    }
  }
  return strTemp;
}

// url decode,ensure that input parameter is utf8 format
// This method is a general method
std::string url_decode(const std::string& str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    if (str[i] == '+') {
      strTemp += ' ';
    } else if (str[i] == '%') {
      if (i + 2 < length) {
        unsigned char high = from_hex((unsigned char)str[++i]);
        unsigned char low = from_hex((unsigned char)str[++i]);
        strTemp += high * 16 + low;
      } else {
        strTemp = "";
        break;
      }
    } else {
      strTemp += str[i];
    }
  }
  return strTemp;
}

}  // namespace rfc3986

/// http_post_form
namespace http_post_form {
// url encode,ensure that input parameter is utf8 format
std::string url_encode(const std::string& str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    if (isalnum((unsigned char)str[i])) {
      strTemp += str[i];
    } else if (str[i] == ' ') {
      strTemp += "+";
    } else {
      strTemp += '%';
      strTemp += to_hex((unsigned char)str[i] >> 4);
      strTemp += to_hex((unsigned char)str[i] % 16);
    }
  }
  return strTemp;
}

// url decode,ensure that input parameter is utf8 format
// This method is a general method
std::string url_decode(const std::string& str) {
  return rfc3986::url_decode(str);
}

}  // namespace http_post_form

}  // namespace url_encode
}  // namespace cloud_base
