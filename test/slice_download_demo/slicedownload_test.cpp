#include "slicedownload_test.h"

#include <cinttypes>

#include <Assistant_v2.h>
#include <slicedownload_mastercontrol.h>

void slicedownload_sync_test(const char* download_url, const char* file_path) {
  auto a2_ptr = std::make_shared<assistant::Assistant_v2>();

  SlicedownloadMastercontrol sdl(a2_ptr);
  sdl.AsyncProcess(download_url, file_path);
  auto fComplete = []() -> void {};
  auto myNext = [](uint64_t v) { printf("smoothing_speed: %" PRIu64 "\n", v); };
  auto hi_c = sdl.RegSpeedCallback(myNext, fComplete);
  sdl.SyncWait();
}
