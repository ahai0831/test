#pragma once
#ifndef SCOPEGUARD_V1_H_
#define SCOPEGUARD_V1_H_
#include <functional>

namespace scopeguard_internal{
class ScopeGuard {
 private:
  /// typedef void(*DestructorType)();
  /// invalid transform while downgrading a lambda with state

  typedef std::function<void()> DestructorType;
  DestructorType destructor_;
  ScopeGuard() = delete;
  ScopeGuard(ScopeGuard const&) = delete;
  ScopeGuard& operator=(ScopeGuard const&) = delete;
  ScopeGuard(ScopeGuard&&) = delete;

 public:
  ScopeGuard(DestructorType destructor) : destructor_(destructor) {}
  ~ScopeGuard() {
    if (nullptr != destructor_) {
      destructor_();
      destructor_ = nullptr;
    }
  }
  void dismiss() { destructor_ = nullptr; }
};
}    // end namespace scopeguard_internal
/// define some safe release macros for scopeguard

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

#endif
