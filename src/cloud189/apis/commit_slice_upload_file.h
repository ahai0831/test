/**
 * @file commit_slice_upload_file.h
 * @date 2019-11-01
 * @copyright Copyright (c) 2019
 */
#pragma once
#ifndef COMMIT_SLICE_UPLOAD_FILE_H
#define COMMIT_SLICE_UPLOAD_FILE_H

#include <cinttypes>

#include <Assistant_v3.hpp>

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace CommitSliceUploadFile {

// json字符串包含的字段应于这个函数声明完全一致
// fileCommitUrl,[string],
// [确认文件上传完成URL]
// uploadFileId,[string],
// [表明上传文件Id]
// isLog, [int32_t],
// [客户端日志上传标识,
// 1–客户端行为日志文件上传至指定账户
// 0-不是日志上报]
// opertype, [int32_t],
// [表明上传后操作方式,
// 1-遇到相同文件名(只检查文件名)，执行重命名操作。
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件]
// resumePolicy,[int32_t],
// [表明是否支持断点续传控制，1-断点续传控制策略版本1，空-不支持]
// sliceMD5, [string],
// [表明第1、2、3、4、5等片的MD5大写（换行）为MD5List，并用Signature=hmac_sha1(MD5List,
// SessionSecret) 签名，再计算 Signature 的MD5]
std::string JsonStringHelper(const std::string& fileCommitUrl,
                             const std::string& uploadFileId,
                             const int32_t isLog, const int32_t opertype,
                             const int32_t resumePolicy,
                             const std::string& sliceMD5);

// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：POST
// header规定:Content—Type: text/xml; charset=UTF-8
//
// 需要放到url中的参数：
//
// clientType, [string],
// [json字符串不用传，参数放到url中, 表明客户端类型，PC端值固定位TELEPC]
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
// uploadFileId,[string],
// [从json字符串中作为string解析，参数放到body中，表明上传文件Id]
// isLog, [int32_t],
// [从json字符串中作为int32_t解析，客户端日志上传标识,
// 1–客户端行为日志文件上传至指定账户
// 0-不是日志上报]
// opertype, [int32_t],
// [表明上传后操作方式,从json字符串中作为int32_t解析,
// 1-遇到相同文件名(只检查文件名)，执行重命名操作。
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件]
// resumePolicy,[int32_t],
// [从json字符串中作为int64_t解析，参数放到body中，表明是否支持断点续传控制，1-断点续传控制策略版本1，空-不支持]
// sliceMD5, [string],
// [从json字符串中作为string解析，参数放到body中，表明第1、2、3、4、5等片的MD5大写（换行）为MD5List，并用Signature=hmac_sha1(MD5List,
// SessionSecret) 签名，再计算 Signature 的MD5]
//
// albumId,[int64_t],
// [表明上传图片至云相册，目标相册id,
// 传此参数后无需再调用上传图片关联相册接口photo/relatedToAlbum.action，暂时json字符串中不用传，也不作为请求参数]
// access_token, [string],
// [表明合作应用token，暂时json字符串中不用传，也不作为请求参数]
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace CommitSliceUploadFile
}  // namespace Apis
}  // namespace Cloud189

#endif  // COMMIT_SLICE_UPLOAD_FILE_H
