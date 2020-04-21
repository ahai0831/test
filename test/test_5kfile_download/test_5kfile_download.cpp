#include <future>
#include <memory>

#include "cloud189/restful/folder_downloader/folder_downloader.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

using Cloud189::Restful::FolderDownloader;

//通过文件夹下载拿到file_id和file_name,再通过file_id和file_name去文件下载


static std::promise<void> test_for_finished;
static std::future<void> complete_signal;

static std::promise<void> test_for_folder_finished;
static std::future<void> complete_folder_signal;


void filedownload(std::string file_id,std::string file_name,std::string md5){
  
}

int main(void) {

  Cloud189::SessionHelper::Cloud189Login(
      "22c8d9bf-5b19-4e80-9c2e-5368fe738feb",
      "6FA9027AAEABBC6E8234A2A6E31731A5",
      "dddd3bf5-95c5-4735-a1e2-644a670e617d_family",
      "2D6FBBD2630E71A626614DFFD599696E");
  Json::Value root;
  root["folder_id"] = "2142017390720463";
  root["folder_path"] = "/ceshiwenjian/";
  root["download_path"] = "/Users/zhaozt/Desktop/download1/";
  const auto download_info = restful_common::jsoncpp_helper::WriterHelper(root);
  std::unique_ptr<FolderDownloader> dlr;
  auto callback = [&dlr](const std::string& str) {
    printf("callback is %s\n", str.c_str());
    Json::Value root;
    restful_common::jsoncpp_helper::ReaderHelper(str, root);
    const auto is_complete =
        restful_common::jsoncpp_helper::GetBool(root["is_complete"]);
    
    if (is_complete) {
      test_for_folder_finished.set_value();
      /// 此处顺序不当可致死锁，与析构模型和线程模型有关
      dlr = nullptr;
    }else{
      const auto sub_file_data_str = restful_common::jsoncpp_helper::GetString(root["sub_file_data"]);
        printf("sub_file_data_str is %s \n",sub_file_data_str.c_str());
    }
  };
  dlr = std::make_unique<FolderDownloader>(download_info.c_str(), callback);
  complete_folder_signal = test_for_folder_finished.get_future();
  dlr->AsyncStart();

  complete_folder_signal.wait();

        
        // for (int32_t i = 0; i < length; i++)
        // {
        //   std::cout<<"file_id is"<<callback_data_json["sub_file_data"][i]["file_id"]<<std::endl;
        //   // printf("file_id is %s",callback_data_json[i]["file_id"]);
        //   // printf("file_name is %s",callback_data_json[i]["file_name"]);
        //   // printf("file_id_md5 is %s",callback_data_json[i]["md5"]);
        //第二步将文件id和文件名以及md5传进文件下载线程
        // std::string file_id = callback_data_json[i]["file_id"];
        // std::string file_name = callback_data_json[i]["file_name"];
        // std::string file_md5 = callback_data_json[i]["md5"];
        //std::thread t(filedownload,file_id,file_name,file_md5);
        //   t.detach();
        // }
        
  return 0;
}

