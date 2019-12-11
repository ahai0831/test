/**
 * @file get_slice_upload_status.h
 * @date 2019-11-01
 * @copyright Copyright (c) 2019
 */

#pragma once
#ifndef GET_SLICE_UPLOAD_STATUS_H
#define GET_SLICE_UPLOAD_STATUS_H

#include <cinttypes>

#include <Assistant_v3.hpp>

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace GetSliceUploadStatus {

// json字符串包含的字段应于这个函数声明完全一致
// uploadFileId,[string],
// [表明上传文件Id]
// x_request_id,[string],
// [表明x_request_id]
std::string JsonStringHelper(const std::string& uploadFileId,
                             const std::string& x_request_id);

// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：GET
//
// 需要放到url中的参数：
// uploadFileId,[string],
// version, [string],
// [json字符串不用传, 参数放到url中, 表明客户端版本，调用ProcessVersion方法获取]
// rand, [string],
// [json字符串不用传, 参数放到url中,
// 随机数，现固定为4位随机数加下划线加8位随机数（1234_12345678）]
// clientType, [string],
// [json字符串不用传，参数放到url中, 表明客户端类型，PC端值固定位TELEPC]
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

}  // namespace GetSliceUploadStatus
}  // namespace Apis
}  // namespace Cloud189

#endif  // GET_SLICE_UPLOAD_STATUS_H
