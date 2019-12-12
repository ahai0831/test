/**
 * @file comfirm_upload_file_complete.h
 * @date 2019-10-29
 * @copyright Copyright (c) 2019
 */
#pragma once
#ifndef COMFIRM_UPLOAD_FILE_COMPLETE_H
#define COMFIRM_UPLOAD_FILE_COMPLETE_H

#include <cinttypes>

#include <Assistant_v3.hpp>
// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace ComfirmUploadFileComplete {

// json字符串包含的字段应于这个函数声明完全一致
// fileCommitUrl,[string],
// [确认文件上传完成URL]
// uploadFileId,[string],
// [表明断点续传文件Id]
// x_request_id,[string],
// [表明x_request_id]
// opertype, [int32_t],
// [表明上传后操作方式,
// 1-遇到相同文件名(只检查文件名)，执行重命名操作。
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件]
// isLog, [int32_t],
// [客户端日志上传标识，
// 1–客户端日志文件上传至指定账户
// 0-非客户端日志文件上传]
std::string JsonStringHelper(const std::string& fileCommitUrl,
                             const std::string& uploadFileId,
                             const std::string& x_request_id,
                             const int32_t opertype, const int32_t isLog);

// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：POST
// header规定:Content—Type: application/x-www-form-urlencoded
//
// 需要放到url中的参数：
//
// clientType, [string],
// [json字符串不用传，参数放到url中, 表明客户端类型，PC端值固定位TELEPC]
// version, [string],
// [json字符串不用传, 参数放到url中, 表明客户端版本，调用ProcessVersion方法获取]
// channelId, [string],
// [json字符串不用传, 参数放到url中, 表明渠道id，值固定为web_cloud.189.cn]
// rand, [string]
// [json字符串不用传, 参数放到url中,
// 随机数，现固定为4位随机数加下划线加8位随机数（1234_12345678）]
//
// 需要放到header中的参数：
//
// Date, [string], SessionKey, [string], Signature, [string],
// [这三个参数json字符串不用传，调用SessionHelper直接原子地设置这三个参数]
// X-Request-ID, [string],
// [json字符串不用传, 参数放到header中，调用assitant的UUID模块获取]
//
// 需要放到body中的参数：
//
// uploadFileId,[string],
// [从json字符串中作为int64_t解析，参数放到body中，表明上传文件Id]
// opertype, [int32_t],
// [上传后操作方式
// 1-遇到相同文件名(只检查文件名)，执行重命名操作
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件
// isLog, [int32_t],
// [从json字符串中作为int32_t解析，参数放到body中，客户端日志上传标识，
// 1–客户端日志文件上传至指定账户
// 0-非客户端日志文件上传]
// ResumePolicy, [int32_t],
// [json字符串不用传，表明是否支持断点续传控制，1表示支持,0表示不支持]
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace ComfirmUploadFileComplete
}  // namespace Apis
}  // namespace Cloud189

#endif  // COMFIRM_UPLOAD_FILE_COMPLETE_H
