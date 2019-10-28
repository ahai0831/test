#include <iostream>
#include <string>
#include <thread>

#include <windows.h>

#include <DateHelper/DateHelper.h>

#include "cloud189/session_helper/session_helper.h"

void test_multi_thread(int num) {
  assistant::HttpRequest req_get("");
  req_get.url = "https://api.cloud.189.cn/createUploadFile.action";
  req_get.method = "GET";
  if (Cloud189::SessionHelper::AddCloud189Signature(req_get)) {
    printf("%s: %02d person_session_key: %s\n",
           cloud_base::date_helper::get_time_stamp().c_str(), num,
           req_get.headers.Get("SessionKey").c_str());
  }

  if (Cloud189::SessionHelper::AddCloud189FamilySignature(req_get)) {
    printf("%s: %02d family_session_key: %s\n",
           cloud_base::date_helper::get_time_stamp().c_str(), num,
           req_get.headers.Get("SessionKey").c_str());
  }
}

int main() {
  std::string psk = "21b1eb34-2027-4c4b-8b94-653a44dfd332";
  std::string pss = "24BB41F7AC93046440D7DE9A564C82B2";
  std::string fsk = "7fa2ae9b-6d0f-40ec-8864-2bcf0734f7ac_family";
  std::string fss = "24BB41F7AC93046440D7DE9A564C82B2";

  std::string psk_test, pss_test, fsk_test, fss_test;

  Cloud189::SessionHelper::GetCloud189Session(psk_test, pss_test);
  printf("session info when no login: %s, %s\n", psk_test.c_str(),
         pss_test.c_str());

  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);

  Cloud189::SessionHelper::GetCloud189Session(psk_test, pss_test);
  printf("session info when login: %s, %s\n", psk_test.c_str(),
         pss_test.c_str());

  // test multi thread
  for (int i = 0; i < 15; i++) {
    std::thread test_multi(test_multi_thread, i);
    test_multi.detach();
  }

  Cloud189::SessionHelper::Cloud189Logout();
  printf("%s: logout\n", cloud_base::date_helper::get_time_stamp().c_str());

  psk_test.clear();
  pss_test.clear();
  Cloud189::SessionHelper::GetCloud189Session(psk_test, pss_test);
  printf("session info when logout: %s, %s\n", psk_test.c_str(),
         pss_test.c_str());

  Sleep(1000000000);
  return 0;
}
