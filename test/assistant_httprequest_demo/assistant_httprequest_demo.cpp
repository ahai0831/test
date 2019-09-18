#include <cstdio>
#include <memory>

#include <Assistant_v2.h>

int main(void) {
  auto assistant = std::make_unique<assistant::Assistant_v2>();
  /// GET
  assistant::HttpRequest req_get(
      "https://postman-echo.com/get?foo1=bar1&foo2=bar2");
  auto res_get = assistant->SyncHttpRequest(req_get);
  printf("%d\t%s\nBody:\n%s\n\n", res_get.status_code,
         res_get.effective_url.c_str(), res_get.body.c_str());

  /// POST
  assistant::HttpRequest req_post(
      "POST", "https://postman-echo.com/post",
      "This is expected to be sent back as part of response body.");
  auto res_post = assistant->SyncHttpRequest(req_post);
  printf("%d\t%s\nBody:\n%s\n\n", res_post.status_code,
         res_post.effective_url.c_str(), res_post.body.c_str());

  /// PUT
  assistant::HttpRequest req_put(
      "PUT", "https://postman-echo.com/put",
      "This is expected to be sent back as part of response body.");
  auto res_put = assistant->SyncHttpRequest(req_put);
  printf("%d\t%s\nBody:\n%s\n\n", res_put.status_code,
         res_put.effective_url.c_str(), res_put.body.c_str());

  return 0;
}
