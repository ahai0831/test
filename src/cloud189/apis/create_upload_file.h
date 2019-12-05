/**
 * @file create_upload_file.h
 * @date 2019-10-21
 * @copyright Copyright (c) 2019
 */
#pragma once
#ifndef CREATE_UPLOAD_FILE_H
#define CREATE_UPLOAD_FILE_H

#include <cinttypes>

#include <Assistant_v3.hpp>

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
// 这是用于创建文件上传请求，而每个请求需要的参数都不同，有些参数需要，有些不需要，
// 所以在设计其他请求接口时，这需要结合抓包和文档来看，而且要针对普通文件上传，
// 日志上传，自动备份来分开梳理，大体以抓包为准。。。
// 设置参数时，注意HTTP请求方式GET, POST, PUT的不同，注意参数的位置。
// 在参考文档说明时，注意不要以文档的类型为准，比如long，在实际使用时一般改为int64_t，
// 如果取值比较小，那么就是int32_t
namespace CreateUploadFile {

// 只做json字符串拼接，用StringFormat
// json字符串包含的字段与这个函数声明完全一致
// parent_folder_id,[int64_t],
// [表明上传文件的父文件夹id]
// local_path, [string],
// [表明源文件的本地全路径]
// md5, [string],
// [表明上传文件的md5]
// oper_type, [int32_t],
// [表明上传后操作方式,
// 1-遇到相同文件名(只检查文件名)，执行重命名操作。
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件]
// is_log, [int32_t],
// [客户端日志上传标识，
// 1–客户端日志文件上传至指定账户
// 0-非客户端日志文件上传]
//
// 对于自动备份，需要把oper_type设置为3，is_log设置为0
// 对于日志上传，需要把oper_type设置为1，is_log设置为1，parent_folder_id传-11
// 对于文件上传，需要把oper_type设置为1，is_log设置为0
std::string JsonStringHelper(const int64_t parent_folder_id,
                             const std::string& local_path,
                             const std::string& md5, const int32_t oper_type,
                             const int32_t is_log);
// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
// 请求方式：POST
// header规定：Content—Type: application/x-www-form-urlencoded
// 请求需要的参数放到body
// parentFolderId,[int64_t],
// [从json字符串中作为int64_t解析，参数放到body中，表明上传文件的父文件夹id]
// baseFileId, [int64_t],
// [json字符串中不用传，参数放到body中，值为空]
// fileName, [string],
// [json字符串不用传，参数放到body中,
// 从json字符串中的local_path中提取，参数放到body中，表明创建的上传名字名称]
// size, [int64_t],
// [json字符串不用传，参数放到body中,
// 通过从json字符串中的local_path对应的文件路径来获取本地文件大小，表明上传文件大小（字节）]
// md5, [string],
// [从json字符串中作为string解析，参数放到body中，表明上传文件的md5]
// lastWrite, [string],
// [json字符串不用传，参数放到body中,通过从json字符串中的local_path对应的文件路径来获取本地文件最后修改时间，表明源文件最后修改时间（2019-09-27
// 08:41:34.846）]
// localPath, [string],
// [从json字符串中作为string解析，参数放到body中，表明源文件的本地全路径]
// opertype, [int32_t],
// [从json字符串中作为int32_t解析，参数放到body中，表明上传后操作方式,
// 1-遇到相同文件名(只检查文件名)，执行重命名操作。
// 3-遇到相同文件名（只检查文件名），执行覆盖原文件]
// flag, [int32_t],
// [json字符串不用传, 参数放到body中,flag的值固定为1，用于信安检查异常标识]
// resumePolicy, [int32_t],
// [json字符串不用传, 参数放到body中,表明是否支持断点续传控制，1表示支持,
// 0表示不支持]
// isLog, [int32_t],
// [从json字符串中作为int32_t解析，参数放到body中，客户端日志上传标识，
// 1–客户端日志文件上传至指定账户
// 0-非客户端日志文件上传]
// clientType, [string],
// [json字符串不用传，参数放到url中, 表明客户端类型，PC端值固定位TELEPC]
// version, [string],
// [json字符串不用传, 参数放到url中, 表明客户端版本，需要调用函数获取]
// channelId, [string],
// [json字符串不用传, 参数放到url中, 表明渠道id，值固定为web_cloud.189.cn]
// rand, [string]
// [json字符串不用传, 参数放到url中,
// 随机数，现固定为4位随机数加下划线加8位随机数（1234_12345678）] SessionKey,
// [string], Signature, [string], Date, [string],
// [这三个参数json字符串不用传，参数放到header中，调用SessionHelper直接原子地设置这三个参数]
// X-Request-ID, [string],
// [json字符串不用传, 参数放到header中，调用assitant的UUID模块获取]
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace CreateUploadFile
}  // namespace Apis
}  // namespace Cloud189

#endif  // CREATE_UPLOAD_FILE_H
