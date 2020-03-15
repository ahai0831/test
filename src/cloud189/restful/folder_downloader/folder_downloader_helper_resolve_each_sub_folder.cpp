#include "folder_downloader_helper.h"

#include <rx_multiworker.hpp>

#include "cloud189/apis/list_files.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using assistant::HttpRequest;
using Cloud189::Restful::details::folderdownloader_thread_data;
using httpbusiness::folder_downloader::proof::folder_downloader_proof;
using httpbusiness::folder_downloader::proof::folder_downloader_stage;
using httpbusiness::folder_downloader::proof::ProofObsCallback;
using httpbusiness::folder_downloader::proof::stage_result;
using restful_common::jsoncpp_helper::GetInt64;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;
using rx_assistant::HttpResult;

using Cloud189::Apis::ListFiles::HttpRequestEncode;
using Cloud189::Apis::ListFiles::HttpResponseDecode;
using Cloud189::Apis::ListFiles::JsonStringHelper;

namespace {
typedef struct {
  std::string file_id;
  std::string file_name;
  std::string md5;
} sub_file;
typedef struct {
  std::string folder_id;
  std::string folder_name;
} sub_folder;
typedef struct __folderdownload_mateiral_type {
  std::string source_server_folder_id;
  std::string target_local_folder_path;
  std::string x_request_id;
  std::weak_ptr<rx_uv_fs::uv_loop_with_thread> thread;
} folderdownload_mateiral_type;
typedef struct __folderdownload_report_type {
  std::string parent_folder_id;
  std::string local_folder_path;
  std::vector<sub_file> sub_file_data;
} folderdownload_report_type;
typedef struct __folderdownload_done_type {
  std::string source_server_folder_id;
  std::string target_local_folder_path;
  bool valid_local_path;
  std::vector<sub_file> sub_files;
  std::vector<sub_folder> sub_folders;
  std::string x_request_id;
  std::weak_ptr<rx_uv_fs::uv_loop_with_thread> thread;
  __folderdownload_done_type() : valid_local_path(false) {}
} folderdownload_done_type;

typedef struct {
  std::string source_server_folder_id;
  int32_t page_num;
  int32_t page_size;
  std::string x_request_id;
} listfiles_mateiral_type;
typedef struct {
  // std::string source_server_folder_id;
  std::vector<sub_file> sub_files;
  std::vector<sub_folder> sub_folders;
} listfiles_done_type;

/// 在此定义以分页机制列举单层远端目录的MultiWorker结构
typedef httpbusiness::rx_multi_worker_helper<
    listfiles_mateiral_type, listfiles_done_type, listfiles_done_type>
    ListFiles;
/// 对AsyncWork和AfterWork，必须保证其无副作用，或至少是线程安全的
void listfiles_async_work(
    const ListFiles::Material& material,
    const ListFiles::IntermediateProductsFunction intermedia_callback) {
  HttpRequest page_num_listfiles_request("");
  HttpRequestEncode(JsonStringHelper(material.source_server_folder_id, 0, 0, 0,
                                     0, 0, "filename", false, material.page_num,
                                     material.page_size, material.x_request_id),
                    page_num_listfiles_request);
  ///// test
  // page_num_listfiles_request.extends.Set("proxy", "http://127.0.0.1:8888");

  rx_assistant::rx_httpresult::create(page_num_listfiles_request)
      .map([material, intermedia_callback](
               HttpResult value) -> ListFiles::IntermediateProducts {
        ListFiles::IntermediateProducts result;
        std::string response_info;
        HttpResponseDecode(value.res, value.req, response_info);
        Json::Value response_json;
        ReaderHelper(response_info, response_json);
        // result.source_server_folder_id = material.source_server_folder_id;

        const auto& folder = response_json["fileList"]["folder"];
        for (const auto& x : folder) {
          result.sub_folders.emplace_back(
              sub_folder{GetString(x["id"]), GetString(x["name"])});
        }

        const auto& file = response_json["fileList"]["file"];
        for (const auto& x : file) {
          result.sub_files.emplace_back(sub_file{
              GetString(x["id"]), GetString(x["name"]), GetString(x["md5"])});
        }
        intermedia_callback(result);
        return result;
      })
      .publish()
      .connect();
}
void listfiles_after_work(const ListFiles::IntermediateProducts& intermedia,
                          const ListFiles::CalledCallbacks& callbacks) {
  callbacks.send_report(intermedia);
}

/// 在此定义遍历远端目录结构的MultiWorker结构
typedef httpbusiness::rx_multi_worker_helper<folderdownload_mateiral_type,
                                             folderdownload_report_type,
                                             folderdownload_done_type>
    FolderDownlaod;
/// 对AsyncWork和AfterWork，必须保证其无副作用，或至少是线程安全的
void folderdownload_async_work(
    const FolderDownlaod::Material& material,
    const FolderDownlaod::IntermediateProductsFunction intermedia_callback) {
  /// 由于本流程过于复杂，将各步骤说明如下：
  /// 1. 获取指定folder_id的count数量
  /// 2. 以分页机制获取完整的文件夹的数据
  /// 3. 确认本地路径是否已存在，未存在则尝试创建后，再次确认
  /// 4. 把做完的记录交给intermedia_callback

  HttpRequest get_filenum_listfiles_request("");
  HttpRequestEncode(
      JsonStringHelper(material.source_server_folder_id, 0, 0, 0, 0, 0,
                       "filename", false, 1, 1, material.x_request_id),
      get_filenum_listfiles_request);
  ///// test
  // get_filenum_listfiles_request.extends.Set("proxy",
  // "http://127.0.0.1:8888");

  rx_assistant::rx_httpresult::create(get_filenum_listfiles_request)
      .map([](HttpResult value) -> int64_t {
        std::string response_info;
        HttpResponseDecode(value.res, value.req, response_info);
        Json::Value response_json;
        ReaderHelper(response_info, response_json);

        const auto count = GetInt64(response_json["fileList"]["count"]);
        return count;
      })
      .flat_map(
          [material](int64_t value) -> rxcpp::observable<listfiles_done_type> {
            /// TODO:
            /// 这样暂时修复了count为0导致的阻塞性问题，但不优雅，需要用rx的方式优雅处理下
            if (0 == value) {
              return rxcpp::observable<>::just(listfiles_done_type{});
            }
            ListFiles::MaterialVector materials;
            const auto page_size = 1000L;
            auto page_num = 1L;
            for (int64_t i = 0; i < value; i += page_size, ++page_num) {
              materials.emplace_back(listfiles_mateiral_type{
                  material.source_server_folder_id, page_num, page_size,
                  material.x_request_id});
            }
            auto listfiles_unique = ListFiles::Create(
                materials, listfiles_async_work, listfiles_after_work, 1);
            std::shared_ptr<ListFiles> listfiles_shared =
                std::move(listfiles_unique);
            /// TODO: 此处可优化内存占用，使用+重载后，sum(）操作符可用
            return listfiles_shared->GetDataSource()
                .scan(listfiles_done_type{},
                      [listfiles_shared](
                          listfiles_done_type seed,
                          listfiles_done_type v) -> listfiles_done_type {
                        seed.sub_files.reserve(seed.sub_files.size() +
                                               v.sub_files.size());
                        seed.sub_files.insert(seed.sub_files.end(),
                                              v.sub_files.begin(),
                                              v.sub_files.end());
                        seed.sub_folders.reserve(seed.sub_folders.size() +
                                                 v.sub_folders.size());
                        seed.sub_folders.insert(seed.sub_folders.end(),
                                                v.sub_folders.begin(),
                                                v.sub_folders.end());
                        return seed;
                      })
                .last();
          })

      .flat_map(
          [material](listfiles_done_type&) -> rxcpp::observable<bool> {
            return rx_uv_fs::rx_uv_fs_factory::Mkdir(
                       material.thread, material.target_local_folder_path)
                .flat_map([material](int32_t) -> rxcpp::observable<bool> {
                  return rx_uv_fs::rx_uv_fs_factory::Stat(
                             material.thread, material.target_local_folder_path)
                      .map([](int32_t value) -> bool {
                        return 0x4000 == value;
                      });
                });
          },
          [material, intermedia_callback](listfiles_done_type res1, bool res2)
              -> FolderDownlaod::IntermediateProducts {
            FolderDownlaod::IntermediateProducts intermediate;
            intermediate.source_server_folder_id =
                material.source_server_folder_id;
            intermediate.target_local_folder_path =
                material.target_local_folder_path;
            intermediate.x_request_id = material.x_request_id;
            intermediate.thread = material.thread;
            intermediate.sub_files.swap(res1.sub_files);
            intermediate.sub_folders.swap(res1.sub_folders);
            intermediate.valid_local_path = res2;

            intermedia_callback(intermediate);
            return intermediate;
          })
      .publish()
      .connect();
}
void folderdownload_after_work(
    const FolderDownlaod::IntermediateProducts& intermedia,
    const FolderDownlaod::CalledCallbacks& callbacks) {
  for (const auto& sub_folder : intermedia.sub_folders) {
    callbacks.extra_material(folderdownload_mateiral_type{
        sub_folder.folder_id,
        intermedia.target_local_folder_path + sub_folder.folder_name + "/",
        intermedia.x_request_id, intermedia.thread});
  }

  callbacks.send_report(folderdownload_report_type{
      intermedia.source_server_folder_id, intermedia.target_local_folder_path,
      intermedia.sub_files});

  /// TODO: 需要检查主控是否已经“frozen”，如有需尽快停止整个multi_worker流程
}
}  // namespace
namespace Cloud189 {
namespace Restful {
namespace folder_downloader_helper {
namespace details {
ProofObsCallback resolve_each_sub_folder(
    const std::weak_ptr<folderdownloader_thread_data>& thread_data_weak) {
  return [thread_data_weak](folder_downloader_proof)
             -> rxcpp::observable<folder_downloader_proof> {
    rxcpp::observable<folder_downloader_proof> result =
        rxcpp::observable<>::just(folder_downloader_proof{
            folder_downloader_stage::ResolveEachSubFolder,
            stage_result::GiveupRetry});
    do {
      auto thread_data = thread_data_weak.lock();
      if (nullptr == thread_data) {
        break;
      }
      const auto& source_server_folder_id =
          thread_data->remote_root_server_folder_id.load();
      std::string target_local_folder_path =
          thread_data->download_path +
          thread_data->remote_root_server_folder_name.load() + "/";
      const auto& x_request_id = thread_data->x_request_id;
      const auto& uv_thread = thread_data->uv_thread;
      FolderDownlaod::MaterialVector materials{folderdownload_mateiral_type{
          source_server_folder_id, target_local_folder_path, x_request_id,
          uv_thread}};

      auto folderdownload_unique = FolderDownlaod::Create(
          materials, folderdownload_async_work, folderdownload_after_work, 1);
      std::shared_ptr<FolderDownlaod> folderdownload_shared =
          std::move(folderdownload_unique);

      result =
          folderdownload_shared->GetDataSource()
              .tap([folderdownload_shared, thread_data_weak](
                       folderdownload_report_type& value) -> void {
                do {
                  auto thread_data = thread_data_weak.lock();
                  if (nullptr == thread_data) {
                    break;
                  }
                  const auto& data_callback = thread_data->data_callback;
                  Json::Value data_json;
                  data_json["download_folder_path"] = value.local_folder_path;
                  Json::Value sub_file_data;
                  for (const auto& sub_file : value.sub_file_data) {
                    Json::Value sub;
                    sub["file_id"] = sub_file.file_id;
                    sub["file_name"] = sub_file.file_name;
                    sub["md5"] = sub_file.md5;
                    sub_file_data.append(sub);
                  }
                  data_json["sub_file_data"] = sub_file_data;
                  std::string data_info;
                  WriterHelper(data_json, data_info);
                  data_callback(data_info);
                } while (false);
              })
              .last()
              .map([folderdownload_shared](
                       folderdownload_report_type&) -> folder_downloader_proof {
                return folder_downloader_proof{
                    folder_downloader_stage::ResolveEachSubFolder,
                    stage_result::Succeeded};
              });
    } while (false);
    return result;
  };
}
}  // namespace details
}  // namespace folder_downloader_helper
}  // namespace Restful
}  // namespace Cloud189
