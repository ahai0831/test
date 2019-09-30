#include "base64.h"

#include <cinttypes>
#include <new>
#include <string>
namespace {
const char base64_enc_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
uint64_t base64_enc_len(uint64_t len) { return ((len + 2) / 3 * 4) + 1; }
/// encoded should alloc at least n(by base64_enc_len) bytes
uint64_t base64_enc(char *encoded, const char *string, uint64_t len) {
  uint64_t i;
  char *p;

  p = encoded;
  for (i = 0; i < len - 2; i += 3) {
    *p++ = base64_enc_table[(string[i] >> 2) & 0x3F];
    *p++ = base64_enc_table[((string[i] & 0x3) << 4) |
                            ((int)(string[i + 1] & 0xF0) >> 4)];
    *p++ = base64_enc_table[((string[i + 1] & 0xF) << 2) |
                            ((int)(string[i + 2] & 0xC0) >> 6)];
    *p++ = base64_enc_table[string[i + 2] & 0x3F];
  }
  if (i < len) {
    *p++ = base64_enc_table[(string[i] >> 2) & 0x3F];
    if (i == (len - 1)) {
      *p++ = base64_enc_table[((string[i] & 0x3) << 4)];
      *p++ = '=';
    } else {
      *p++ = base64_enc_table[((string[i] & 0x3) << 4) |
                              ((int)(string[i + 1] & 0xF0) >> 4)];
      *p++ = base64_enc_table[((string[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
  }

  *p++ = '\0';
  return p - encoded;
}

static const int base64_dec_table[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
    7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 0,  0,  0,  0,  63, 0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

uint64_t base64_dec_len(const void *data, const uint64_t len) {
  if (len == 0) {
    return 0;
  }

  unsigned char *p = (unsigned char *)data;
  uint64_t j = 0, pad1 = len % 4 || p[len - 1] == '=',
           pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
  const uint64_t last = (len - pad1) / 4 << 2;

  return last / 4 * 3 + pad1 + pad2;
}
void base64_dec(const void *data, const uint64_t len, unsigned char *out) {
  if (len == 0) {
    return;
  }

  unsigned char *p = (unsigned char *)data;
  uint64_t j = 0, pad1 = len % 4 || p[len - 1] == '=',
           pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
  const uint64_t last = (len - pad1) / 4 << 2;
  unsigned char *str = out;

  for (uint64_t i = 0; i < last; i += 4) {
    int n = base64_dec_table[p[i]] << 18 | base64_dec_table[p[i + 1]] << 12 |
            base64_dec_table[p[i + 2]] << 6 | base64_dec_table[p[i + 3]];
    str[j++] = n >> 16;
    str[j++] = n >> 8 & 0xFF;
    str[j++] = n & 0xFF;
  }
  if (pad1) {
    int n = base64_dec_table[p[last]] << 18 | base64_dec_table[p[last + 1]]
                                                  << 12;
    str[j++] = n >> 16;
    if (pad2) {
      n |= base64_dec_table[p[last + 2]] << 6;
      str[j++] = n >> 8 & 0xFF;
    }
  }
}

}  // namespace

namespace cloud_base {
void Base64Encode(const char *string, uint64_t len, char *&encoded,
                  uint64_t &encoded_len) {
  encoded_len = base64_enc_len(len);
  encoded = new (std::nothrow) char[static_cast<size_t>(encoded_len)];
  encoded_len = base64_enc(encoded, string, len);
}

void Base64Decode(const char *encoded, uint64_t len, char *&decoded,
                  uint64_t &decoded_len) {
  decoded_len = base64_dec_len(encoded, len);
  decoded = new (std::nothrow) char[static_cast<size_t>(decoded_len)];
  base64_dec(encoded, len, (unsigned char *)decoded);
}

/// In 32bits, `std::string` should be no more than 2GB.
std::string Base64Encode(const std::string &string) {
  std::string result;
  /// In 32bits, `std::string` should be no more than 2GB.
  auto encoded_len = static_cast<size_t>(base64_enc_len(string.size()));
  result.resize(encoded_len);
  encoded_len = static_cast<size_t>(base64_enc(
      const_cast<char *>(result.c_str()), string.c_str(), string.size()));
  result.resize(encoded_len);
  return result;
}

/// In 32bits, `std::string` should be no more than 2GB.
std::string Base64Decode(const std::string &encoded) {
  std::string result;
  auto decoded_len = base64_dec_len(encoded.c_str(), encoded.size());
  result.resize(static_cast<size_t>(decoded_len));
  base64_dec(encoded.c_str(), encoded.size(), (unsigned char *)result.c_str());
  return result;
}

std::string Base64Decode_raw(const std::string &encoded) {
  auto result = Base64Decode(encoded);
  auto len = result.size();
  for (auto iter = result.rbegin(); result.rend() != iter && '\0' == *iter;
       ++iter, --len) {
  }
  result.resize(len);
  return result;
}

}  // namespace cloud_base
