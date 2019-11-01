/**
 * @file get_file_download_url.h
 * @date 2019-10-31
 * @copyright Copyright (c) 2019
 */

#pragma once
#ifndef GET_FILE_DOWNLOAD_URL_H
#define GET_FILE_DOWNLOAD_URL_H

#include <cinttypes>

#include <http_primitives.h>

// 企业云的顶层命名空间
namespace EnterpriseCloud {
namespace Apis {
namespace GetFileDownloadUrl {

// json字符串包含的字段应于这个函数声明完全一致
// fileId,[int64_t],
// [表明文件Id]
// coshareId,[int64_t],
// [表明共享文件ID,dt为2时(协作空间内文件)为共享文件的ID，其余情况为空]
// corpId,[int64_t],
// [表明企业Id]
// dt, [int32_t],
// [表明下载类型，1表示企业空间文件,2表示协作文件夹内文件,3表示工作空间文件]
// isPreview ,[int32_t],
// [表明是否用于预览，1表示预览功能，0或空表示下载功能]
std::string JsonStringHelper(const int64_t fileId, const int64_t coshareId,
                             const int64_t corpId, const int32_t dt,
                             const int32_t flag);

// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：GET
//
// 需要放到url中的参数：
// fileId,[int64_t],
// [从json字符串中作为int64_t解析,表明文件Id]
// coshareId,[int64_t],
// [从json字符串中作为int64_t解析，表明共享文件ID,dt为2时(协作空间内文件)为共享文件的ID，其余情况为空]
// corpId,[int64_t],
// [从json字符串中作为int64_t解析,表明企业Id]
// dt, [int32_t],
// [从json字符串中作为int32_t解析，表明下载类型，1表示企业空间文件,2表示协作文件夹内文件,3表示工作空间文件]
// flag,[int32_t],
// [从json字符串中作为int32_t解析，表明是否用于预览，1表示预览功能，0或空表示下载功能]
// clientType, [string],
// [json字符串不用传, 表明客户端类型，PC端值固定位CORPPC]
// version, [string],
// [json字符串不用传, 表明客户端版本，调用ProcessVersion方法获取]
// rand, [string]
// [json字符串不用传,
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

}  // namespace GetFileDownloadUrl
}  // namespace Apis
}  // namespace EnterpriseCloud

#endif  // GET_FILE_DOWNLOAD_URL_H
