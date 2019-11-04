#pragma once
#ifndef _CORE_CURL_GLOBAL_H__
#define _CORE_CURL_GLOBAL_H__
#include <curl/curl.h>
namespace assistant {
namespace core {
/// 用法：定义在某处即可，会自动执行curl的global初始化
struct libcurl_global_closure {
  libcurl_global_closure() { static libcurl_global_init_clean global; }
  ~libcurl_global_closure() = default;

  libcurl_global_closure(libcurl_global_closure&&) = delete;
  libcurl_global_closure(const libcurl_global_closure&) = delete;
  libcurl_global_closure& operator=(const libcurl_global_closure&) = delete;

 private:
  /// 定义为私有，避免外部误调用
  struct libcurl_global_init_clean {
    libcurl_global_init_clean() { curl_global_init(CURL_GLOBAL_ALL); }
    ~libcurl_global_init_clean() { curl_global_cleanup(); }

    libcurl_global_init_clean(libcurl_global_init_clean&&) = delete;
    libcurl_global_init_clean(const libcurl_global_init_clean&) = delete;
    libcurl_global_init_clean& operator=(const libcurl_global_init_clean&) =
        delete;
  };
};
}  // namespace core
}  // namespace assistant
#endif  // _CORE_CURL_GLOBAL_H__
