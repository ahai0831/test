#include <gtest/gtest.h>

#include <speed_counter.hpp>

TEST(speedcounter_test, normal_speed_counter) {
  httpbusiness::speed_counter count_speed;
  std::thread external_thread([&count_speed]() -> void {
    for (int i = 0; i < 200; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      count_speed.finished_bytes += 10 * i;
    }
  });
  std::thread setfinish_thread([&count_speed]() -> void {
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    count_speed.finished_flag = true;
  });
  /// 阻塞直至完成
  std::promise<void> wait;
  auto wait_signal = wait.get_future();
  count_speed.RegSubscription([](int64_t v) { printf("%" PRIi64 "\n", v); },
                              [&wait]() {
                                printf("OnComplete\n");
                                wait.set_value();
                              });

  wait_signal.wait();
  if (setfinish_thread.joinable()) {
    setfinish_thread.join();
  }
  if (external_thread.joinable()) {
    external_thread.join();
  }
}

TEST(speedcounter_test, speed_counter_with_stop) {
  httpbusiness::speed_counter_with_stop count_speed;
  std::thread external_thread([&count_speed]() -> void {
    for (int i = 0; i < 200; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      count_speed.Add(10 * i);
    }
  });
  std::thread setfinish_thread([&count_speed]() -> void {
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    count_speed.Stop();
  });
  /// 阻塞直至完成
  std::promise<void> wait;
  auto wait_signal = wait.get_future();
  count_speed.RegSubscription([](int64_t v) { printf("%" PRIi64 "\n", v); },
                              [&wait]() {
                                printf("OnComplete\n");
                                wait.set_value();
                              });

  wait_signal.wait();
  if (setfinish_thread.joinable()) {
    setfinish_thread.join();
  }
  if (external_thread.joinable()) {
    external_thread.join();
  }
}
