#include "cloud189_uploader.h"

/// TODO: 需要对rx_md5进行调整，避免引用顺序导致的编译问题
#include <rx_md5.hpp>

#include <rx_uploader.hpp>

#include <rx_assistant.hpp>
#include <tools/safecontainer.hpp>

using assistant::HttpRequest;
using assistant::tools::lockfree_string_closure;
using httpbusiness::uploader::proof::stage_result;
using httpbusiness::uploader::proof::uploader_proof;
using httpbusiness::uploader::proof::uploader_stage;
using rx_assistant::HttpResult;
using rx_assistant::rx_assistant_factory;
using rx_assistant::md5::md5_async_factory;

namespace Cloud189 {
namespace Restful {

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
  const std::string last_upload_id;
  /// 禁用移动复制默认构造和=号操作符，必须通过显式指定所有必须字段进行初始化
  uploader_thread_data(const std::string& file_path,
                       const std::string& last_trans_md5,
                       const std::string& last_trans_upload_id)
      : local_filepath(file_path),
        last_md5(last_trans_md5),
        last_upload_id(last_trans_upload_id) {}
  /// 线程安全的数据成员
  lockfree_string_closure<std::string> file_md5;
  lockfree_string_closure<std::string> upload_id;
  decltype(md5_async_factory::create("", nullptr, nullptr)) md5_unique;

 private:
  uploader_thread_data() = delete;
  uploader_thread_data(uploader_thread_data const&) = delete;
  uploader_thread_data& operator=(uploader_thread_data const&) = delete;
  uploader_thread_data(uploader_thread_data&&) = delete;
};

}  // namespace details

Uploader::Uploader(const std::string& upload_info) {
  /// TODO: 根据传入信息，初始化线程信息结构体
  if (true) {
    thread_data = std::make_shared<details::uploader_thread_data>("", "", "");
  }

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
              if (!md5.empty() && thread_data->last_md5 == md5 &&
                  !thread_data->last_upload_id.empty()) {
                thread_data->file_md5 = md5;
                thread_data->upload_id = thread_data->last_upload_id;
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
  httpbusiness::uploader::proof::ProofObsCallback create_upload;
  create_upload =
      [thread_data_weak](uploader_proof) -> rxcpp::observable<uploader_proof> {
    HttpRequest create_upload_request("");
    /// TODO: 从线程中提取创建续传所需的信息，利用相应的Encoder进行请求的处理

    /// TODO: 需提供一个网络库的Helper工具方法，用于提供网络库对象的弱指针
    auto assistant_v3 = std::make_shared<assistant::Assistant_v3>();
    auto assistant_weak = std::weak_ptr<assistant::Assistant_v3>(assistant_v3);
    rx_assistant_factory factory(assistant_weak);
    auto obs = factory.create(create_upload_request);

    /// TODO: 需提供根据HttpResponse解析得到结果的json字符串
    std::function<std::string(HttpResult&)> solve_create_upload;

    /// TODO: 需提供根据结果的json字符串得到proof的lambda表达式
    std::function<uploader_proof(const std::string&)> get_create_upload_result;

    return obs.map(solve_create_upload).map(get_create_upload_result);
  };
  /// create_upload end.
  //////////////////////////////////////////////////////////////////////////

  httpbusiness::uploader::proof::ProofObsCallback check_upload;
  httpbusiness::uploader::proof::ProofObsCallback file_uplaod;
  httpbusiness::uploader::proof::ProofObsCallback file_commit;
}

Uploader::~Uploader() = default;

}  // namespace Restful
}  // namespace Cloud189
