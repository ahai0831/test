
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
  /// 禁用全部构造函数
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
/// 保存相关的Ast数据结构定义
struct AstInfo {
  AstInfo();
  /// 禁用移动构造、复制构造和=号操作符

  /// 保存相应总控容器
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

struct Proxy {
  static void SetProxy(const std::string &proxy_info);
  static std::string GetProxy();

 private:
  Proxy() = delete;
  Proxy(Config &&) = delete;
  Proxy(const Proxy &) = delete;
  Proxy &operator=(const Proxy &) = delete;
};
}  // namespace general_restful_sdk_ast

#endif
