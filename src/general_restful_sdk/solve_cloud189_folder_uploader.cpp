#include "solve_cloud189_folder_uploader.h"

#include "ast_singleton.h"

#include "log_system/log_system.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using general_restful_sdk_ast::log_system::LogInfo;
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

using ::Cloud189::Restful::FolderUploader;

namespace general_restful_sdk_ast {
namespace Cloud189 {
int32_t CreateFolderUpload(const std::string &folder_info,
                           void (*on_callback)(const char *),
                           std::string &success_uuid) {
  int32_t res_flag = -1;
  const auto &ast = GetAstInfo();
  Json::Value upload_json;
  do {
    ReaderHelper(folder_info, upload_json);
    if (!upload_json.isNull() && !upload_json.isObject()) {
      break;
    }
    /// 解析其中的uuid字段
    const auto uuid = GetString(upload_json["uuid"]);
    if (uuid.empty()) {
      break;
    }
    /// 解析其中的其他必须字段，进行兼容
    const auto domain = GetString(upload_json["domain"]);
    if (domain.compare("Cloud189") != 0) {
      break;
    }
    const auto operation = GetString(upload_json["operation"]);
    /// operation无需校验，由AstProcess进行分发，确保"operation"匹配才会走入此流程

    /// 是否满足启动上传的条件，若是则继续；若否则返回false
    /// 根据文档，余下的字段是业务字段，由Uploader自身进行兼容

    /// TODO: 由于FolderUploader特殊，临时在此检验业务字段
    /// 后续应统一调整到Valid中进行校验
    const auto local_folder_path = GetString(upload_json["local_folder_path"]);
    const auto server_folder_path =
        GetString(upload_json["server_folder_path"]);
    const auto parent_folder_id = GetString(upload_json["parent_folder_id"]);
    if (local_folder_path.empty() || server_folder_path.empty() ||
        parent_folder_id.empty()) {
      break;
    }
    if ('/' != local_folder_path.back()) {
      break;
    }
    if ('/' != server_folder_path.back()) {
      break;
    }
    /// 生成回传回调的中间回调
    std::shared_ptr<int32_t> ec_init = std::make_shared<int32_t>(0);
    std::weak_ptr<int32_t> ec_init_weak(ec_init);
    std::function<void(const std::string &)> callback =
        [ec_init_weak, uuid, domain, operation,
         on_callback](const std::string &info) -> void {
      Json::Value callback_data_json;
      ReaderHelper(info, callback_data_json);
      const auto &ast = GetAstInfo();
      do {
        // const auto stage = GetInt(callback_data_json["stage"]);
        bool is_complete = false;
        is_complete = GetBool(callback_data_json["is_complete"]);
        if (!is_complete) {
          callback_data_json.removeMember("is_complete");
        }
        // bool init_success = false;

        /// 还需插入uuid,domain,operation等字段
        callback_data_json["uuid"] = uuid;
        callback_data_json["domain"] = domain;
        callback_data_json["operation"] = operation;

        const auto info_outer = WriterHelper(callback_data_json);
        LogInfo("[DoFolderUpload] OnCallback info_outer: %s",
                info_outer.c_str());
        /// 将信息回调给外部
        on_callback(info_outer.c_str());

        /// 回调中：检测是否是is_complete，如果是则清理相应的资源
        if (is_complete) {
          ast->uuid_map.Delete(uuid);
          ast->cloud189_folder_uploader_map.Delete(uuid);
        }
      } while (false);
    };  /// 回传回调的中间回调 结束
    auto folder_uploader =
        std::make_unique<FolderUploader>(folder_info, callback);
    auto &uploader_obj = *folder_uploader;
    /// 检查初始化是否有意外，初始化有意外，则无需加入任务容器，也无需启动之
    if (*ec_init != 0) {
      res_flag = *ec_init;
      //      break;
    }
    /// 加入任务容器
    ast->uuid_map.Put(uuid, (1 << 2));
    auto key_in_map = uuid;
    ast->cloud189_folder_uploader_map.Emplace(key_in_map, folder_uploader);

    ///  至此认为初始化并启动总控成功
    success_uuid = uuid;
    res_flag = 0;

  } while (false);

  return res_flag;
}

void StartFolderUpload(const std::string &success_uuid) {
  do {
    const auto &ast = GetAstInfo();
    if (nullptr == ast) {
      break;
    }
    ast->cloud189_folder_uploader_map.FindDelegate(
        success_uuid, [](const std::unique_ptr<FolderUploader> &uploader) {
          if (nullptr != uploader) {
            uploader->AsyncStart();
          }
        });
  } while (false);
}

int32_t UserCancelFolderUpload(const std::string &cancel_uuid) {
  int32_t cancel_res = -1;
  do {
    const auto &ast = GetAstInfo();
    if (nullptr == ast) {
      break;
    }
    bool find_result = false;
    ast->cloud189_folder_uploader_map.FindDelegate(
        cancel_uuid,
        [&find_result](const std::unique_ptr<FolderUploader> &uploader) {
          if (nullptr != uploader) {
            uploader->UserCancel();
            find_result = true;
          }
        });
    if (!find_result) {
      break;
    }
    cancel_res = 0;
  } while (false);
  return cancel_res;
}

}  // namespace Cloud189

}  // namespace general_restful_sdk_ast
