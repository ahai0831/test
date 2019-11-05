#pragma once
#ifndef _CORE_CURL_GLOBAL_H__
#define _CORE_CURL_GLOBAL_H__
#include <curl/curl.h>
namespace assistant {
namespace core {
/// �÷���������ĳ�����ɣ����Զ�ִ��curl��global��ʼ��
struct libcurl_global_closure {
  libcurl_global_closure() { static libcurl_global_init_clean global; }
  ~libcurl_global_closure() = default;

  libcurl_global_closure(libcurl_global_closure&&) = delete;
  libcurl_global_closure(const libcurl_global_closure&) = delete;
  libcurl_global_closure& operator=(const libcurl_global_closure&) = delete;

 private:
  /// ����Ϊ˽�У������ⲿ�����
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
