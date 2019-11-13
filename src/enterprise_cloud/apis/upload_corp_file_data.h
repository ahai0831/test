/**
 * @file upload_corp_file_data.h
 * @date 2019-10-30
 * @copyright Copyright (c) 2019
 *
 */
#pragma once
#ifndef UPLOAD_CORP_FILE_DATA_H
#define UPLOAD_CORP_FILE_DATA_H

#include <cinttypes>

#include <Assistant_v3.hpp>

// 企业云的顶层命名空间

namespace EnterpriseCloud {
namespace Apis {
namespace UploadFileData {

// json字符串包含的字段应与这个函数声明完全一致
// fileUploadUrl,[string],
// [文件上传URL]
// localPath, [string],
// [表明源文件的本地全路径]
// uploadFileId,[int64_t],
// [表明上传文件Id]
// corpId,[int64_t],
// [表明企业Id]
// fileSource, [int32_t],
// [表明文件来源,1-企业空间文件,2-协作空间文件,3-工作空间文件]
// startOffset,[int64_t],
// [表明续传文件的起始偏移，0为从头开始]
// offsetLength,[int64_t],
// [表明续传文件的偏移长度，-1为从起始位置到最后位置]
// isLog, [int32_t],
// [客户端日志上传标识，
// 1–客户端日志文件上传至指定账户
// 0-非客户端日志文件上传]
std::string JsonStringHelper(const std::string& fileUploadUrl,
                             const std::string& localPath,
                             const int64_t uploadFileId, const int64_t corpId,
                             const int32_t fileSource,
                             const int64_t startOffset,
                             const int64_t offsetLength, const int32_t isLog);

// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：PUT
// header规定:Content—Type: application/octet-stream
//
// 需要放到url中的参数：
// clientType, [string],
// [json字符串不用传，参数放到url中, 表明客户端类型，PC端值固定位CORPPC]
// version, [string],
// [json字符串不用传, 参数放到url中, 表明客户端版本，调用ProcessVersion方法获取]
// rand, [string],
// [json字符串不用传, 参数放到url中,
// 随机数，现固定为4位随机数加下划线加8位随机数（1234_12345678）]
//
// 需要放到header中的参数：
// corpId,[int64_t],
// [从json字符串中作为int64_t解析，表明企业Id]
// UploadFileId,[int64_t],
// [从json字符串中作为int64_t解析，参数放到header中，表明断点续传文件Id]
// UploadFileRange, [string],
// [从json字符串中解析startOffset和offsetLength,计算断点续传文件的上传范围，拼接字符串bytes=起始偏移-最后位置，表明断点续传文件上传范围]
// fileSize, [int64_t],
// [json字符串不用传，从json字符串中的local_path中提取，表明上传文件大小（字节）]
// fileSource, [int32_t],
// [从json字符串中作为int32_t解析,表明文件来源,1-企业空间文件,2-协作空间文件,3-工作空间文件]
// isLog, [int32_t],
// [从json字符串中作为int32_t解析,表明客户端日志上传标识，1–客户端日志文件上传至指定账户,
// 0-非客户端日志文件上传]
// Expect, [string],
// [json字符串不用传，参数为100-continue表示服务器已接收到请求标头，并且客户端继续发送请求正文]
// Content-Length,[int64_t],
// [body的传输长度，可根据数据内容生成]
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

}  // namespace UploadFileData
}  // namespace Apis
}  // namespace EnterpriseCloud

#endif  // UPLOAD_CORP_FILE_DATA_H
