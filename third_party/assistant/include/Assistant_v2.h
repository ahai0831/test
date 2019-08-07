#pragma once
#ifndef ASSISTANT2_H__
#define ASSISTANT2_H__
#include <memory>
#include "http_primitives.h"
namespace assistant {
struct a1_0;
struct Assistant_v2 {
 private:
  std::unique_ptr<a1_0> thread_closure;

 public:
  Assistant_v2();
  ~Assistant_v2();
  void AsyncHttpRequest(const assistant::HttpRequest& request);
  assistant::HttpResponse SyncHttpRequest(
      const assistant::HttpRequest& request);
};
}  // namespace assistant
#ifndef ASSISTANT_USING_STATIC
#include "Assistant_v2.inc"
#endif

#endif  // ASSISTANT2_H__
