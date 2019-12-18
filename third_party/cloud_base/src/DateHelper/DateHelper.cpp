﻿#include "DateHelper.h"

#include <chrono>
#include <ctime>

namespace {
//用于获取相同数据源的时间信息，尽可能保证一致性
inline uint64_t timestamp_millisecond() {
  return std::chrono::time_point_cast<std::chrono::milliseconds>(
             std::chrono::high_resolution_clock::now())
      .time_since_epoch()
      .count();
}

inline std::chrono::high_resolution_clock::time_point timestamp_time_point() {
  return std::chrono::high_resolution_clock::now();
}
}  // namespace
namespace cloud_base {
namespace date_helper {
/// 获取秒级时间戳
uint64_t get_second_time_stamp() { return timestamp_millisecond() / 1000; }
/// 获取毫秒级时间戳
uint64_t get_millisecond_time_stamp() { return timestamp_millisecond(); }

/// 获取GMT时间戳，格式为"%a, %d %b %Y %H:%M:%S GMT"
std::string get_gmt_time_stamp() {
  auto gmt_origin = timestamp_time_point();
  auto gmt_t = std::chrono::high_resolution_clock::to_time_t(gmt_origin);
  auto gmt = gmtime(&gmt_t);
  char buff[64] = {0};
  auto origin_lc_time = setlocale(LC_TIME, "C");
  size_t len =
      strftime(buff, sizeof(buff) - 1, "%a, %d %b %Y %H:%M:%S GMT", gmt);
  setlocale(LC_TIME, origin_lc_time);
  return std::string(buff);
}

/// 获取当前时间戳，格式为"%Y-%m-%d %H:%M:%S.xxx"
std::string get_time_stamp() {
  auto local_origin = timestamp_time_point();
  auto local_t = std::chrono::high_resolution_clock::to_time_t(local_origin);
  auto local = localtime(&local_t);

  char buff[32] = {0};
  size_t len = strftime(buff, sizeof(buff) - 1, "%Y-%m-%d %H:%M:%S.", local);
  std::string str_buff = buff;
  int32_t m_seconds = static_cast<int32_t>(
      std::chrono::time_point_cast<std::chrono::milliseconds>(local_origin)
          .time_since_epoch()
          .count() %
      1000);
  //毫秒结果保留三位，不足三位的左补0
  char m_seconds_temp[4] = {'\0'};
  sprintf(m_seconds_temp, "%03d", m_seconds);
  str_buff.append(m_seconds_temp);
  return str_buff;
}

}  // namespace date_helper
}  // namespace cloud_base
