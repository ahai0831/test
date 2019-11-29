#include <rx_md5.hpp>

#include <Assistant_v3.hpp>

#include <speed_counter.hpp>

#include <gtest/gtest.h>

TEST(rxmd5_test, async_md5) {
  const auto tmpPath = L"rxmd5_test.tmp";
  const char testData[] = "abcdefghijklmn";

  /// Alloc a tmpFile, write Data
  FILE *fp = _wfsopen(tmpPath, L"w+", _SH_DENYNO);
  EXPECT_NE(fp, nullptr);
  assistant::tools::scope_guard guard_fp([&fp]() -> void {
    if (nullptr != fp) {
      fclose(fp);
      fp = nullptr;
    }
  });
  fwrite(testData, sizeof(testData[0]), sizeof(testData) / sizeof(testData[0]),
         fp);
  fflush(fp);
  /// Get Filesize
  _fseeki64(fp, 0, SEEK_END);
  const auto kFilesize = _ftelli64(fp);
  if (nullptr != fp) {
    fclose(fp);
    fp = nullptr;
  }
  /// Protect tmpFile
  FILE *file_protect = _wfsopen(tmpPath, L"r", _SH_DENYWR);
  EXPECT_NE(file_protect, nullptr);
  assistant::tools::scope_guard guard_file_protect(
      [&file_protect, tmpPath]() -> void {
        if (nullptr != file_protect) {
          fclose(file_protect);
          file_protect = nullptr;
          _wremove(tmpPath);
        }
      });

  auto hahaha = rx_assistant::md5::md5_async_factory::create(
      assistant::tools::string::wstringToUtf8(tmpPath),
      [](const std::string &s) { printf("%s\n", s.c_str()); });
}

TEST(rxmd5_test, async_md5_with_process) {
  const auto tmpPath = L"rxmd5_test_bigfile.tmp";
  const char testData[] = "abcdefghijklmn";

  /// Alloc a tmpFile, write Data, big enough
  FILE *fp = _wfsopen(tmpPath, L"w+", _SH_DENYNO);
  EXPECT_NE(fp, nullptr);
  assistant::tools::scope_guard guard_fp([&fp]() -> void {
    if (nullptr != fp) {
      fclose(fp);
      fp = nullptr;
    }
  });
  fseek(fp, 0x40000000, SEEK_SET);
  fwrite(testData, sizeof(testData[0]), sizeof(testData) / sizeof(testData[0]),
         fp);
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
  FILE *file_protect = _wfsopen(tmpPath, L"r", _SH_DENYWR);
  EXPECT_NE(file_protect, nullptr);
  assistant::tools::scope_guard guard_file_protect(
      [&file_protect, tmpPath]() -> void {
        if (nullptr != file_protect) {
          fclose(file_protect);
          file_protect = nullptr;
          _wremove(tmpPath);
        }
      });

  /// Use speed counter
  httpbusiness::progress_notifier process;
  process.total_number = kFilesize;
  rx_assistant::md5::Md5ProcessCallback process_callback =
      [&process](int64_t v) -> void { process.finished_number += v; };

  auto hahaha = rx_assistant::md5::md5_async_factory::create(
      assistant::tools::string::wstringToUtf8(tmpPath),
      [&process](const std::string &s) {
        printf("%s\n", s.c_str());
        process.finished_flag = true;
      },
      process_callback);

  process.RegSubscription(
      [](float v) { printf("Md5 process: %.2f%%\n", v * 100); }, []() {});
}
