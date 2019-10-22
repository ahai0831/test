#pragma once
#ifndef DATEHELPER_H
#define DATEHELPER_H

#include <string>
#include <cinttypes>

namespace cloud_base {
namespace date_helper {
/// 获取秒级时间戳
uint64_t get_second_time_stamp();
/// 获取毫秒级时间戳
uint64_t get_millisecond_time_stamp();
/// 获取GMT时间戳，格式为"%a, %d %b %Y %H:%M:%S GMT"
std::string get_gmt_time_stamp();
/// 获取当前时间戳，格式为"%Y-%m-%d %H:%M:%S.xxx"
std::string get_time_stamp();
}  // namespace date_helper
}  // namespace cloud_base
#endif  // DATEHELPER_H
