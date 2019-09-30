//////////////////////////////////////////////////////////////////////////
/// According to such a benchmark: https://github.com/gaspardpetit/base64/

/// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
/// The fastest decoding implementation is polfosol's snippet from Stack
/// Overflow.

/// https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.c
/// For encoding. the Apache, written over 15 years ago, beats most of the code
/// snippets posted around.
//////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _BASE64_H__
#define _BASE64_H__
#include <cinttypes>
#include <string>

namespace cloud_base {
/// should delete encoded; in 32bits encoded must be no more than 4GB
void Base64Encode(const char *string, uint64_t len, char *&encoded,
                  uint64_t &encoded_len);
/// should delete decoded; in 32bits encoded must be no more than 4GB
void Base64Decode(const char *encoded, uint64_t len, char *&decoded,
                  uint64_t &decoded_len);
/// In 32bits, `std::string` should be no more than 2GB.
std::string Base64Encode(const std::string &string);
/// In 32bits, `std::string` should be no more than 2GB.
std::string Base64Decode(const std::string &encoded);
/// In 32bits, `std::string` should be no more than 2GB.
std::string Base64Decode_raw(const std::string &encoded);
}  // namespace cloud_base
#endif  //_BASE64_H__
