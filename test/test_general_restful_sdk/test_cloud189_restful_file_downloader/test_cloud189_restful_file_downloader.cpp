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
  root["file_id"] = "2149022475894387";
  root["file_name"] = "mingw-w64-install (1).exe";
  root["md5"] = "9670C3701F0B546CA63A3E6D7749E59E";
  root["download_folder_path"] = "D:/test/";
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
