#define GENERAL_RESTFUL_SDK_EXPORTS
#include "upload_dll.h"

#include <iostream>
#include <thread>
//#include <unistd.h>
#include <chrono>
#include <functional>
#include <future>

// #include <json/json.h>
#include <tools/uuid.hpp>

#include "ast_singleton.h"
#include "log_system/log_system.h"
#include "solve_cloud189_downloader.h"
#include "solve_cloud189_folder_downloader.h"
#include "solve_cloud189_folder_uploader.h"
#include "solve_cloud189_uploader.h"

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using general_restful_sdk_ast::log_system::LogInfo;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

void AstProcess(const char *process_info, OnProcessStart on_start,
                OnProcessCallback on_callback) {
  LogInfo("[AstProcess] OnReceived process_info: %s", process_info);
  /// 处理uuid
  Json::Value on_start_json;
  /// -10000代表尚未被支持的domain或operation，或json字符串非法
  on_start_json["start_result"] = int32_t(-10000);
  Json::Value process_info_json;
  ReaderHelper(process_info, process_info_json);
  do {
    if (!process_info_json.isObject() && !process_info_json.isNull()) {
      break;
    }
    auto uuid = GetString((process_info_json["uuid"]));
    if (uuid.empty()) {
      uuid = assistant::tools::uuid::generate();
      process_info_json["uuid"] = uuid;
    }
    on_start_json["uuid"] = uuid.c_str();
    const auto domain = GetString(process_info_json["domain"]);
    on_start_json["domain"] = domain;
    const auto operation = GetString(process_info_json["operation"]);
    on_start_json["operation"] = operation;
    /// 解析其中的其他非必须字段，进行兼容
    auto x_request_id = GetString(process_info_json["x_request_id"]);
    if (x_request_id.empty() || x_request_id != uuid) {
      x_request_id = uuid;
      process_info_json["x_request_id"] = x_request_id;
    }

    const auto solved_info = WriterHelper((process_info_json));
    /// 完全抛给总控处理
    if (domain.compare("Cloud189") == 0 && operation.compare("DoUpload") == 0) {
      auto cloud189_doupload_res =
          general_restful_sdk_ast::Cloud189::DoUpload(solved_info, on_callback);
      on_start_json["start_result"] = cloud189_doupload_res;
    } else if (domain.compare("Cloud189") == 0 &&
               operation.compare("DoFolderUpload") == 0) {
      auto cloud189_dofolderupload_res =
          general_restful_sdk_ast::Cloud189::DoFolderUpload(solved_info,
                                                            on_callback);
      on_start_json["start_result"] = cloud189_dofolderupload_res;
    } else if (domain.compare("Cloud189") == 0 &&
               operation.compare("DoDownload") == 0) {
      auto cloud189_dodownload_res =
          general_restful_sdk_ast::Cloud189::DoDownload(solved_info,
                                                        on_callback);
      on_start_json["start_result"] = cloud189_dodownload_res;
    } else if (domain.compare("Cloud189") == 0 &&
               operation.compare("DoFolderDownload") == 0) {
      auto cloud189_dofolderdownload_res =
          general_restful_sdk_ast::Cloud189::DoFolderDownload(solved_info,
                                                              on_callback);
      on_start_json["start_result"] = cloud189_dofolderdownload_res;
    }

  } while (false);
  const auto on_start_str = WriterHelper(on_start_json);
  LogInfo("[AstProcess] OnResolved result: %s", on_start_str.c_str());
  on_start(on_start_str.c_str());
}

void AstConfig(const char *json_str, OnConfigFinished on_config_finished) {
  LogInfo("[AstConfig] OnReceived json_str: %s", json_str);

  Json::Value root_value;
  Json::Value json_res;
  ReaderHelper(json_str, root_value);

  do {
    if (!root_value.isObject() && !root_value.isNull()) {
      break;
    }

    /// 处理cloud189Session
    const auto save_session = root_value["save_cloud189_session"];
    auto a1 = save_session.isNull();
    auto a3 = save_session.isArray();
    auto a4 = save_session.size();

    if (!save_session.isNull() && save_session.isArray() &&
        save_session.size() == 4) {
      /// 仅当这样才处理cloud189Session
      const auto cloud189SessionKey = GetString(save_session[0]);
      const auto cloud189SessionSecret = GetString(save_session[1]);
      const auto cloud189SessionKeyFamily = GetString(save_session[2]);
      const auto cloud189SessionSecretFamily = GetString(save_session[3]);
      json_res["cloud189SessionKey"] = cloud189SessionKey;
      json_res["cloud189SessionSecret"] = cloud189SessionSecret;
      json_res["cloud189SessionKeyFamily"] = cloud189SessionKeyFamily;
      json_res["cloud189SessionSecretFamily"] = cloud189SessionSecretFamily;
      general_restful_sdk_ast::Config::StoreCloud189Session(
          cloud189SessionKey, cloud189SessionSecret, cloud189SessionKeyFamily,
          cloud189SessionSecretFamily);
    }

    /// 处理save_enterprisecloud_session
    Json::Value save_enterprisecloud_session =
        root_value["save_enterprisecloud_session"];
    if (!save_enterprisecloud_session.isNull() &&
        save_enterprisecloud_session.isArray() &&
        save_enterprisecloud_session.size() == 2) {
      /// 仅当这样才处理save_enterprisecloud_session
      const auto enterprisecloudSessionKey =
          GetString(save_enterprisecloud_session[0]);
      const auto enterprisecloudSessionSecret =
          GetString(save_enterprisecloud_session[1]);
      json_res["ecloud189SessionKey"] = enterprisecloudSessionKey;
      json_res["ecloud189SessionSecret"] = enterprisecloudSessionSecret;
      /// TODO: StoreEnterprisecloudSession
    }

    /************解析logout字段*********************/
    const auto logout = GetString(root_value["logout"]);

    /************解析logoutEnterprise字段*********************/
    const auto logoutEnterprise = GetString(root_value["logoutEnterprise"]);

    /************解析proxyInfo字段*********************/
    const auto proxy_info = GetString(root_value["proxy_info"]);
    if (!proxy_info.empty()) {
      json_res["proxy_info"] = Json::Value(proxy_info);
      general_restful_sdk_ast::Proxy::SetProxy(proxy_info);
    }

    /***************json字符串拼接****************************/
    ///
    ///
    if (!logout.empty()) {
      json_res["logout"] = Json::Value(logout);
    }

    if (!logoutEnterprise.empty()) {
      json_res["logoutEnterprise"] = Json::Value(logoutEnterprise);
    }

  } while (false);
  // json result
  std::string config_result_str =
      restful_common::jsoncpp_helper::WriterHelper(json_res);
  LogInfo("[AstConfig] OnResolved result: %s", config_result_str.c_str());

  on_config_finished(config_result_str.c_str());
}
