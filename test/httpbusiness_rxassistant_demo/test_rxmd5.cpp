#include <rx_md5.hpp>

#include <Assistant_v3.hpp>

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
