/**
 * @file create_corp_upload_file.h
 * @date 2019-10-30
 * @copyright Copyright (c) 2019
 *
 */
#pragma once
#ifndef CREATE_CORP_UPLOAD_FILE_H
#define CREATE_CORP_UPLOAD_FILE_H

#include <cinttypes>

#include <http_primitives.h>

// 企业云的顶层命名空间
namespace EnterpriseCloud {
namespace Apis {
namespace CreateUploadFile {

// 只做json字符串拼接，用StringFormat
// json字符串包含的字段应与这个函数声明完全一致
//
// localPath, [string],
// [表明源文件的本地全路径，外部传入用于解析fileName和fileSize]
// corpId,[int64_t],
// [表明企业Id]
// parentId,[int64_t],
// [表明父文件夹Id]
// md5, [string],
// [表明上传文件的md5]
// fileSource, [int32_t],
// [表明文件来源,1-企业空间文件,2-协作空间文件,3-工作空间文件]
// coshareId,[string],
// [表明共享ID,在工作空间中为空,在协作空间中为共享文件的ID]
// isLog, [int32_t],
// [客户端日志上传标识，
// 1–客户端日志文件上传至指定账户
// 0-非客户端日志文件上传]
//
// 对于文件上传，需要把fileSource设置为对应文件来源值，isLog设置为0
// 对于自动备份，需要把fileSource设置为3，isLog设置为0
// 对于日志上传，需要把fileSource设置为1，isLog设置为1

std::string JsonStringHelper(const std::string& localPath, int64_t corpId,
                             int64_t parentId, const std::string& md5,
                             int32_t fileSource, const std::string coshareId,
                             int32_t isLog);
// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：POST
// header规定：Content—Type: application/x-www-form-urlencoded
//
// 需要放到url中的参数：
//
// clientType, [string],
// [json字符串不用传, 表明客户端类型，PC端值固定位CORPPC]
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
// corpId,[int64_t],
// [从json字符串中作为int64_t解析,表明企业Id]
// parentId,[int64_t],
// [从json字符串中作为int64_t解析,表明父文件夹Id]
// baseFileId, [string]
// [json字符串不用传, 值为空]
// fileName, [string],
// [json字符串不用传，从json字符串中的local_path中提取，表明创建的上传文件名称]
// fileSize, [int64_t],
// [json字符串不用传，从json字符串中的local_path中提取，表明上传文件大小（字节）]
// md5, [string],
// [从json字符串中作为string解析,表明上传文件的md5]
// fileSource, [int32_t],
// [表明文件来源,1-企业空间文件,2-协作空间文件,3-工作空间文件]
// coshareId,[int64_t],
// [从json字符串中作为int64_t解析，表明共享ID,在工作空间中为空,在协作空间中为共享文件的ID]
// isLog, [int32_t],
// [从json字符串中作为int32_t解析，客户端日志上传标识,
// 1–客户端行为日志文件上传至指定账户
// 0-不是日志上报]

// workRootFolderId,[int64_t],
// [表明文件工作空间根目录，暂时json字符串中不用传，也不作为请求参数]
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace CreateUploadFile
}  // namespace Apis
}  // namespace EnterpriseCloud

#endif  // CREATE_CORP_UPLOAD_FILE_H
