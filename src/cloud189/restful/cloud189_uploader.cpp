#include "cloud189_uploader.h"

/// 避免因Windows.h在WinSock2.h之前被包含导致的问题
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <rx_assistant.hpp>
#include <rx_md5.hpp>
#include <rx_uploader.hpp>
#include <speed_counter.hpp>
#include <tools/safecontainer.hpp>
#include <tools/string_convert.hpp>

#include "cloud189/apis/comfirm_upload_file_complete.h"
#include "cloud189/apis/create_upload_file.h"
#include "cloud189/apis/get_upload_status.h"
#include "cloud189/apis/upload_file_data.h"
#include "cloud189/error_code/nderror.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using assistant::HttpRequest;
using assistant::tools::lockfree_string_closure;
using httpbusiness::uploader::proof::stage_result;
using httpbusiness::uploader::proof::uploader_proof;
using httpbusiness::uploader::proof::uploader_stage;
using rx_assistant::HttpResult;

/// TODO: 既然是源文件，那么对比较复杂的命名空间使用，都可以使用using
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetInt64;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

namespace Cloud189 {
namespace Restful {

namespace details {

struct uploader_internal_data {
 private:
  /// 在此占据某个文件的相应句柄
  FILE* file_protect;
  /// 以ScopeGuard机制在析构时释放此句柄
  assistant::tools::scope_guard guard_file_protect;

 public:
  std::unique_ptr<httpbusiness::uploader::rx_uploader> const master_control;
  /// 以file_protect为标志，如果此指针已为空，则什么都不用做
  bool Valid() { return nullptr != file_protect; }
  /// 保存此回调，以供调用
  const httpbusiness::uploader::rx_uploader::CompleteCallback
      null_file_callback;

  explicit uploader_internal_data(
      const std::string& file_path,
      const httpbusiness::uploader::proof::proof_obs_packages& proof_orders,
      const httpbusiness::uploader::rx_uploader::CompleteCallback data_callback)
      : file_protect(
            _wfsopen(assistant::tools::string::utf8ToWstring(file_path).c_str(),
                     L"r", _SH_DENYWR)),
        guard_file_protect([this]() {
          if (nullptr != file_protect) {
            fclose(file_protect);
            file_protect = nullptr;
          }
        }),
        null_file_callback(data_callback),
        master_control(std::make_unique<httpbusiness::uploader::rx_uploader>(
            proof_orders, data_callback)) {}
  ~uploader_internal_data() = default;
  /// 由于scope_guard的存在，无需再显式地禁用另外四个构造函数
};

struct uploader_thread_data {
 public:
  /// const的数据成员，必须在构造函数中进行显式的初始化
  const std::string local_filepath;
  const std::string last_md5;
  const std::string last_upload_id;
  const std::string parent_folder_id;
  const std::string x_request_id;
  const int32_t oper_type;
  const int32_t is_log;
  /// 禁用移动复制默认构造和=号操作符，必须通过显式指定所有必须字段进行初始化
  uploader_thread_data(const std::string& file_path,
                       const std::string& last_trans_md5,
                       const std::string& last_trans_upload_id,
                       const std::string& parent_folder_id_,
                       const std::string& x_request_id_,
                       const int32_t& oper_type_, const int32_t& is_log_)
      : local_filepath(file_path),
        last_md5(last_trans_md5),
        last_upload_id(last_trans_upload_id),
        parent_folder_id(parent_folder_id_),
        x_request_id(x_request_id_),
        oper_type(oper_type_),
        is_log(is_log_),
        file_size({0}),
        data_exist({false}),
        already_upload_bytes({0}),
        current_upload_bytes({0}),
        int32_error_code({0}),
        frozen({false}),
        speed_count(std::make_unique<httpbusiness::speed_counter_with_stop>()) {
  }
  /// 线程安全的数据成员
  lockfree_string_closure<std::string> file_md5;
  lockfree_string_closure<std::string> upload_file_id;
  lockfree_string_closure<std::string> file_upload_url;
  lockfree_string_closure<std::string> file_commit_url;
  /// TODO: 原子量需要进行初始化
  std::atomic<int64_t>
      file_size;  /// 保存文件大小，由于文件已被锁定，全流程不变
  std::atomic<bool> data_exist;  /// 保存文件是否可秒传
  std::atomic<int64_t> already_upload_bytes;  /// 获取续传状态中得到的已传数据量
  std::atomic<int64_t> current_upload_bytes;  /// 上传文件数据中每次的已传数据量
  std::atomic<int32_t> int32_error_code;  /// 错误码
  /// 以下为确认文件上传完成后解析到的字段
  lockfree_string_closure<std::string> commit_id;
  lockfree_string_closure<std::string> commit_name;
  lockfree_string_closure<std::string> commit_size;
  lockfree_string_closure<std::string> commit_md5;
  lockfree_string_closure<std::string> commit_create_date;
  lockfree_string_closure<std::string> commit_rev;
  lockfree_string_closure<std::string> commit_user_id;
  lockfree_string_closure<std::string> commit_request_id;
  lockfree_string_closure<std::string> commit_is_safe;

  /// 保存计速器，以进行平滑速度计算，以及1S一次的数据推送
  std::unique_ptr<httpbusiness::speed_counter_with_stop> speed_count;
  std::weak_ptr<httpbusiness::uploader::rx_uploader::rx_uploader_data>
      master_control_data;

  /// 保存停止标志位
  std::atomic<bool> frozen;
  /// 保存当前请求的UUID，以供停止请求
  lockfree_string_closure<std::string> current_request_uuid;

 private:
  uploader_thread_data() = delete;
  uploader_thread_data(uploader_thread_data const&) = delete;
  uploader_thread_data& operator=(uploader_thread_data const&) = delete;
  uploader_thread_data(uploader_thread_data&&) = delete;
};

}  // namespace details

}  // namespace Restful
}  // namespace Cloud189

using Cloud189::Restful::details::uploader_thread_data;

/// 将构造函数完全进行模块化分解，避免单个函数承担过多功能
/// 在此将Uploader所需的函数模块抽取出来
namespace {

/// 根据传入的字符串，对线程数据进行初始化
std::shared_ptr<uploader_thread_data> InitThreadData(
    const std::string& upload_info) {
  /// 根据传入信息，初始化线程信息结构体
  Json::Value json_value;
  ReaderHelper(upload_info, json_value);
  /// 改为jsoncpp_helper内的工具函数进行解析
  const auto& local_filepath = GetString(json_value["localPath"]);
  const auto& last_md5 = GetString(json_value["md5"]);
  const auto& last_upload_id = GetString(json_value["uploadFileId"]);
  const auto& parent_folder_id = GetString(json_value["parentFolderId"]);
  const auto oper_type = GetInt(json_value["opertype"]);
  const auto is_log = GetInt(json_value["isLog"]);
  std::string x_request_id = GetString(json_value["X-Request-ID"]);
  if (x_request_id.empty()) {
    x_request_id = assistant::tools::uuid::generate();
  }
  return std::make_shared<uploader_thread_data>(
      local_filepath, last_md5, last_upload_id, parent_folder_id, x_request_id,
      oper_type, is_log);
}
/// 生成总控使用的完成回调
const httpbusiness::uploader::rx_uploader::CompleteCallback
GenerateDataCallback(
    const std::weak_ptr<uploader_thread_data>& thread_data_weak,
    const std::function<void(const std::string&)>& data_callback) {
  /// 为计速器提供每秒一次的回调
  auto thread_data = thread_data_weak.lock();
  if (nullptr != thread_data) {
    thread_data->speed_count->RegSubscription(
        [thread_data_weak, data_callback](uint64_t smooth_speed) {
          /// 每秒一次的回调信息字段
          Json::Value info;
          info["speed"] = int64_t(smooth_speed);
          auto thread_data = thread_data_weak.lock();
          if (nullptr != thread_data) {
            info["md5"] = thread_data->file_md5.load();
            info["upload_id"] = thread_data->upload_file_id.load();
            info["file_size"] = thread_data->file_size.load();
            info["X-Request-ID"] = thread_data->x_request_id;
            /// 已传输数据量
            auto transferred_size = thread_data->already_upload_bytes.load() +
                                    thread_data->current_upload_bytes.load();
            if (transferred_size > thread_data->file_size.load()) {
              transferred_size = thread_data->file_size.load();
            }
            info["transferred_size"] = transferred_size;

            auto mc_data = thread_data->master_control_data.lock();
            int32_t current_stage = -1;
            int32_t ec = thread_data->int32_error_code.load();
            if (nullptr != mc_data) {
              current_stage = int32_t(mc_data->current_stage.load());
            }
            info["stage"] = current_stage;
            if (current_stage >= 3) {
              info["data_exist"] = thread_data->data_exist.load();
              info["file_upload_url"] = thread_data->file_upload_url.load();
            }
            /// 在errorcode机制完善后，需要在ec为0的情况下，才加相应业务字段
            if (current_stage >= 5 && ec == 0) {
              info["commit_file_id"] = thread_data->commit_id.load();
              info["commit_name"] = thread_data->commit_name.load();
              info["commit_size"] = thread_data->commit_size.load();
              info["commit_md5"] = thread_data->commit_md5.load();
              info["commit_create_date"] =
                  thread_data->commit_create_date.load();
              info["commit_rev"] = thread_data->commit_rev.load();
              info["commit_user_id"] = thread_data->commit_user_id.load();
              // info["commit_is_safe"] = thread_data->commit_is_safe.load();
            }
          }
          data_callback(WriterHelper(info));
        },
        []() {});
  }

  /// 提供流程完成（成功和失败均包括在内）时的回调
  return [thread_data_weak,
          data_callback](const httpbusiness::uploader::rx_uploader&) -> void {
    /// TODO: 在此处取出相应的信息，传递给回调
    Json::Value info;
    info["is_complete"] = bool(true);
    info["stage"] = int32_t(uploader_stage::UploadFinal);

    auto thread_data = thread_data_weak.lock();
    if (nullptr != thread_data) {
      /// 需保证计速器回调不再发送
      thread_data->speed_count->Stop();
      /// 为MT的CRT的线程模型不一致问题进行变通处理，
      /// 在此处强行迫使相应的订阅尽早提前取消
      thread_data->speed_count = nullptr;

      info["md5"] = thread_data->file_md5.load();
      info["upload_id"] = thread_data->upload_file_id.load();
      info["file_size"] = thread_data->file_size.load();
      info["X-Request-ID"] = thread_data->x_request_id;
      /// 已传输数据量
      auto transferred_size = thread_data->already_upload_bytes.load() +
                              thread_data->current_upload_bytes.load();
      if (transferred_size > thread_data->file_size.load()) {
        transferred_size = thread_data->file_size.load();
      }
      info["transferred_size"] = transferred_size;
      info["data_exist"] = thread_data->data_exist.load();
      info["file_upload_url"] = thread_data->file_upload_url.load();
      info["int32_error_code"] = thread_data->int32_error_code.load();
      /// 在errorcode机制完善后，需要在ec为0的情况下，才加相应业务字段
      int32_t ec = thread_data->int32_error_code.load();
      if (ec == 0) {
        info["commit_file_id"] = thread_data->commit_id.load();
        info["commit_name"] = thread_data->commit_name.load();
        info["commit_size"] = thread_data->commit_size.load();
        info["commit_md5"] = thread_data->commit_md5.load();
        info["commit_create_date"] = thread_data->commit_create_date.load();
        info["commit_rev"] = thread_data->commit_rev.load();
        info["commit_user_id"] = thread_data->commit_user_id.load();
        // info["commit_is_safe"] = thread_data->commit_is_safe.load();
      }
    }

    data_callback(WriterHelper(info));
  };
}
/// 根据弱指针，初始化各RX指令
httpbusiness::uploader::proof::proof_obs_packages GenerateOrders(
    const std::weak_ptr<uploader_thread_data>& thread_data_weak) {
  //////////////////////////////////////////////////////////////////////////
  /// 计算MD5指令
  /// 输入proof，根据线程信息中的路径信息，异步地进行MD5计算并返回proof
  httpbusiness::uploader::proof::ProofObsCallback calculate_md5;
  calculate_md5 =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    auto thread_data = thread_data_weak.lock();
    std::string file_path =
        nullptr != thread_data ? thread_data->local_filepath : std::string();

    /// 计算MD5开始前，获取文件大小
    uint64_t file_size = 0;
    cloud_base::filesystem_helper::GetFileSize(
        assistant::tools::string::utf8ToWstring(file_path), file_size);
    if (nullptr != thread_data) {
      thread_data->file_size.store(file_size);
    }

    /// 一旦计算成功，还需比较是否已有上一次的续传记录
    /// 检验上一次续传记录规则：上一次续传MD5和上传ID非空；MD5一致；认为上次的续传记录有效
    std::function<uploader_proof(std::string&)> md5_result_callback =
        [thread_data_weak](std::string& md5) -> uploader_proof {
      uploader_proof calculate_md5_result_proof = {
          uploader_stage::CalculateMd5, stage_result::GiveupRetry,
          uploader_stage::UploadInitial, 0, 0};
      do {
        auto thread_data = thread_data_weak.lock();
        if (nullptr == thread_data) {
          break;
        }
        /// 增加对用户手动取消的处理
        if (thread_data->frozen.load()) {
          calculate_md5_result_proof.result = stage_result::UserCanceled;
          thread_data->int32_error_code =
              Cloud189::ErrorCode::nderr_usercanceled;
          break;
        }

        /// “可能”检查上一次续传记录则尝试CheckUpload；
        /// 否则一律直接CreateUpload
        const auto& last_md5 = thread_data->last_md5;
        const auto& last_upload_id = thread_data->last_upload_id;
        if (!md5.empty() && last_md5 == md5 && !last_upload_id.empty()) {
          thread_data->file_md5 = md5;
          thread_data->upload_file_id = last_upload_id;
          calculate_md5_result_proof.result = stage_result::Succeeded;
          calculate_md5_result_proof.next_stage = uploader_stage::CheckUpload;
          break;
        }
        if (!md5.empty()) {
          thread_data->file_md5 = md5;
          calculate_md5_result_proof.result = stage_result::Succeeded;
          calculate_md5_result_proof.next_stage = uploader_stage::CreateUpload;
          break;
        }
      } while (false);
      return calculate_md5_result_proof;
    };
    /// 用于及时停止MD5计算过程 2019.12.16
    std::function<bool(void)> md5_check_stop =
        rx_assistant::rx_md5::details::DefaultCheckStopCallback;
    if (nullptr != thread_data) {
      const auto& frozen = thread_data->frozen;
      md5_check_stop = [thread_data, &frozen]() -> bool {
        return !frozen.load();
      };
    }
    return rx_assistant::rx_md5::create(file_path, md5_check_stop)
        .map(md5_result_callback);
  };
  /// calculate_md5 end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 创建续传指令
  /// 输入proof，根据线程信息中的路径信息，异步地进行MD5计算并返回proof
  httpbusiness::uploader::proof::ProofObsCallback create_upload;
  create_upload =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    /// 初始化请求失败，应如此返回
    rxcpp::observable<uploader_proof> result = rxcpp::observable<>::just(
        uploader_proof{uploader_stage::CreateUpload, stage_result::GiveupRetry,
                       uploader_stage::UploadFinal, 0, 0});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      /// HttpRequestEncode
      HttpRequest create_upload_request("");
      const auto& file_path = thread_data->local_filepath;
      const auto file_md5 = thread_data->file_md5.load();
      const auto& parent_folder_id = thread_data->parent_folder_id;
      const auto& x_request_id = thread_data->x_request_id;
      const auto oper_type = thread_data->oper_type;
      const auto is_log = thread_data->is_log;
      Cloud189::Apis::CreateUploadFile::HttpRequestEncode(
          Cloud189::Apis::CreateUploadFile::JsonStringHelper(
              parent_folder_id, file_path, file_md5, x_request_id, oper_type,
              is_log),
          create_upload_request);
      /// process, uuid , for potential stop
      std::string unused_uuid = assistant::tools::uuid::generate();
      create_upload_request.extends.Set("uuid", unused_uuid);
      thread_data->current_request_uuid.store(unused_uuid);
      /// 请求初始化完毕

      /// 提供根据HttpResponse解析得到结果的json字符串
      std::function<std::string(HttpResult&)> solve_create_upload =
          [](HttpResult& create_upload_http_result) -> std::string {
        std::string decode_result;
        Cloud189::Apis::CreateUploadFile::HttpResponseDecode(
            create_upload_http_result.res, create_upload_http_result.req,
            decode_result);
        return decode_result;
      };

      /// 提供根据结果的json字符串得到proof的lambda表达式
      std::function<uploader_proof(const std::string&)>
          get_create_upload_result =
              [thread_data_weak](
                  const std::string& create_upload_result) -> uploader_proof {
        auto create_upload_proof = uploader_proof{
            uploader_stage::CreateUpload, stage_result::RetrySelf,
            uploader_stage::CreateUpload, 0, 0};
        do {
          if (create_upload_result.empty()) {
            break;
          }
          auto thread_data = thread_data_weak.lock();
          if (nullptr == thread_data) {
            break;
          }
          /// 请求到此已完成，无需再记录UUID
          thread_data->current_request_uuid.Clear();

          /// 增加对用户手动取消的处理
          if (thread_data->frozen.load()) {
            create_upload_proof.result = stage_result::UserCanceled;
            thread_data->int32_error_code =
                Cloud189::ErrorCode::nderr_usercanceled;
            break;
          }
          Json::Value json_value;
          if (!ReaderHelper(create_upload_result, json_value)) {
            break;
          }
          const auto is_success = GetBool(json_value["isSuccess"]);
          const auto file_data_exists = GetInt(json_value["fileDataExists"]);
          if (is_success) {
            thread_data->upload_file_id.store(
                GetString(json_value["uploadFileId"]));
            thread_data->file_upload_url.store(
                GetString(json_value["fileUploadUrl"]));
            thread_data->file_commit_url.store(
                GetString(json_value["fileCommitUrl"]));
          }
          if (is_success && file_data_exists == 1) {
            /// 成功且文件可秒传
            create_upload_proof.result = stage_result::Succeeded;
            create_upload_proof.next_stage = uploader_stage::FileCommit;
            /// 创建续传流程中：若可秒传，则将already_upload_bytes为文件大小，current_upload_bytes为0
            thread_data->data_exist.store(true);
            thread_data->already_upload_bytes = thread_data->file_size.load();
            thread_data->current_upload_bytes = 0;
            break;
          }
          if (is_success) {
            /// 成功且文件不可秒传
            create_upload_proof.result = stage_result::Succeeded;
            create_upload_proof.next_stage = uploader_stage::FileUplaod;
            /// 创建续传流程中：若不可秒传，则将already_upload_bytes为0，current_upload_bytes也为0
            thread_data->already_upload_bytes = 0;
            thread_data->current_upload_bytes = 0;
            break;
          }
          const auto http_statuc_code = GetInt(json_value["httpStatusCode"]);
          const auto int32_error_code = GetInt(json_value["int32ErrorCode"]);
          thread_data->int32_error_code.store(int32_error_code);
          /// 最坏的失败，无需重试，比如特定的4xx的错误码，情形如：登录信息失效、空间不足等
          if (4 == (http_statuc_code / 100) &&
              (int32_error_code == Cloud189::ErrorCode::nderr_sessionbreak ||
               int32_error_code == Cloud189::ErrorCode::nderr_session_expired ||
               int32_error_code == Cloud189::ErrorCode::nderr_sessiontimeout ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_usersessionboisnull ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_not_enough_quota ||
               int32_error_code == Cloud189::ErrorCode::nderr_limit_exceeded ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_parent_not_folder ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_invalid_parent_folder ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_userdayflowoverlimited)) {
            create_upload_proof.result = stage_result::GiveupRetry;
            create_upload_proof.next_stage = uploader_stage::UploadFinal;
            break;
          }
          /// 除此以外，均应按照指数增幅进行重试
          /// 什么也不用做，默认的result即是stage_result::RetrySelf
        } while (false);
        return create_upload_proof;
      };

      /// 生成数据源成功，保存到result
      result = rx_assistant::rx_httpresult::create(create_upload_request)
                   .map(solve_create_upload)
                   .map(get_create_upload_result);
    } while (false);
    return result;
  };
  /// create_upload end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  ///获取续传状态指令
  httpbusiness::uploader::proof::ProofObsCallback check_upload;
  check_upload =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    /// 初始化请求失败，应如此返回
    rxcpp::observable<uploader_proof> result = rxcpp::observable<>::just(
        uploader_proof{uploader_stage::CheckUpload, stage_result::GiveupRetry,
                       uploader_stage::UploadFinal, 0, 0});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      /// 线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
      HttpRequest check_upload_request("");
      const auto upload_id = thread_data->upload_file_id.load();
      const auto& x_request_id = thread_data->x_request_id;
      Cloud189::Apis::GetUploadFileStatus::HttpRequestEncode(
          Cloud189::Apis::GetUploadFileStatus::JsonStringHelper(upload_id,
                                                                x_request_id),
          check_upload_request);
      /// process, uuid , for potential stop
      std::string unused_uuid = assistant::tools::uuid::generate();
      check_upload_request.extends.Set("uuid", unused_uuid);
      thread_data->current_request_uuid.store(unused_uuid);
      /// 请求初始化完毕

      /// 提供根据HttpResponse解析得到结果的json字符串
      std::function<std::string(HttpResult&)> solve_check_upload =
          [](HttpResult& check_upload_http_result) -> std::string {
        std::string decode_result;
        Cloud189::Apis::GetUploadFileStatus::HttpResponseDecode(
            check_upload_http_result.res, check_upload_http_result.req,
            decode_result);
        return decode_result;
      };

      /// 提供根据结果的json字符串得到proof的lambda表达式
      std::function<uploader_proof(const std::string&)>
          get_check_upload_result =
              [thread_data_weak](
                  const std::string& check_upload_result) -> uploader_proof {
        auto check_upload_proof =
            uploader_proof{uploader_stage::CheckUpload, stage_result::RetrySelf,
                           uploader_stage::CheckUpload, 0, 0};
        do {
          if (check_upload_result.empty()) {
            break;
          }
          auto thread_data = thread_data_weak.lock();
          if (nullptr == thread_data) {
            break;
          }
          /// 请求到此已完成，无需再记录UUID
          thread_data->current_request_uuid.Clear();

          /// 增加对用户手动取消的处理
          if (thread_data->frozen.load()) {
            check_upload_proof.result = stage_result::UserCanceled;
            thread_data->int32_error_code =
                Cloud189::ErrorCode::nderr_usercanceled;
            break;
          }
          Json::Value json_value;
          if (!ReaderHelper(check_upload_result, json_value)) {
            break;
          }
          const auto is_success = GetBool(json_value["isSuccess"]);
          const auto file_data_exists = GetInt(json_value["fileDataExists"]);
          const auto already_upload_bytes = GetInt64(json_value["size"]);
          if (is_success) {
            thread_data->file_upload_url.store(
                GetString(json_value["fileUploadUrl"]));
            thread_data->file_commit_url.store(
                GetString(json_value["fileCommitUrl"]));
          }
          /// 特殊处理：file_data_exists为0，但size和文件大小一样大，此时不是秒传，但
          /// 仍可直接进行FileCommit。原因是本次上传之前的提交发生了服务器错误。
          if (is_success && file_data_exists == 0 &&
              already_upload_bytes == thread_data->file_size.load()) {
            check_upload_proof.result = stage_result::Succeeded;
            check_upload_proof.next_stage = uploader_stage::FileCommit;
            thread_data->already_upload_bytes = thread_data->file_size.load();
            thread_data->current_upload_bytes = 0;
            break;
          }
          if (is_success && file_data_exists == 1) {
            /// 成功且文件可秒传
            check_upload_proof.result = stage_result::Succeeded;
            check_upload_proof.next_stage = uploader_stage::FileCommit;
            /// 查询续传记录流程中：若可秒传，则令already_upload_bytes为文件大小，current_upload_bytes为0
            thread_data->data_exist.store(true);
            thread_data->already_upload_bytes = thread_data->file_size.load();
            thread_data->current_upload_bytes = 0;
            break;
          }
          if (is_success) {
            /// 成功且文件不可秒传
            check_upload_proof.result = stage_result::Succeeded;
            check_upload_proof.next_stage = uploader_stage::FileUplaod;
            /// 查询续传记录流程中：若不可秒传且无偏移记录，则将already_upload_bytes为0，current_upload_bytes也为0；
            /// 查询续传记录流程中：若不可秒传且有偏移记录，则将already_upload_bytes为偏移长度，current_upload_bytes也为0；
            thread_data->already_upload_bytes = already_upload_bytes;
            thread_data->current_upload_bytes = 0;
            break;
          }
          const auto http_statuc_code = restful_common::jsoncpp_helper::GetInt(
              json_value["httpStatusCode"]);
          const auto int32_error_code = restful_common::jsoncpp_helper::GetInt(
              json_value["int32ErrorCode"]);
          thread_data->int32_error_code.store(int32_error_code);
          /// 最坏的失败，无需重试，比如特定的4xx的错误码，情形如：登录信息失效、空间不足等
          if (4 == (http_statuc_code / 100) &&
              (int32_error_code == Cloud189::ErrorCode::nderr_sessionbreak ||
               int32_error_code == Cloud189::ErrorCode::nderr_session_expired ||
               int32_error_code == Cloud189::ErrorCode::nderr_sessiontimeout ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_usersessionboisnull ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_not_enough_quota ||
               int32_error_code == Cloud189::ErrorCode::nderr_limit_exceeded ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_parent_not_folder ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_invalid_parent_folder ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_userdayflowoverlimited)) {
            check_upload_proof.result = stage_result::GiveupRetry;
            check_upload_proof.next_stage = uploader_stage::UploadFinal;
            break;
          }
          if (602 == http_statuc_code) {
            /// 602 特定错误，需要重新创建续传
            check_upload_proof.result = stage_result::RetryTargetStage;
            check_upload_proof.next_stage = uploader_stage::CreateUpload;
            break;
          }
          /// 除此以外，均应按照指数增幅进行重试，并记录suggest_waittime
          /// 默认的result即是stage_result::RetrySelf
          /// TODO: 还需确认，是否能确实取到此字段
          check_upload_proof.suggest_waittime =
              GetInt(json_value["waitingTime"]);
        } while (false);
        return check_upload_proof;
      };
      /// 生成数据源成功，保存到result
      result = rx_assistant::rx_httpresult::create(check_upload_request)
                   .map(solve_check_upload)
                   .map(get_check_upload_result);
    } while (false);
    return result;
  };
  /// check_upload end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 上传文件指令
  httpbusiness::uploader::proof::ProofObsCallback file_upload;
  file_upload =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    /// 初始化请求失败，应如此返回
    rxcpp::observable<uploader_proof> result = rxcpp::observable<>::just(
        uploader_proof{uploader_stage::FileUplaod, stage_result::GiveupRetry,
                       uploader_stage::UploadFinal, 0, 0});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }

      /// 上传文件这一流程即将开始时，清空current_upload_bytes字段
      /// 避免涉及到弱网络有重传时，对进度记录不准确
      thread_data->current_upload_bytes.store(0);

      /// TODO: 这一段的代码风格和内存占用，有待完善
      HttpRequest file_upload_request("");
      /// 从线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
      const std::string file_path = thread_data->local_filepath;
      const std::string upload_id = thread_data->upload_file_id.load();
      const std::string x_request_id = thread_data->x_request_id;
      const int64_t start_offset = thread_data->already_upload_bytes;
      /// TODO:offset_length是否可以不传在encode中计算
      const int64_t offset_length = thread_data->already_upload_bytes;
      const std::string file_upload_url = thread_data->file_upload_url.load();
      std::string file_upload_json_str =
          Cloud189::Apis::UploadFileData::JsonStringHelper(
              file_upload_url, file_path, upload_id, x_request_id, start_offset,
              offset_length);
      /// HttpRequestEncode
      Cloud189::Apis::UploadFileData::HttpRequestEncode(file_upload_json_str,
                                                        file_upload_request);
      /// process, uuid , for potential stop
      std::string unused_uuid = assistant::tools::uuid::generate();
      file_upload_request.extends.Set("uuid", unused_uuid);
      thread_data->current_request_uuid.store(unused_uuid);
      /// 保存thread_data，避免在传输线程中反复lock
      /// 为需要使用的变量定义引用
      auto& current_upload_bytes = thread_data->current_upload_bytes;
      auto& Add = thread_data->speed_count->Add;
      file_upload_request.retval_func = [thread_data, &current_upload_bytes,
                                         Add](uint64_t value) {
        current_upload_bytes += value;
        Add(value);
      };
      /// 使用rx_assistant::rx_httpresult::create 创建数据源，下同
      auto obs = rx_assistant::rx_httpresult::create(file_upload_request);

      std::function<std::string(HttpResult&)> solve_file_uplaod =
          [](HttpResult& file_upload_http_result) -> std::string {
        std::string decode_result;
        Cloud189::Apis::UploadFileData::HttpResponseDecode(
            file_upload_http_result.res, file_upload_http_result.req,
            decode_result);
        return decode_result;
      };

      /// 提供根据结果的json字符串得到proof的lambda表达式
      std::function<uploader_proof(const std::string&)> get_file_uplaod_result =
          [thread_data_weak](
              const std::string& file_upload_result) -> uploader_proof {
        auto file_uplaod_proof = uploader_proof{
            uploader_stage::FileUplaod, stage_result::RetryTargetStage,
            uploader_stage::CheckUpload, 0, 0};
        do {
          if (file_upload_result.empty()) {
            break;
          }
          auto thread_data = thread_data_weak.lock();
          if (nullptr == thread_data) {
            break;
          }
          /// 请求到此已完成，无需再记录UUID
          thread_data->current_request_uuid.Clear();

          /// 增加对用户手动取消的处理
          if (thread_data->frozen.load()) {
            file_uplaod_proof.result = stage_result::UserCanceled;
            thread_data->int32_error_code =
                Cloud189::ErrorCode::nderr_usercanceled;
            break;
          }
          Json::Value json_value;
          if (!restful_common::jsoncpp_helper::ReaderHelper(file_upload_result,
                                                            json_value)) {
            break;
          }
          const auto is_success =
              restful_common::jsoncpp_helper::GetBool(json_value["isSuccess"]);
          /// 交卷，填写此文件已上传数据量
          file_uplaod_proof.transfered_length =
              thread_data->already_upload_bytes.load() +
              thread_data->current_upload_bytes.load();
          if (is_success) {
            /// 成功
            file_uplaod_proof.result = stage_result::Succeeded;
            file_uplaod_proof.next_stage = uploader_stage::FileCommit;
            break;
          }
        } while (false);
        // 除上述情况外的一切情况，提交失败应前往CheckUpload
        return file_uplaod_proof;
      };
      result = obs.map(solve_file_uplaod).map(get_file_uplaod_result);
    } while (false);
    return result;
  };
  /// file uplaod end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 确认上传指令
  httpbusiness::uploader::proof::ProofObsCallback file_commit;
  file_commit =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    /// 初始化请求失败，应如此返回
    rxcpp::observable<uploader_proof> result = rxcpp::observable<>::just(
        uploader_proof{uploader_stage::FileCommit, stage_result::GiveupRetry,
                       uploader_stage::UploadFinal, 0, 0});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      HttpRequest file_commit_request("");
      /// 从线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
      const std::string upload_id = thread_data->upload_file_id.load();
      const std::string x_request_id = thread_data->x_request_id;
      const int32_t oper_type = thread_data->oper_type;
      const int32_t is_log = thread_data->is_log;
      const std::string file_commit_url = thread_data->file_commit_url.load();
      std::string file_commit_json_str =
          Cloud189::Apis::ComfirmUploadFileComplete::JsonStringHelper(
              file_commit_url, upload_id, x_request_id, oper_type, is_log);
      /// HttpRequestEncode
      Cloud189::Apis::ComfirmUploadFileComplete::HttpRequestEncode(
          file_commit_json_str, file_commit_request);
      /// process, uuid , for potential stop
      std::string unused_uuid = assistant::tools::uuid::generate();
      file_commit_request.extends.Set("uuid", unused_uuid);
      thread_data->current_request_uuid.store(unused_uuid);
      /// 使用rx_assistant::rx_httpresult::create 创建数据源
      auto obs = rx_assistant::rx_httpresult::create(file_commit_request);

      std::function<std::string(HttpResult&)> solve_file_commit =
          [](HttpResult& file_commit_http_result) -> std::string {
        std::string decode_result;
        Cloud189::Apis::ComfirmUploadFileComplete::HttpResponseDecode(
            file_commit_http_result.res, file_commit_http_result.req,
            decode_result);
        return decode_result;
      };
      /// 提供根据结果的json字符串得到proof的lambda表达式
      std::function<uploader_proof(const std::string&)> get_file_commit_result =
          [thread_data_weak](
              const std::string& file_commit_result) -> uploader_proof {
        auto file_commit_proof = uploader_proof{
            uploader_stage::FileCommit, stage_result::RetryTargetStage,
            uploader_stage::CheckUpload, 0, 0};
        do {
          if (file_commit_result.empty()) {
            break;
          }
          auto thread_data = thread_data_weak.lock();
          if (nullptr == thread_data) {
            break;
          }
          /// 请求到此已完成，无需再记录UUID
          thread_data->current_request_uuid.Clear();

          Json::Value json_value;
          if (!restful_common::jsoncpp_helper::ReaderHelper(file_commit_result,
                                                            json_value)) {
            break;
          }
          const auto is_success =
              restful_common::jsoncpp_helper::GetBool(json_value["isSuccess"]);
          /// 增加对用户手动取消的处理
          /// 在提交这一步对frozen标志位的检查应延后，避免流程成功但告知结束回调失败
          if (!is_success && thread_data->frozen.load()) {
            file_commit_proof.result = stage_result::UserCanceled;
            thread_data->int32_error_code =
                Cloud189::ErrorCode::nderr_usercanceled;
            break;
          }
          if (is_success) {
            /// 确认文件上传完成后解析到的字段
            thread_data->commit_id.store(
                restful_common::jsoncpp_helper::GetString(json_value["id"]));
            thread_data->commit_name.store(
                restful_common::jsoncpp_helper::GetString(json_value["name"]));
            thread_data->commit_size.store(
                restful_common::jsoncpp_helper::GetString(json_value["size"]));
            thread_data->commit_md5.store(
                restful_common::jsoncpp_helper::GetString(json_value["md5"]));
            thread_data->commit_create_date.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["createDate"]));
            thread_data->commit_rev.store(
                restful_common::jsoncpp_helper::GetString(json_value["rev"]));
            thread_data->commit_user_id.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["userId"]));
            thread_data->commit_request_id.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["requestId"]));
            thread_data->commit_is_safe.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["isSafe"]));
          }
          if (is_success) {
            file_commit_proof.result = stage_result::Succeeded;
            file_commit_proof.next_stage = uploader_stage::UploadFinal;
            break;
          }
          const auto http_statuc_code = restful_common::jsoncpp_helper::GetInt(
              json_value["httpStatusCode"]);
          const auto int32_error_code = restful_common::jsoncpp_helper::GetInt(
              json_value["int32ErrorCode"]);
          thread_data->int32_error_code.store(int32_error_code);
          /// 最坏的失败，无需重试，比如特定的4xx的错误码，情形如：登录信息失效、空间不足等
          if (4 == (http_statuc_code / 100) &&
              (int32_error_code == Cloud189::ErrorCode::nderr_sessionbreak ||
               int32_error_code == Cloud189::ErrorCode::nderr_session_expired ||
               int32_error_code == Cloud189::ErrorCode::nderr_sessiontimeout ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_usersessionboisnull ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_not_enough_quota ||
               int32_error_code == Cloud189::ErrorCode::nderr_limit_exceeded ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_parent_not_folder ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_invalid_parent_folder ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_userdayflowoverlimited)) {
            file_commit_proof.result = stage_result::GiveupRetry;
            file_commit_proof.next_stage = uploader_stage::UploadFinal;
            break;
          }
          file_commit_proof.suggest_waittime =
              restful_common::jsoncpp_helper::GetInt(json_value["waitingTime"]);
        } while (false);
        // 除上述情况外的一切情况，提交失败应前往CheckUpload
        return file_commit_proof;
      };
      result = obs.map(solve_file_commit).map(get_file_commit_result);
    } while (false);
    return result;
  };

  /// file uplaod end.
  //////////////////////////////////////////////////////////////////////////
  return httpbusiness::uploader::proof::proof_obs_packages{
      std::move(calculate_md5), std::move(create_upload),
      std::move(check_upload), std::move(file_upload), std::move(file_commit)};
}

}  // namespace

namespace Cloud189 {
namespace Restful {
Uploader::Uploader(const std::string& upload_info,
                   std::function<void(const std::string&)> data_callback)
    : thread_data(InitThreadData(upload_info)),
      data(std::make_unique<details::uploader_internal_data>(
          thread_data->local_filepath, GenerateOrders(thread_data),
          GenerateDataCallback(thread_data, data_callback))) {
  thread_data->master_control_data = data->master_control->data;
  if (!data->Valid()) {
    thread_data->int32_error_code =
        Cloud189::ErrorCode::nderr_file_access_error;
    data->null_file_callback(*data->master_control);
  }
}

void Uploader::AsyncStart() {
  if (data->Valid()) {
    data->master_control->AsyncStart();
  }
}

void Uploader::SyncWait() {
  if (data->Valid()) {
    data->master_control->SyncWait();
  }
}
/// 在用户手动点击“暂停”或取消，或“退出登录”等需要迫使流程立即失败的场景
/// 非阻塞调用，函数返回不代表立即“取消成功”
/// 将迫使流程尽可能早结束，可能存在延迟
/// 在流程即将完成时，未必能“取消成功”，仍有可能“成功完成”
void Uploader::UserCancel() {
  thread_data->frozen.store(true);
  /// MD5计算已做处理，无需另行处理

  /// 使正在进行的费时操作（如网络请求、数据传输等）中断
  const auto current_request_uuid = thread_data->current_request_uuid.load();
  if (!current_request_uuid.empty()) {
    assistant::HttpRequest stop_req(
        assistant::HttpRequest::Opts::SPCECIALOPERATORS_STOPCONNECT);
    stop_req.extends.Set("uuids", current_request_uuid);
    rx_assistant::rx_httpresult::create(stop_req).publish().connect();
  }
}

Uploader::~Uploader() = default;

/// 为此Uploader提供一个Helper函数，用于生成合规的json字符串
std::string uploader_info_helper(const std::string& local_path,
                                 const std::string& last_md5,
                                 const std::string& last_upload_id,
                                 const std::string& parent_folder_id,
                                 const std::string& x_request_id,
                                 const int32_t oper_type,
                                 const int32_t is_log) {
  Json::Value json_value;
  json_value["localPath"] = local_path;
  json_value["last_md5"] = last_md5;
  json_value["uploadFileId"] = last_upload_id;
  json_value["parentFolderId"] = parent_folder_id;
  json_value["opertype"] = oper_type;
  json_value["isLog"] = is_log;
  json_value["X-Request-ID"] = x_request_id;

  return WriterHelper(json_value);
}
}  // namespace Restful
}  // namespace Cloud189
