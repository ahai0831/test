#include <future>
#include <memory>

#include "cloud189/restful/file_downloader/file_downloader.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using Cloud189::Restful::Downloader;

static std::promise<void> test_for_finished;
static std::future<void> complete_signal;

int main(void) {
  Cloud189::SessionHelper::Cloud189Login(
      "5ed26416-0c6f-4005-9e7c-1beddd529a47",
      "B94DF5E701085B74B515F5D87AC7DE55",
      "dddd3bf5-95c5-4735-a1e2-644a670e617d_family",
      "2D6FBBD2630E71A626614DFFD599696E");
  Json::Value root;
  root["file_id"] = "9147022472321875";
  root["file_name"] = "MSYS-20111123.zip";
  root["md5"] = "CCE436DE44B5ACF0F4F9A96BD4C1EF03";
  root["download_folder_path"] = "D:/test/";
  // root["last_download_breakpoint_data"] =
  // "eyJkb3dubG9hZF9mb2xkZXJfcGF0aCI6IkQ6L3Rlc3QvIiwiZmlsZV9pZCI6IjkxNDcwMjI0NzIzMjE4NzUiLCJmaWxlX25hbWUiOiJNU1lTLTIwMTExMTIzLnppcCIsIm1kNSI6IkNDRTQzNkRFNDRCNUFDRjBGNEY5QTk2QkQ0QzFFRjAzIiwidGVtcF9kb3dubG9hZF9maWxlX3BhdGgiOiJEOi90ZXN0L01TWVMtMjAxMTExMjMuemlwXzkuZWNkbCIsInRvX2JlX2NvbnRpbnVlZCI6eyJsZW5ndGgiOjQ3MjQwMzAzLCJvZmZzZXQiOjM3NjkzNzh9LCJ2ZXIiOiIwLjEifQ==";
  const auto download_info = restful_common::jsoncpp_helper::WriterHelper(root);
  std::unique_ptr<Downloader> dlr;
  auto callback = [&dlr](const std::string& str) {
    printf("%s\n", str.c_str());
    Json::Value root;
    restful_common::jsoncpp_helper::ReaderHelper(str, root);
    const auto is_complete =
        restful_common::jsoncpp_helper::GetBool(root["is_complete"]);
    if (is_complete) {
      dlr = nullptr;
      test_for_finished.set_value();
    }
  };
  dlr = std::make_unique<Downloader>(download_info.c_str(), callback);
  complete_signal = test_for_finished.get_future();
  dlr->AsyncStart();

  complete_signal.wait();

  return 0;
}
