#include "solve_cloud189_folder_downloader.h"

#include "ast_singleton.h"

#include "log_system/log_system.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using general_restful_sdk_ast::log_system::LogInfo;
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

using ::Cloud189::Restful::FolderDownloader;

namespace general_restful_sdk_ast {
namespace Cloud189 {
int32_t DoFolderDownload(const std::string &download_info,
                         void (*on_callback)(const char *)) {
  int32_t res_flag = -1;
  const auto &ast = GetAstInfo();
  Json::Value download_json;
  do {
    ReaderHelper(download_info, download_json);
    if (!download_json.isNull() && !download_json.isObject()) {
      break;
    }
    /// 解析其中的uuid字段
    const auto uuid = GetString(download_json["uuid"]);
    if (uuid.empty()) {
      break;
    }
    /// 解析其中的其他必须字段，进行兼容
    const auto domain = GetString(download_json["domain"]);
    if (domain.compare("Cloud189") != 0) {
      break;
    }
    const auto operation = GetString(download_json["operation"]);
    /// operation无需校验，由AstProcess进行分发，确保"operation"匹配才会走入此流程

    /// 是否满足启动上传的条件，若是则继续；若否则返回false
    /// 根据文档，余下的字段是业务字段，由Uploader自身进行兼容

    /// 生成回传回调的中间回调

    std::function<void(const std::string &)> callback =
        [uuid, domain, operation,
         on_callback](const std::string &info) -> void {
      Json::Value callback_data_json;
      ReaderHelper(info, callback_data_json);
      const auto &ast = GetAstInfo();
      do {
        bool is_complete = false;
        is_complete = GetBool(callback_data_json["is_complete"]);
        if (!is_complete) {
          callback_data_json.removeMember("is_complete");
        }
        bool init_success = false;

        /// 还需插入uuid,domain,operation等字段
        callback_data_json["uuid"] = uuid;
        callback_data_json["domain"] = domain;
        callback_data_json["operation"] = operation;

        const auto info_outer = WriterHelper(callback_data_json);
        LogInfo("[DoFolderDownload] OnCallback info_outer: %s",
                info_outer.c_str());
        /// 将信息回调给外部
        on_callback(info_outer.c_str());

        /// 回调中：检测是否是is_complete，如果是则清理相应的资源
        if (is_complete) {
          ast->uuid_map.Delete(uuid);
          ast->cloud189_folder_downloader_map.Delete(uuid);
        }
      } while (false);
    };  /// 回传回调的中间回调 结束
    auto folder_downloader =
        std::make_unique<FolderDownloader>(download_info, callback);
    auto &folder_downloader_obj = *folder_downloader;
    /// TODO：
    /// 检查初始化是否有意外，初始化有意外，则无需加入任务容器，也无需启动之

    /// 加入任务容器
    ast->uuid_map.Put(uuid, (1 << 3));
    auto key_in_map = uuid;
    ast->cloud189_folder_downloader_map.Emplace(key_in_map, folder_downloader);

    /// 启动任务
    folder_downloader_obj.AsyncStart();
    ///  至此认为初始化并启动总控成功
    res_flag = 0;

  } while (false);

  return res_flag;
}

}  // namespace Cloud189

}  // namespace general_restful_sdk_ast
