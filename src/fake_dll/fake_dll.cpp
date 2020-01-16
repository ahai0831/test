#define GENERAL_RESTFUL_SDK_EXPORTS
#include "fake_dll.h"

#include <iostream>
#include <thread>
//#include <unistd.h>
#include <json\json.h>
#include <chrono>
#include <functional>
#include <future>

void AstProcess(const char *process_info, OnProcessStart on_start,
                OnProcessCallback on_callback) {
  printf("%s", process_info);
  on_start("f7c6408b-a0df-c66e-4844-c41d8f235f11");

  //由于promise是uncopyable的，所以用了move去捕获。c++14
  std::thread t([on_callback]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 1s睡眠
    on_callback(".callback1\n");

    std::this_thread::sleep_for(std::chrono::seconds(1));  // 1s睡眠
    on_callback("..callback2\n");

    std::this_thread::sleep_for(std::chrono::seconds(1));  // 1s睡眠
    on_callback("...callback finished\n");
  });

  t.detach();
}

void AstConfig(const char *json_str, OnConfigFinished on_config_finished) {
  Json::Value json_str_value = json_str;
  do {
    if (4 != json_str_value.size()) {
      std::cout << "length error" << std::endl;
      break;
    }
    for (const auto &i : json_str_value) {
      std::cout << i.asString() << std::endl;
    }
  } while (false);
}
