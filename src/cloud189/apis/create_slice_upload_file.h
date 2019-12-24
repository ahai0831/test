/**
 * @file create_slice_upload_file.h
 * @date 2019-11-01
 * @copyright Copyright (c) 2019
 */
#pragma once
#ifndef CREATE_SLICE_UPLOAD_FILE_H
#define CREATE_SLICE_UPLOAD_FILE_H

#include <cinttypes>

#include <http_primitives.h>

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace CreateSliceUploadFile {

// 只做json字符串拼接，用StringFormat
// json字符串包含的字段应与这个函数声明完全一致
//
// localPath, [string],
// [表明源文件的本地全路径，外部传入用于解析fileName和size]
// parentFolderId,[string],
// [表明父文件夹Id]
// md5, [string],
// [表明上传文件的md5]
// x_request_id,[string],
// [表明x_request_id]
// isLog, [int32_t],
// [客户端日志上传标识,
// 1–客户端行为日志文件上传至指定账户
// 0-不是日志上报]
// opertype, [int32_t],
// [表明上传后操作方式,
// 1-遇到相同文件名(只检查文件名)，执行重命名操作。
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件]
std::string JsonStringHelper(const std::string& localPath,
                             const std::string& parentFolderId,
                             const std::string& md5,
                             const std::string& x_request_id,
                             const int32_t isLog, const int32_t opertype);

// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：POST
// header规定：Content—Type: application/xml; charset=utf-8
//
// 需要放到url中的参数：
//
// clientType, [string],
// [json字符串不用传, 表明客户端类型，PC端值固定位TELEPC]
// version, [string],
// [json字符串不用传, 表明客户端版本，调用ProcessVersion方法获取]
// rand, [string],
// [json字符串不用传,随机数,现固定为4位随机数加下划线加8位随机数（1234_12345678）]
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
// parentFolderId,[string],
// [从json字符串中作为string解析,表明父文件夹Id]
// fileName, [string],
// [json字符串不用传，从json字符串中的local_path中提取，表明创建的上传文件名称]
// md5, [string],
// [从json字符串中作为string解析,表明上传文件的md5]
// size,[int64_t],
// [json字符串不用传，从json字符串中的local_path中提取，表明上传文件大小（字节）]
// sliceSize,[int64_t],
// [json字符串不用传，从json字符串中的local_path中提取size并计算，表明分片的大小，为3M的倍数]
// isLog, [int32_t],
// [从json字符串中作为int32_t解析，客户端日志上传标识,
// 1–客户端行为日志文件上传至指定账户
// 0-不是日志上报]
// opertype, [int32_t],
// [表明上传后操作方式,从json字符串中作为int32_t解析,
// 1-遇到相同文件名(只检查文件名)，执行重命名操作。
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件]

// totalSlice,[int64_t],
// [总分片号（从1开始，递增，最大不要超过9000），会通过文件大小计算，暂时json字符串中不用传，也不作为请求参数]

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace CreateSliceUploadFile
}  // namespace Apis
}  // namespace Cloud189

#endif  // CREATE_SLICE_UPLOAD_FILE_H
