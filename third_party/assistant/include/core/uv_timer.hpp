#pragma once
#ifndef _CORE_UV_TIMER_H__
#define _CORE_UV_TIMER_H__
#include <functional>
#include <tuple>

#include <uv.h>

namespace assistant {
namespace core {
/// libuv_timer_closure
/// 注意，必须使此闭包的内存分配于堆上
/// 用法：初始化时，或在Init中传入事件循环句柄和事件的回调函数
/// Start()和Stop()应在事件循环所在的线程调用
struct libuv_timer_closure {
 private:
  typedef std::function<void(void)> Callback;
  /// TODO: 优化为结构体，优化性能
  typedef std::tuple<uv_timer_t, Callback> notify_wrapper;
  notify_wrapper wrapper;  /// 用于适配对libuv定时器事件回调

 public:
  libuv_timer_closure() = default;
  libuv_timer_closure(uv_loop_t &loop, Callback cb) { Init(loop, cb); }
  /// 允许默认构造，以便延迟初始化
  void Init(uv_loop_t &loop, Callback cb) {
    uv_timer_t &handle = std::get<0>(wrapper);
    Callback &callback = std::get<1>(wrapper);
    uv_timer_init(&loop, &handle);
    handle.data = static_cast<void *>(&wrapper);
    callback = cb;
  }
  //// 定时器启动
  void Start(uint64_t timeout, uint64_t repeat) {
    uv_timer_t &handle = std::get<0>(wrapper);
    Callback &callback = std::get<1>(wrapper);
    if (nullptr != callback) {
      uv_timer_start(&handle, timer_notify_cb, timeout, repeat);
    }
  }
  //// 定时器停止
  void Stop() {
    uv_timer_t &handle = std::get<0>(wrapper);
    uv_timer_stop(&handle);
  }

 private:
  static void timer_notify_cb(uv_timer_t *handle) {
    std::get<1> (*static_cast<notify_wrapper *>(handle->data))();
  }
  /// 禁用其他隐式的构造函数
  libuv_timer_closure(libuv_timer_closure const &) = delete;
  libuv_timer_closure &operator=(libuv_timer_closure const &) = delete;
  libuv_timer_closure(libuv_timer_closure &&) = delete;
};
}  // namespace core
}  // namespace assistant
#endif  // _CORE_UV_TIMER_H__
