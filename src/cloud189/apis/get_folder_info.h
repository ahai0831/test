/**
 * @file get_folder_info.h
 * @date 2020-02-11
 * @copyright Copyright (c) 2020
 */
#pragma once
#ifndef GET_FOLDER_INFO_H
#define GET_FOLDER_INFO_H

#include <cinttypes>

#include <http_primitives.h>

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace GetFolderInfo {
std::string JsonStringHelper(const std::string folderId,
                             const std::string folderPath,
                             const int32_t pathList, const int32_t dt,
                             const std::string shareId,
                             const std::string groupSpaceId,
                             const std::string x_request_id);

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace GetFolderInfo
}  // namespace Apis
}  // namespace Cloud189

#endif  // GET_FOLDER_INFO_H
