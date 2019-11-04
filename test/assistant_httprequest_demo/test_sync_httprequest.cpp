#include <memory>

#include <gtest/gtest.h>

#include <Assistant_v3.hpp>

TEST(sync_httprequest, http_get) {
  std::shared_ptr<assistant::Assistant_v3> assist =
      std::make_shared<assistant::Assistant_v3>();

  /// GET
  assistant::HttpRequest req_get(
      "https://postman-echo.com/get?foo1=bar1&foo2=bar2");
  auto res_get = assist->SyncHttpRequest(req_get);
  EXPECT_EQ(res_get.status_code, 200);
}

TEST(sync_httprequest, http_post) {
  std::shared_ptr<assistant::Assistant_v3> assist =
      std::make_shared<assistant::Assistant_v3>();

  /// POST
  assistant::HttpRequest req_post(
      "POST", "https://postman-echo.com/post",
      "This is expected to be sent back as part of response body.");
  auto res_post = assist->SyncHttpRequest(req_post);
  EXPECT_EQ(res_post.status_code, 200);
}
TEST(sync_httprequest, http_put) {
  std::shared_ptr<assistant::Assistant_v3> assist =
      std::make_shared<assistant::Assistant_v3>();

  /// PUT
  assistant::HttpRequest req_put(
      "PUT", "https://postman-echo.com/put",
      "This is expected to be sent back as part of response body.");
  auto res_put = assist->SyncHttpRequest(req_put);
  EXPECT_EQ(res_put.status_code, 200);
}

TEST(sync_httprequest, solve_http_request) {
  std::shared_ptr<assistant::Assistant_v3> assist =
      std::make_shared<assistant::Assistant_v3>();

  /// PUT
  assistant::HttpRequest req_put(
      "PUT", "https://postman-echo.com/put",
      "This is expected to be sent back as part of response body.");
  auto res_put = assist->SyncHttpRequest(req_put);
  EXPECT_EQ(res_put.status_code, 200);
  const auto kCURLcode =
      strtol(res_put.extends.Get("CURLcode").c_str(), nullptr, 0);
  const auto kContentLengthDownload = strtoll(
      res_put.extends.Get("content_length_download").c_str(), nullptr, 0);
  const auto kSizeDownload =
      strtoll(res_put.extends.Get("size_download").c_str(), nullptr, 0);
  const auto kSpeedDownload =
      strtoll(res_put.extends.Get("speed_download").c_str(), nullptr, 0);
  const auto kContentLengthUpload =
      strtoll(res_put.extends.Get("content_length_upload").c_str(), nullptr, 0);
  const auto kSizeUpload =
      strtoll(res_put.extends.Get("size_upload").c_str(), nullptr, 0);
  const auto kRedirectUrl = res_put.extends.Get("redirect_url");
  EXPECT_EQ(kCURLcode, 0);
  EXPECT_EQ(kContentLengthUpload, kSizeUpload);
}
