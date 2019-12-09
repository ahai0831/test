#include <Assistant_v3.hpp>

static struct InitAssistantOnce {
  InitAssistantOnce() {
    /// 注意插桩是全局的
    /// 注意桩函数的类型类似 std::function<void(assistant::HttpRequest &)>
    /// 或者 std::function<void(assistant::HttpResponse &)>
    auto& request_pile = assistant::Assistant_v3::HttprequestPile();
    /// print request
    request_pile.Add([](assistant::HttpRequest& req) -> void {
      printf("InPile: Do request: %s\n", req.url.c_str());
    });
    // set proxy
    const auto kProxy = "http://127.0.0.1:8888";
    printf("Default proxy in Assistant_v3: %s\n", kProxy);
    request_pile.Add([=](assistant::HttpRequest& req) -> void {
      req.extends.Set("proxy", kProxy);
    });

    auto& response_pile = assistant::Assistant_v3::HttpresponsePile();
    /// print response
    response_pile.Add([](assistant::HttpResponse& res) -> void {
      if (2 == res.status_code / 100) {
        printf("InPile: %d--------%s--------Body:\n%s\n\n", res.status_code,
               res.effective_url.c_str(), res.body.c_str());
      } else {
        printf("InPile: %d--------%s--------Body:\n%s\nExtends:\n%s\n\n",
               res.status_code, res.effective_url.c_str(), res.body.c_str(),
               res.extends.extends_str().c_str());
      }
    });
  }
} init_assistant_once;
