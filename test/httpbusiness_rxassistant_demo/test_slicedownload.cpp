#include <cinttypes>

#include <Assistant_v3.hpp>
#include <slicedownload_mastercontrol.hpp>

#include <gtest/gtest.h>

void slicedownload_sync_test(const char* download_url, const char* file_path) {
  auto a2_ptr = std::make_shared<assistant::Assistant_v3>();

  httpbusiness::SlicedownloadMastercontrol sdl(a2_ptr);
  sdl.AsyncProcess(download_url, file_path);
  auto fComplete = []() -> void {};
  auto myNext = [](uint64_t v) { printf("smooth_speed: %" PRIu64 "\n", v); };
  auto hi_c = sdl.RegSpeedCallback(myNext, fComplete);
  sdl.SyncWait();
}

void slicedownload_sync_5seconds_test(const char* download_url,
                                      const char* file_path) {
  auto a2_ptr = std::make_shared<assistant::Assistant_v3>();

  httpbusiness::SlicedownloadMastercontrol sdl(a2_ptr);
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

TEST(slicedownload_test, default_gtest) { ASSERT_TRUE(true); }
const auto tmpPath_0 = L"test_movie.mp4";
const auto tmpPath_0_utf8 = "test_movie.mp4";
const auto tmpPath = L"cloud189_v8.2.0_1568206533185.apk";
const auto tmpPath_utf8 = "cloud189_v8.2.0_1568206533185.apk";
TEST(slicedownload_test, sync_wait) {
  /// Alloc a tmpFile
  FILE* fp = _wfsopen(tmpPath_0, L"w+", _SH_DENYNO);
  assistant::tools::scope_guard guard([&fp]() -> void {
    if (nullptr != fp) {
      fclose(fp);
      fp = nullptr;
      _wremove(tmpPath_0);
    }
  });
  slicedownload_sync_test("https://www.w3school.com.cn/i/movie.mp4",
                          tmpPath_0_utf8);
  ASSERT_TRUE(true);
}

TEST(slicedownload_test, sync_wait_5s) {
  /// Alloc a tmpFile
  FILE* fp = _wfsopen(tmpPath, L"w+", _SH_DENYNO);
  assistant::tools::scope_guard guard([&fp]() -> void {
    if (nullptr != fp) {
      fclose(fp);
      fp = nullptr;
      _wremove(tmpPath);
    }
  });
  slicedownload_sync_5seconds_test(
      "http://download.cloud.189.cn/download/client/android/"
      "cloud189_v8.2.0_1568206533185.apk",
      tmpPath_utf8);
  ASSERT_TRUE(true);
}
