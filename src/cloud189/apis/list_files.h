/**
 * @file list_files.h
 * @date 2020-03-10
 * @copyright Copyright (c) 2020
 */
#pragma once
#ifndef LIST_FILES_H
#define LIST_FILES_H

#include <cinttypes>

#include <http_primitives.h>

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace ListFiles {
std::string JsonStringHelper(const std::string folderId, int32_t recursive,
                             int32_t fileType, int32_t mediaType,
                             int32_t mediaAttr, int32_t iconOption,
                             const std::string orderBy, bool descending,
                             int64_t pageNum, int64_t pageSize,
                             const std::string x_request_id);

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace ListFiles
}  // namespace Apis
}  // namespace Cloud189

#endif  // LIST_FILES_H
