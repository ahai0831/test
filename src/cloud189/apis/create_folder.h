/**
 * @file create_folder.h
 * @date 2020-02-11
 * @copyright Copyright (c) 2020
 */
#pragma once
#ifndef CREATE_FOLDER_H
#define CREATE_FOLDER_H

#include <cinttypes>

#include <http_primitives.h>

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace CreateFolder {
std::string JsonStringHelper(const std::string parentFolderId,
                             const std::string relativePath,
                             const std::string folderName,
                             const std::string x_request_id);

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace CreateFolder
}  // namespace Apis
}  // namespace Cloud189

#endif  // GET_FOLDER_INFO_H
