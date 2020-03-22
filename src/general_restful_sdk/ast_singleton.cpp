
/// Defines some singleton impl.

/// A singleton for AstConfig

#include "ast_singleton.h"

#include <memory>
#include <mutex>

#include <rx_assistant.hpp>

#include "cloud189/session_helper/session_helper.h"
#include "log_system/log_system.h"

using general_restful_sdk_ast::log_system::LogInfo;

namespace {
/// 保存相关的Ast智能指针和线程安全加载标志位
std::once_flag flag;
std::shared_ptr<general_restful_sdk_ast::AstInfo> ast_ptr;

void InitAstPtr() {
  ast_ptr = std::make_shared<general_restful_sdk_ast::AstInfo>();
  const auto assistant_tmp =
      rx_assistant::default_asssitant_v3::get_assistant();
}

/// 保存线程安全的字符串容器
assistant::tools::lockfree_string_closure<std::string> proxy_string;

}  // namespace

namespace general_restful_sdk_ast {
bool Config::StoreCloud189Session(
    const std::string &cloud189_session_key,
    const std::string &cloud189_session_secret,
    const std::string &familycloud_session_key,
    const std::string &familycloud_session_secret) {
  LogInfo("[Config] StoreCloud189Session");
  return Cloud189::SessionHelper::Cloud189Login(
      cloud189_session_key, cloud189_session_secret, familycloud_session_key,
      familycloud_session_secret);
}

void Config::ClearCloud189Session() {
  LogInfo("[Config] ClearCloud189Session");
  return Cloud189::SessionHelper::Cloud189Logout();
}

///  必须保证每个流程的信息都从此获取，不应在其他地方保存AstInfo的副本
std::shared_ptr<AstInfo> general_restful_sdk_ast::GetAstInfo() {
  std::call_once(flag, InitAstPtr);
  return ast_ptr;
}

AstInfo::AstInfo() {}

void Proxy::SetProxy(const std::string &proxy_info) {
  LogInfo("[Proxy] SetProxy: %s", proxy_info.c_str());
  proxy_string.store(proxy_info);
}

std::string Proxy::GetProxy() { return proxy_string.load(); }

}  // namespace general_restful_sdk_ast
