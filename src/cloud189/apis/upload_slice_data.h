/**
 * @file upload_slice_data.h
 * @date 2019-11-01
 * @copyright Copyright (c) 2019
 */
#pragma once
#ifndef UPLOAD_SLICE_DATA_H
#define UPLOAD_SLICE_DATA_H

#include <cinttypes>

#include <http_primitives.h>

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace UploadSliceData {

// json字符串包含的字段应与这个函数声明完全一致
// fileUploadUrl,[string],
// [文件上传URL]
// localPath, [string],
// [表明源文件的本地全路径]
// uploadFileId,[string],
// [表明上传文件Id]
// x_request_id,[string],
// [表明x_request_id]
// startOffset,[int64_t],
// [表明续传文件的起始偏移，0为从头开始]
// offsetLength,[int64_t],
// [表明续传文件的偏移长度，-1为从起始位置到最后位置]
// UploadSliceId,[int64_t],
// [表明分片Id]
// MD5, [string],
// [表明分片的MD5值]
std::string JsonStringHelper(
    const std::string& fileUploadUrl, const std::string& localPath,
    const std::string& uploadFileId, const std::string& x_request_id,
    const int64_t startOffset, const int64_t offsetLength,
    const int64_t UploadSliceId, const std::string& MD5);

// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：PUT
// header规定:Content—Type:空
//
// 需要放到header中的参数：
// UploadFileId,[string],
// [从json字符串中作为string解析，参数放到header中，表明文件Id]
// UploadFileRange, [string],
// [从json字符串中解析startOffset和offsetLength,计算断点续传文件的上传范围，拼接字符串:起始偏移-最后位置/*，表明断点续传文件上传范围]
// resumePolicy,[int32_t],
// [从json字符串中作为int32_t解析，参数放到body中，表明是否支持断点续传控制，1-断点续传控制策略版本1，空-不支持]
// MD5, [string],
// [从json字符串中作为string解析，表明分片的MD5值]
// UploadSliceId,[int32_t],
// [从json字符串中作为int64_t解析，参数放到header中，表明分片Id]
// Date, [string], SessionKey, [string], Signature, [string],
// [这三个参数json字符串不用传，调用SessionHelper直接原子地设置这三个参数]
// X-Request-ID, [string],
// [json字符串不用传, 参数放到header中，调用assitant的UUID模块获取]
//
// IsPhtBckp,[int64_t],
// [表明相册备份打开标识)，暂时json字符串中不用传，也不作为请求参数]
// AccessToken, [string],
// [表明非空时会，取代sessionKey参数，暂时json字符串中不用传，也不作为请求参数]
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace UploadSliceData
}  // namespace Apis
}  // namespace Cloud189

#endif  // UPLOAD_SLICE_DATA_H
