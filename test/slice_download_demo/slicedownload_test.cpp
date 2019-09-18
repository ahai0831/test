#include "slicedownload_test.h"

#include <cinttypes>

#include <Assistant_v2.h>
#include <slicedownload_mastercontrol.h>
static struct InitAssistantOnce {
  InitAssistantOnce() {
    auto& pile = assistant::a1_0::HttprequestPile();
    /// print request
    pile.Put([](assistant::HttpRequest& req) -> void {
      printf("Do request: %s\n", req.url.c_str());
    });
    /// set proxy
    pile.Put([](assistant::HttpRequest& req) -> void {
      req.extends.Set("proxy", "http://127.0.0.1:9999");
    });
  }
} init_assistant_once;

void slicedownload_sync_test(const char* download_url, const char* file_path) {
  auto a2_ptr = std::make_shared<assistant::Assistant_v2>();

  SlicedownloadMastercontrol sdl(a2_ptr);
  sdl.AsyncProcess(download_url, file_path);
  auto fComplete = []() -> void {};
  auto myNext = [](uint64_t v) { printf("smooth_speed: %" PRIu64 "\n", v); };
  auto hi_c = sdl.RegSpeedCallback(myNext, fComplete);
  sdl.SyncWait();
}

void slicedownload_sync_5seconds_test(const char* download_url,
                                      const char* file_path) {
  auto a2_ptr = std::make_shared<assistant::Assistant_v2>();

  SlicedownloadMastercontrol sdl(a2_ptr);
  sdl.AsyncProcess(download_url, file_path);
  auto fComplete = []() -> void {};
  auto myNext = [](uint64_t v) { printf("smooth_speed: %" PRIu64 "\n", v); };
  auto hi_c = sdl.RegSpeedCallback(myNext, fComplete);
  std::thread stop_thread(
      [&sdl](int32_t million_seconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(million_seconds));
        sdl.AsyncStop();
      },
      5000);
  sdl.SyncWait();
  if (stop_thread.joinable()) {
    stop_thread.join();
  }
}
