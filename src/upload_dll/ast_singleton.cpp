
/// Defines some singleton impl.

/// A singleton for AstConfig

#include "ast_singleton.h"

#include <memory>
#include <mutex>

#include <rx_assistant.hpp>

#include "cloud189/session_helper/session_helper.h"

namespace {
/// ������ص�Ast����ָ����̰߳�ȫ���ر�־λ
std::once_flag flag;
std::shared_ptr<general_restful_sdk_ast::AstInfo> ast_ptr;

void InitAstPtr() {
  ast_ptr = std::make_shared<general_restful_sdk_ast::AstInfo>();
  const auto assistant_tmp =
      rx_assistant::default_asssitant_v3::get_assistant();
}

}  // namespace

namespace general_restful_sdk_ast {
bool Config::StoreCloud189Session(
    const std::string &cloud189_session_key,
    const std::string &cloud189_session_secret,
    const std::string &familycloud_session_key,
    const std::string &familycloud_session_secret) {
  return Cloud189::SessionHelper::Cloud189Login(
      cloud189_session_key, cloud189_session_secret, familycloud_session_key,
      familycloud_session_secret);
}

void Config::ClearCloud189Session() {
  return Cloud189::SessionHelper::Cloud189Logout();
}

///  ���뱣֤ÿ�����̵���Ϣ���Ӵ˻�ȡ����Ӧ�������ط�����AstInfo�ĸ���
std::shared_ptr<AstInfo> general_restful_sdk_ast::GetAstInfo() {
  std::call_once(flag, InitAstPtr);
  return ast_ptr;
}

AstInfo::AstInfo() {}

}  // namespace general_restful_sdk_ast
