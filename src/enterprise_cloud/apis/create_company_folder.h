﻿/**
 * @file create_company_folder.h
 * @date 2019-11-04
 * @copyright Copyright (c) 2019
 */
#pragma once
#ifndef CREATE_COMPANY_FOLDER_H
#define CREATE_COMPANY_FOLDER_H

#include <cinttypes>

#include <http_primitives.h>

// 企业云的顶层命名空间
namespace EnterpriseCloud {
namespace Apis {
namespace CreateCompanyFolder {

// 只做json字符串拼接，用StringFormat
// json字符串包含的字段应与这个函数声明完全一致
//
// corpId,[int64_t],
// [表明企业Id]
// parentId,[int64_t],
// [表明父目录ID]
// fileName, [string],
// [表明文件夹名称]
std::string JsonStringHelper(const int64_t corpId, const int64_t parentId,
                             const std::string& fileName);
// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：GET
//
// 需要放到url中的参数：
//
// corpId,[int64_t],
// [从json字符串中作为int64_t解析，表明企业Id]
// parentId,[int64_t],
// [从json字符串中作为int64_t解析，表明父目录ID]
// fileName, [string],
// [从json字符串中作为string解析，表明文件夹名称]
// clientType, [string],
// [json字符串不用传，参数放到url中, 表明客户端类型，PC端值固定位CORPPC]
// version, [string],
// [json字符串不用传, 参数放到url中, 表明客户端版本，调用ProcessVersion方法获取]
// rand, [string],
// [json字符串不用传, 参数放到url中,
// 随机数，现固定为4位随机数加下划线加8位随机数（1234_12345678）]
//
// 需要放到header中的参数：
// Date, [string], SessionKey, [string], Signature, [string],
// [这三个参数json字符串不用传，调用SessionHelper直接原子地设置这三个参数]
// X-Request-ID, [string],
// [json字符串不用传, 参数放到header中，调用assitant的UUID模块获取]
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace CreateCompanyFolder
}  // namespace Apis
}  // namespace EnterpriseCloud

#endif  // CREATE_COMPANY_FOLDER_H
