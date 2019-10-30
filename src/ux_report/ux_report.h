/**
 * @file ux_report.h
 * @date 2019-10-30
 * @copyright Copyright (c) 2019
 */
#pragma once
#ifndef UX_REPORT_H
#define UX_REPORT_H

#include <cinttypes>

#include <http_primitives.h>

// 云涛上报请求命名空间
namespace UxReport {
// json字符串包含的字段应于这个函数声明完全一致
// data, [string],
// [表明要上传的日志数据（加密后的）]

std::string JsonStringHelper(const std::string& data);

// 请求方式：POST
// header规定：Content—Type: application/x-www-form-urlencoded
// SdkVersion, [string],
// [表明PC云涛SDK的版本号，参数放到header中,调用ProcessVersion方法获取]
// OsType, [string],
// [表明系统类型，参数放到header中,PC端值为5]
// TODO(tiany):host and uri
// X-Request-ID, [string],
// [json字符串不用传, 参数放到header中，调用assitant的UUID模块获取]
// data, [string],
// [从json字符串中作为string解析,表明要上传的日志数据（加密后的）]

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);
}  // namespace UxReport
#endif  // UX_REPORT_H
