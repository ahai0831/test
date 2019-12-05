#include "cloud189_slice_uploader.h"

/// TODO: 需要对rx_md5进行调整，避免引用顺序导致的编译问题
#include <json/json.h>
#include <rx_md5.hpp>

#include <rx_uploader.hpp>

#include <rx_assistant.hpp>
#include <tools/safecontainer.hpp>

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
using rx_assistant::HttpResult;
using rx_assistant::rx_assistant_factory;
using rx_assistant::md5::md5_async_factory;

namespace Cloud189 {
namespace RestfulSlice {

namespace details {

struct uploader_internal_data {
  /// 在此占据某个文件的相应句柄

  /// 以ScopeGuard机制在析构时释放此句柄
  uploader_internal_data() = default;
  ~uploader_internal_data() = default;
};
// typedef httpbusiness::uploader::rx_uploader rx_uploader;
//
// httpbusiness::uploader::proof::proof_obs_packages pkgs;

struct uploader_thread_data {
 public:
  /// const的数据成员，必须在构造函数中进行显式的初始化
  const std::string local_filepath;
  const std::string last_md5;
  const int64_t last_upload_id;
  const int64_t parent_folder_id;
  const int64_t start_offset;
  const int64_t offset_length;
  const int32_t upload_slice_id;
  const int64_t per_slice_size;
  const int32_t resume_policy;
  const int32_t oper_type;
  const int32_t is_log;
  /// 禁用移动复制默认构造和=号操作符，必须通过显式指定所有必须字段进行初始化
  uploader_thread_data(
      const std::string& file_path, const std::string& last_trans_md5,
      const int64_t& last_trans_upload_id, const int64_t& parent_folder_id_,
      const int64_t& start_offset_, const int64_t& offset_length_,
      const int32_t& upload_slice_id_, const int64_t& per_slice_size_,
      const int32_t& resume_policy_, const int32_t& oper_type_,
      const int32_t& is_log_)
      : local_filepath(file_path),
        last_md5(last_trans_md5),
        last_upload_id(last_trans_upload_id),
        parent_folder_id(parent_folder_id_),
        start_offset(start_offset_),
        offset_length(offset_length_),
        upload_slice_id(upload_slice_id_),
        per_slice_size(per_slice_size_),
        resume_policy(resume_policy_),
        oper_type(oper_type_),
        is_log(is_log_) {}
  /// 线程安全的数据成员
  lockfree_string_closure<std::string> file_md5;
  lockfree_string_closure<std::string> file_upload_url;
  lockfree_string_closure<std::string> file_commit_url;
  lockfree_string_closure<std::string> md5_list_result;
  assistant::tools::safemap_closure<int32_t, std::string> slice_md5_map;
  std::atomic<int64_t> upload_id;
  std::atomic<int64_t> upload_bytes;
  decltype(md5_async_factory::create("", nullptr, nullptr)) md5_unique;

 private:
  uploader_thread_data() = delete;
  uploader_thread_data(uploader_thread_data const&) = delete;
  uploader_thread_data& operator=(uploader_thread_data const&) = delete;
  uploader_thread_data(uploader_thread_data&&) = delete;
};

}  // namespace details

Uploader::Uploader(const std::string& upload_info) {
  /// 根据传入信息，初始化线程信息结构体
  do {
    if (upload_info.empty()) {
      break;
    }
    Json::Value json_str;
    Json::CharReaderBuilder reader_builder;
    Json::CharReaderBuilder::strictMode(&reader_builder.settings_);
    std::unique_ptr<Json::CharReader> const reader(
        reader_builder.newCharReader());
    if (nullptr == reader) {
      break;
    }
    if (!reader->parse(upload_info.c_str(),
                       upload_info.c_str() + upload_info.size(), &json_str,
                       nullptr)) {
      break;
    }
    std::string local_filepath =
        restful_common::jsoncpp_helper::GetString(json_str["localPath"]);
    std::string last_md5 =
        restful_common::jsoncpp_helper::GetString(json_str["md5"]);
    int64_t last_upload_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["uploadFileId"]);
    int64_t parent_folder_id =
        restful_common::jsoncpp_helper::GetInt64(json_str["parentFolderId"]);
    int64_t start_offset =
        restful_common::jsoncpp_helper::GetInt64(json_str["startOffset"]);
    int64_t offset_length =
        restful_common::jsoncpp_helper::GetInt64(json_str["offsetLength"]);
    int32_t upload_slice_id =
        restful_common::jsoncpp_helper::GetInt(json_str["uploadSliceId"]);
    int64_t per_slice_size =
        restful_common::jsoncpp_helper::GetInt64(json_str["perSliceSize"]);
    int32_t resume_policy =
        restful_common::jsoncpp_helper::GetInt(json_str["resumePolicy"]);
    int32_t oper_type =
        restful_common::jsoncpp_helper::GetInt(json_str["opertype"]);
    int32_t is_log = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);
    std::string slice_md5_list;
    std::string slice_md5;
    auto solve_slice_md5 = [&slice_md5](const std::string& temp) {
      slice_md5 = temp;
    };
    thread_data->slice_md5_map.FindDelegate(upload_slice_id, solve_slice_md5);
    thread_data = std::make_shared<details::uploader_thread_data>(
        local_filepath, last_md5, last_upload_id, parent_folder_id,
        start_offset, offset_length, upload_slice_id, per_slice_size,
        resume_policy, oper_type, is_log);
  } while (false);
  /// 根据弱指针，初始化各RX指令
  auto thread_data_weak =
      std::weak_ptr<details::uploader_thread_data>(thread_data);
  //////////////////////////////////////////////////////////////////////////
  /// 计算MD5指令
  /// 输入proof，根据线程信息中的路径信息，异步地进行MD5计算并返回proof
  httpbusiness::uploader::proof::ProofObsCallback calculate_md5;
  calculate_md5 =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    return rxcpp::observable<>::create<uploader_proof>(
        [thread_data_weak](rxcpp::subscriber<uploader_proof> s) -> void {
          auto thread_data = thread_data_weak.lock();
          std::string file_path = nullptr != thread_data
                                      ? thread_data->local_filepath
                                      : std::string();
          std::function<void(const std::string&)> md5_result_callback =
              [s, thread_data_weak](const std::string& md5) -> void {
            uploader_proof calculate_md5_result_proof = {
                uploader_stage::CalculateMd5, stage_result::GiveupRetry,
                uploader_stage::UploadInitial, 0, 0};
            do {
              auto thread_data = thread_data_weak.lock();
              if (nullptr == thread_data) {
                break;
              }
              if (!md5.empty() && thread_data->last_md5 == md5) {
                thread_data->file_md5 = md5;
                calculate_md5_result_proof.result = stage_result::Succeeded;
                calculate_md5_result_proof.next_stage =
                    uploader_stage::CheckUpload;
                break;
              }
              if (!md5.empty()) {
                thread_data->file_md5 = md5;
                calculate_md5_result_proof.result = stage_result::Succeeded;
                calculate_md5_result_proof.next_stage =
                    uploader_stage::CreateUpload;
                break;
              }
            } while (false);
            s.on_next(calculate_md5_result_proof);
            s.on_completed();
          };
          auto md5_unique =
              md5_async_factory::create(file_path, md5_result_callback);
          if (nullptr != thread_data) {
            thread_data->md5_unique = std::move(md5_unique);
          }
        });
  };
  /// calculate_md5 end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 创建续传指令
  /// 输入proof，根据线程信息中的路径信息，异步地进行MD5计算并返回proof
  httpbusiness::uploader::proof::ProofObsCallback create_slice_upload;
  create_slice_upload =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    HttpRequest create_slice_upload_request("");
    auto thread_data = thread_data_weak.lock();
    /// 线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
    std::string file_path =
        nullptr != thread_data ? thread_data->local_filepath : std::string();
    std::string file_md5 =
        nullptr != thread_data ? thread_data->last_md5 : std::string();
    int64_t parent_folder_id =
        nullptr != thread_data ? thread_data->parent_folder_id : int64_t();
    int32_t oper_type =
        nullptr != thread_data ? thread_data->oper_type : int32_t();
    int32_t is_log = nullptr != thread_data ? thread_data->is_log : int32_t();
    std::string json_str =
        Cloud189::Apis::CreateSliceUploadFile::JsonStringHelper(
            file_path, parent_folder_id, file_md5, is_log, oper_type);
    /// Encode
    Cloud189::Apis::CreateSliceUploadFile::HttpRequestEncode(
        json_str, create_slice_upload_request);
    /// 提供一个网络库的Helper工具方法，用于提供网络库对象的弱指针
    auto assistant_v3 = std::make_shared<assistant::Assistant_v3>();
    auto assistant_weak = std::weak_ptr<assistant::Assistant_v3>(assistant_v3);
    rx_assistant_factory factory(assistant_weak);
    auto obs = factory.create(create_slice_upload_request);

    /// 提供根据HttpResponse解析得到结果的json字符串
    std::function<std::string(HttpResult&)> solve_create_slice_upload =
        [](HttpResult& create_slice_upload_httpResult) -> std::string {
      std::string result;
      Cloud189::Apis::CreateSliceUploadFile::HttpResponseDecode(
          create_slice_upload_httpResult.res,
          create_slice_upload_httpResult.req, result);
      return result;
    };

    /// 提供根据结果的json字符串得到proof的lambda表达式
    std::function<uploader_proof(const std::string&)>
        get_create_slice_upload_result =
            [thread_data](const std::string& create_slice_upload_result)
        -> uploader_proof {
      auto create_slice_upload_proof = uploader_proof{
          uploader_stage::CreateUpload, stage_result::GiveupRetry,
          uploader_stage::UploadFinal, 0, 0};
      Json::Value json_value;
      do {
        if (create_slice_upload_result.empty()) {
          break;
        }
        Json::CharReaderBuilder reader_builder;
        Json::CharReaderBuilder::strictMode(&reader_builder.settings_);
        std::unique_ptr<Json::CharReader> const reader(
            reader_builder.newCharReader());
        if (nullptr == reader) {
          break;
        }
        if (!reader->parse(create_slice_upload_result.c_str(),
                           create_slice_upload_result.c_str() +
                               create_slice_upload_result.size(),
                           &json_value, nullptr)) {
          break;
        }
        if (nullptr == thread_data) {
          break;
        }
        if (json_value["isSuccess"].asBool()) {
          if (json_value["fileDataExists"].asBool()) {
            /// 成功且文件存在
            create_slice_upload_proof.result = stage_result::Succeeded;
            create_slice_upload_proof.next_stage = uploader_stage::FileCommit;
            break;
          } else {
            /// 成功且文件不存在
            create_slice_upload_proof.result = stage_result::Succeeded;
            create_slice_upload_proof.next_stage = uploader_stage::CheckUpload;
            break;
          }
        }
        auto http_statuc_code = json_value["httpStatusCode"].asInt();
        auto int32_error_code = json_value["int32ErrorCode"].asInt();
        if (4 == (http_statuc_code / 100) &&
            ((int32_error_code == ErrorCode::nderr_infosecurityerrorcode ||
              int32_error_code == ErrorCode::nderr_permission_denied))) {
          /// 4XX 权限不足或信安相关错误重试一次
          create_slice_upload_proof.result = stage_result::RetrySelf;
          create_slice_upload_proof.next_stage = uploader_stage::CreateUpload;
          break;
        }
        if ((5 == (http_statuc_code / 100))) {
          /// 5XX 按重试规则重试
          create_slice_upload_proof.result = stage_result::RetrySelf;
          create_slice_upload_proof.next_stage = uploader_stage::CreateUpload;
          break;
        }
        if (601 == http_statuc_code) {
          /// 601 按重试规则重试
          create_slice_upload_proof.result = stage_result::RetrySelf;
          create_slice_upload_proof.next_stage = uploader_stage::CreateUpload;
          create_slice_upload_proof.suggest_waittime =
              json_value["waitingTime"].asInt();
          break;
        }
        if (602 == http_statuc_code) {
          /// 602 重新创建续传
          create_slice_upload_proof.result = stage_result::RetryTargetStage;
          create_slice_upload_proof.next_stage = uploader_stage::CreateUpload;
          break;
        }
      } while (false);
      thread_data->file_upload_url.store(
          json_value["fileUploadUrl"].asString());
      thread_data->file_commit_url.store(
          json_value["fileCommitUrl"].asString());
      return create_slice_upload_proof;
    };
    return obs.map(solve_create_slice_upload)
        .map(get_create_slice_upload_result);
  };
  /// create_upload end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  ///获取续传状态指令
  httpbusiness::uploader::proof::ProofObsCallback check_slice_upload;
  check_slice_upload =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    HttpRequest check_slice_upload_request("");
    auto thread_data = thread_data_weak.lock();
    /// 线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
    int64_t upload_id =
        nullptr != thread_data ? thread_data->last_upload_id : int64_t();
    std::string json_str =
        Cloud189::Apis::GetSliceUploadStatus::JsonStringHelper(upload_id);
    /// Encode
    Cloud189::Apis::GetSliceUploadStatus::HttpRequestEncode(
        json_str, check_slice_upload_request);
    /// 提供一个网络库的Helper工具方法，用于提供网络库对象的弱指针
    auto assistant_v3 = std::make_shared<assistant::Assistant_v3>();
    auto assistant_weak = std::weak_ptr<assistant::Assistant_v3>(assistant_v3);
    rx_assistant_factory factory(assistant_weak);
    auto obs = factory.create(check_slice_upload_request);

    /// 提供根据HttpResponse解析得到结果的json字符串
    std::function<std::string(HttpResult&)> solve_check_slice_upload =
        [](HttpResult& check_slice_upload_httpResult) -> std::string {
      std::string result;
      Cloud189::Apis::GetSliceUploadStatus::HttpResponseDecode(
          check_slice_upload_httpResult.res, check_slice_upload_httpResult.req,
          result);
      return result;
    };
    /// 提供根据结果的json字符串得到proof的lambda表达式
    std::function<uploader_proof(const std::string&)>
        get_check_slice_upload_result =
            [](const std::string& check_slice_upload_result) -> uploader_proof {
      auto check_slice_upload_proof =
          uploader_proof{uploader_stage::CheckUpload, stage_result::GiveupRetry,
                         uploader_stage::UploadFinal, 0, 0};
      Json::Value json_value;
      do {
        if (check_slice_upload_result.empty()) {
          break;
        }
        Json::CharReaderBuilder reader_builder;
        Json::CharReaderBuilder::strictMode(&reader_builder.settings_);
        std::unique_ptr<Json::CharReader> const reader(
            reader_builder.newCharReader());
        if (nullptr == reader) {
          break;
        }
        if (!reader->parse(check_slice_upload_result.c_str(),
                           check_slice_upload_result.c_str() +
                               check_slice_upload_result.size(),
                           &json_value, nullptr)) {
          break;
        }
        if (json_value["isSuccess"].asBool()) {
          if (json_value["fileDataExists"].asBool()) {
            /// 成功且文件存在
            check_slice_upload_proof.result = stage_result::Succeeded;
            check_slice_upload_proof.next_stage = uploader_stage::FileCommit;
            break;
          } else {
            /// 成功且文件不存在
            check_slice_upload_proof.result = stage_result::Succeeded;
            check_slice_upload_proof.next_stage = uploader_stage::FileUplaod;
            break;
          }
        }
        auto http_statuc_code = json_value["httpStatusCode"].asInt();
        auto int32_error_code = json_value["int32ErrorCode"].asInt();
        if (4 == (http_statuc_code / 100) &&
            (int32_error_code == ErrorCode::nderr_infosecurityerrorcode ||
             int32_error_code == ErrorCode::nderr_permission_denied)) {
          /// 4XX 权限不足或信安相关错误重试一次
          check_slice_upload_proof.result = stage_result::RetrySelf;
          check_slice_upload_proof.next_stage = uploader_stage::CheckUpload;
        }
        if ((5 == (http_statuc_code / 100))) {
          /// 5XX 按重试规则重试
          check_slice_upload_proof.result = stage_result::RetrySelf;
          check_slice_upload_proof.next_stage = uploader_stage::CheckUpload;
          break;
        }
        if (601 == http_statuc_code) {
          /// 601 按重试规则重试
          check_slice_upload_proof.result = stage_result::RetrySelf;
          check_slice_upload_proof.next_stage = uploader_stage::CheckUpload;
          check_slice_upload_proof.suggest_waittime =
              json_value["waitingTime"].asInt();
          break;
        }
        if (602 == http_statuc_code) {
          /// 602 重新创建续传
          check_slice_upload_proof.result = stage_result::RetryTargetStage;
          check_slice_upload_proof.next_stage = uploader_stage::CreateUpload;
          break;
        }
      } while (false);
      check_slice_upload_proof.transfered_length = json_value["size"].asInt64();
      return check_slice_upload_proof;
    };
    return obs.map(solve_check_slice_upload).map(get_check_slice_upload_result);
  };
  /// check_upload end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 上传文件指令
  httpbusiness::uploader::proof::ProofObsCallback slice_file_upload;
  slice_file_upload =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    HttpRequest slice_file_upload_request("");
    auto thread_data = thread_data_weak.lock();
    /// 从线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
    std::string file_path =
        nullptr != thread_data ? thread_data->local_filepath : std::string();
    std::string file_upload_url = nullptr != thread_data
                                      ? thread_data->file_upload_url.load()
                                      : std::string();
    int32_t upload_slice_id =
        nullptr != thread_data ? thread_data->upload_slice_id : int32_t();
    std::string slice_md5;
    auto solve_slice_md5 = [&slice_md5](const std::string& temp) {
      slice_md5 = temp;
    };
    thread_data->slice_md5_map.FindDelegate(upload_slice_id, solve_slice_md5);
    int64_t upload_id =
        nullptr != thread_data ? thread_data->last_upload_id : int64_t();
    int64_t start_offset =
        nullptr != thread_data ? thread_data->start_offset : int64_t();
    int64_t offset_length =
        nullptr != thread_data ? thread_data->offset_length : int64_t();
    int32_t resume_policy =

        nullptr != thread_data ? thread_data->resume_policy : int32_t();

    std::string json_str = Cloud189::Apis::UploadSliceData::JsonStringHelper(
        file_upload_url, file_path, upload_id, start_offset, offset_length,
        resume_policy, upload_slice_id, slice_md5);
    /// Encode
    Cloud189::Apis::UploadSliceData::HttpRequestEncode(
        json_str, slice_file_upload_request);
    slice_file_upload_request.retval_func = [thread_data_weak](uint64_t value) {
      do {
        auto thread_data = thread_data_weak.lock();
        if (nullptr == thread_data) {
          break;
        }
        thread_data->upload_bytes += value;
      } while (false);
    };
    /// 提供一个网络库的Helper工具方法，用于提供网络库对象的弱指针
    auto assistant_v3 = std::make_shared<assistant::Assistant_v3>();
    auto assistant_weak = std::weak_ptr<assistant::Assistant_v3>(assistant_v3);
    rx_assistant_factory factory(assistant_weak);
    auto obs = factory.create(slice_file_upload_request);

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
      auto thread_data = thread_data_weak.lock();
      int64_t upload_bytes =
          nullptr != thread_data ? thread_data->upload_bytes.load() : int64_t();
      auto slice_file_uplaod_proof = uploader_proof{
          uploader_stage::FileUplaod, stage_result::RetryTargetStage,
          uploader_stage::CheckUpload, 0, upload_bytes};
      Json::Value json_value;
      do {
        if (file_upload_result.empty()) {
          break;
        }
        Json::CharReaderBuilder reader_builder;
        Json::CharReaderBuilder::strictMode(&reader_builder.settings_);
        std::unique_ptr<Json::CharReader> const reader(
            reader_builder.newCharReader());
        if (nullptr == reader) {
          break;
        }
        if (!reader->parse(
                file_upload_result.c_str(),
                file_upload_result.c_str() + file_upload_result.size(),
                &json_value, nullptr)) {
          break;
        }
        if (json_value["isSuccess"].asBool()) {
          /// 成功
          slice_file_uplaod_proof.result = stage_result::Succeeded;
          slice_file_uplaod_proof.next_stage = uploader_stage::FileCommit;
          break;
        }
      } while (false);
      // 除上述情况外的一切情况，提交失败应前往CheckUpload
      slice_file_uplaod_proof.transfered_length += upload_bytes;
      return slice_file_uplaod_proof;
    };
    return obs.map(solve_slice_file_uplaod).map(get_slice_file_uplaod_result);
  };
  /// file uplaod end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 确认上传指令
  httpbusiness::uploader::proof::ProofObsCallback slice_file_commit;
  slice_file_commit =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    HttpRequest slice_file_commit_request("");
    /// 从线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
    auto thread_data = thread_data_weak.lock();
    int64_t upload_id =
        nullptr != thread_data ? thread_data->last_upload_id : int64_t();
    int32_t oper_type =
        nullptr != thread_data ? thread_data->oper_type : int32_t();
    int32_t is_log = nullptr != thread_data ? thread_data->is_log : int32_t();
    int32_t resume_policy =
        nullptr != thread_data ? thread_data->resume_policy : int32_t();
    std::string file_commit_url = nullptr != thread_data
                                      ? thread_data->file_commit_url.load()
                                      : std::string();
    std::string slice_md5_list = nullptr != thread_data
                                     ? thread_data->md5_list_result.load()
                                     : std::string();
    std::string json_str =
        Cloud189::Apis::CommitSliceUploadFile::JsonStringHelper(
            file_commit_url, upload_id, is_log, oper_type, resume_policy,
            slice_md5_list);
    /// Encode
    Cloud189::Apis::CommitSliceUploadFile::HttpRequestEncode(
        json_str, slice_file_commit_request);

    /// 提供一个网络库的Helper工具方法，用于提供网络库对象的弱指针
    auto assistant_v3 = std::make_shared<assistant::Assistant_v3>();
    auto assistant_weak = std::weak_ptr<assistant::Assistant_v3>(assistant_v3);
    rx_assistant_factory factory(assistant_weak);
    auto obs = factory.create(slice_file_commit_request);

    /// 提供根据HttpResponse解析得到结果的json字符串
    std::function<std::string(HttpResult&)> solve_slice_file_commit =
        [](HttpResult& file_commit_httpResult) -> std::string {
      std::string result;
      Cloud189::Apis::CommitSliceUploadFile::HttpResponseDecode(
          file_commit_httpResult.res, file_commit_httpResult.req, result);
      return result;
    };

    /// 提供根据结果的json字符串得到proof的lambda表达式
    std::function<uploader_proof(const std::string&)>
        get_slice_file_commit_result =
            [](const std::string& file_commit_result) -> uploader_proof {
      auto slice_file_commit_proof = uploader_proof{
          uploader_stage::FileCommit, stage_result::RetryTargetStage,
          uploader_stage::CheckUpload, 0, 0};
      Json::Value json_value;
      do {
        if (file_commit_result.empty()) {
          break;
        }
        Json::CharReaderBuilder reader_builder;
        Json::CharReaderBuilder::strictMode(&reader_builder.settings_);
        std::unique_ptr<Json::CharReader> const reader(
            reader_builder.newCharReader());
        if (nullptr == reader) {
          break;
        }
        if (!reader->parse(
                file_commit_result.c_str(),
                file_commit_result.c_str() + file_commit_result.size(),
                &json_value, nullptr)) {
          break;
        }
        if (json_value["isSuccess"].asBool()) {
          /// 成功
          slice_file_commit_proof.result = stage_result::Succeeded;
          slice_file_commit_proof.next_stage = uploader_stage::UploadFinal;
          break;
        }
        auto http_statuc_code = json_value["httpStatusCode"].asInt();
        auto int32_error_code = json_value["int32ErrorCode"].asInt();
        if (4 == (http_statuc_code / 100) &&
            (int32_error_code == ErrorCode::nderr_userdayflowoverlimited ||
             int32_error_code == ErrorCode::nderr_no_diskspace ||
             int32_error_code == ErrorCode::nderr_over_filesize_error ||
             int32_error_code == ErrorCode::nderr_invalid_parent_folder ||
             int32_error_code ==
                 ErrorCode::nderr_errordownloadfileinvalidsessionkey)) {
          /// 4XX 以上错误不重试直接结束
          slice_file_commit_proof.result = stage_result::GiveupRetry;
          slice_file_commit_proof.next_stage = uploader_stage::UploadFinal;
          break;
        }
        // 除上述情况外的一切情况，提交失败应前往CheckUpload
      } while (false);
      return slice_file_commit_proof;
    };
    return obs.map(solve_slice_file_commit).map(get_slice_file_commit_result);
  };
  /// file uplaod end.
  //////////////////////////////////////////////////////////////////////////
}

Uploader::~Uploader() = default;

/// 为此Uploader提供一个Helper函数，用于生成合规的json字符串
std::string uploader_info_helper(
    const std::string& local_path, const std::string& md5,
    const std::string slice_md5, const std::string slice_md5_list,
    const int64_t last_upload_id, const int64_t parent_folder_id,
    const int64_t start_offset, const int64_t offset_length,
    const int32_t upload_slice_id, const int64_t per_slice_size,
    const int32_t resume_policy, const int32_t oper_type,
    const int32_t is_log) {
  std::string uploader_json_str = "";
  do {
    if (local_path.empty() || md5.empty()) {
      break;
    }
    Json::Value json_value;
    Json::StreamWriterBuilder wbuilder;
    wbuilder.settings_["indentation"] = "";
    json_value["localPath"] = local_path;
    json_value["md5"] = md5;
    json_value["uploadFileId"] = last_upload_id;
    json_value["parentFolderId"] = parent_folder_id;
    json_value["startOffset"] = start_offset;
    json_value["offsetLength"] = offset_length;
    json_value["uploadSliceId"] = upload_slice_id;
    json_value["perSliceSize"] = per_slice_size;
    json_value["resumePolicy"] = resume_policy;
    json_value["opertype"] = oper_type;
    json_value["isLog"] = is_log;
    uploader_json_str = Json::writeString(wbuilder, json_value);
  } while (false);
  return uploader_json_str;
}
}  // namespace RestfulSlice
}  // namespace Cloud189
