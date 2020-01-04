#include <cinttypes>
#include <cstdio>
#include <memory>

#include <Assistant_v3.hpp>

int main(void) {
  printf("Hello GN.\n");

  std::shared_ptr<assistant::Assistant_v3> assist =
      std::make_shared<assistant::Assistant_v3>();

  assistant::HttpRequest req_get(
      "https://postman-echo.com/get?foo1=bar1&foo2=bar2");
  auto res_get = assist->SyncHttpRequest(req_get);
  printf("InPile: %d--------%s--------Body:\n%s\n\n", res_get.status_code,
         res_get.effective_url.c_str(), res_get.body.c_str());

  return 0;
}
