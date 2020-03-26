#include "cloud189_folder_uploader.h"

#include <rx_assistant.hpp>
#include <rx_multiworker.hpp>
#include <rx_uv_fs.hpp>
#include <tools/safecontainer.hpp>
#include <tools/string_convert.hpp>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

#include "cloud189/apis/create_folder.h"
#include "cloud189/error_code/error_code.h"

using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

namespace {
typedef struct __folderupload_mateiral_type {
  std::string target_server_folder_path;
  std::string source_local_folder_path;
} folderupload_mateiral_type;
typedef struct __folderupload_report_type {
  std::string parent_folder_id;
  std::string local_folder_path;
  std::vector<std::string> sub_file_data;
} folderupload_report_type;
typedef struct __folderupload_done_type {
  std::string target_server_folder_path;
  std::string source_local_folder_path;
  bool valid_local_path;
  std::string server_folder_id;
  int32_t get_folder_id_result;
  std::vector<std::string> sub_file;
  std::vector<std::string> sub_folder;
  __folderupload_done_type()
      : valid_local_path(true), get_folder_id_result(0) {}
} folderupload_done_type;
}  // namespace

struct folderupload_worker_function_generator;

namespace Cloud189 {
namespace Restful {

/// 对上传总控的要求
/// 内部数据支持多线程访问，线程安全
namespace details {

struct folderuploader_internal_data {
  std::unique_ptr<folderupload_worker_function_generator> folderupload_unique;
  rxcpp::connectable_observable<folderupload_report_type> data_source;
  /// TODO: 临时性保存一下传入 的信息，后续改成构造时初始化const string
  assistant::tools::lockfree_string_closure<std::string> origin_info;
};
struct folderuploader_thread_data {
  std::shared_ptr<rx_uv_fs::uv_loop_with_thread> uv_thread;
  folderuploader_thread_data()
      : uv_thread(std::make_shared<rx_uv_fs::uv_loop_with_thread>()),
        frozen(false),
        int32_error_code(0) {}

  std::atomic<bool> frozen;
  std::atomic<int32_t> int32_error_code;
};
}  // namespace details
}  // namespace Restful
}  // namespace Cloud189

using Cloud189::Restful::details::folderuploader_internal_data;
using Cloud189::Restful::details::folderuploader_thread_data;

typedef struct folderupload_worker_function_generator {
 private:
  typedef httpbusiness::rx_multi_worker<folderupload_mateiral_type,
                                        folderupload_report_type>
      FolderUpload;
  explicit folderupload_worker_function_generator(
      const std::weak_ptr<folderuploader_thread_data>& weak,
      const FolderUpload::MaterialVector& materials)
      : thread_data_weak(weak) {
    auto folderupload_worker =
        std::bind(&folderupload_worker_function_generator::folderupload_worker,
                  this, std::placeholders::_1);
    folderupload_unique =
        std::move(FolderUpload::Create(materials, folderupload_worker, 1));
  }
  /// 禁用默认构造、复制构造、移动构造和=号操作符
  folderupload_worker_function_generator() = delete;
  folderupload_worker_function_generator(
      folderupload_worker_function_generator&& httpresult) = delete;
  folderupload_worker_function_generator(
      const folderupload_worker_function_generator&) = delete;
  folderupload_worker_function_generator& operator=(
      const folderupload_worker_function_generator&) = delete;

 public:
  typedef FolderUpload::MaterialVector MaterialVector;
  typedef FolderUpload::DataSource DataSource;
  typedef FolderUpload::Report Report;
  ~folderupload_worker_function_generator() = default;

  /// TODO: 解除与folderuploader_thread_data的耦合 2019.12.9
  static std::unique_ptr<folderupload_worker_function_generator> Create(
      const std::weak_ptr<folderuploader_thread_data>& weak,
      const MaterialVector& materials) {
    return std::unique_ptr<folderupload_worker_function_generator>(new (
        std::nothrow) folderupload_worker_function_generator(weak, materials));
  }
  DataSource GetDataSource() { return folderupload_unique->data_source; }
  void Stop() { folderupload_unique->Stop(); }

 private:
  std::weak_ptr<folderuploader_thread_data> thread_data_weak;
  FolderUpload::MultiWorkerUnique folderupload_unique;

  typedef folderupload_done_type DoneType;

  void folderupload_worker(FolderUpload::CalledCallbacks callbacks) {
    FolderUpload::MaterialType material_unique = nullptr;
    if (!callbacks.check_stop()) {
      material_unique = callbacks.get_material();
    }
    /// 注意这里是新增的worker的对应的选项，要么新增，要么不变（指物料池空）
    callbacks.get_material_result(nullptr != material_unique ? 1 : 0);
    if (nullptr != material_unique) {
      async_worker_function(
          *material_unique,
          std::bind(
              &folderupload_worker_function_generator::after_work_callback,
              this, std::placeholders::_1, std::placeholders::_2),
          callbacks);
    }
  }
  void async_worker_function(
      FolderUpload::Material material,
      std::function<void(DoneType&, FolderUpload::CalledCallbacks)>
          done_callback,
      FolderUpload::CalledCallbacks callbacks) {
    std::function<void(DoneType&)> on_done =
        std::bind(done_callback, std::placeholders::_1, callbacks);
    auto& thread_data_weak = this->thread_data_weak;
    // 1. 调用传入的物料，检测目标本地路径是否为有效的目录
    // 2. 继续调用传入的物料，获取远端路径的folderid，若不存在则创建
    // 3. 再次调用传入的无聊，获取本地路径下的子文件和子目录
    rxcpp::observable<>::just(material)
        .flat_map(
            [thread_data_weak](FolderUpload::Material material) {
              auto thread_data = thread_data_weak.lock();
              rxcpp::observable<bool> obs = rxcpp::observable<>::just(false);
              if (nullptr != thread_data) {
                obs = rx_uv_fs::rx_uv_fs_factory::Stat(
                          thread_data->uv_thread,
                          material.source_local_folder_path)
                          .map([](int32_t v) -> bool { return 0x4000 == v; });
              }
              /// 调用rx_uv_fs的stat进行判断，返回一个bool的Observable
              return obs;
            },
            [](FolderUpload::Material material, bool valid_local_path) {
              DoneType unfinished;
              unfinished.target_server_folder_path =
                  material.target_server_folder_path;
              unfinished.source_local_folder_path =
                  material.source_local_folder_path;
              unfinished.valid_local_path = valid_local_path;

              /// 返回一个DoneType
              return unfinished;
            })
        .flat_map(
            [](DoneType material) {
              rxcpp::observable<std::string> obs =
                  rxcpp::observable<>::just(std::string());
              do {
                /// 若valid_local_path为false，则直接无需发请求
                if (!material.valid_local_path) {
                  break;
                }
                assistant::HttpRequest request("");
                std::vector<std::string> path_split;
                assistant::tools::string::StringSplit(
                    material.target_server_folder_path, "/", path_split);
                if (path_split.empty()) {
                  break;
                }
                const auto folder_name = path_split.back();
                path_split.pop_back();
                std::string relative_path;
                for (const auto& x : path_split) {
                  relative_path += x + "/";
                }
                if (relative_path.empty()) {
                  relative_path += "/";
                }
                /// TODO:
                /// "-11"不应在此处写死，应由外部提供一个“root_folder_id”字段
                Cloud189::Apis::CreateFolder::HttpRequestEncode(
                    Cloud189::Apis::CreateFolder::JsonStringHelper(
                        "-11", relative_path, folder_name, ""),
                    request);

                /// Jsut for test.
                // request.extends.Set("proxy", "http://127.0.0.1:8888");

                /// TODO: 这里的实现未完备，应增加无网络，5XX等情况下的重试处理
                obs = rx_assistant::rx_httpresult::create(request).map(
                    [](rx_assistant::HttpResult result) {
                      std::string result_json;
                      Cloud189::Apis::CreateFolder::HttpResponseDecode(
                          result.res, result.req, result_json);
                      Json::Value root;
                      ReaderHelper(result_json, root);
                      auto folder_id = GetString(root["id"]);
                      return folder_id;
                    });
                /// 调用rx_httpresult进行处理，并对结果进行处理，返回一个std::string的数据源
                /// 以“”，代表远端目录创建失败，或严重的错误
              } while (false);
              return obs;
            },
            [](DoneType material, std::string server_folder_id) {
              DoneType& unfinished = material;
              material.server_folder_id = server_folder_id;
              /// 返回一个DoneType
              return unfinished;
            })
        .flat_map(
            [thread_data_weak](DoneType material) {
              /// 若valid_local_path为false，则直接无需遍历此路径下的子目录和子文件
              /// 若server_folder_id为空字符串，则同上
              /// 调用rx_ux_fs的scandir操作，返回一个std::tuple<std::vector<std::string>,std::vector<std::string>>
              auto thread_data = thread_data_weak.lock();
              rxcpp::observable<rx_uv_fs::uv_fs_scandir::Type> obs =
                  rxcpp::observable<>::just(rx_uv_fs::uv_fs_scandir::Type());
              do {
                if (nullptr == thread_data) {
                  break;
                }
                if (!material.valid_local_path) {
                  break;
                }
                if (material.server_folder_id.empty() ||
                    '\0' == material.server_folder_id.front()) {
                  break;
                }
                obs = rx_uv_fs::rx_uv_fs_factory::Scandir(
                    thread_data->uv_thread, material.source_local_folder_path);
              } while (false);
              return obs;
            },
            [](DoneType material, rx_uv_fs::uv_fs_scandir::Type sub_contents) {
              DoneType& unfinished = material;
              unfinished.sub_file = std::move(sub_contents.files);
              unfinished.sub_folder = std::move(sub_contents.dirs);

              return unfinished;
            })
        .tap(on_done)
        .publish()
        .connect();
  }
  void after_work_callback(DoneType& done_result,
                           FolderUpload::CalledCallbacks callbacks) {
    /// 产生额外物料，调用extra_material
    for (const auto& subfolder_name : done_result.sub_folder) {
      const auto produced_material = folderupload_mateiral_type{
          done_result.target_server_folder_path + subfolder_name + "/",
          done_result.source_local_folder_path + subfolder_name + "/"};
      callbacks.extra_material(produced_material);
    }

    /// 进行报告
    FolderUpload::Report report;
    report.local_folder_path = done_result.source_local_folder_path;
    report.parent_folder_id = done_result.server_folder_id;
    for (const auto& subfile_name : done_result.sub_file) {
      report.sub_file_data.emplace_back(report.local_folder_path +
                                        subfile_name);
    }
    callbacks.send_report(report);

    /// 需要检查主控是否已经“frozen”，如有需尽快停止整个multi_worker流程
    /// 2019.12.16
    auto thread_data = thread_data_weak.lock();
    if (nullptr == thread_data /*|| thread_data->frozen.load()*/) {
      callbacks.serious_error();
    }
    /// 尝试下一轮生产
    FolderUpload::MaterialType material_after = nullptr;
    if (!callbacks.check_stop()) {
      material_after = callbacks.get_material();
    }
    /// 注意这里是旧worker的对应的选项，要么不变，要么减少（指物料池空）
    callbacks.get_material_result(nullptr != material_after ? 0 : -1);
    if (nullptr != material_after) {
      async_worker_function(
          *material_after,
          std::bind(
              &folderupload_worker_function_generator::after_work_callback,
              this, std::placeholders::_1, std::placeholders::_2),
          callbacks);
    }
  }
} folderupload_helper;

Cloud189::Restful::FolderUploader::FolderUploader(
    const std::string& upload_info,
    std::function<void(const std::string&)> data_callback)
    : data(std::make_unique<folderuploader_internal_data>()),
      thread_data(std::make_shared<folderuploader_thread_data>()) {
  folderupload_helper::MaterialVector origin_material;
  /// TODO: 完善此处的逻辑处理，生成正确的字符串物料
  data->origin_info.store(upload_info);
  Json::Value folder_json;
  ReaderHelper(upload_info, folder_json);
  const auto local_folder_path = GetString(folder_json["local_folder_path"]);
  const auto server_folder_path = GetString(folder_json["server_folder_path"]);
  std::vector<std::string> local_folder_split;
  assistant::tools::string::StringSplit(local_folder_path, "/",
                                        local_folder_split);
  const auto target_server_folder_path =
      server_folder_path + local_folder_split.back() + "/";

  folderupload_mateiral_type hiiii{target_server_folder_path,
                                   local_folder_path};
  origin_material.emplace_back(hiiii);
  data->folderupload_unique =
      folderupload_helper::Create(thread_data, origin_material);

  const auto thread_data_weak =
      std::weak_ptr<folderuploader_thread_data>(thread_data);
  /// 为数据源生成匹配的OnNext以及OnComplete回调
  data->data_source =
      data->folderupload_unique->GetDataSource()
          .tap(
              [data_callback](folderupload_report_type report) {
                Json::Value root;
                root["parent_folder_id"] = report.parent_folder_id;
                root["local_folder_path"] = report.local_folder_path;
                Json::Value sub_file_data;
                for (const auto& sub_file : report.sub_file_data) {
                  Json::Value value;
                  value["local_path"] = sub_file;
                  sub_file_data.append(value);
                }
                root["sub_file_data"] = sub_file_data;
                const auto json_str = WriterHelper(root);
                data_callback(json_str);
              },
              [data_callback, thread_data_weak]() {
                Json::Value root;
                root["parent_folder_id"];
                root["local_folder_path"];
                root["sub_file_data"];
                root["is_complete"] = bool(true);
                root["int32_error_code"] = int32_t(0);
                auto thread_data = thread_data_weak.lock();
                if (nullptr != thread_data) {
                  root["int32_error_code"] =
                      thread_data->int32_error_code.load();
                }
                const auto json_str = WriterHelper(root);
                data_callback(json_str);
              })
          .publish();
}

Cloud189::Restful::FolderUploader::~FolderUploader() {}

void Cloud189::Restful::FolderUploader::AsyncStart() {
  data->data_source.connect();
}

void Cloud189::Restful::FolderUploader::SyncWait() {}

void Cloud189::Restful::FolderUploader::UserCancel() {
  thread_data->frozen.store(true);
  thread_data->int32_error_code.store(Cloud189::ErrorCode::nderr_usercanceled);
  data->folderupload_unique->Stop();
}

bool Cloud189::Restful::FolderUploader::Valid() {
  bool result = false;
  do {
    /// TODO: 待FolderUploader完备后，再此处进行检验
    /// 供外部调用。

    result = true;
  } while (false);
  return result;
}
