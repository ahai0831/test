/**
 * @file get_corp_upload_status.h
 * @date 2019-10-30
 * @copyright Copyright (c) 2019
 */

#pragma once
#ifndef GET_CORP_UPLOAD_STATUS_H
#define GET_CORP_UPLOAD_STATUS_H

#include <cinttypes>

#include <http_primitives.h>

// 企业云的顶层命名空间
namespace EnterpriseCloud {
namespace Apis {
namespace GetUploadFileStatus {

// json字符串包含的字段应于这个函数声明完全一致
// uploadFileId,[int64_t],
// [表明上传文件Id]
// corpId,[int64_t],
// [表明企业Id]
// isLog, [int32_t],
// [客户端日志上传标识，
// 1–客户端日志文件上传至指定账户
// 0-非客户端日志文件上传]

std::string JsonStringHelper(const int64_t uploadFileId, const int64_t corpId,
                             const int32_t isLog);

// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：GET
//
// 需要放到url中的参数：
// uploadFileId,[int64_t],
// [从json字符串中作为int64_t解析，表明上传文件Id]
// corpId,[int64_t],
// [从json字符串中作为int64_t解析，表明企业Id]
// isLog, [int32_t],
// [从json字符串中作为int32_t解析,表明客户端日志上传标识，1–客户端日志文件上传至指定账户,
// 0-非客户端日志文件上传] clientType, [string],
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

// workRootFolderId,[int64_t],
// [表明文件工作空间根目录，暂时json字符串中不用传，也不作为请求参数]
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace GetUploadFileStatus
}  // namespace Apis
}  // namespace EnterpriseCloud

#endif  // GET_CORP_UPLOAD_STATUS_H
