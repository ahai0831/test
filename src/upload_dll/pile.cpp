
/// 此文件专用相关网络请求插桩。
/// 保证此文件参与编译即可，无需在外部调用相应的struct或方法。

#include <Assistant_v3.hpp>

#include "ast_singleton.h"
#include "log_system/log_system.h"
using general_restful_sdk_ast::log_system::LogInfo;

static struct InitAssistantOnce {
  InitAssistantOnce() {
    /// 注意插桩是全局的
    /// 注意桩函数的类型类似 std::function<void(assistant::HttpRequest &)>
    /// 或者 std::function<void(assistant::HttpResponse &)>
    auto& request_pile = assistant::Assistant_v3::HttprequestPile();

    /// set proxy
    request_pile.Add([=](assistant::HttpRequest& req) -> void {
      auto proxy_info = general_restful_sdk_ast::Proxy::GetProxy();
      if (!proxy_info.empty()) {
        req.extends.Set("proxy", proxy_info);
      }
    });

    /// print request
    request_pile.Add([](assistant::HttpRequest& req) -> void {
      LogInfo("[HttpRequest] %s %s\nheaders: %s\nextends:%s",
              req.method.c_str(), req.url.c_str(),
              req.headers.extends_str().c_str(),
              req.extends.extends_str().c_str());
    });

    auto& response_pile = assistant::Assistant_v3::HttpresponsePile();
    /// print response
    response_pile.Add([](assistant::HttpResponse& res) -> void {
      LogInfo("[HttpResponse] %d %s\nheaders: %s\nextends:%s\n%s",
              res.status_code, res.effective_url.c_str(),
              res.headers.extends_str().c_str(),
              res.extends.extends_str().c_str(), res.body.c_str());
    });
  }
} init_assistant_once;
