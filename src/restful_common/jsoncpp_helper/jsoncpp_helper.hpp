/**
 * @brief restful_common::jsoncpp_helper .hpp
 * @date 2019-10-22
 *
 * @copyright Copyright 2019
 *
 */
#pragma once
#ifndef JSONCPP_HELPER_JSONCPP_HELPER_H__
#define JSONCPP_HELPER_JSONCPP_HELPER_H__
#include <cinttypes>

#include <json\json.h>
namespace restful_common {
namespace jsoncpp_helper {
// get string
inline std::string GetString(const Json::Value &value) {
  return value.isString() ? value.asString() : std::string("");
}

// get bool
inline bool GetBool(const Json::Value &value) {
  return value.isBool() ? value.asBool() : false;
}

// get int
inline int GetInt(const Json::Value &value) {
  return value.isInt() ? value.asInt() : 0;
}

// get int64
inline int64_t GetInt64(const Json::Value &value) {
  return value.isInt64() ? value.asInt64() : 0;
}

// get uint64
inline uint64_t GetUint64(const Json::Value &value) {
  return value.isUInt64() ? value.asUInt64() : 0;
}

// get uint
inline uint32_t GetUint(const Json::Value &value) {
  return value.isUInt() ? value.asUInt() : 0;
}

// get double
inline double GetDouble(const Json::Value &value) {
  return value.isDouble() ? value.asDouble() : 0;
}

}  // namespace jsoncpp_helper
}  // namespace restful_common
#endif  // JSONCPP_HELPER_JSONCPP_HELPER_H__
