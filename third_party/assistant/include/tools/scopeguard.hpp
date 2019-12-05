#pragma once
#ifndef TOOLS_SCOPEGUARD_H__
#define TOOLS_SCOPEGUARD_H__
#include <functional>
// #ifdef WIN32
// #include <windows.h>
// #endif
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

/// define some safe release macros for scopeguard
inline static void SafeDelete(void*& p) {
  if (nullptr != p) {
    delete p;
    p = nullptr;
  }
}

inline static void SafeFree(void*& p) {
  if (nullptr != p) {
    free(p);
    p = nullptr;
  }
}

// #ifdef WIN32
// inline static void SafeCloseHandle(HANDLE& h) {
//   if (INVALID_HANDLE_VALUE != h && ::CloseHandle(h) != 0) {
//     h = INVALID_HANDLE_VALUE;
//   }
// }
// #endif

}  // namespace tools
}  // namespace assistant

#endif  // !TOOLS_SCOPEGUARD_H__
