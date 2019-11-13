/**
 * @file comfirm_corp_upload_file_complete.h
 * @date 2019-10-30
 * @copyright Copyright (c) 2019
 */
#pragma once
#ifndef COMFIRM_CORP_UPLOAD_FILE_COMPLETE_H
#define COMFIRM_CORP_UPLOAD_FILE_COMPLETE_H

#include <cinttypes>

#include <Assistant_v3.hpp>

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace EnterpriseCloud {
namespace Apis {
namespace ComfirmUploadFileComplete {

// json字符串包含的字段应于这个函数声明完全一致
// fileCommitUrl,[string],
// [确认文件上传完成URL]
// uploadFileId,[int64_t],
// [表明上传文件Id]
// corpId,[int64_t],
// [表明企业Id]
// fileSource, [int32_t],
// [表明文件来源,1-企业空间文件,2-协作空间文件,3-工作空间文件]
// opertype, [int32_t],
// [表明上传后操作方式,
// 1-遇到相同文件名(只检查文件名)，执行重命名操作。
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件]
// coshareId,[int64_t],
// [表明共享ID,在工作空间中为空,在协作空间中为共享文件的ID]
// isLog, [int32_t],
// [客户端日志上传标识，
// 1–客户端日志文件上传至指定账户
// 0-非客户端日志文件上传]

std::string JsonStringHelper(const std::string& fileCommitUrl,
                             const int64_t uploadFileId, const int64_t corpId,
                             const int32_t fileSource, const int32_t opertype,
                             const std::string& coshareId, const int32_t isLog);

// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：POST
// header规定:Content—Type: application/x-www-form-urlencoded
//
// 需要放到url中的参数：
//
// clientType, [string],
// [json字符串不用传，参数放到url中, 表明客户端类型，PC端值固定位CORPPC]
// version, [string],
// [json字符串不用传, 参数放到url中, 表明客户端版本，调用ProcessVersion方法获取]
// rand, [string]
// [json字符串不用传, 参数放到url中,
// 随机数，现固定为4位随机数加下划线加8位随机数（1234_12345678）]
//
// 需要放到header中的参数：
//
// Date, [string], SessionKey, [string], Signature, [string],
// [这三个参数json字符串不用传,调用SessionHelper直接原子地设置这三个参数]
// X-Request-ID, [string],
// [json字符串不用传, 参数放到header中，调用assitant的UUID模块获取]
//
// 需要放到body中的参数：
//
// uploadFileId,[int64_t],
// [从json字符串中作为int64_t解析，参数放到body中，表明上传文件Id]
// corpId,[int64_t],
// [从json字符串中作为int64_t解析,表明企业Id]
// opertype, [int32_t],
// [从json字符串中作为int64_t解析,上传后操作方式
// 1-遇到相同文件名(只检查文件名)，执行重命名操作
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件
// fileSource, [int32_t],
// [从json字符串中作为int32_t解析,表明文件来源,1-企业空间文件,2-协作空间文件,3-工作空间文件]
// coshareId,[string],
// [从json字符串中作为string解析，表明共享ID,在工作空间中为空,在协作空间中为共享文件的ID]
// isLog, [int32_t],
// [从json字符串中作为int32_t解析，参数放到body中，客户端日志上传标识，
// 1–客户端日志文件上传至指定账户
// 0-非客户端日志文件上传]

// workRootFolderId,[int64_t],
// [表明文件工作空间根目录，暂时json字符串中不用传，也不作为请求参数]
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace ComfirmUploadFileComplete
}  // namespace Apis
}  // namespace EnterpriseCloud

#endif  // COMFIRM_CORP_UPLOAD_FILE_COMPLETE_H
