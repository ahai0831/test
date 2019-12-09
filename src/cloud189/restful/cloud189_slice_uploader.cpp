#include "cloud189_slice_uploader.h"

/// 避免因Windows.h在WinSock2.h之前被包含导致的问题
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <rx_assistant.hpp>
#include <rx_md5.hpp>
#include <rx_uploader.hpp>
#include <tools/safecontainer.hpp>
#include <tools/string_convert.hpp>

#include "cloud189/apis/commit_slice_upload_file.h"
#include "cloud189/apis/create_slice_upload_file.h"
#include "cloud189/apis/get_slice_upload_status.h"
#include "cloud189/apis/upload_slice_data.h"
#include "cloud189/error_code/nderror.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using assistant::HttpRequest;
using assistant::tools::lockfree_string_closure;
using httpbusiness::uploader::proof::stage_result;
using httpbusiness::uploader::proof::uploader_proof;
using httpbusiness::uploader::proof::uploader_stage;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;
using rx_assistant::HttpResult;
// using rx_assistant::rx_assistant_factory;
// using rx_assistant::md5::md5_async_factory;
/// TODO: 既然是源文件，那么对比较复杂的命名空间使用，都可以使用using
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetInt64;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

namespace Cloud189 {
namespace Restful {

namespace details {

struct sliceuploader_internal_data {
 private:
  /// 在此占据某个文件的相应句柄
  FILE* file_protect;
  /// 以ScopeGuard机制在析构时释放此句柄
  assistant::tools::scope_guard guard_file_protect;

 public:
  std::unique_ptr<httpbusiness::uploader::rx_uploader> const master_control;

  explicit sliceuploader_internal_data(
      const std::string& file_path,
      const httpbusiness::uploader::proof::proof_obs_packages& proof_orders,
      const httpbusiness::uploader::rx_uploader::CompleteCallback
          complete_callback)
      : file_protect(
            _wfsopen(assistant::tools::string::utf8ToWstring(file_path).c_str(),
                     L"r", _SH_DENYWR)),
        guard_file_protect([this]() {
          if (nullptr != file_protect) {
            fclose(file_protect);
            file_protect = nullptr;
          }
        }),
        master_control(std::make_unique<httpbusiness::uploader::rx_uploader>(
            proof_orders, complete_callback)) {}
  ~sliceuploader_internal_data() = default;
  /// 由于scope_guard的存在，无需再显式地禁用另外四个构造函数
};

struct sliceuploader_thread_data {
 public:
  /// const的数据成员，必须在构造函数中进行显式的初始化
  const std::string local_filepath;
  const std::string last_md5;
  const std::string last_upload_id;
  const std::string parent_folder_id;
  const int64_t per_slice_size;
  const int32_t resume_policy;
  const int32_t oper_type;
  const int32_t is_log;
  /// 禁用移动复制默认构造和=号操作符，必须通过显式指定所有必须字段进行初始化
  sliceuploader_thread_data(const std::string& file_path,
                            const std::string& last_trans_md5,
                            const std::string& last_trans_upload_id,
                            const std::string& parent_folder_id_,
                            const int64_t& per_slice_size_,
                            const int32_t& resume_policy_,
                            const int32_t& oper_type_, const int32_t& is_log_)
      : local_filepath(file_path),
        last_md5(last_trans_md5),
        last_upload_id(last_trans_upload_id),
        parent_folder_id(parent_folder_id_),
        per_slice_size(per_slice_size_),
        resume_policy(resume_policy_),
        oper_type(oper_type_),
        is_log(is_log_) {}
  /// 线程安全的数据成员
  lockfree_string_closure<std::string> file_md5;
  lockfree_string_closure<std::string> upload_file_id;
  lockfree_string_closure<std::string> file_upload_url;
  lockfree_string_closure<std::string> file_commit_url;
  lockfree_string_closure<std::string> md5_list_result;
  assistant::tools::safemap_closure<int32_t, std::string> slice_md5_map;
  std::atomic<int64_t> already_upload_bytes;  // 获取续传状态中得到的已传数据量
  std::atomic<int64_t> current_upload_bytes;  // 上传文件数据中每次的已传数据量
  /// 以下为确认文件上传完成后解析到的字段
  lockfree_string_closure<std::string> commit_id;
  lockfree_string_closure<std::string> commit_name;
  lockfree_string_closure<std::string> commit_size;
  lockfree_string_closure<std::string> commit_md5;
  lockfree_string_closure<std::string> commit_create_date;
  lockfree_string_closure<std::string> commit_rev;
  lockfree_string_closure<std::string> commit_user_id;
  lockfree_string_closure<std::string> commit_is_safe;

 private:
  sliceuploader_thread_data() = delete;
  sliceuploader_thread_data(sliceuploader_thread_data const&) = delete;
  sliceuploader_thread_data& operator=(sliceuploader_thread_data const&) =
      delete;
  sliceuploader_thread_data(sliceuploader_thread_data&&) = delete;
};

}  // namespace details

}  // namespace Restful
}  // namespace Cloud189
namespace {
/// 根据传入的字符串，对线程数据进行初始化
/// TODO: 这一块占代码行数太多，需要简化 2019.12.9
std::shared_ptr<Cloud189::Restful::details::sliceuploader_thread_data>
InitThreadData(const std::string& upload_info) {
  /// 根据传入信息，初始化线程信息结构体
  Json::Value json_str;
  ReaderHelper(upload_info, json_str);
  std::string local_filepath = GetString(json_str["localPath"]);
  std::string last_md5 = GetString(json_str["md5"]);
  std::string last_upload_id = GetString(json_str["uploadFileId"]);
  std::string parent_folder_id = GetString(json_str["parentFolderId"]);
  int64_t per_slice_size = GetInt64(json_str["perSliceSize"]);
  int32_t resume_policy = GetInt(json_str["resumePolicy"]);
  int32_t oper_type = GetInt(json_str["opertype"]);
  int32_t is_log = GetInt(json_str["isLog"]);

  std::string slice_md5_list;
  std::string slice_md5;
  auto solve_slice_md5 = [&slice_md5](const std::string& temp) {
    slice_md5 = temp;
  };
  // thread_data->slice_md5_map.FindDelegate(upload_slice_id, solve_slice_md5);
  return std::make_shared<
      Cloud189::Restful::details::sliceuploader_thread_data>(
      local_filepath, last_md5, last_upload_id, parent_folder_id,
      per_slice_size, resume_policy, oper_type, is_log);
}
/// 生成总控使用的完成回调
const httpbusiness::uploader::rx_uploader::CompleteCallback
GenerateCompleteCallback(
    const std::weak_ptr<Cloud189::Restful::details::sliceuploader_thread_data>&
        thread_data_weak,
    const std::function<void(const std::string&)>& complete_callback) {
  return [thread_data_weak, complete_callback](
             const httpbusiness::uploader::rx_uploader&) -> void {
    /// TODO: 在此处取出相应的信息，传递给回调
    complete_callback("");
  };
}
/// 根据弱指针，初始化各RX指令
httpbusiness::uploader::proof::proof_obs_packages GenerateOrders(
    const std::weak_ptr<Cloud189::Restful::details::sliceuploader_thread_data>&
        thread_data_weak) {
  //////////////////////////////////////////////////////////////////////////
  /// 计算MD5指令
  /// 输入proof，根据线程信息中的路径信息，异步地进行MD5计算并返回proof
  httpbusiness::uploader::proof::ProofObsCallback calculate_md5;
  calculate_md5 =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    /// TODO: 需要简化MD5的代码，并加入异步计算各分片MD5的流程 2019.12.9
    auto thread_data = thread_data_weak.lock();
    std::string file_path =
        nullptr != thread_data ? thread_data->local_filepath : std::string();
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
        /// TODO: 需加入对上一次的UploadFileId是否非空的判断，并将其进行处理
        /// TODO: 受限于last_uploadfileid类型还未改，需完善
        if (!md5.empty() && thread_data->last_md5 == md5) {
          thread_data->file_md5 = md5;
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
    return rx_assistant::rx_md5::create(file_path).map(md5_result_callback);
  };
  /// calculate_md5 end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 创建续传指令
  /// 输入proof，根据线程信息中的路径信息，异步地进行MD5计算并返回proof
  httpbusiness::uploader::proof::ProofObsCallback create_slice_upload;
  create_slice_upload =
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
      HttpRequest create_slice_upload_request("");
      const std::string file_path = thread_data->local_filepath;
      const std::string file_md5 = thread_data->file_md5.load();
      const std::string parent_folder_id = thread_data->parent_folder_id;
      const int32_t oper_type = thread_data->oper_type;
      const int32_t is_log = thread_data->is_log;
      std::string create_slice_upload_json_str =
          Cloud189::Apis::CreateSliceUploadFile::JsonStringHelper(
              file_path, parent_folder_id, file_md5, is_log, oper_type);
      /// HttpRequestEncode
      Cloud189::Apis::CreateSliceUploadFile::HttpRequestEncode(
          create_slice_upload_json_str, create_slice_upload_request);
      /// 使用rx_assistant::rx_httpresult::create 创建数据源，下同
      auto obs =
          rx_assistant::rx_httpresult::create(create_slice_upload_request);

      /// 提供根据HttpResponse解析得到结果的json字符串
      std::function<std::string(HttpResult&)> solve_create_slice_upload =
          [](HttpResult& create_slice_upload_http_result) -> std::string {
        std::string decode_result;
        Cloud189::Apis::CreateSliceUploadFile::HttpResponseDecode(
            create_slice_upload_http_result.res,
            create_slice_upload_http_result.req, decode_result);
        return decode_result;
      };

      /// 提供根据结果的json字符串得到proof的lambda表达式
      std::function<uploader_proof(const std::string&)>
          get_create_slice_upload_result =
              [thread_data_weak](const std::string& create_slice_upload_result)
          -> uploader_proof {
        auto create_slice_upload_proof = uploader_proof{
            uploader_stage::CreateUpload, stage_result::RetrySelf,
            uploader_stage::CreateUpload, 0, 0};
        do {
          if (create_slice_upload_result.empty()) {
            break;
          }
          auto thread_data = thread_data_weak.lock();
          if (nullptr == thread_data) {
            break;
          }
          Json::Value json_value;
          if (!restful_common::jsoncpp_helper::ReaderHelper(
                  create_slice_upload_result, json_value)) {
            break;
          }
          const auto is_success =
              restful_common::jsoncpp_helper::GetBool(json_value["isSuccess"]);
          const auto file_data_exists = restful_common::jsoncpp_helper::GetInt(
              json_value["fileDataExists"]);
          if (is_success) {
            thread_data->upload_file_id.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["uploadFileId"]));
            thread_data->file_upload_url.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["fileUploadUrl"]));
            thread_data->file_commit_url.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["fileCommitUrl"]));
          }

          if (is_success && file_data_exists) {
            /// 成功且文件存在
            create_slice_upload_proof.result = stage_result::Succeeded;
            create_slice_upload_proof.next_stage = uploader_stage::FileCommit;
            break;
            if (is_success) {
              /// 成功且文件不存在
              create_slice_upload_proof.result = stage_result::Succeeded;
              create_slice_upload_proof.next_stage =
                  uploader_stage::CheckUpload;
              break;
            }
          }
          const auto http_statuc_code = restful_common::jsoncpp_helper::GetInt(
              json_value["httpStatusCode"]);
          const auto int32_error_code = restful_common::jsoncpp_helper::GetInt(
              json_value["int32ErrorCode"]);

          /// 最坏的失败，无需重试，比如特定的4xx的错误码，情形如：登录信息失效、空间不足等
          if (4 == (http_statuc_code / 100) &&
              (int32_error_code == Cloud189::ErrorCode::nderr_sessionbreak ||
               int32_error_code == Cloud189::ErrorCode::nderr_session_expired ||
               int32_error_code == Cloud189::ErrorCode::nderr_no_diskspace)) {
            create_slice_upload_proof.result = stage_result::GiveupRetry;
            create_slice_upload_proof.next_stage = uploader_stage::UploadFinal;
            break;
          }
          /// 除此以外，均应按照指数增幅进行重试
          /// 什么也不用做，默认的result即是stage_result::RetrySelf
        } while (false);
        return create_slice_upload_proof;
      };
      /// 生成数据源成功，保存到result
      result = obs.map(solve_create_slice_upload)
                   .map(get_create_slice_upload_result);
    } while (false);
    return result;
  };
  /// create_upload end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  ///获取续传状态指令
  httpbusiness::uploader::proof::ProofObsCallback check_slice_upload;
  check_slice_upload =
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
      HttpRequest check_slice_upload_request("");
      /// 线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
      std::string upload_id = thread_data->upload_file_id.load();
      std::string json_str =
          Cloud189::Apis::GetSliceUploadStatus::JsonStringHelper(upload_id);
      /// HttpRequestEncode
      Cloud189::Apis::GetSliceUploadStatus::HttpRequestEncode(
          json_str, check_slice_upload_request);
      /// 使用rx_assistant::rx_httpresult::create 创建数据源，下同
      auto obs =
          rx_assistant::rx_httpresult::create(check_slice_upload_request);

      /// 提供根据HttpResponse解析得到结果的json字符串
      std::function<std::string(HttpResult&)> solve_check_slice_upload =
          [](HttpResult& check_slice_upload_http_result) -> std::string {
        std::string decode_result;
        Cloud189::Apis::GetSliceUploadStatus::HttpResponseDecode(
            check_slice_upload_http_result.res,
            check_slice_upload_http_result.req, decode_result);
        return decode_result;
      };

      /// 提供根据结果的json字符串得到proof的lambda表达式
      std::function<uploader_proof(const std::string&)>
          get_check_slice_upload_result =
              [thread_data_weak](const std::string& check_slice_upload_result)
          -> uploader_proof {
        auto check_slice_upload_proof =
            uploader_proof{uploader_stage::CheckUpload, stage_result::RetrySelf,
                           uploader_stage::CheckUpload, 0, 0};
        do {
          if (check_slice_upload_result.empty()) {
            break;
          }
          auto thread_data = thread_data_weak.lock();
          if (nullptr == thread_data) {
            break;
          }
          Json::Value json_value;
          if (!restful_common::jsoncpp_helper::ReaderHelper(
                  check_slice_upload_result, json_value)) {
            break;
          }
          const auto is_success =
              restful_common::jsoncpp_helper::GetBool(json_value["isSuccess"]);
          const auto file_data_exists = restful_common::jsoncpp_helper::GetInt(
              json_value["fileDataExists"]);
          if (is_success) {
            thread_data->upload_file_id.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["uploadFileId"]));
            thread_data->file_upload_url.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["fileUploadUrl"]));
            thread_data->file_commit_url.store(
                restful_common::jsoncpp_helper::GetString(
                    json_value["fileCommitUrl"]));
            thread_data->already_upload_bytes.store(
                restful_common::jsoncpp_helper::GetInt64(json_value["size"]));
          }
          if (is_success && file_data_exists) {
            /// 成功且文件可秒传
            check_slice_upload_proof.result = stage_result::Succeeded;
            check_slice_upload_proof.next_stage = uploader_stage::FileCommit;
            break;
          }
          if (is_success) {
            /// 成功且文件不可秒传
            check_slice_upload_proof.result = stage_result::Succeeded;
            check_slice_upload_proof.next_stage = uploader_stage::FileUplaod;
            break;
          }
          const auto http_statuc_code = restful_common::jsoncpp_helper::GetInt(
              json_value["httpStatusCode"]);
          const auto int32_error_code = restful_common::jsoncpp_helper::GetInt(
              json_value["int32ErrorCode"]);

          /// 最坏的失败，无需重试，比如特定的4xx的错误码，情形如：登录信息失效、空间不足等
          if (4 == (http_statuc_code / 100) &&
              (int32_error_code == Cloud189::ErrorCode::nderr_sessionbreak ||
               int32_error_code == Cloud189::ErrorCode::nderr_session_expired ||
               int32_error_code == Cloud189::ErrorCode::nderr_no_diskspace)) {
            check_slice_upload_proof.result = stage_result::GiveupRetry;
            check_slice_upload_proof.next_stage = uploader_stage::UploadFinal;
            break;
          }
          if (602 == http_statuc_code) {
            /// 602 特定错误，需要重新创建续传
            check_slice_upload_proof.result = stage_result::RetryTargetStage;
            check_slice_upload_proof.next_stage = uploader_stage::CreateUpload;
            break;
          }
          /// 除此以外，均应按照指数增幅进行重试，并记录suggest_waittime
          /// 默认的result即是stage_result::RetrySelf
          check_slice_upload_proof.suggest_waittime =
              restful_common::jsoncpp_helper::GetInt(json_value["waitingTime"]);
        } while (false);
        return check_slice_upload_proof;
      };

      /// 生成数据源成功，保存到result
      result =
          obs.map(solve_check_slice_upload).map(get_check_slice_upload_result);
    } while (false);
    return result;
  };
  /// check_upload end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 上传文件指令
  httpbusiness::uploader::proof::ProofObsCallback slice_file_upload;
  slice_file_upload =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    /// TODO: 需要加入分片上传文件的流程 2019.12.9
    /// 初始化请求失败，应如此返回
    rxcpp::observable<uploader_proof> result = rxcpp::observable<>::just(
        uploader_proof{uploader_stage::FileUplaod, stage_result::GiveupRetry,
                       uploader_stage::UploadFinal, 0, 0});

    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      HttpRequest slice_file_upload_request("");
      /// 从线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
      std::string file_path = thread_data->local_filepath;
      std::string file_upload_url = thread_data->file_upload_url.load();
      /// TODO:分片数的计算优化
      int32_t upload_slice_id = 1;
      // int32_t upload_slice_id = thread_data->upload_slice_id;
      /// TODO:分片的md5计算优化
      std::string slice_md5;
      auto solve_slice_md5 = [&slice_md5](const std::string& temp) {
        slice_md5 = temp;
      };
      thread_data->slice_md5_map.FindDelegate(upload_slice_id, solve_slice_md5);
      std::string upload_id = thread_data->upload_file_id.load();
      int64_t start_offset = thread_data->already_upload_bytes;
      int64_t offset_length = thread_data->already_upload_bytes;
      int32_t resume_policy = thread_data->resume_policy;

      std::string slice_file_upload_json_str =
          Cloud189::Apis::UploadSliceData::JsonStringHelper(
              file_upload_url, file_path, upload_id, start_offset,
              offset_length, resume_policy, upload_slice_id, slice_md5);
      /// HttpRequestEncode
      Cloud189::Apis::UploadSliceData::HttpRequestEncode(
          slice_file_upload_json_str, slice_file_upload_request);
      slice_file_upload_request.retval_func =
          [thread_data_weak](uint64_t value) {
            do {
              auto thread_data = thread_data_weak.lock();
              if (nullptr == thread_data) {
                break;
              }
              thread_data->current_upload_bytes.store(value);
            } while (false);
          };
      /// 使用rx_assistant::rx_httpresult::create 创建数据源，下同
      auto obs = rx_assistant::rx_httpresult::create(slice_file_upload_request);

      /// 需提供根据HttpResponse解析得到结果的json字符串
      std::function<std::string(HttpResult&)> solve_slice_file_uplaod =
          [](HttpResult& file_upload_httpResult) -> std::string {
        std::string result;
        Cloud189::Apis::UploadSliceData::HttpResponseDecode(
            file_upload_httpResult.res, file_upload_httpResult.req, result);
        return result;
      };

      /// 提供根据结果的json字符串得到proof的lambda表达式
      std::function<uploader_proof(const std::string&)>
          get_slice_file_uplaod_result =
              [thread_data_weak](
                  const std::string& file_upload_result) -> uploader_proof {
        auto slice_file_uplaod_proof = uploader_proof{
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
          Json::Value json_value;
          if (!restful_common::jsoncpp_helper::ReaderHelper(file_upload_result,
                                                            json_value)) {
            break;
          }
          const auto is_success =
              restful_common::jsoncpp_helper::GetBool(json_value["isSuccess"]);
          if (is_success) {
            /// 成功
            slice_file_uplaod_proof.result = stage_result::Succeeded;
            slice_file_uplaod_proof.next_stage = uploader_stage::FileCommit;
            break;
          }
          slice_file_uplaod_proof.suggest_waittime =
              restful_common::jsoncpp_helper::GetInt(json_value["waitingTime"]);
        } while (false);
        // 除上述情况外的一切情况，提交失败应前往CheckUpload
        return slice_file_uplaod_proof;
      };
      result =
          obs.map(solve_slice_file_uplaod).map(get_slice_file_uplaod_result);
    } while (false);
    return result;
  };
  /// file uplaod end
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 确认上传指令
  httpbusiness::uploader::proof::ProofObsCallback slice_file_commit;
  slice_file_commit =
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
      HttpRequest slice_file_commit_request("");
      /// 从线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
      std::string upload_id = thread_data->last_upload_id;
      int32_t oper_type = thread_data->oper_type;
      int32_t is_log = thread_data->is_log;
      int32_t resume_policy = thread_data->resume_policy;
      std::string file_commit_url = thread_data->file_commit_url.load();
      std::string slice_md5_list = thread_data->md5_list_result.load();
      std::string slice_file_commit_json_str =
          Cloud189::Apis::CommitSliceUploadFile::JsonStringHelper(
              file_commit_url, upload_id, is_log, oper_type, resume_policy,
              slice_md5_list);
      /// HttpRequestEncode
      Cloud189::Apis::CommitSliceUploadFile::HttpRequestEncode(
          slice_file_commit_json_str, slice_file_commit_request);
      /// 使用rx_assistant::rx_httpresult::create 创建数据源
      auto obs = rx_assistant::rx_httpresult::create(slice_file_commit_request);

      /// 提供根据HttpResponse解析得到结果的json字符串
      std::function<std::string(HttpResult&)> solve_slice_file_commit =
          [](HttpResult& file_commit_http_result) -> std::string {
        std::string decode_result;
        Cloud189::Apis::CommitSliceUploadFile::HttpResponseDecode(
            file_commit_http_result.res, file_commit_http_result.req,
            decode_result);
        return decode_result;
      };

      /// 提供根据结果的json字符串得到proof的lambda表达式
      std::function<uploader_proof(const std::string&)>
          get_slice_file_commit_result =
              [thread_data_weak](
                  const std::string& file_commit_result) -> uploader_proof {
        auto slice_file_commit_proof = uploader_proof{
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
          Json::Value json_value;
          if (!restful_common::jsoncpp_helper::ReaderHelper(file_commit_result,
                                                            json_value)) {
            break;
          }
          const auto is_success =
              restful_common::jsoncpp_helper::GetBool(json_value["isSuccess"]);

          /// 确认文件上传完成后解析到的字段
          if (is_success) {
            thread_data->commit_id.store(GetString(json_value["id"]));
            thread_data->commit_name.store(GetString(json_value["name"]));
            thread_data->commit_size.store(GetString(json_value["size"]));
            thread_data->commit_md5.store(GetString(json_value["md5"]));
            thread_data->commit_create_date.store(
                GetString(json_value["createDate"]));
            thread_data->commit_rev.store(GetString(json_value["rev"]));
            thread_data->commit_user_id.store(GetString(json_value["userId"]));
            thread_data->commit_is_safe.store(GetString(json_value["isSafe"]));
          }

          if (is_success) {
            slice_file_commit_proof.result = stage_result::Succeeded;
            slice_file_commit_proof.next_stage = uploader_stage::UploadFinal;
            break;
          }
          const auto http_statuc_code = restful_common::jsoncpp_helper::GetInt(
              json_value["httpStatusCode"]);
          const auto int32_error_code = restful_common::jsoncpp_helper::GetInt(
              json_value["int32ErrorCode"]);
          /// 最坏的失败，无需重试，比如特定的4xx的错误码，情形如：登录信息失效、空间不足等
          if (4 == (http_statuc_code / 100) &&
              (int32_error_code == Cloud189::ErrorCode::nderr_sessionbreak ||
               int32_error_code == Cloud189::ErrorCode::nderr_session_expired ||
               int32_error_code == Cloud189::ErrorCode::nderr_no_diskspace ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_userdayflowoverlimited ||
               int32_error_code == Cloud189::ErrorCode::nderr_no_diskspace ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_over_filesize_error ||
               int32_error_code ==
                   Cloud189::ErrorCode::nderr_invalid_parent_folder)) {
            slice_file_commit_proof.result = stage_result::GiveupRetry;
            slice_file_commit_proof.next_stage = uploader_stage::UploadFinal;
            break;
          }
          slice_file_commit_proof.suggest_waittime =
              restful_common::jsoncpp_helper::GetInt(json_value["waitingTime"]);
        } while (false);
        // 除上述情况外的一切情况，提交失败应前往CheckUpload
        return slice_file_commit_proof;
      };
      result =
          obs.map(solve_slice_file_commit).map(get_slice_file_commit_result);
    } while (false);
    return result;
  };
  /// file uplaod end.
  //////////////////////////////////////////////////////////////////////////
  return httpbusiness::uploader::proof::proof_obs_packages{
      std::move(calculate_md5), std::move(create_slice_upload),
      std::move(check_slice_upload), std::move(slice_file_upload),
      std::move(slice_file_commit)};
}
}  // namespace

namespace Cloud189 {
namespace Restful {
SliceUploader::SliceUploader(
    const std::string& upload_info,
    std::function<void(const std::string&)> complete_callback)
    : thread_data(InitThreadData(upload_info)),
      data(std::make_unique<details::sliceuploader_internal_data>(
          thread_data->local_filepath, GenerateOrders(thread_data),
          GenerateCompleteCallback(thread_data, complete_callback))) {}

void SliceUploader::AsyncStart() {}

void SliceUploader::SyncWait() {}

SliceUploader::~SliceUploader() = default;

/// 为此Uploader提供一个Helper函数，用于生成合规的json字符串
std::string sliceuploader_info_helper(
    const std::string& local_path, const std::string& md5,
    /*const std::string slice_md5, const std::string slice_md5_list,*/
    const std::string& last_upload_id, const std::string& parent_folder_id,
    /*const int32_t upload_slice_id,*/ const int64_t per_slice_size,
    const int32_t resume_policy, const int32_t oper_type,
    const int32_t is_log) {
  Json::Value json_value;

  json_value["localPath"] = local_path;
  json_value["md5"] = md5;
  json_value["uploadFileId"] = last_upload_id;
  json_value["parentFolderId"] = parent_folder_id;
  json_value["perSliceSize"] = per_slice_size;
  json_value["resumePolicy"] = resume_policy;
  json_value["opertype"] = oper_type;
  json_value["isLog"] = is_log;
  return restful_common::jsoncpp_helper::WriterHelper(json_value);
}
}  // namespace Restful
}  // namespace Cloud189
