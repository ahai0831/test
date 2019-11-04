#include <gtest/gtest.h>

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#include <Assistant_v3.hpp>

/// 展示如何优雅地进行插桩
/// 这个类是仅此源文件可访问
/// 但插入桩中的函数为整个二进制有效（跨越各源文件，以及各Assistant_v3的对象）
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
    /// set proxy
//     const auto kProxy = "http://127.0.0.1:9999";
//     printf("Default proxy in Assistant_v3: %s\n", kProxy);
//     request_pile.Add([=](assistant::HttpRequest& req) -> void {
//       req.extends.Set("proxy", kProxy);
//     });

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
