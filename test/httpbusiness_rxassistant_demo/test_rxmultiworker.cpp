#include <gtest/gtest.h>

#include <thread>

#include <rx_multiworker.hpp>
TEST(rxmultiworker_test, init) {
  using soboring = httpbusiness::rx_multi_worker<int32_t, bool>;
  soboring::WorkerCallback worker =
      [](soboring::GetMaterialCallback get_material,
         soboring::GetMaterialResultCallback get_material_result,
         soboring::ExtraMaterialCallback extra_material,
         soboring::ReportCallback send_report,
         soboring::CheckStopCallback check_stop,
         soboring::SeriousErrorCallback serious_error) {
        std::function<void(int32_t)> do_some_delay = [](int32_t v) -> void {
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          printf("%d\n", v);
        };
        /// 线程模型，接收一个参数，在detach的线程中，处理物料，并调用回调
        std::function<void(int32_t, std::function<void(bool)>)>
            async_worker_function =
                [do_some_delay](
                    int32_t material,
                    std::function<void(bool)> done_callback) -> void {
          std::thread th([do_some_delay, material, done_callback]() -> void {
            do_some_delay(material);
            /// 后续可根据do_some_delay的返回做判断

            done_callback(true);
          });
          th.detach();
        };

        static std::function<void(bool)> do_some_delay_callback =
            [send_report, get_material, get_material_result,
             async_worker_function,
             check_stop /*, &do_some_delay_callback*/](bool report) -> void {
          /// 不产生额外物料，无需调用extra_material

          /// 进行报告
          send_report(true);

          /// 尝试下一轮生产
          decltype(get_material()) material_after = nullptr;
          if (!check_stop()) {
            material_after = get_material();
          }
          get_material_result(nullptr != material_after ? 0 : -1);
          if (nullptr != material_after) {
            async_worker_function(*material_after, do_some_delay_callback);
          }
        };
        decltype(get_material()) material_unique = nullptr;
        if (!check_stop()) {
          material_unique = get_material();
        }
        get_material_result(nullptr != material_unique ? 1 : 0);
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
