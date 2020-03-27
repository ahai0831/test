#include "list_files.h"

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>
#include <pugixml.hpp>

#include <process_common/process_common_helper.h>
#include <tools/string_format.hpp>

#include "cloud189/error_code/error_code.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
#include "restful_common/rand_helper/rand_helper.hpp"
#include "cloud189/params_helper/params_helper.hpp"


using Cloud189::ParamsHelper::GetClientType;
using Cloud189::ParamsHelper::GetChannelId;
using Cloud189::ParamsHelper::GetHost;
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetInt64;
using restful_common::jsoncpp_helper::GetString;
using cloud_base::process_common_helper::GetCurrentApplicationVersion;

namespace {
// 这些是请求中一些固定的参数
const static std::string uri = "/listFiles.action";
const static std::string method = "GET";
const static int flag = 1;

std::string GetURI() { return uri; }
std::string GetMethod() { return method; }

int GetFlag() { return flag; }

}  // namespace
namespace Cloud189 {
namespace Apis {
namespace ListFiles {

// 用于构建一个json字符串，包含查询文件上传需要的参数
std::string JsonStringHelper(const std::string folderId, int32_t recursive,
                             int32_t fileType, int32_t mediaType,
                             int32_t mediaAttr, int32_t iconOption,
                             const std::string orderBy, bool descending,
                             int64_t pageNum, int64_t pageSize,
                             const std::string x_request_id) {
  return assistant::tools::string::StringFormat(
	  R"({"folderId":"%s","recursive":%d,"fileType":%d,"mediaType":%d,"mediaAttr":%d,"iconOption":%d,"orderBy":"%s","descending":%d,"pageNum":)" "%" PRId64 "" R"(, "pageSize":)" "%" PRId64 "" R"(, "X-Request-ID": "%s"})",
      folderId.c_str(), recursive, fileType, mediaType, mediaAttr, iconOption,
      orderBy.c_str(), descending ? 1 : 0, pageNum, pageSize,
      x_request_id.c_str());
}

// 查询文件上传状态请求
bool HttpRequestEncode(const std::string& params_json,
                       assistant::HttpRequest& request) {
  bool is_ok = false;
  do {
    Json::Value json_str;
    if (!restful_common::jsoncpp_helper::ReaderHelper(params_json, json_str)) {
      break;
    }
    request.url = GetHost() + GetURI();
    request.method = GetMethod();

    // add session key, signature and date.
    Cloud189::SessionHelper::AddCloud189Signature(request);

    std::string folderId = GetString(json_str["folderId"]);
    int32_t recursive = GetInt(json_str["recursive"]);
    int32_t fileType = GetInt(json_str["fileType"]);
    int32_t mediaType = GetInt(json_str["mediaType"]);
    int32_t mediaAttr = GetInt(json_str["mediaAttr"]);
    int32_t iconOption = GetInt(json_str["iconOption"]);
    std::string orderBy = GetString(json_str["orderBy"]);
    int32_t descending = GetInt(json_str["descending"]);
    int64_t pageNum = GetInt64(json_str["pageNum"]);
    int64_t pageSize = GetInt64(json_str["pageSize"]);
    std::string x_request_id = GetString(json_str["X-Request-ID"]);

    //  set url params
    request.url += assistant::tools::string::StringFormat(
        "?folderId=%s&flag=%d&recursive=%d&fileType=%d&mediaType=%d&mediaAttr=%"
		    "d&iconOption=%d&orderBy=%s&descending=%d&pageNum=%" PRId64 "&pageSize=%" PRId64 "&x_"
        "request_id=%s"
        "&clientType=%s&version=%s&channelId=%s&rand=%s",
        folderId.c_str(), GetFlag(), recursive, fileType, mediaType, mediaAttr,
        iconOption, orderBy.c_str(), descending, pageNum, pageSize,
        x_request_id.c_str(), GetClientType().c_str(),
        GetCurrentApplicationVersion().c_str(),
        GetChannelId().c_str(),
        restful_common::rand_helper::GetRandString().c_str());

    // set header
    request.headers.Set("X-Request-ID", x_request_id);

    is_ok = true;
  } while (false);

  return is_ok;
}

bool HttpResponseDecode(const assistant::HttpResponse& response,
                        const assistant::HttpRequest& request,
                        std::string& response_info) {
  bool is_success = false;
  Json::Value result_json;
  Json::Value fileList;
  Json::Value folder;
  Json::Value folderChild;
  Json::Value file;
  Json::Value fileChild;
  pugi::xml_document result_xml;
  auto http_status_code = response.status_code;
  auto curl_code = atoi(response.extends.Get("CURLcode").c_str());
  do {
    if (0 == curl_code && http_status_code / 100 == 2) {
      if (response.body.size() == 0) {
        break;
      }
      auto res = result_xml.load_string(response.body.c_str());
      if (res.status != pugi::xml_parse_status::status_ok) {
        break;
      }
      auto root = result_xml.child("listFiles");
      result_json["lastRev"] = root.child("lastRev").text().as_string();
      // fileList 节点
      auto file_list = root.child("fileList");
      fileList["count"] = file_list.child("count").text().as_int();

      // folder 节点
      for (auto folder_node = file_list.child("folder"); folder_node;
           folder_node = folder_node.next_sibling("folder")) {
        folderChild["id"] = folder_node.child("id").text().as_string();
        folderChild["parentId"] =
            folder_node.child("parentId").text().as_string();
        folderChild["groupSpaceId"] =
            folder_node.child("groupSpaceId").text().as_int();
        folderChild["name"] = folder_node.child("name").text().as_string();
        folderChild["starLabel"] =
            folder_node.child("starLabel").text().as_int();
        folderChild["createDate"] =
            folder_node.child("createDate").text().as_string();
        folderChild["lastOpTime"] =
            folder_node.child("lastOpTime").text().as_string();
        folderChild["rev"] = folder_node.child("rev").text().as_int();
        folderChild["fileCount"] =
            folder_node.child("fileCount").text().as_int();
        folderChild["fileCata"] = folder_node.child("fileCata").text().as_int();
        folder.append(folderChild);
      }
      fileList["folder"] = folder;
      // file 节点
      for (auto file_node = file_list.child("file"); file_node;
           file_node = file_node.next_sibling("file")) {
        fileChild["id"] = file_node.child("id").text().as_string();
        fileChild["name"] = file_node.child("name").text().as_string();
        fileChild["starLabel"] = file_node.child("starLabel").text().as_int();
        fileChild["size"] = file_node.child("size").text().as_int();
        fileChild["md5"] = file_node.child("md5").text().as_string();
        fileChild["createDate"] =
            file_node.child("createDate").text().as_string();
        fileChild["lastOpTime"] =
            file_node.child("lastOpTime").text().as_string();
        fileChild["mediaType"] = file_node.child("mediaType").text().as_int();
        fileChild["rev"] = file_node.child("rev").text().as_int();
        fileChild["fileCata"] = file_node.child("fileCata").text().as_int();
        file.append(fileChild);
      }
      fileList["file"] = file;
      result_json["fileList"] = fileList;
    } else if (0 == curl_code && http_status_code / 100 != 2) {
      if (!response.body.empty()) {
        /*****************解析error节点***********************/
        auto prs = result_xml.load_string(response.body.c_str());
        if (prs.status != pugi::xml_parse_status::status_ok) {
          break;
        }
        auto folder = result_xml.child("error");
        std::string errCode = folder.child("code").text().as_string();
        result_json["message"] = folder.child("message").text().as_string();
        /********************************************************/
        result_json["errorCode"] = errCode;
        result_json["int32ErrorCode"] =
            Cloud189::ErrorCode::int32ErrCode(errCode.c_str());
      }
      break;
    } else {
      break;
    }
    is_success = true;
  } while (false);
  result_json["isSuccess"] = is_success;
  result_json["httpStatusCode"] = http_status_code;
  result_json["curlCode"] = curl_code;
  response_info = restful_common::jsoncpp_helper::WriterHelper(result_json);
  return is_success;
}

}  // namespace ListFiles
}  // namespace Apis
}  // namespace Cloud189
