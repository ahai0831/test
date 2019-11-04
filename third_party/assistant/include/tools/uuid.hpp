#pragma once
#ifndef _TOOLS_UUID_H__
#define _TOOLS_UUID_H__
#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#pragma warning(disable : 4996)
#endif

#include <cinttypes>
#include <cstdio>
#include <string>

#if defined(_WIN32)
#include <objbase.h>
#elif defined(__linux__)
#include <uuid/uuid.h>
#else
#include <random>
#endif

namespace assistant {
namespace tools {
namespace uuid {
#if defined(_WIN32)
static std::string generate() {
  GUID guid = {0};
  std::string flag;
  if (CoCreateGuid(&guid) == S_OK) {
    /// In vs, _snprintf wouldn't append a '\0' when count==len
    auto len = _snprintf(
        nullptr, 0, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
        guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]);
    flag.resize(len);
    auto len_2 =
        _snprintf(const_cast<char *>(flag.data()), len,
                  "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                  guid.Data1, guid.Data2, guid.Data3, guid.Data4[0],
                  guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4],
                  guid.Data4[5], guid.Data4[6], guid.Data4[7]);
  }
  return flag;
}
#elif defined(__linux__)
static std::string generate() {
  uuid_t uu = {0};
  std::string flag;
  uuid_generate(uu);
  const size_t UUIDLEN = 36;
  /// 36-byte string (plus trailing '\0')
  flag.reserve(UUIDLEN + 1);
  flag.resize(UUIDLEN);
  uuid_unparse_upper(uu, const_cast<char *>(flag.data()));
  return flag;
}
#else
namespace {
static uint64_t random_64bits() {
  static std::random_device rd;
  static std::mt19937_64 gen(rd());
  static std::uniform_int_distribution<uint64_t> dis(0ULL);
  return static_cast<uint64_t>(dis(gen));
}
typedef union {
  uint64_t Data[2];
  struct {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
  } Uuid;
} UuidData;
}  // namespace

static std::string generate() {
  UuidData uuid = {random_64bits(), random_64bits()};
  std::string flag;
#if _MSC_VER
#define snprintf _snprintf
#endif
  auto len =
      snprintf(nullptr, 0, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
               uuid.Uuid.Data1, uuid.Uuid.Data2, uuid.Uuid.Data3,
               uuid.Uuid.Data4[0], uuid.Uuid.Data4[1], uuid.Uuid.Data4[2],
               uuid.Uuid.Data4[3], uuid.Uuid.Data4[4], uuid.Uuid.Data4[5],
               uuid.Uuid.Data4[6], uuid.Uuid.Data4[7]);
  flag.resize(len + 1);
  auto len_2 = snprintf(
      const_cast<char *>(flag.data()), len,
      "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", uuid.Uuid.Data1,
      uuid.Uuid.Data2, uuid.Uuid.Data3, uuid.Uuid.Data4[0], uuid.Uuid.Data4[1],
      uuid.Uuid.Data4[2], uuid.Uuid.Data4[3], uuid.Uuid.Data4[4],
      uuid.Uuid.Data4[5], uuid.Uuid.Data4[6], uuid.Uuid.Data4[7]);
  flag.resize(len_2);
#if _MSC_VER
#undef snprintf
#endif

  return flag;
}

#endif
}  // namespace uuid

}  // namespace tools
}  // namespace assistant
#endif  // _TOOLS_UUID_H__
