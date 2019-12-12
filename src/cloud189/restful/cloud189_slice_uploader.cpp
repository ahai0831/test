#include "cloud189_slice_uploader.h"

/// 避免因Windows.h在WinSock2.h之前被包含导致的问题
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <HashAlgorithm/SHA1Digest.h>
#include <HashAlgorithm/md5.h>
#include <rx_assistant.hpp>
#include <rx_md5.hpp>
#include <rx_multiworker.hpp>
#include <rx_uploader.hpp>
#include <speed_counter.hpp>
#include <tools/safecontainer.hpp>
#include <tools/string_convert.hpp>

#include "HashAlgorithm/SHA1Digest.h"
#include "HashAlgorithm/md5.h"

#include "cloud189/apis/commit_slice_upload_file.h"
#include "cloud189/apis/create_slice_upload_file.h"
#include "cloud189/apis/get_slice_upload_status.h"
#include "cloud189/apis/upload_slice_data.h"
#include "cloud189/error_code/nderror.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using assistant::HttpRequest;
using assistant::tools::lockfree_string_closure;
using httpbusiness::uploader::proof::stage_result;
using httpbusiness::uploader::proof::uploader_proof;
using httpbusiness::uploader::proof::uploader_stage;
using rx_assistant::HttpResult;
// TODO: 既然是源文件，那么对比较复杂的命名空间使用，都可以使用using
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetInt64;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

namespace {
struct slicemd5_worker_function_generator;
struct sliceupload_worker_function_generator;
}  // namespace

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
  /// 以file_protect为标志，如果此指针已为空，则什么都不用做
  bool Valid() { return nullptr != file_protect; }
  /// 保存此回调，以供调用
  const httpbusiness::uploader::rx_uploader::CompleteCallback
      null_file_callback;

  explicit sliceuploader_internal_data(
      const std::string& file_path,
      const httpbusiness::uploader::proof::proof_obs_packages& proof_orders,
      const httpbusiness::uploader::rx_uploader::CompleteCallback data_callback)
      :  /// TODO:
         /// 需要增加机制，一旦此句柄在构造后为空，则进行frozen，避免进行无用操作
         /// 进行frozen以后，显式地将失败信息传递一次
        file_protect(
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
  const std::string x_request_id;
  const int64_t per_slice_size;
  const int32_t oper_type;
  const int32_t is_log;
  /// 禁用移动复制默认构造和=号操作符，必须通过显式指定所有必须字段进行初始化
  sliceuploader_thread_data(const std::string& file_path,
                            const std::string& last_trans_md5,
                            const std::string& last_trans_upload_id,
                            const std::string& parent_folder_id_,
                            const std::string& x_request_id_,
                            const int64_t& per_slice_size_,
                            const int32_t& oper_type_, const int32_t& is_log_)
      : local_filepath(file_path),
        last_md5(last_trans_md5),
        last_upload_id(last_trans_upload_id),
        parent_folder_id(parent_folder_id_),
        x_request_id(x_request_id_),
        per_slice_size(per_slice_size_),
        oper_type(oper_type_),
        is_log(is_log_),
        file_size({0}),
        data_exist({false}),
        already_upload_bytes({0}),
        current_upload_bytes({0}),
        int32_error_code({0}),
        frozen({false}) {}
  /// 线程安全的数据成员
  lockfree_string_closure<std::string> file_md5;
  lockfree_string_closure<std::string> upload_file_id;
  lockfree_string_closure<std::string> file_upload_url;
  lockfree_string_closure<std::string> file_commit_url;
  assistant::tools::safemap_closure<int32_t, std::string>
      slice_md5_map;  // 保存各分片MD5
  assistant::tools::safemap_closure<int32_t, int32_t>
      slice_result_map;  // 保存各分片请求的结果
  std::atomic<int64_t>
      file_size;  /// 保存文件大小，由于文件已被锁定，全流程不变
  std::atomic<bool> data_exist;  /// 保存文件是否可秒传
  lockfree_string_closure<std::string>
      slice_exist;  // 保存已完成的分片序号字符串
  std::atomic<int64_t> already_upload_bytes;  // 获取续传状态中得到的已传数据量
  std::atomic<int64_t> current_upload_bytes;  // 上传文件数据中每次的已传数据量
  std::atomic<int32_t> int32_error_code;  /// 错误码
  /// 以下为确认文件上传完成后解析到的字段
  lockfree_string_closure<std::string> commit_id;
  lockfree_string_closure<std::string> commit_name;
  lockfree_string_closure<std::string> commit_size;
  lockfree_string_closure<std::string> commit_md5;
  lockfree_string_closure<std::string> commit_create_date;
  lockfree_string_closure<std::string> commit_rev;
  lockfree_string_closure<std::string> commit_user_id;
  lockfree_string_closure<std::string> commit_is_safe;

  /// 保存用于计算各分片MD5的multi_worker对象
  std::unique_ptr<slicemd5_worker_function_generator> slicemd5_unique;
  std::unique_ptr<sliceupload_worker_function_generator> sliceupload_unique;

  /// 保存计速器，以进行平滑速度计算，以及1S一次的数据推送
  httpbusiness::speed_counter_with_stop speed_count;
  std::weak_ptr<httpbusiness::uploader::rx_uploader::rx_uploader_data>
      master_control_data;

  /// 保存停止标志位
  std::atomic<bool> frozen;

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

using Cloud189::Restful::details::sliceuploader_thread_data;
/// 分片计算MD5以及分片进行数据上传
namespace {
typedef struct slicemd5_worker_function_generator {
 private:
  typedef httpbusiness::rx_multi_worker<std::tuple<int64_t, int64_t, int32_t>,
                                        std::tuple<int32_t, std::string>>
      SliceMd5;
  explicit slicemd5_worker_function_generator(
      const std::weak_ptr<sliceuploader_thread_data>& weak,
      const SliceMd5::MaterialVector& materials)
      : thread_data_weak(weak) {
    auto slicemd5_worker = std::bind(&slicemd5_helper::slicemd5_worker, this,
                                     std::placeholders::_1);
    slicemd5_unique =
        std::move(SliceMd5::Create(materials, slicemd5_worker, 3));
  }
  /// 禁用默认构造、复制构造、移动构造和=号操作符
  slicemd5_worker_function_generator() = delete;
  slicemd5_worker_function_generator(
      slicemd5_worker_function_generator&& httpresult) = delete;
  slicemd5_worker_function_generator(
      const slicemd5_worker_function_generator&) = delete;
  slicemd5_worker_function_generator& operator=(
      const slicemd5_worker_function_generator&) = delete;

 public:
  typedef SliceMd5::MaterialVector MaterialVector;
  typedef SliceMd5::DataSource DataSource;
  typedef SliceMd5::Report Report;
  ~slicemd5_worker_function_generator() = default;
  /// TODO:
  /// 从实现的角度看，仅用到了线程数据内的文件路径，可仅传入此字段的智能指针
  /// TODO: 解除与sliceuploader_thread_data的耦合 2019.12.9
  static std::unique_ptr<slicemd5_worker_function_generator> Create(
      const std::weak_ptr<sliceuploader_thread_data>& weak,
      const MaterialVector& materials) {
    return std::unique_ptr<slicemd5_worker_function_generator>(
        new (std::nothrow) slicemd5_worker_function_generator(weak, materials));
  }
  //// 增加一个MaterialHelper工具方法，辅助生成物料
  static MaterialVector MaterialHelper(const int64_t file_length,
                                       const int64_t slice_size) {
    MaterialVector materials;
    const int64_t length = file_length;
    const auto& slicesize = slice_size;
    int32_t slice_id = 1;
    for (int64_t i = 0; i < length; i += slicesize, ++slice_id) {
      const auto range_left = i;
      const auto range_right =
          i + slicesize > length ? length - 1 : i + slicesize - 1;
      materials.emplace_back(
          std::make_tuple(range_left, range_right, slice_id));
    }
    return materials;
  }
  DataSource GetDataSource() { return slicemd5_unique->data_source; }
  void Stop() { slicemd5_unique->Stop(); }

 private:
  std::weak_ptr<sliceuploader_thread_data> thread_data_weak;
  SliceMd5::MultiWorkerUnique slicemd5_unique;

  void slicemd5_worker(SliceMd5::CalledCallbacks callbacks) {
    SliceMd5::MaterialType material_unique = nullptr;
    if (!callbacks.check_stop()) {
      material_unique = callbacks.get_material();
    }
    /// 注意这里是新增的worker的对应的选项，要么新增，要么不变（指物料池空）
    callbacks.get_material_result(nullptr != material_unique ? 1 : 0);
    if (nullptr != material_unique) {
      async_worker_function(
          *material_unique,
          std::bind(&slicemd5_worker_function_generator::after_work_callback,
                    this, std::placeholders::_1, std::placeholders::_2),
          callbacks);
    }
  }
  void async_worker_function(
      SliceMd5::Material material,
      std::function<void(const SliceMd5::Report&, SliceMd5::CalledCallbacks)>
          done_callback,
      SliceMd5::CalledCallbacks callbacks) {
    auto thread_data = thread_data_weak.lock();
    if (nullptr != thread_data) {
      const auto& file_path = thread_data->local_filepath;
      const auto& range_left = std::get<0>(material);
      const auto& range_right = std::get<1>(material);
      const auto& slice_id = std::get<2>(material);
      auto obs =
          rx_assistant::rx_md5::create(file_path, range_left, range_right);
      auto publish_obs =
          obs.map([slice_id](std::string& md5) -> SliceMd5::Report {
               return std::make_tuple(slice_id, md5);
             })
              .tap(
                  [done_callback, callbacks](SliceMd5::Report& report) -> void {
                    done_callback(report, callbacks);
                  })
              .publish();

      /// TODO: 此处可完善
      publish_obs.connect();
    }
  }
  void after_work_callback(const SliceMd5::Report& report,
                           SliceMd5::CalledCallbacks callbacks) {
    /// 不产生额外物料，无需调用extra_material

    /// 进行报告
    callbacks.send_report(report);

    /// 尝试下一轮生产
    SliceMd5::MaterialType material_after = nullptr;
    if (!callbacks.check_stop()) {
      material_after = callbacks.get_material();
    }
    /// 注意这里是旧worker的对应的选项，要么不变，要么减少（指物料池空）
    callbacks.get_material_result(nullptr != material_after ? 0 : -1);
    if (nullptr != material_after) {
      async_worker_function(
          *material_after,
          std::bind(&slicemd5_worker_function_generator::after_work_callback,
                    this, std::placeholders::_1, std::placeholders::_2),
          callbacks);
    }
  }
} slicemd5_helper;

typedef struct sliceupload_worker_function_generator {
 private:
  typedef httpbusiness::rx_multi_worker<std::tuple<int64_t, int64_t, int32_t>,
                                        std::tuple<int32_t, bool>>
      SliceUpload;
  explicit sliceupload_worker_function_generator(
      const std::weak_ptr<sliceuploader_thread_data>& weak,
      const SliceUpload::MaterialVector& materials)
      : thread_data_weak(weak) {
    auto sliceupload_worker = std::bind(&sliceupload_helper::sliceupload_worker,
                                        this, std::placeholders::_1);
    sliceupload_unique =
        std::move(SliceUpload::Create(materials, sliceupload_worker, 3));
  }
  /// 禁用默认构造、复制构造、移动构造和=号操作符
  sliceupload_worker_function_generator() = delete;
  sliceupload_worker_function_generator(
      sliceupload_worker_function_generator&& httpresult) = delete;
  sliceupload_worker_function_generator(
      const sliceupload_worker_function_generator&) = delete;
  sliceupload_worker_function_generator& operator=(
      const sliceupload_worker_function_generator&) = delete;

 public:
  typedef SliceUpload::MaterialVector MaterialVector;
  typedef SliceUpload::DataSource DataSource;
  typedef SliceUpload::Report Report;
  ~sliceupload_worker_function_generator() = default;

  /// TODO: 解除与sliceuploader_thread_data的耦合 2019.12.9
  static std::unique_ptr<sliceupload_worker_function_generator> Create(
      const std::weak_ptr<sliceuploader_thread_data>& weak,
      const MaterialVector& materials) {
    return std::unique_ptr<sliceupload_worker_function_generator>(new (
        std::nothrow) sliceupload_worker_function_generator(weak, materials));
  }
  //// 增加一个MaterialHelper工具方法，辅助生成物料
  static MaterialVector MaterialHelper(const int64_t file_length,
                                       const int64_t slice_size) {
    MaterialVector materials;
    const int64_t length = file_length;
    const auto& slicesize = slice_size;
    int32_t slice_id = 1;
    for (int64_t i = 0; i < length; i += slicesize, ++slice_id) {
      const auto range_left = i;
      const auto range_right =
          i + slicesize > length ? length - 1 : i + slicesize - 1;
      materials.emplace_back(
          std::make_tuple(range_left, range_right, slice_id));
    }
    return materials;
  }
  DataSource GetDataSource() { return sliceupload_unique->data_source; }
  void Stop() { sliceupload_unique->Stop(); }

 private:
  std::weak_ptr<sliceuploader_thread_data> thread_data_weak;
  SliceUpload::MultiWorkerUnique sliceupload_unique;

  void sliceupload_worker(SliceUpload::CalledCallbacks callbacks) {
    SliceUpload::MaterialType material_unique = nullptr;
    if (!callbacks.check_stop()) {
      material_unique = callbacks.get_material();
    }
    /// 注意这里是新增的worker的对应的选项，要么新增，要么不变（指物料池空）
    callbacks.get_material_result(nullptr != material_unique ? 1 : 0);
    if (nullptr != material_unique) {
      async_worker_function(
          *material_unique,
          std::bind(&sliceupload_worker_function_generator::after_work_callback,
                    this, std::placeholders::_1, std::placeholders::_2),
          callbacks);
    }
  }
  void async_worker_function(SliceUpload::Material material,
                             std::function<void(const SliceUpload::Report&,
                                                SliceUpload::CalledCallbacks)>
                                 done_callback,
                             SliceUpload::CalledCallbacks callbacks) {
    auto thread_data = thread_data_weak.lock();
    auto thread_data_weak_temp = thread_data_weak;
    if (nullptr != thread_data) {
      /// 生成单个分片请求
      assistant::HttpRequest per_sliceupload_req("");
      const auto& range_left = std::get<0>(material);
      const auto& range_right = std::get<1>(material);
      const auto& slice_id = std::get<2>(material);

      /// 根据thread_data中相应信息和以上的信息，拼出单个分片请求
      const auto& file_upload_url = thread_data->file_upload_url.load();
      const auto& file_path = thread_data->local_filepath;
      const auto& upload_id = thread_data->upload_file_id.load();
      const auto& x_request_id = thread_data->x_request_id;
      std::string current_slice_md5;
      auto solve_slice_md5 = [&current_slice_md5](const std::string& temp) {
        current_slice_md5 = temp;
      };
      thread_data->slice_md5_map.FindDelegate(slice_id, solve_slice_md5);

      const std::string per_sliceupload_str =
          Cloud189::Apis::UploadSliceData::JsonStringHelper(
              file_upload_url, file_path, upload_id, x_request_id, range_left,
              range_right, slice_id, current_slice_md5);
      /// HttpRequestEncode
      Cloud189::Apis::UploadSliceData::HttpRequestEncode(per_sliceupload_str,
                                                         per_sliceupload_req);
      /// HttpRequestEncode 完毕

      /// 保存thread_data，避免在传输线程中反复lock
      /// 为需要使用的变量定义引用
      auto& current_upload_bytes = thread_data->current_upload_bytes;
      auto& Add = thread_data->speed_count.Add;
      per_sliceupload_req.retval_func = [thread_data, &current_upload_bytes,
                                         Add](uint64_t value) {
        current_upload_bytes += value;
        Add(value);
      };

      std::function<bool(const rx_assistant::HttpResult&,
                         assistant::HttpRequest&, int32_t&)>
          sliceupload_solve_callback =
              [thread_data_weak_temp](
                  const rx_assistant::HttpResult& file_upload_http_result,
                  assistant::HttpRequest&, int32_t&) -> bool {
        /// 无需循环+成功：（非常重要，需要考虑一些，看似“失败”，但最终提交会成功的情况）
        /// 无需循环+失败：彻底失败：（循环次数达到上限，或其他严重情况）
        /// 在此情况下，无需循环，并发出严重错误信号。此时其他worker也不再重试。
        /// 需要循环：非已知的错误
        /// TODO: 需要完善错误处理
        bool solve_flag = false;
        auto thread_data = thread_data_weak_temp.lock();
        do {
          if (nullptr == thread_data) {
            break;
          }
          std::string decode_result;
          Cloud189::Apis::UploadSliceData::HttpResponseDecode(
              file_upload_http_result.res, file_upload_http_result.req,
              decode_result);
          if (decode_result.empty()) {
            break;
          }
          Json::Value json_value;
          if (!restful_common::jsoncpp_helper::ReaderHelper(decode_result,
                                                            json_value)) {
            break;
          }
          std::string header_slice_id;
          if (!file_upload_http_result.req.headers.Get("Edrive-UploadSliceId",
                                                       header_slice_id) ||
              header_slice_id.empty()) {
            break;
          }
          const auto is_success =
              restful_common::jsoncpp_helper::GetBool(json_value["isSuccess"]);
          if (is_success) {
            thread_data->slice_result_map.Set(stoi(header_slice_id), 0);
            solve_flag = true;
          }
          if (!is_success) {
            thread_data->slice_result_map.Set(stoi(header_slice_id), 1);
          }
        } while (false);
        return solve_flag;
      };

      /// 根据最后一次响应判定当前分片的结果
      std::function<sliceupload_helper::Report(rx_assistant::HttpResult&)>
          result_2_report = [thread_data](rx_assistant::HttpResult& http_result)
          -> sliceupload_helper::Report {
        /// 根据rx_assistant::HttpResult内的slice_id的信息，找到线程数据内对应的线程安全Map内的值
        /// 此值为0，返回true；此值不存在，或非0，返回false
        std::string header_slice_id;
        std::tuple<int32_t, bool> slice_result_tuple;
        if (http_result.req.headers.Get("Edrive-UploadSliceId",
                                        header_slice_id) &&
            !header_slice_id.empty()) {
          int32_t slice_result;
          auto solve_slice_result = [&slice_result](const int32_t& temp) {
            slice_result = temp;
          };
          thread_data->slice_result_map.FindDelegate(stoi(header_slice_id),
                                                     solve_slice_result);
          if (slice_result) {
            slice_result_tuple =
                std::make_tuple(int32_t(stoi(header_slice_id)), bool(true));
          } else {
            slice_result_tuple =
                std::make_tuple(int32_t(stoi(header_slice_id)), bool(false));
          }
        }
        return slice_result_tuple;
      };

      std::function<void(sliceupload_helper::Report&)> tap_callback =
          std::bind(done_callback, std::placeholders::_1, callbacks);
      rx_assistant::rx_httpresult::loop_with_delay(
          rx_assistant::rx_httpresult::create(per_sliceupload_req),
          sliceupload_solve_callback)
          .map(result_2_report)
          .tap(tap_callback)
          .publish()
          .connect();
    }
  }
  void after_work_callback(const SliceUpload::Report& report,
                           SliceUpload::CalledCallbacks callbacks) {
    /// 不产生额外物料，无需调用extra_material

    /// 进行报告
    callbacks.send_report(report);

    /// 尝试下一轮生产
    SliceUpload::MaterialType material_after = nullptr;
    if (!callbacks.check_stop()) {
      material_after = callbacks.get_material();
    }
    /// 注意这里是旧worker的对应的选项，要么不变，要么减少（指物料池空）
    callbacks.get_material_result(nullptr != material_after ? 0 : -1);
    if (nullptr != material_after) {
      async_worker_function(
          *material_after,
          std::bind(&sliceupload_worker_function_generator::after_work_callback,
                    this, std::placeholders::_1, std::placeholders::_2),
          callbacks);
    }
  }
} sliceupload_helper;

}  // namespace

namespace {
/// 根据传入的字符串，对线程数据进行初始化
/// TODO: 这一块占代码行数太多，需要简化 2019.12.9
std::shared_ptr<Cloud189::Restful::details::sliceuploader_thread_data>
InitThreadData(const std::string& upload_info) {
  /// 根据传入信息，初始化线程信息结构体
  Json::Value json_str;
  ReaderHelper(upload_info, json_str);
  const auto& local_filepath = GetString(json_str["localPath"]);
  const auto& last_md5 = GetString(json_str["md5"]);
  const auto& last_upload_id = GetString(json_str["uploadFileId"]);
  const auto& parent_folder_id = GetString(json_str["parentFolderId"]);
  const auto per_slice_size = GetInt64(json_str["perSliceSize"]);
  const auto oper_type = GetInt(json_str["opertype"]);
  const auto is_log = GetInt(json_str["isLog"]);
  auto& x_request_id = GetString(json_str["X-Request-ID"]);
  if (x_request_id.empty()) {
    x_request_id = assistant::tools::uuid::generate();
  }
  std::string slice_md5_list;
  std::string slice_md5;
  auto solve_slice_md5 = [&slice_md5](const std::string& temp) {
    slice_md5 = temp;
  };
  // thread_data->slice_md5_map.FindDelegate(upload_slice_id, solve_slice_md5);
  return std::make_shared<
      Cloud189::Restful::details::sliceuploader_thread_data>(
      local_filepath, last_md5, last_upload_id, parent_folder_id, x_request_id,
      per_slice_size, oper_type, is_log);
}
/// 生成总控使用的完成回调
const httpbusiness::uploader::rx_uploader::CompleteCallback
GenerateDataCallback(
    const std::weak_ptr<Cloud189::Restful::details::sliceuploader_thread_data>&
        thread_data_weak,
    const std::function<void(const std::string&)>& data_callback) {
  /// 为计速器提供每秒一次的回调
  auto thread_data = thread_data_weak.lock();
  if (nullptr != thread_data) {
    thread_data->speed_count.RegSubscription(
        [thread_data_weak, data_callback](uint64_t smooth_speed) {
          /// 每秒一次的回调信息字段
          Json::Value info;
          info["speed"] = int64_t(smooth_speed);
          auto thread_data = thread_data_weak.lock();
          if (nullptr != thread_data) {
            info["md5"] = thread_data->file_md5.load();
            info["upload_id"] = thread_data->upload_file_id.load();
            info["X-Request-ID"] = thread_data->x_request_id;
            info["file_size"] = thread_data->file_size.load();
            /// 已传输数据量
            auto transferred_size = thread_data->already_upload_bytes.load() +
                                    thread_data->current_upload_bytes.load();
            if (transferred_size > thread_data->file_size.load()) {
              transferred_size = thread_data->file_size.load();
            }
            info["transferred_size"] = transferred_size;

            int32_t current_stage = -1;
            auto mc_data = thread_data->master_control_data.lock();
            if (nullptr != mc_data) {
              current_stage = int32_t(mc_data->current_stage.load());
            }
            info["stage"] = current_stage;
            if (current_stage >= 3) {
              info["data_exist"] = thread_data->data_exist.load();
              info["file_upload_url"] = thread_data->file_upload_url.load();
            }
            /// 在errorcode机制完善后，需要在ec为0的情况下，才加相应业务字段
            int32_t ec = thread_data->int32_error_code.load();
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
    }
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
    data_callback(WriterHelper(info));
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
    /// 注意，算完整个文件MD5后，还需计算各分片的MD5
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

        /// “可能”检查上一次续传记录则尝试CheckUpload；
        /// 否则一律直接CreateUpload
        const auto& last_md5 = thread_data->last_md5;
        const auto& last_upload_id = thread_data->last_upload_id;
        if (!md5.empty() && last_md5 == md5 && !last_upload_id.empty()) {
          thread_data->file_md5 = md5;
          thread_data->upload_file_id = last_upload_id;
          calculate_md5_result_proof.result = stage_result::Unfinished;
          calculate_md5_result_proof.next_stage = uploader_stage::CheckUpload;
          break;
        }
        if (!md5.empty()) {
          thread_data->file_md5 = md5;
          calculate_md5_result_proof.result = stage_result::Unfinished;
          calculate_md5_result_proof.next_stage = uploader_stage::CreateUpload;
          break;
        }
      } while (false);
      return calculate_md5_result_proof;
    };

    /// 计算整个文件MD5的结果得到处理后，计算此文件各分片的MD5
    std::function<slicemd5_helper::DataSource(uploader_proof&)>
        generate_slicemd5 =
            [thread_data_weak](uploader_proof&) -> slicemd5_helper::DataSource {
      /// 一个单值的“起负面作用”的数据源，作为最坏情况下的处理
      slicemd5_helper::DataSource result = rxcpp::observable<>::just(
          std::make_tuple(int32_t(-1), std::string()));
      do {
        auto thread_data = thread_data_weak.lock();
        if (nullptr == thread_data) {
          break;
        }

        /// 需已知文件长度，以进行分割
        const auto file_length = thread_data->file_size.load();
        const auto slicesize = thread_data->per_slice_size;

        auto materials =
            slicemd5_helper::MaterialHelper(file_length, slicesize);
        auto slicemd5_unique =
            slicemd5_helper::Create(thread_data_weak, materials);
        if (nullptr == slicemd5_unique) {
          break;
        }
        result = slicemd5_unique->GetDataSource();
        thread_data->slicemd5_unique = std::move(slicemd5_unique);
      } while (false);
      return result;
    };

    /// 处理并记录各分片的MD5
    std::function<void(slicemd5_helper::Report&)> slicemd5_tap =
        [thread_data_weak](slicemd5_helper::Report& report) -> void {
      auto thread_data = thread_data_weak.lock();
      if (nullptr != thread_data) {
        const auto& slice_id = std::get<0>(report);
        const auto& slice_md5 = std::get<1>(report);
        thread_data->slice_md5_map.Set(slice_id, slice_md5);
      }
    };

    /// 严格校验各分片MD5是否算好，连同之前整个文件MD5的结果，交卷
    std::function<uploader_proof(slicemd5_helper::Report&)>
        slicemd5_result_callback =
            std::bind([thread_data_weak]() -> uploader_proof {
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
                  calculate_md5_result_proof.result =
                      stage_result::UserCanceled;
                  thread_data->int32_error_code =
                      Cloud189::ErrorCode::nderr_usercanceled;
                  break;
                }
                if (thread_data->file_md5.empty()) {
                  break;
                }
                bool is_any_slice_md5_invalid = false;
                auto check_per_slice_md5 =
                    [&is_any_slice_md5_invalid](
                        const int32_t slice_id,
                        const std::string& slice_md5) -> void {
                  if (slice_id <= 0 || slice_md5.empty()) {
                    is_any_slice_md5_invalid = true;
                  }
                };
                thread_data->slice_md5_map.ForeachDelegate(check_per_slice_md5);
                if (is_any_slice_md5_invalid) {
                  break;
                }
                if (!thread_data->upload_file_id.empty()) {
                  calculate_md5_result_proof.result = stage_result::Succeeded;
                  calculate_md5_result_proof.next_stage =
                      uploader_stage::CheckUpload;
                  break;
                }

                /// 至此，认为整个文件MD5和各分片MD5均有效，而无上一次的续传信息，应直接CreateUpload
                calculate_md5_result_proof.result = stage_result::Succeeded;
                calculate_md5_result_proof.next_stage =
                    uploader_stage::CreateUpload;
              } while (false);

              return calculate_md5_result_proof;
            });

    return rx_assistant::rx_md5::create(file_path)
        .map(md5_result_callback)
        .flat_map(generate_slicemd5)
        .tap(slicemd5_tap)
        .last()
        .map(slicemd5_result_callback);
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
      /// HttpRequestEncode
      HttpRequest create_slice_upload_request("");
      const auto& file_path = thread_data->local_filepath;
      const auto file_md5 = thread_data->file_md5.load();
      const auto& parent_folder_id = thread_data->parent_folder_id;
      const auto& x_request_id = thread_data->x_request_id;
      const auto oper_type = thread_data->oper_type;
      const auto is_log = thread_data->is_log;
      Cloud189::Apis::CreateSliceUploadFile::HttpRequestEncode(
          Cloud189::Apis::CreateSliceUploadFile::JsonStringHelper(
              file_path, parent_folder_id, file_md5, x_request_id, is_log,
              oper_type),
          create_slice_upload_request);
      /// 请求初始化完毕

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
          /// 增加对用户手动取消的处理
          if (thread_data->frozen.load()) {
            create_slice_upload_proof.result = stage_result::UserCanceled;
            thread_data->int32_error_code =
                Cloud189::ErrorCode::nderr_usercanceled;
            break;
          }
          Json::Value json_value;
          if (!restful_common::jsoncpp_helper::ReaderHelper(
                  create_slice_upload_result, json_value)) {
            break;
          }
          const auto is_success = GetBool(json_value["isSuccess"]);
          const auto upload_status = GetInt(json_value["uploadStatus"]);
          const bool data_exist = 4 == upload_status;
          if (is_success) {
            thread_data->upload_file_id.store(
                GetString(json_value["uploadFileId"]));
            thread_data->file_upload_url.store(
                GetString(json_value["fileUploadUrl"]));
            thread_data->file_commit_url.store(
                GetString(json_value["fileCommitUrl"]));
          }

          if (is_success && data_exist) {
            /// 成功且文件可秒传
            create_slice_upload_proof.result = stage_result::Succeeded;
            create_slice_upload_proof.next_stage = uploader_stage::FileCommit;
            /// 创建续传流程中：若可秒传，则将already_upload_bytes为文件大小，current_upload_bytes为0
            thread_data->data_exist.store(true);
            thread_data->already_upload_bytes = thread_data->file_size.load();
            thread_data->current_upload_bytes = 0;
            break;
          }
          if (is_success) {
            /// 成功且文件不可秒传
            create_slice_upload_proof.result = stage_result::Succeeded;
            create_slice_upload_proof.next_stage = uploader_stage::FileUplaod;
            /// 创建续传流程中：若不可秒传，则将already_upload_bytes为0，current_upload_bytes也为0
            thread_data->slice_exist.Clear();
            thread_data->already_upload_bytes = 0;
            thread_data->current_upload_bytes = 0;
            break;
          }

          const auto http_statuc_code = GetInt(json_value["httpStatusCode"]);
          const auto int32_error_code = GetInt(json_value["int32ErrorCode"]);

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
      result = rx_assistant::rx_httpresult::create(create_slice_upload_request)
                   .map(solve_create_slice_upload)
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
      /// HttpRequestEncode
      HttpRequest check_slice_upload_request("");
      const auto upload_id = thread_data->upload_file_id.load();
      const auto& x_request_id = thread_data->x_request_id;
      Cloud189::Apis::GetSliceUploadStatus::HttpRequestEncode(
          Cloud189::Apis::GetSliceUploadStatus::JsonStringHelper(upload_id,
                                                                 x_request_id),
          check_slice_upload_request);
      /// 请求初始化完毕

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
          /// 增加对用户手动取消的处理
          if (thread_data->frozen.load()) {
            check_slice_upload_proof.result = stage_result::UserCanceled;
            thread_data->int32_error_code =
                Cloud189::ErrorCode::nderr_usercanceled;
            break;
          }
          Json::Value json_value;
          if (!ReaderHelper(check_slice_upload_result, json_value)) {
            break;
          }
          const auto is_success = GetBool(json_value["isSuccess"]);
          const auto upload_status = GetInt(json_value["uploadStatus"]);
          const auto slice_exist = GetString(json_value["sliceExist"]);
          if (is_success) {
            thread_data->file_upload_url.store(
                GetString(json_value["fileUploadUrl"]));
            thread_data->file_commit_url.store(
                GetString(json_value["fileCommitUrl"]));
          }

          /// 若可秒传，则将already_upload_bytes为文件大小，current_upload_bytes为0。
          /// 查询续传记录流程中：若不可秒传且无偏移记录，则将already_upload_bytes为0，current_upload_bytes也为0；
          /// 若不可秒传但有已存在的分片，则将already_upload_bytes为已有分片的size之和，current_upload_bytes也为0；

          /// 特殊处理，2，代表数据已经传完，需要去FileCommit，但不代表秒传
          if (is_success && upload_status == 2) {
            check_slice_upload_proof.result = stage_result::Succeeded;
            check_slice_upload_proof.next_stage = uploader_stage::FileCommit;
            thread_data->already_upload_bytes = thread_data->file_size.load();
            thread_data->current_upload_bytes = 0;
            break;
          }
          /// 4，秒传；-1，已提交，所需做的处理和秒传一致
          if (is_success && (upload_status == 4 || upload_status == -1)) {
            /// 分片中，查询续传记录流程中：若可秒传，则令already_upload_bytes为文件大小，current_upload_bytes为0
            check_slice_upload_proof.result = stage_result::Succeeded;
            check_slice_upload_proof.next_stage = uploader_stage::FileCommit;
            thread_data->data_exist.store(true);
            thread_data->already_upload_bytes = thread_data->file_size.load();
            thread_data->current_upload_bytes = 0;
            break;
          }
          if (is_success && upload_status == 1 && slice_exist.empty()) {
            /// 分片中，查询续传记录流程中：不可秒传且无已存在的分片，则令already_upload_bytes为0，current_upload_bytes为0
            check_slice_upload_proof.result = stage_result::Succeeded;
            check_slice_upload_proof.next_stage = uploader_stage::FileUplaod;
            thread_data->already_upload_bytes = 0;
            thread_data->current_upload_bytes = 0;
            thread_data->slice_exist.Clear();
          }
          if (is_success && upload_status == 1 && !slice_exist.empty()) {
            /// 分片中，查询续传记录流程中：不可秒传且存在已有分片，则令already_upload_bytes为已有分片的size之和，current_upload_bytes为0
            check_slice_upload_proof.result = stage_result::Succeeded;
            check_slice_upload_proof.next_stage = uploader_stage::FileUplaod;
            /// 计算流程，获取到全部分片的序号；如果没有最后一片，直接*分片大小；如果有最后一片，还要减去文件尾差量偏移
            /// TODO: 此流程待验证
            int64_t end_offset = 0;
            std::set<int32_t> slice_id_set;
            std::vector<std::string> slice_id_vector;
            assistant::tools::string::StringSplit(slice_exist, ",",
                                                  slice_id_vector);
            for (const auto& x : slice_id_vector) {
              if (!x.empty()) {
                slice_id_set.emplace(strtol(x.c_str(), nullptr, 0));
              }
            }
            if (!slice_id_set.empty()) {
              /// 文件非分片大小的倍数，需要处理
              const auto file_size = thread_data->file_size.load();
              const auto& slice_size = thread_data->per_slice_size;
              if (file_size % slice_size != 0 &&
                  *slice_id_set.crbegin() ==
                      int32_t(1 + file_size / slice_size)) {
                end_offset = slice_size - (file_size % slice_size);
              }
            }
            thread_data->already_upload_bytes =
                slice_id_set.size() * thread_data->per_slice_size - end_offset;
            thread_data->current_upload_bytes = 0;
            thread_data->slice_exist.store(slice_exist);
          }
          /// 额外处理，若分片作废
          if (upload_status == 3) {
            check_slice_upload_proof.result = stage_result::RetryTargetStage;
            check_slice_upload_proof.next_stage = uploader_stage::CreateUpload;
            thread_data->already_upload_bytes = 0;
            thread_data->current_upload_bytes = 0;
            thread_data->slice_exist.Clear();
            break;
          }

          const auto http_statuc_code = GetInt(json_value["httpStatusCode"]);
          const auto int32_error_code = GetInt(json_value["int32ErrorCode"]);

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
              GetInt(json_value["waitingTime"]);
        } while (false);
        return check_slice_upload_proof;
      };

      /// 生成数据源成功，保存到result
      result = rx_assistant::rx_httpresult::create(check_slice_upload_request)
                   .map(solve_check_slice_upload)
                   .map(get_check_slice_upload_result);
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
    /// 加入分片上传文件的流程，用上multi_worker的sliceupload流程
    /// 初始化请求失败，应如此返回
    rxcpp::observable<uploader_proof> result = rxcpp::observable<>::just(
        uploader_proof{uploader_stage::FileUplaod, stage_result::GiveupRetry,
                       uploader_stage::UploadFinal, 0, 0});

    /// 根据thread_data内的数据，生成应返回的proof
    std::function<uploader_proof(sliceupload_helper::Report&)>
        generate_sliceupload_proof =
            std::bind([thread_data_weak]() -> uploader_proof {
              auto proof = uploader_proof{uploader_stage::FileUplaod,
                                          stage_result::GiveupRetry,
                                          uploader_stage::UploadFinal, 0, 0};
              do {
                auto thread_data = thread_data_weak.lock();
                if (nullptr == thread_data) {
                  break;
                }
                /// 增加对用户手动取消的处理
                if (thread_data->frozen.load()) {
                  proof.result = stage_result::UserCanceled;
                  thread_data->int32_error_code =
                      Cloud189::ErrorCode::nderr_usercanceled;
                  break;
                }

                /// 比较应有分片数是否符合（和slice_md5_map的size比较即可）
                /// TODO: 需要根据需上传的分片数进行处理
                const auto should_slice_number =
                    thread_data->slice_md5_map.size();
                const auto actual_slice_number =
                    thread_data->slice_result_map.size();
                if (should_slice_number != actual_slice_number) {
                  break;
                }

                bool is_any_sliceupload_fails = false;
                /// 遍历各分片结果，是否正常（key均大于0，且value均==0）
                auto foreach_cb = [&is_any_sliceupload_fails](
                                      int32_t slice_id,
                                      int32_t slice_result) -> void {
                  if (slice_id <= 0 || slice_result != 0) {
                    is_any_sliceupload_fails = true;
                  }
                };
                thread_data->slice_result_map.ForeachDelegate(foreach_cb);
                if (is_any_sliceupload_fails) {
                  break;
                }
                /// 此处已排除所有验证分片上传结果存在失败的情况
                proof.result = stage_result::Succeeded;
                proof.next_stage = uploader_stage::FileCommit;
              } while (false);
              return proof;
            });
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }

      /// 上传文件这一流程即将开始时，清空current_upload_bytes字段
      /// 避免涉及到弱网络有重传时，对进度记录不准确
      thread_data->current_upload_bytes.store(0);

      /// 需已知文件长度，以进行分割
      const auto file_length = thread_data->file_size.load();
      const auto slicesize = thread_data->per_slice_size;

      auto materials =
          sliceupload_helper::MaterialHelper(file_length, slicesize);
      /// 每次开始传输前，初始化记录各分片上传结果的Map
      thread_data->slice_result_map.Clear();
      /// 需要处理需处理的物料
      const auto slice_exist = thread_data->slice_exist.load();
      if (!slice_exist.empty()) {
        decltype(materials) should_resolve;
        /// 生成已有分片序号
        std::set<int32_t> slice_id_set;
        std::vector<std::string> slice_id_vector;
        assistant::tools::string::StringSplit(slice_exist, ",",
                                              slice_id_vector);
        for (const auto& x : slice_id_vector) {
          if (!x.empty()) {
            slice_id_set.emplace(strtol(x.c_str(), nullptr, 0));
          }
        }
        for (const auto& material : materials) {
          const auto& slice_id = std::get<2>(material);
          auto iter = slice_id_set.find(slice_id);
          if (slice_id_set.end() != iter) {
            /// 此分片无需上传，标定其结果为已完成
            thread_data->slice_result_map.Set(slice_id, 0);
          } else {
            /// 此分片需上传
            should_resolve.emplace_back(material);
          }
        }
        /// 处理完毕，保存到原materials中
        materials.swap(should_resolve);
      }

      auto sliceupload_unique =
          sliceupload_helper::Create(thread_data_weak, materials);
      if (nullptr == sliceupload_unique) {
        break;
      }

      result = sliceupload_unique->GetDataSource().last().map(
          generate_sliceupload_proof);
      thread_data->sliceupload_unique = std::move(sliceupload_unique);
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
      std::string upload_id = thread_data->upload_file_id.load();
      std::string x_request_id = thread_data->x_request_id;
      int32_t oper_type = thread_data->oper_type;
      int32_t is_log = thread_data->is_log;
      std::string file_commit_url = thread_data->file_commit_url.load();
      /// 获取Session
      std::string get_session_key;
      std::string get_session_secret;
      if (!Cloud189::SessionHelper::GetCloud189Session(get_session_key,
                                                       get_session_secret)) {
        break;
      }
      /// 生成MD5List
      std::string slice_md5_list;
      auto generate_md5_list = [&slice_md5_list](
                                   const int32_t slice_id,
                                   const std::string& md5) -> void {
        slice_md5_list += md5 + "\n";
      };
      thread_data->slice_md5_map.ForeachDelegate(generate_md5_list);
      /// 对生成的MD5List做HMAC_SHA1加密
      /// data为MD5List，key为SessionSecret
      std::string slice_md5_list_signature =
          cloud_base::hash_algorithm::GenerateSha1Digest(get_session_secret,
                                                         slice_md5_list);
      /// 对加密结果做MD5加密
      cloud_base::hash_algorithm::MD5 md5_sig(slice_md5_list_signature);
      auto signature_md5 = md5_sig.hex_string();

      std::string slice_file_commit_json_str =
          Cloud189::Apis::CommitSliceUploadFile::JsonStringHelper(
              file_commit_url, upload_id, x_request_id, is_log, oper_type,
              signature_md5);
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

          /// 增加对用户手动取消的处理
          /// 在提交这一步对frozen标志位的检查应延后，避免流程成功但告知结束回调失败
          if (!is_success && thread_data->frozen.load()) {
            slice_file_commit_proof.result = stage_result::UserCanceled;
            thread_data->int32_error_code =
                Cloud189::ErrorCode::nderr_usercanceled;
            break;
          }
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
          thread_data->int32_error_code.store(int32_error_code);
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
    std::function<void(const std::string&)> data_callback)
    : thread_data(InitThreadData(upload_info)),
      data(std::make_unique<details::sliceuploader_internal_data>(
          thread_data->local_filepath, GenerateOrders(thread_data),
          GenerateDataCallback(thread_data, data_callback))) {
  thread_data->master_control_data = data->master_control->data;
  if (!data->Valid()) {
    thread_data->int32_error_code =
        Cloud189::ErrorCode::nderr_file_access_error;
    data->null_file_callback(*data->master_control);
  }
}

void SliceUploader::AsyncStart() {
  if (data->Valid()) {
    data->master_control->AsyncStart();
  }
}

void SliceUploader::SyncWait() {
  if (data->Valid()) {
    data->master_control->SyncWait();
  }
}

/// 在用户手动点击“暂停”或取消，或“退出登录”等需要迫使流程立即失败的场景
/// 非阻塞调用，函数返回不代表立即“取消成功”
/// 将迫使流程尽可能早结束，可能存在延迟
/// 在流程即将完成时，未必能“取消成功”，仍有可能“成功完成”
void SliceUploader::UserCancel() {
  thread_data->frozen.store(true);
  /// TODO: 仍需完善，迫使正在进行的费时操作（如MD5计算、数据传输等）中断
}

SliceUploader::~SliceUploader() = default;

/// 为此Uploader提供一个Helper函数，用于生成合规的json字符串
std::string sliceuploader_info_helper(
    const std::string& local_path, const std::string& last_md5,
    const std::string& last_upload_id, const std::string& parent_folder_id,
    const std::string& x_request_id, const int64_t per_slice_size,
    const int32_t oper_type, const int32_t is_log) {
  Json::Value json_value;
  json_value["localPath"] = local_path;
  json_value["md5"] = last_md5;
  json_value["uploadFileId"] = last_upload_id;
  json_value["parentFolderId"] = parent_folder_id;
  json_value["perSliceSize"] = per_slice_size;
  json_value["opertype"] = oper_type;
  json_value["isLog"] = is_log;
  json_value["X-Request-ID"] = x_request_id;

  return WriterHelper(json_value);
}
}  // namespace Restful
}  // namespace Cloud189
