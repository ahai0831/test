/**
 * @file get_folder_info.h
 * @date 2019-11-04
 * @copyright Copyright (c) 2019
 */

#pragma once
#ifndef GET_FOLDER_INFO_H
#define GET_FOLDER_INFO_H

#include <cinttypes>

#include <Assistant_v3.hpp>

// 企业云的顶层命名空间
namespace EnterpriseCloud {
namespace Apis {
namespace GetFolderInfo {

// json字符串包含的字段应于这个函数声明完全一致
// corpId,[int64_t],
// [表明企业Id]
// folderPath, [string],
// [表明文件夹路径，以/分隔，格式如/a/b/c，路径开头不用添加“工作空间“、“协作空间”、“企业空间”等名称]
// fileSource, [int32_t],
// [表明文件来源,1-企业空间文件,2-协作空间文件,3-工作空间文件]
// coshareId, [string],
// [表明共享ID,若为协作空间文件夹，则coshareId是必填，其他情况为空]

std::string JsonStringHelper(const int64_t corpId,
                             const std::string& folderPath,
                             const int32_t fileSource,
                             const std::string& coshareId);
// jsoncpp parse 原地解析
// jsoncpp reader 严格模式
//
// 请求方式：GET
//
// 需要放到url中的参数：
//
// corpId,[int64_t],
// [从json字符串中作为int64_t解析，表明企业Id]
// folderPath, [string],
// [从json字符串中作为string解析，文件夹路径，以/分隔，格式如/a/b/c，路径开头不用添加“工作空间“、“协作空间”、“企业空间”等名称]
// fileSource, [int32_t],
// [从json字符串中作为int32_t解析,表明文件来源,1-企业空间文件,2-协作空间文件,3-工作空间文件]
// coshareId, [string],
// [从json字符串中作为string解析,表明共享ID,若为协作空间文件夹，则coshareId是必填，其他情况为空]
// clientType, [string],
// [json字符串不用传，参数放到url中, 表明客户端类型，PC端值固定位CORPPC]
// version, [string],
// [json字符串不用传, 参数放到url中, 表明客户端版本，调用ProcessVersion方法获取]
// rand, [string],
// [json字符串不用传, 参数放到url中,
// 随机数，现固定为4位随机数加下划线加8位随机数（1234_12345678）]
//
// fileId,[int64_t],
// [表明文件夹ID,非必传,实际请求时为空,暂时json字符串中不用传,也不作为请求参数]
// parentId,[int64_t],
// [表明父目录ID,为空时查询根目录文件,非必传]
//
// 需要放到header中的参数：
// Date, [string], SessionKey, [string], Signature, [string],
// [这三个参数json字符串不用传，调用SessionHelper直接原子地设置这三个参数]
// X-Request-ID, [string],
// [json字符串不用传, 参数放到header中，调用assitant的UUID模块获取]
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace GetFolderInfo
}  // namespace Apis
}  // namespace EnterpriseCloud

#endif  // GET_FOLDER_INFO_H
