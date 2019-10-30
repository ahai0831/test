#include "ux_report.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <cinttypes>

#include <json/json.h>

#include <tools/string_format.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

namespace UxReport {

bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {

  // TODO(tiany): base64

  // TODO(tiany): get host

  // TODO(tiany): get uri

  // TODO(tiany): set request header and body

  //返回 request
}

}  // namespace UxReport
