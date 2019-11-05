#pragma once
#ifndef _CORE_LOOP_HPP__
#define _CORE_LOOP_HPP__
#include <uv.h>

#include "core/uv_async.hpp"
#include "tools/scopeguard.hpp"

namespace assistant {
namespace core {
/// 特定线程循环的基本事件循环闭包
struct thread_loop_closure {
  uv_loop_t thread_loop;
  assistant::tools::scope_guard thread_loop_guard;
  assistant::core::libuv_async_closure stop_notify;
  /// 停止通知对应的回调：使事件循环停止
  void stop_notify_cb() {
    uv_walk(&thread_loop, walker, nullptr);
    uv_stop(&thread_loop);
  }
  void LoopRun() { return (void)uv_run(&thread_loop, UV_RUN_DEFAULT); }
  void LoopStop() { stop_notify.Notify(); }
  thread_loop_closure()
      : thread_loop({0}),
        thread_loop_guard([this]() { uv_loop_close(&thread_loop); }) {
    /// 必须在构造方法中执行loop的初始化和退出循环句柄的初始化；
    /// 否则外部的初始化和对析构的调用时序是不确定的，
    /// 潜在地会导致对象即使析构，loop也不会停止（进程无法退出）
    uv_loop_init(&thread_loop);
    stop_notify.Init(thread_loop,
                     std::bind(&thread_loop_closure::stop_notify_cb, this));
  }
  static void walker(uv_handle_t *handle, void *arg) {
    uv_close(handle, nullptr);
  }
};

}  // namespace core

}  // namespace assistant

#endif  /// !_CORE_LOOP_HPP__
