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
#include <memory>

#include <json/json.h>
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

// 规范读取，使用严格模式且原地解析
inline bool ReaderHelper(const std::string &json_info,
                         Json::Value &json_value) {
  bool flag = false;
  do {
    if (json_info.empty()) {
      break;
    }
    Json::CharReaderBuilder reader_builder;
    Json::CharReaderBuilder::strictMode(&reader_builder.settings_);
    std::unique_ptr<Json::CharReader> const reader_ptr(
        reader_builder.newCharReader());
    if (nullptr == reader_ptr) {
      break;
    }
    if (!reader_ptr->parse(json_info.c_str(),
                           json_info.c_str() + json_info.size(), &json_value,
                           nullptr)) {
      break;
    }
    flag = true;
  } while (false);
  return flag;
}

// 规范写入，无格式
inline std::string WriterHelper(const Json::Value &json_value) {
  std::string json_info;
  Json::StreamWriterBuilder wbuilder;
  wbuilder.settings_["indentation"] = "";
  json_info = Json::writeString(wbuilder, json_value);
  return json_info;
}

inline void WriterHelper(const Json::Value &json_value,
                         std::string &json_info) {
  json_info = WriterHelper(json_value);
}

}  // namespace jsoncpp_helper
}  // namespace restful_common
#endif  // JSONCPP_HELPER_JSONCPP_HELPER_H__
