/**
 * @file file_data_download.h
 * @date 2020-02-21
 * @copyright Copyright (c) 2020
 */
#pragma once
#ifndef FILEDATA_DOWNLOAD_H
#define FILEDATA_DOWNLOAD_H

#include <cinttypes>

#include "http_primitives.h"

// 天翼云的顶层命名空间（包括个人云和家庭云）
namespace Cloud189 {
namespace Apis {
namespace FileDataDownload {
	std::string JsonStringHelper(const std::string downloadUrl,
		const std::string x_request_id);

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request);

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info);

}  // namespace FileDataDownload
}  // namespace Apis
}  // namespace Cloud189

#endif  // FILEDATA_DOWNLOAD_H
