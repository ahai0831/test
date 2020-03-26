#include "ffi_simulation.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"
using restful_common::jsoncpp_helper::GetBool;
using restful_common::jsoncpp_helper::GetInt;
using restful_common::jsoncpp_helper::GetString;
using restful_common::jsoncpp_helper::ReaderHelper;
using restful_common::jsoncpp_helper::WriterHelper;

void test_ast_config() {
  Json::Value save_cloud189_session;
  save_cloud189_session.append("fd523f52-ea9e-4ec3-89a6-863000273550");
  save_cloud189_session.append("ADFC7C94446DFC9DACD1F2C611797661");
  save_cloud189_session.append("956406b2-6844-4ddb-b472-47b30a7d4fa6_family");
  save_cloud189_session.append("ADFC7C94446DFC9DACD1F2C611797661");
  Json::Value config;
  config["save_cloud189_session"] = save_cloud189_session;
  // config["proxy_info"] = "http://127.0.0.1:8888";
  const auto config_str = WriterHelper(config);
  printf("%s\n", config_str.c_str());
  AstConfig(config_str.c_str(),
            [](const char *on_config) { printf("OnConfig: %s\n", on_config); });
}
