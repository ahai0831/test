#include "cloud189_uploader.h"
// #define WIN32_LEAN_AND_MEAN
// #include <Assistant_v3.hpp>
/// 避免因Windows.h在WinSock2.h之前被包含导致的问题
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <rx_assistant.hpp>
#include <rx_md5.hpp>
#include <rx_uploader.hpp>
#include <tools/safecontainer.hpp>

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
using rx_assistant::rx_assistant_factory;
using rx_assistant::md5::md5_async_factory;
/// TODO: 既然是源文件，那么对比较复杂的命名空间使用，都可以使用using

namespace Cloud189 {
namespace Restful {

namespace details {

struct uploader_internal_data {
  /// 在此占据某个文件的相应句柄

  /// 以ScopeGuard机制在析构时释放此句柄
  uploader_internal_data() = default;
  ~uploader_internal_data() = default;
};

struct uploader_thread_data {
 public:
  /// const的数据成员，必须在构造函数中进行显式的初始化
  const std::string local_filepath;
  const std::string last_md5;
  const int64_t last_upload_id;
  const int64_t parent_folder_id;
  const int64_t start_offset;
  const int64_t offset_length;
  const int32_t oper_type;
  const int32_t is_log;
  /// 禁用移动复制默认构造和=号操作符，必须通过显式指定所有必须字段进行初始化
  uploader_thread_data(const std::string& file_path,
                       const std::string& last_trans_md5,
                       const int64_t& last_trans_upload_id,
                       const int64_t& parent_folder_id_,
                       const int64_t& start_offset_,
                       const int64_t& offset_length_, const int32_t& oper_type_,
                       const int32_t& is_log_)
      : local_filepath(file_path),
        last_md5(last_trans_md5),
        last_upload_id(last_trans_upload_id),
        parent_folder_id(parent_folder_id_),
        start_offset(start_offset_),
        offset_length(offset_length_),
        oper_type(oper_type_),
        is_log(is_log_) {}
  /// 线程安全的数据成员
  lockfree_string_closure<std::string> file_md5;
  lockfree_string_closure<std::string> file_upload_url;
  lockfree_string_closure<std::string> file_commit_url;
  std::atomic<int64_t> upload_id;
  std::atomic<int64_t> upload_bytes;

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
    /// TODO: 改为jsoncpp_helper内的工具函数进行解析
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
    int32_t oper_type =
        restful_common::jsoncpp_helper::GetInt(json_str["opertype"]);
    int32_t is_log = restful_common::jsoncpp_helper::GetInt(json_str["isLog"]);
    thread_data = std::make_shared<details::uploader_thread_data>(
        local_filepath, last_md5, last_upload_id, parent_folder_id,
        start_offset, offset_length, oper_type, is_log);
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
      HttpRequest create_upload_request("");
      const std::string& file_path = thread_data->local_filepath;
      const std::string file_md5 = thread_data->file_md5.load();
      const int64_t parent_folder_id = thread_data->parent_folder_id;
      const int32_t oper_type = thread_data->oper_type;
      const int32_t is_log = thread_data->is_log;
      const std::string create_upload_json_str =
          Cloud189::Apis::CreateUploadFile::JsonStringHelper(
              parent_folder_id, file_path, file_md5, oper_type, is_log);
      /// HttpRequestEncode
      Cloud189::Apis::CreateUploadFile::HttpRequestEncode(
          create_upload_json_str, create_upload_request);
      /// 使用rx_assistant::rx_httpresult::create 创建数据源，下同
      auto obs = rx_assistant::rx_httpresult::create(create_upload_request);

      /// 提供根据HttpResponse解析得到结果的json字符串
      std::function<std::string(HttpResult&)> solve_create_upload =
          [](HttpResult& create_upload_httpResult) -> std::string {
        std::string result;
        Cloud189::Apis::CreateUploadFile::HttpResponseDecode(
            create_upload_httpResult.res, create_upload_httpResult.req, result);
        return result;
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
          Json::Value json_value;
          Json::CharReaderBuilder reader_builder;
          Json::CharReaderBuilder::strictMode(&reader_builder.settings_);
          std::unique_ptr<Json::CharReader> const reader(
              reader_builder.newCharReader());
          if (nullptr == reader) {
            break;
          }
          if (!reader->parse(
                  create_upload_result.c_str(),
                  create_upload_result.c_str() + create_upload_result.size(),
                  &json_value, nullptr)) {
            break;
          }
          // 				  if (nullptr == thread_data) {
          // 					  break;
          // 				  }
          const auto is_success =
              restful_common::jsoncpp_helper::GetBool(json_value["isSuccess"]);
          const auto file_data_exists = restful_common::jsoncpp_helper::GetBool(
              json_value["fileDataExists"]);
          if (is_success) {
            /// TODO: 保存此续传记录的fileUploadUrl、fileCommitUrl和uploadFileId
            // 			  thread_data->file_upload_url.store(
            // 				  json_value["fileUploadUrl"].asString());
            // 			  thread_data->file_commit_url.store(
            // 				  json_value["fileCommitUrl"].asString());
          }
          if (is_success && file_data_exists) {
            /// 成功且文件可秒传
            create_upload_proof.result = stage_result::Succeeded;
            create_upload_proof.next_stage = uploader_stage::FileCommit;
            /// TODO: 加上对URL的处理
            break;
          }
          if (is_success) {
            /// 成功且文件不存在
            create_upload_proof.result = stage_result::Succeeded;
            create_upload_proof.next_stage = uploader_stage::CheckUpload;
            /// TODO: 加上对URL的处理
            break;
          }
          const auto http_statuc_code = restful_common::jsoncpp_helper::GetInt(
              json_value["httpStatusCode"]);
          const auto int32_error_code = restful_common::jsoncpp_helper::GetInt(
              json_value["int32ErrorCode"]);
          ////
          ///最坏的失败，是无需重试的，比如特定的4xx的错误码，情形如：登录信息失效、空间不足等
          if (4 == (http_statuc_code / 100)) {
            create_upload_proof.result = stage_result::GiveupRetry;
            create_upload_proof.next_stage = uploader_stage::UploadFinal;
            break;
          }

          /// 初次以外，均应按照指数增幅进行重试
          /// 什么也不用做，默认的result即是stage_result::RetrySelf

        } while (false);
        return create_upload_proof;
      };

      /// 生成数据源成功，保存到result
      result = obs.map(solve_create_upload).map(get_create_upload_result);
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
    HttpRequest check_upload_request("");
    auto thread_data = thread_data_weak.lock();
    /// 线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
    int64_t upload_id =
        nullptr != thread_data ? thread_data->last_upload_id : int64_t();
    std::string json_str =
        Cloud189::Apis::GetUploadFileStatus::JsonStringHelper(upload_id);
    /// Encode
    Cloud189::Apis::GetUploadFileStatus::HttpRequestEncode(
        json_str, check_upload_request);
    //     /// 提供一个网络库的Helper工具方法，用于提供网络库对象的弱指针
    //     auto assistant_v3 = std::make_shared<assistant::Assistant_v3>();
    //     auto assistant_weak =
    //     std::weak_ptr<assistant::Assistant_v3>(assistant_v3);
    //     rx_assistant_factory factory(assistant_weak);
    //     auto obs = factory.create(check_upload_request);
    auto obs = rx_assistant::rx_httpresult::create(check_upload_request);

    /// 提供根据HttpResponse解析得到结果的json字符串
    std::function<std::string(HttpResult&)> solve_check_upload =
        [](HttpResult& check_upload_httpResult) -> std::string {
      std::string result;
      Cloud189::Apis::GetUploadFileStatus::HttpResponseDecode(
          check_upload_httpResult.res, check_upload_httpResult.req, result);
      return result;
    };

    /// 提供根据结果的json字符串得到proof的lambda表达式
    std::function<uploader_proof(const std::string&)> get_check_upload_result =
        [](const std::string& check_upload_result) -> uploader_proof {
      auto check_upload_proof =
          uploader_proof{uploader_stage::CheckUpload, stage_result::GiveupRetry,
                         uploader_stage::UploadFinal, 0, 0};
      Json::Value json_value;
      do {
        if (check_upload_result.empty()) {
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
                check_upload_result.c_str(),
                check_upload_result.c_str() + check_upload_result.size(),
                &json_value, nullptr)) {
          break;
        }
        if (json_value["isSuccess"].asBool()) {
          if (json_value["fileDataExists"].asBool()) {
            /// 成功且文件存在
            check_upload_proof.result = stage_result::Succeeded;
            check_upload_proof.next_stage = uploader_stage::FileCommit;
            break;
          } else {
            /// 成功且文件不存在
            check_upload_proof.result = stage_result::Succeeded;
            check_upload_proof.next_stage = uploader_stage::FileUplaod;
            break;
          }
        }
        auto http_statuc_code = json_value["httpStatusCode"].asInt();
        auto int32_error_code = json_value["int32ErrorCode"].asInt();
        if (4 == (http_statuc_code / 100) &&
            (int32_error_code == ErrorCode::nderr_infosecurityerrorcode ||
             int32_error_code == ErrorCode::nderr_permission_denied)) {
          /// 4XX 权限不足或信安相关错误重试一次
          check_upload_proof.result = stage_result::RetrySelf;
          check_upload_proof.next_stage = uploader_stage::CheckUpload;
        }
        if ((5 == (http_statuc_code / 100))) {
          /// 5XX 按重试规则重试
          check_upload_proof.result = stage_result::RetrySelf;
          check_upload_proof.next_stage = uploader_stage::CheckUpload;
          break;
        }
        if (601 == http_statuc_code) {
          /// 601 按重试规则重试
          check_upload_proof.result = stage_result::RetrySelf;
          check_upload_proof.next_stage = uploader_stage::CheckUpload;
          check_upload_proof.suggest_waittime =
              json_value["waitingTime"].asInt();
          break;
        }
        if (602 == http_statuc_code) {
          /// 602 重新创建续传
          check_upload_proof.result = stage_result::RetryTargetStage;
          check_upload_proof.next_stage = uploader_stage::CreateUpload;
          break;
        }
      } while (false);
      check_upload_proof.transfered_length = json_value["size"].asInt64();
      return check_upload_proof;
    };
    return obs.map(solve_check_upload).map(get_check_upload_result);
  };
  /// check_upload end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 上传文件指令
  httpbusiness::uploader::proof::ProofObsCallback file_upload;
  file_upload =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    HttpRequest file_upload_request("");
    auto thread_data = thread_data_weak.lock();
    /// 从线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
    std::string file_path =
        nullptr != thread_data ? thread_data->local_filepath : std::string();
    int64_t upload_id =
        nullptr != thread_data ? thread_data->last_upload_id : int64_t();
    int64_t start_offset =
        nullptr != thread_data ? thread_data->start_offset : int64_t();
    int64_t offset_length =
        nullptr != thread_data ? thread_data->offset_length : int64_t();
    std::string file_upload_url = nullptr != thread_data
                                      ? thread_data->file_upload_url.load()
                                      : std::string();
    std::string json_str = Cloud189::Apis::UploadFileData::JsonStringHelper(
        file_upload_url, file_path, upload_id, start_offset, offset_length);
    /// Encode
    Cloud189::Apis::UploadFileData::HttpRequestEncode(json_str,
                                                      file_upload_request);
    file_upload_request.retval_func = [thread_data_weak](uint64_t value) {
      do {
        auto thread_data = thread_data_weak.lock();
        if (nullptr == thread_data) {
          break;
        }
        thread_data->upload_bytes += value;
      } while (false);
    };
    //     /// 提供一个网络库的Helper工具方法，用于提供网络库对象的弱指针
    //     auto assistant_v3 = std::make_shared<assistant::Assistant_v3>();
    //     auto assistant_weak =
    //     std::weak_ptr<assistant::Assistant_v3>(assistant_v3);
    //     rx_assistant_factory factory(assistant_weak);
    //     auto obs = factory.create(file_upload_request);
    auto obs = rx_assistant::rx_httpresult::create(file_upload_request);

    /// 需提供根据HttpResponse解析得到结果的json字符串
    std::function<std::string(HttpResult&)> solve_file_uplaod =
        [](HttpResult& file_upload_httpResult) -> std::string {
      std::string result;
      Cloud189::Apis::UploadFileData::HttpResponseDecode(
          file_upload_httpResult.res, file_upload_httpResult.req, result);
      return result;
    };

    /// 提供根据结果的json字符串得到proof的lambda表达式
    std::function<uploader_proof(const std::string&)> get_file_uplaod_result =
        [thread_data_weak](
            const std::string& file_upload_result) -> uploader_proof {
      auto thread_data = thread_data_weak.lock();
      int64_t upload_bytes =
          nullptr != thread_data ? thread_data->upload_bytes.load() : int64_t();
      auto file_uplaod_proof = uploader_proof{
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
          file_uplaod_proof.result = stage_result::Succeeded;
          file_uplaod_proof.next_stage = uploader_stage::FileCommit;
          break;
        }
      } while (false);
      // 除上述情况外的一切情况，提交失败应前往CheckUpload
      file_uplaod_proof.transfered_length += upload_bytes;
      return file_uplaod_proof;
    };
    return obs.map(solve_file_uplaod).map(get_file_uplaod_result);
  };
  /// file uplaod end.
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  /// 确认上传指令
  httpbusiness::uploader::proof::ProofObsCallback file_commit;
  file_commit =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    HttpRequest file_commit_request("");
    /// 从线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理
    auto thread_data = thread_data_weak.lock();
    int64_t upload_id =
        nullptr != thread_data ? thread_data->last_upload_id : int64_t();
    int32_t oper_type =
        nullptr != thread_data ? thread_data->oper_type : int32_t();
    int32_t is_log = nullptr != thread_data ? thread_data->is_log : int32_t();
    std::string file_commit_url = nullptr != thread_data
                                      ? thread_data->file_commit_url.load()
                                      : std::string();
    std::string json_str =
        Cloud189::Apis::ComfirmUploadFileComplete::JsonStringHelper(
            file_commit_url, upload_id, oper_type, is_log);
    /// Encode
    Cloud189::Apis::ComfirmUploadFileComplete::HttpRequestEncode(
        json_str, file_commit_request);

    //     /// 提供一个网络库的Helper工具方法，用于提供网络库对象的弱指针
    //     auto assistant_v3 = std::make_shared<assistant::Assistant_v3>();
    //     auto assistant_weak =
    //     std::weak_ptr<assistant::Assistant_v3>(assistant_v3);
    //     rx_assistant_factory factory(assistant_weak);
    //     auto obs = factory.create(file_commit_request);
    auto obs = rx_assistant::rx_httpresult::create(file_commit_request);

    /// 提供根据HttpResponse解析得到结果的json字符串
    std::function<std::string(HttpResult&)> solve_file_commit =
        [](HttpResult& file_commit_httpResult) -> std::string {
      std::string result;
      Cloud189::Apis::ComfirmUploadFileComplete::HttpResponseDecode(
          file_commit_httpResult.res, file_commit_httpResult.req, result);
      return result;
    };

    /// 提供根据结果的json字符串得到proof的lambda表达式
    std::function<uploader_proof(const std::string&)> get_file_commit_result =
        [](const std::string& file_commit_result) -> uploader_proof {
      auto file_commit_proof = uploader_proof{
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
          file_commit_proof.result = stage_result::Succeeded;
          file_commit_proof.next_stage = uploader_stage::UploadFinal;
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
          file_commit_proof.result = stage_result::GiveupRetry;
          file_commit_proof.next_stage = uploader_stage::UploadFinal;
          break;
        }
        // 除上述情况外的一切情况，提交失败应前往CheckUpload
      } while (false);
      return file_commit_proof;
    };
    return obs.map(solve_file_commit).map(get_file_commit_result);
  };
  /// file uplaod end.
  //////////////////////////////////////////////////////////////////////////
}

Uploader::~Uploader() = default;

/// 为此Uploader提供一个Helper函数，用于生成合规的json字符串
std::string uploader_info_helper(
    const std::string& local_path, const std::string& md5,
    const int64_t last_upload_id, const int64_t parent_folder_id,
    const int64_t start_offset, const int64_t offset_length,
    const int32_t oper_type, const int32_t is_log) {
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
    json_value["opertype"] = oper_type;
    json_value["isLog"] = is_log;
    uploader_json_str = Json::writeString(wbuilder, json_value);
  } while (false);
  return uploader_json_str;
}
}  // namespace Restful
}  // namespace Cloud189
