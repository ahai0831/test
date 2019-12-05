#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif
#include <gtest/gtest.h>

#include <thread>

#include <rx_multiworker.hpp>

#include <rx_md5.hpp>
#include <rx_uploader.hpp>
#include <tools/safecontainer.hpp>

TEST(rxmultiworker_test, init) {
  using soboring = httpbusiness::rx_multi_worker<int32_t, bool>;
  soboring::WorkerCallback worker = [](soboring::CalledCallbacks callbacks) {
    std::function<void(int32_t)> do_some_delay = [](int32_t v) -> void {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      printf("%d\n", v);
    };
    /// 线程模型，接收一个参数，在detach的线程中，处理物料，并调用回调
    std::function<void(int32_t, std::function<void(bool)>)>
        async_worker_function =
            [do_some_delay](int32_t material,
                            std::function<void(bool)> done_callback) -> void {
      std::thread th([do_some_delay, material, done_callback]() -> void {
        do_some_delay(material);
        /// 后续可根据do_some_delay的返回做判断

        done_callback(true);
      });
      th.detach();
    };

    static std::function<void(bool)> do_some_delay_callback =
        [callbacks, async_worker_function](bool report) -> void {
      /// 不产生额外物料，无需调用extra_material

      /// 进行报告
      callbacks.send_report(true);

      /// 尝试下一轮生产
      decltype(callbacks.get_material()) material_after = nullptr;
      if (!callbacks.check_stop()) {
        material_after = callbacks.get_material();
      }
      callbacks.get_material_result(nullptr != material_after ? 0 : -1);
      if (nullptr != material_after) {
        async_worker_function(*material_after, do_some_delay_callback);
      }
    };
    decltype(callbacks.get_material()) material_unique = nullptr;
    if (!callbacks.check_stop()) {
      material_unique = callbacks.get_material();
    }
    callbacks.get_material_result(nullptr != material_unique ? 1 : 0);
    if (nullptr != material_unique) {
      async_worker_function(*material_unique, do_some_delay_callback);
    }
  };

  /// 初始物料
  std::vector<int32_t> materials{12, 3, 5, 7, 9, 13, 17, 19, 23, 29, 31};

  auto ddd = soboring::Create(materials, worker, 2);
  std::thread stop_thread([&ddd]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    ddd->Stop();
  });
  ddd->data_source.as_blocking().subscribe(
      [](bool v) { printf("success?%d\n", v ? 1 : 0); },
      []() { printf("complete\n"); });
  if (stop_thread.joinable()) {
    stop_thread.join();
  }
  // 	std::this_thread::sleep_for(std::chrono::milliseconds(30000));
  ASSERT_TRUE(true);
}

// using SliceMd5 =
//     httpbusiness::rx_multi_worker<std::tuple<int64_t, int64_t, int32_t>,
//                                   std::tuple<int32_t, std::string>>;
struct slicemd5_worker_function_generator;
struct uploader_thread_data {
 public:
  /// const的数据成员，必须在构造函数中进行显式的初始化
  const std::string local_filepath;
  const std::string last_md5;
  const std::string last_upload_id;
  const int64_t fileupload_slicesize;
  /// 禁用移动复制默认构造和=号操作符，必须通过显式指定所有必须字段进行初始化
  uploader_thread_data(const std::string& file_path,
                       const std::string& last_trans_md5,
                       const std::string& last_trans_upload_id,
                       const int64_t slice_size)
      : local_filepath(file_path),
        last_md5(last_trans_md5),
        last_upload_id(last_trans_upload_id),
        fileupload_slicesize(slice_size) /*, slice_number({0})*/ {}
  /// 线程安全的数据成员
  assistant::tools::lockfree_string_closure<std::string> file_md5;
  assistant::tools::safemap_closure<int32_t, std::string> slice_md5;
  assistant::tools::lockfree_string_closure<std::string> upload_id;
  std::unique_ptr<slicemd5_worker_function_generator> slicemd5_unique;

 private:
  uploader_thread_data() = delete;
  uploader_thread_data(uploader_thread_data const&) = delete;
  uploader_thread_data& operator=(uploader_thread_data const&) = delete;
  uploader_thread_data(uploader_thread_data&&) = delete;
};
typedef struct slicemd5_worker_function_generator {
 private:
  typedef httpbusiness::rx_multi_worker<std::tuple<int64_t, int64_t, int32_t>,
                                        std::tuple<int32_t, std::string>>
      SliceMd5;
  explicit slicemd5_worker_function_generator(
      const std::weak_ptr<uploader_thread_data>& weak,
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
  ~slicemd5_worker_function_generator() = default;
  static std::unique_ptr<slicemd5_worker_function_generator> Create(
      const std::weak_ptr<uploader_thread_data>& weak,
      const MaterialVector& materials) {
    return std::unique_ptr<slicemd5_worker_function_generator>(
        new (std::nothrow) slicemd5_worker_function_generator(weak, materials));
  }
  DataSource GetDataSource() { return slicemd5_unique->data_source; }
  void Stop() { slicemd5_unique->Stop(); }

 private:
  std::weak_ptr<uploader_thread_data> thread_data_weak;
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

TEST(rxmultiworker_test, Slice_MD5) {
  /// 生成测试文件
  const auto tmpPath = L"rxmd5_test_bigfile_2.tmp";
  const char testData[] = "abcdefghijklmn";

  /// Alloc a tmpFile, write Data, big enough
  FILE* fp = _wfsopen(tmpPath, L"w+", _SH_DENYNO);
  EXPECT_NE(fp, nullptr);
  assistant::tools::scope_guard guard_fp([&fp]() -> void {
    if (nullptr != fp) {
      fclose(fp);
      fp = nullptr;
    }
  });
  fseek(fp, 0x40000000, SEEK_SET);
  fwrite(testData, sizeof(testData[0]),
         sizeof(testData) / sizeof(testData[0]) - 1, fp);
  printf("Flush about 1GB file to disk, taking a while......\n");
  fflush(fp);
  /// Get Filesize
  _fseeki64(fp, 0, SEEK_END);
  const auto kFilesize = _ftelli64(fp);
  if (nullptr != fp) {
    fclose(fp);
    fp = nullptr;
  }
  /// Protect tmpFile
  FILE* file_protect = _wfsopen(tmpPath, L"r", _SH_DENYWR);
  EXPECT_NE(file_protect, nullptr);
  assistant::tools::scope_guard guard_file_protect(
      [&file_protect, tmpPath]() -> void {
        if (nullptr != file_protect) {
          fclose(file_protect);
          file_protect = nullptr;
          _wremove(tmpPath);
        }
      });

  /// 具体流程：
  /// 根据已有信息：
  /// 文件路径。
  /// 文件分片长度。
  /// 输出：
  /// 文件的整体MD5
  /// 以分片号为序号的MD5
  std::shared_ptr<uploader_thread_data> thread_data =
      std::make_shared<uploader_thread_data>(
          assistant::tools::string::wstringToUtf8(tmpPath), "", "",
          6 * (2 << 20));
  std::weak_ptr<uploader_thread_data> thread_data_weak = thread_data;

  auto md5_obs =
      rx_assistant::rx_md5::create(thread_data->local_filepath.c_str());

  /// 流程，先算好文件的整片MD5，保存起来后，进行文件各分片的计算
  auto slicemd5_obs = md5_obs.flat_map([thread_data_weak](
                                           const std::string& file_md5)
                                           -> slicemd5_helper::DataSource {
    slicemd5_helper::DataSource result;
    auto thread_data = thread_data_weak.lock();
    if (nullptr != thread_data) {
      thread_data->file_md5.store(file_md5);
      /// 准备好计算分片所需物料
      /// 需已知文件长度，以进行分割
      uint64_t file_length = 0;
      auto file_exist = cloud_base::filesystem_helper::GetFileSize(
          assistant::tools::string::utf8ToWstring(thread_data->local_filepath),
          file_length);
      slicemd5_helper::MaterialVector materials;
      const int64_t length = file_length;
      const auto& slicesize = thread_data->fileupload_slicesize;
      int32_t slice_id = 1;
      for (int64_t i = 0; i < length; i += slicesize, ++slice_id) {
        const auto range_left = i;
        const auto range_right =
            i + slicesize > length ? length - 1 : i + slicesize - 1;
        materials.emplace_back(
            std::make_tuple(range_left, range_right, slice_id));
      }

      /// 建立计算文件分片MD5的控制块
      thread_data->slicemd5_unique =
          slicemd5_helper::Create(thread_data_weak, materials);
      result = thread_data->slicemd5_unique->GetDataSource();
    }
    return result;
  });

  /// 流程，保存好文件各分片的MD5
  /// TODO: 和上一次的续传记录进行比对的操作需补充
  auto proof_obs =
      slicemd5_obs
          .tap([thread_data_weak](const std::tuple<int32_t, std::string>& tp) {
            auto thread_data = thread_data_weak.lock();
            if (nullptr != thread_data) {
              thread_data->slice_md5.Set(std::get<0>(tp), std::get<1>(tp));
            }
          })
          .last()
          .map([thread_data_weak](const std::tuple<int32_t, std::string>&)
                   -> httpbusiness::uploader::proof::uploader_proof {
            httpbusiness::uploader::proof::uploader_proof
                calculate_md5_result_proof = {
                    httpbusiness::uploader::proof::uploader_stage::CalculateMd5,
                    httpbusiness::uploader::proof::stage_result::Succeeded,
                    httpbusiness::uploader::proof::uploader_stage::
                        UploadInitial,
                    0, 0};

            int i = 0;
            ++i;
            return calculate_md5_result_proof;
          });

  proof_obs.as_blocking().subscribe(
      [](httpbusiness::uploader::proof::uploader_proof) {});
}
