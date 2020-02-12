#include <cinttypes>
#include <cstdio>
#include <memory>

#include <Assistant_v3.hpp>

#include "cloud189/restful/cloud189_uploader.h"
#include "cloud189/session_helper/session_helper.h"

// template<class T>
// void realdelete(T&& pointer)
// {
// 	if (nullptr != pointer)
// 	{
// 		delete pointer;
// 		pointer = nullptr;
// 	}
// }
// 
// template <class ...Args>
// void ex(Args&&... args){
// // 	int arr[] = {(ex(args), 0)...};
// // 	std::initializer_list<int>((realdelete(args),0)...);
// 	std::initializer_list<int>(([&]{if (nullptr != args)
// 	{
// 		delete args;
// 		args = nullptr;
// 	}
// 	}(), 0)...);
// }
// template<typename T>
// void rd2(T&& t)
// {
// 	if (nullptr != t)
// 	{
// 		delete t;
// 		t = nullptr;
// 	}
// }
// 
// template<typename ...Args>
// void myexpand(Args&&... args)
// {
// 	int arr[] = { (rd2(args), 0)... };
// }
// 
// template<typename ...Args>
// void myexpand_2(Args&&... args)
// {
// 	std::initializer_list<int>{(rd2(std::forward< Args>(args)), 0)...};
// }


int main(void) {

	auto abc = new(std::nothrow) char();

	assistant::tools::SafeDelete(abc);

// 	auto a = new char();
// 	void* b = nullptr;
// 	ex(a,b);

  printf("Hello GN.\n");

  /// Test a httpreq
  std::shared_ptr<assistant::Assistant_v3> assist =
      std::make_shared<assistant::Assistant_v3>();

  assistant::HttpRequest req_get(
      "https://postman-echo.com/get?foo1=bar1&foo2=bar2");
  auto res_get = assist->SyncHttpRequest(req_get);
  printf("InPile: %d--------%s--------Body:\n%s\n\n", res_get.status_code,
         res_get.effective_url.c_str(), res_get.body.c_str());

  /// Mock the login process
  /// Set yourselfs session for cloud189
  Cloud189::SessionHelper::Cloud189Login("3c832028-a420-4b56-9d92-2244e54bb7b2",
                                         "E925BAAB6D50103E5E21BA2562B9831A",
                                         "-", "-");

  /// Generate upload info
  std::string test_uploader_info;
  {
    std::string file_path = "D:\\jpg_jpg.jpg";
    std::string parent_id = "-11";
    std::string last_upload_file_id = "1384315735897436096";
    std::string last_md5 = "";  //可有可无，如果要续传则必须传入，否则重新上传
    std::string x_request_id = "";
    int32_t oper_type = 1;
    int32_t is_log = 0;
    //可以无，如果有则优先选择续传，如果失效则重新上传
    test_uploader_info = Cloud189::Restful::uploader_info_helper(
        file_path, last_md5, last_upload_file_id, parent_id, x_request_id,
        oper_type, is_log);
  }

  Cloud189::Restful::Uploader up(
      test_uploader_info,
      [](const std::string& info) { printf("%s\n", info.c_str()); });

  up.AsyncStart();
  up.SyncWait();

  Cloud189::SessionHelper::Cloud189Logout();
  return 0;
}
