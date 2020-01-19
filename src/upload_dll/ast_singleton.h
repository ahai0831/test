
/// Defines some singleton impl.

/// A singleton for AstConfig

#ifndef GENERAL_RESTFUL_SDK_AST_SINGLETON
#define GENERAL_RESTFUL_SDK_AST_SINGLETON

#include <memory>
#include <string>

#include <tools/safecontainer.hpp>

#include "cloud189/restful/cloud189_uploader.h"

namespace general_restful_sdk_ast {
struct Config {
  /// ����ȫ�����캯��
  static bool StoreCloud189Session(
      const std::string &cloud189_session_key,
      const std::string &cloud189_session_secret,
      const std::string &familycloud_session_key,
      const std::string &familycloud_session_secret);

  static void ClearCloud189Session();

 private:
  Config() = delete;
  Config(Config &&) = delete;
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;
};
/// ������ص�Ast���ݽṹ����
struct AstInfo {
  AstInfo();
  /// �����ƶ����졢���ƹ����=�Ų�����

  /// ������Ӧ�ܿ�����
  assistant::tools::safemap_closure<std::string, int32_t> uuid_map;
  assistant::tools::safemap_closure<
      std::string, std::unique_ptr<::Cloud189::Restful::Uploader>>
      cloud189_uploader_map;

 private:
  AstInfo(AstInfo &&) = delete;
  AstInfo(const AstInfo &) = delete;
  AstInfo &operator=(const AstInfo &) = delete;
};

std::shared_ptr<AstInfo> GetAstInfo();

}  // namespace general_restful_sdk_ast

#endif
