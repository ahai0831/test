#include <gtest/gtest.h>

#include <Assistant_v3.hpp>

TEST(fileupload, read_mmap) {
  std::shared_ptr<assistant::Assistant_v3> assist =
      std::make_shared<assistant::Assistant_v3>();

  const auto tmpPath = L"upload_test.tmp";
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

  /// Do Upload
  assistant::HttpRequest req_fileupload("PUT", "https://postman-echo.com/put",
                                        std::string());
  req_fileupload.extends.Set("upload_filepath",
                             assistant::tools::string::wstringToUtf8(tmpPath));
  req_fileupload.extends.Set("upload_filesize", std::to_string(kFilesize));
  req_fileupload.extends.Set("upload_offset", std::to_string(0));
  req_fileupload.extends.Set("upload_length", std::to_string(kFilesize));
  auto res_fileupload = assist->SyncHttpRequest(req_fileupload);

  EXPECT_EQ(res_fileupload.status_code, 200);
  const auto kUploadSize =
      strtoll(res_fileupload.extends.Get("size_upload").c_str(), nullptr, 0);
  EXPECT_EQ(kUploadSize, kFilesize);
}
