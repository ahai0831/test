#pragma once
#ifndef TOOLS_SCOPEGUARD_H__
#define TOOLS_SCOPEGUARD_H__
#include <functional>

namespace assistant {
namespace tools {
class scope_guard {
 private:
  /// typedef void(*DestructorType)();
  /// invalid transform while downgrading a lambda with state

  typedef std::function<void()> DestructorType;
  DestructorType destructor_;
  scope_guard() = delete;
  scope_guard(scope_guard const&) = delete;
  scope_guard& operator=(scope_guard const&) = delete;
  scope_guard(scope_guard&&) = delete;

 public:
  scope_guard(DestructorType destructor) : destructor_(destructor) {}
  ~scope_guard() {
    if (nullptr != destructor_) {
      destructor_();
      destructor_ = nullptr;
    }
  }
  void dismiss() { destructor_ = nullptr; }
};
}  // namespace tools
}  // namespace assistant

/// define some safe release macros for scopeguard
/// TODO：转换为内联静态函数实现
#define GUARD_SAFE_DELETE(p) \
  do {                       \
    if ((p) != nullptr) {    \
      delete (p);            \
      (p) = nullptr;         \
    }                        \
  } while (false);

#define GUARD_SAFE_DELETE_ARR(p) \
  do {                           \
    if ((p) != nullptr) {        \
      delete[](p);               \
      (p) = nullptr;             \
    }                            \
  } while (false);

#define GUARD_SAFE_FREE(p) \
  do {                     \
    if ((p) != nullptr) {  \
      free(p);             \
      (p) = nullptr;       \
    }                      \
  } while (false);

#ifdef WIN32
#define SAFE_CLOSEHANDLE(p) \
  do {                      \
    if ((p) != NULL) {      \
      ::CloseHandle(p);     \
      (p) = NULL;           \
    }                       \
  } while (false);
#endif

#endif  // !TOOLS_SCOPEGUARD_H__
