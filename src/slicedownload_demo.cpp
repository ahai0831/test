
#include <chrono>
#include <cinttypes>
#include <thread>

#include <Assistant_v2.h>
#include <slicedownload_mastercontrol.h>

int main() {
  auto a2_ptr = std::make_shared<assistant::Assistant_v2>();
  rx_assistant::rx_assistant_factory ast_factory(a2_ptr);
  std::string req_str("https://download.cloud.189.cn/file/downloadFile.action?dt=51&expired=1564909358845&sk=87ce179e-0305-40e1-9578-93986a0ec365&ufi=8152222650355362&zyc=5&token=cloud16&sig=qTklsKx9VnTTfXSDHdnOgY%2FgoJY%3D");
      

  SlicedownloadMastercontrol sdl(a2_ptr);
  sdl.AsyncProcess(req_str, "xp2.iso");
  auto another_thread = std::thread([&sdl]() {
    // 	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    // 	sdl.AsyncStop();
  });
  auto fNext = [](const char* str) -> void { printf("%s\n", str); };
  auto fComplete = []() -> void {};
  auto myNext = [fNext](uint64_t v) {
    auto str = std::to_string(v);
    fNext(str.c_str());
  };
  auto hi_c = sdl.RegSpeedCallback(myNext, fComplete);
  sdl.SyncWait();

  if (another_thread.joinable()) {
    another_thread.join();
  }
  // 	std::this_thread::sleep_for(std::chrono::milliseconds(500));

  return 0;
}
