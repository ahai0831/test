#include <future>
#include <memory>

#include "cloud189/restful/folder_downloader/folder_downloader.h"
#include "cloud189/restful/file_downloader/file_downloader.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using Cloud189::Restful::FolderDownloader;
using Cloud189::Restful::Downloader;
//通过文件夹下载拿到file_id和file_name,再通过file_id和file_name去文件下载


static std::promise<void> test_for_finished[500];
static std::future<void> complete_signal[500];



static std::promise<void> test_for_folder_finished;
static std::future<void> complete_folder_signal;


void filedownload(std::string file_id,std::string file_name,std::string md5,std::string download_path, int i){
  Json::Value file_json;
  
  file_json["file_id"] = file_id;
  file_json["file_name"] = file_name;
  file_json["md5"] = md5;
  file_json["download_folder_path"] = download_path;

  const auto download_info = restful_common::jsoncpp_helper::WriterHelper(file_json);
  std::unique_ptr<Downloader> dlr;
  auto callback = [&dlr,i](const std::string& str) {
    printf("file_download_callback%s\n", str.c_str());
    Json::Value root;
    restful_common::jsoncpp_helper::ReaderHelper(str, root);
    const auto is_complete =
        restful_common::jsoncpp_helper::GetBool(root["is_complete"]);
    const auto file_size = restful_common::jsoncpp_helper::GetInt(root["file_size"]);
    const auto file_name = restful_common::jsoncpp_helper::GetInt(root["file_size"]);
    if (file_size == 0)
    {
      printf("异常！！");
    }
    
    if (is_complete) {
      dlr = nullptr;
      test_for_finished[i].set_value();
    }
  };
  dlr = std::make_unique<Downloader>(download_info.c_str(), callback);
  dlr->AsyncStart();

  complete_signal[i].wait();
}

int main(void) {

  for (int  i = 0; i < 500; i++)
{
  complete_signal[i] = test_for_finished[i].get_future();
}

  Cloud189::SessionHelper::Cloud189Login(
      "22c8d9bf-5b19-4e80-9c2e-5368fe738feb",
      "6FA9027AAEABBC6E8234A2A6E31731A5",
      "dddd3bf5-95c5-4735-a1e2-644a670e617d_family",
      "2D6FBBD2630E71A626614DFFD599696E");
   
  Json::Value root;
  root["folder_id"] = "2142017390720463";
  root["folder_path"] = "docs";
  root["download_path"] = "/Users/zhaozt/Desktop/download1/";
  const auto download_info = restful_common::jsoncpp_helper::WriterHelper(root);
  std::unique_ptr<FolderDownloader> dlr;
  auto callback = [&dlr](const std::string& str) {
    printf("folder_download_callback is %s\n", str.c_str());
    Json::Value root;
    restful_common::jsoncpp_helper::ReaderHelper(str, root);
    const auto is_complete =
        restful_common::jsoncpp_helper::GetBool(root["is_complete"]);
    
    if (is_complete) {
      test_for_folder_finished.set_value();
      /// 此处顺序不当可致死锁，与析构模型和线程模型有关
      dlr = nullptr;
    }else{
      
        
      int32_t size = root["sub_file_data"].size(); //得到json数组的大小
      printf("test,name is %s",root["sub_file_data"][1]["file_name"].asString().c_str());
      for (static int i = 0; i < 500; i++) //逐个获取数组中对象的值
      {
        std::string file_id = root["sub_file_data"][i]["file_id"].asString();
        std::string file_name = root["sub_file_data"][i]["file_name"].asString();
        std::string md5 = root["sub_file_data"][i]["md5"].asString();
        std::thread t(filedownload,file_id.c_str(),file_name.c_str(),md5.c_str(),root["download_path"].asString().c_str(),i);
        t.join();
      }
      
        
    }
  };
  dlr = std::make_unique<FolderDownloader>(download_info.c_str(), callback);
  complete_folder_signal = test_for_folder_finished.get_future();
  dlr->AsyncStart();

  complete_folder_signal.wait();

        
  return 0;
}

