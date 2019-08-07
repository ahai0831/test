#pragma once
#ifndef V2_CURL_GLOBAL_H__
#define V2_CURL_GLOBAL_H__
#include <curl/curl.h>

struct LibcurlGlobalInitClean {
  LibcurlGlobalInitClean() { curl_global_init(CURL_GLOBAL_ALL); }
  ~LibcurlGlobalInitClean() { curl_global_cleanup(); }
};
struct CurlGlobal {
  CurlGlobal() { static struct LibcurlGlobalInitClean global; }
};
#endif  // V2_CURL_GLOBAL_H__
