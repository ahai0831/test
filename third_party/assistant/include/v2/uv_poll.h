#pragma once
#ifndef V2_UV_POLL_H__
#define V2_UV_POLL_H__
#include <functional>

#include <curl/curl.h>
#include <uv.h>

/// libuv_poll_closure
/// 用法：向Create传入事件循环句柄和socket描述符和事件的回调函数
/// Start()和Stop()应在事件循环所在的线程调用
struct libuv_poll_closure {
 private:
  typedef std::function<void(int status, int events, curl_socket_t s)> Callback;
  // public:
  typedef std::tuple<uv_poll_t, curl_socket_t, Callback> notify_wrapper;

 private:
  notify_wrapper wrapper;  /// 用于适配对libuv poll事件回调

 public:
  static libuv_poll_closure *Create(uv_loop_t &loop, curl_socket_t s,
                                    Callback cb) {
    return new (std::nothrow) libuv_poll_closure(loop, s, cb);
  }
  /// 不会自动Stop；故销毁之前，若已Start，必须Stop
  static void AsyncDestroy(libuv_poll_closure *&poll) {
    if (nullptr != poll) {
      poll->Close();
      poll = nullptr;
    }
  }
  //// poll启动
  void Start(int32_t events) {
    uv_poll_t &handle = std::get<0>(wrapper);
    Callback &callback = std::get<2>(wrapper);
    if (nullptr != callback) {
      uv_poll_start(&handle, events, poll_notify_cb);
    }
  }
  //// poll停止
  void Stop() {
    uv_poll_t &handle = std::get<0>(wrapper);
    uv_poll_stop(&handle);
  }

 private:
  /// 禁用从外部调用构造方法
  libuv_poll_closure(uv_loop_t &loop, curl_socket_t s, Callback cb) {
    Init(loop, s, cb);
  }
  /// 初始化
  void Init(uv_loop_t &loop, curl_socket_t s, Callback cb) {
    uv_poll_t &handle = std::get<0>(wrapper);
    curl_socket_t &sockfd = std::get<1>(wrapper);
    Callback &callback = std::get<2>(wrapper);
    sockfd = s;
    uv_poll_init_socket(&loop, &handle, sockfd);
    handle.data = static_cast<void *>(&wrapper);
    callback = cb;
  }
  /// 禁用其他隐式的构造函数
  libuv_poll_closure() = delete;
  libuv_poll_closure(libuv_poll_closure const &) = delete;
  libuv_poll_closure &operator=(libuv_poll_closure const &) = delete;
  libuv_poll_closure(libuv_poll_closure &&) = delete;
  static void poll_notify_cb(uv_poll_t *handle, int status, int events) {
    curl_socket_t &sockfd =
        std::get<1>(*static_cast<notify_wrapper *>(handle->data));
    std::get<2> (*static_cast<notify_wrapper *>(handle->data))(status, events,
                                                               sockfd);
  }
  static void poll_close_cb(uv_handle_t *handle) {
    auto poll = static_cast<libuv_poll_closure *>(handle->data);
    delete (poll);
  }
  void Close() {
    uv_poll_t &handle = std::get<0>(wrapper);
    uv_close((uv_handle_t *)&handle, poll_close_cb);
  }
};
#endif  // V2_UV_POLL_H__
