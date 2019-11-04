#include <gtest/gtest.h>

#include <Assistant_v3.hpp>

TEST(filedownload, write_mmap) {
  std::shared_ptr<assistant::Assistant_v3> assist =
      std::make_shared<assistant::Assistant_v3>();

  const auto kTestUrl = "https://www.w3school.com.cn/i/movie.mp4";
  const auto tmpPath = L"test.mp4";
  /// Get Filesize
  assistant::HttpRequest req_filesize(kTestUrl);
  req_filesize.extends.Set("range", "0-0");
  req_filesize.extends.Set("header_only", "true");

  auto res_filesize = assist->SyncHttpRequest(req_filesize);
  EXPECT_EQ(res_filesize.status_code, 206);
  const auto kFilesize = strtoll(
      res_filesize.extends.Get("content_range_tatal").c_str(), nullptr, 0);

  /// Alloc a tmpFile
  FILE *fp = _wfsopen(tmpPath, L"w+", _SH_DENYNO);
  assistant::tools::scope_guard guard([&fp, tmpPath]() -> void {
    if (nullptr != fp) {
      fclose(fp);
      fp = nullptr;
      _wremove(tmpPath);
    }
  });

  /// Download File
  assistant::HttpRequest req_filedownload(res_filesize.effective_url);
  req_filedownload.extends.Set(
      "range", std::to_string(0) + "-" + std::to_string(kFilesize - 1));
  req_filedownload.extends.Set(
      "download_filepath", assistant::tools::string::wstringToUtf8(tmpPath));
  req_filedownload.extends.Set("download_filesize", std::to_string(kFilesize));
  req_filedownload.extends.Set("download_offset", std::to_string(0));
  req_filedownload.extends.Set("download_length", std::to_string(kFilesize));
  /// 对下载类传输http请求，应将传输超时（未收到1Byte）缩短到10S（默认值60S）
  /// 避免过长时间占着worker数但不贡献下载速率 2019.9.22
  req_filedownload.extends.Set("transfer_timeout", std::to_string(10));
  auto res_filedownload = assist->SyncHttpRequest(req_filedownload);
  EXPECT_EQ(res_filedownload.status_code, 206);
  const auto kDownloadSize = strtoll(
      res_filedownload.extends.Get("size_download").c_str(), nullptr, 0);
  EXPECT_EQ(kFilesize, kDownloadSize);
}
