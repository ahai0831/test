/**
 * @file get_download_address.h
 * @date 2020-02-21
 * @copyright Copyright (c) 2020
 */
#pragma once
#ifndef DOWNLOAD_ADDRESS_H
#define DOWNLOAD_ADDRESS_H

#include <cinttypes>

#include "http_primitives.h"

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace GetDownloadAddress {
std::string JsonStringHelper(const std::string fileId, int32_t dt,
                             const std::string shareId,
                             const std::string groupSpaceId, bool shorts,
                             const std::string x_request_id);

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace GetDownloadAddress
}  // namespace Apis
}  // namespace Cloud189

#endif  // DOWNLOAD_ADDRESS_H
