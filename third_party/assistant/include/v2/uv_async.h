#pragma once
#ifndef V2_UV_ASYNC_H__
#define V2_UV_ASYNC_H__
#include <functional>
#include <tuple>

#include <uv.h>
/// libuv_async_closure
/// 注意，必须使此闭包的内存分配于堆上
/// 用法：初始化时，或在Init中传入事件循环句柄和事件的回调函数
/// Notify()应在事件循环线程的外部线程调用
struct libuv_async_closure {
 private:
  typedef std::function<void(void)> Callback;
  typedef std::tuple<uv_async_t, Callback> notify_wrapper;
  notify_wrapper wrapper;  /// 用于适配对libuv异步事件回调

 public:
  static void async_notify_cb(uv_async_t *handle) {
    std::get<1> (*static_cast<notify_wrapper *>(handle->data))();
  }
  libuv_async_closure() = default;
  libuv_async_closure(uv_loop_t &loop, Callback cb) { Init(loop, cb); }
  /// 允许默认构造，以便延迟初始化
  void Init(uv_loop_t &loop, Callback cb) {
    uv_async_t &handle = std::get<0>(wrapper);
    Callback &callback = std::get<1>(wrapper);
    uv_async_init(&loop, &handle, async_notify_cb);
    handle.data = static_cast<void *>(&wrapper);
    callback = cb;
  }
  //// （外部线程中）对事件循环发起通知
  void Notify() {
    uv_async_t &handle = std::get<0>(wrapper);
    Callback &callback = std::get<1>(wrapper);
    if (nullptr != callback && (!uv_is_closing((uv_handle_t *)&handle))) {
      uv_async_send(&handle);
    }
  }

 private:
  /// 禁用其他隐式的构造函数
  libuv_async_closure(libuv_async_closure const &) = delete;
  libuv_async_closure &operator=(libuv_async_closure const &) = delete;
  libuv_async_closure(libuv_async_closure &&) = delete;
};
#endif  // V2_UV_ASYNC_H__
