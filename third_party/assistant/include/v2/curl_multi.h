#pragma once
#ifndef V2_CURL_MULTI_H__
#define V2_CURL_MULTI_H__
#include <cassert>
#include <functional>

#include <curl/curl.h>

#include "v2/curl_easy.h"
#include "v2/uv_poll.h"
#include "v2/uv_timer.h"

/// 保存multi句柄以及相应easy映射关系的闭包
struct a6_1 {
 public:
  CURLM *multi;
  a3_8 easy;

 private:
  /// 接收到 ClearMultiStack 指令后，此标志位置为true
  /// 此标志位为true，则不再绑定 easy 到 multi
  bool frozen;
  scopeguard_internal::ScopeGuard guard;

 public:
  a6_1()
      : multi(nullptr), frozen(false), guard([this]() {
          assert(easy.empty());
          ClearMultiStack(*this);
          curl_multi_cleanup(multi);
          multi = nullptr;
        }) {
    multi = curl_multi_init();
  }
  static void BindEasy(a3_8::Value &easy_closure, a6_1 &multi) {
    if (!multi.frozen) {
      auto &easy_handle = easy_closure->get_easy();
      multi.easy.Put(easy_handle, easy_closure);
      curl_multi_add_handle(multi.multi, easy_handle);
    }
  }
  static a3_8::Value UnbindEasy(const a3_8::Key &easy_handle, a6_1 &multi) {
    a3_8::Value value;
    value = multi.easy.Get(easy_handle);
    curl_multi_remove_handle(multi.multi, value->get_easy());
    return value;
  }
  static void ClearMultiStack(a6_1 &multi) {
    multi.frozen = true;
    for (const auto &x : multi.easy) {
      curl_multi_remove_handle(multi.multi, x.first);
    }
    multi.easy.Clear();
  }
};

/// 提供multi socket 流程所需的全局信息闭包，并支持配置multi选项
struct a6_3 {
 private:
  typedef std::function<void(CURLMsg *message)> Callback;
  /// 保存multi句柄，需配置；multi socket全流程必须用到
  CURLM *multi;
  /// 保存事件循环句柄，用于初始化定时器句柄，以及poll句柄
  uv_loop_t *loop;
  /// 保存multi_socket流程中libuv使用的定时器句柄
  libuv_timer_closure socket_timer;
  Callback msg_cb;

 public:
  a6_3(CURLM *multi_handle, uv_loop_t *loop_handle, Callback cb)
      : multi(multi_handle),
        loop(loop_handle),
        socket_timer(*loop_handle, std::bind(&a6_3::socket_timer_cb, this)),
        msg_cb(cb) {
    curl_multi_setopt(multi, CURLMOPT_TIMERFUNCTION, timer_cb);
    curl_multi_setopt(multi, CURLMOPT_SOCKETFUNCTION, sock_cb);
    curl_multi_setopt(multi, CURLMOPT_TIMERDATA, static_cast<void *>(this));
    curl_multi_setopt(multi, CURLMOPT_SOCKETDATA, static_cast<void *>(this));
  }

 private:
  void socket_timer_cb() {
    /// curl_multi_socket_action 启动socket流程
    int running_handles = 0;
    curl_multi_socket_action(multi, CURL_SOCKET_TIMEOUT, 0, &running_handles);

    /// check multi info
    check_multi_info();
  }
  void socket_poll_cb(int status, int events, curl_socket_t s) {
    int running_handles = 0;
    int flags = 0;
    if (events & UV_READABLE) {
      flags |= CURL_CSELECT_IN;
    }
    if (events & UV_WRITABLE) {
      flags |= CURL_CSELECT_OUT;
    }
    /// curl_multi_socket_action 驱动socket进行传输
    curl_multi_socket_action(multi, s, flags, &running_handles);

    /// check multi info
    check_multi_info();
  }
  static int timer_cb(CURLM *multi, long timeout_ms, void *userp) {
    auto &global_info = *static_cast<a6_3 *>(userp);
    if (timeout_ms < 0) {
      global_info.socket_timer.Stop();
    } else {
      /// From example:
      /// 0 means directly call socket_action, but we'll do it in a bit
      timeout_ms = (0 >= timeout_ms) ? 1 : timeout_ms;
      global_info.socket_timer.Start(timeout_ms, 0);
    }
    return 0;
  }
  static int sock_cb(CURL *e, curl_socket_t s, int action, void *userp,
                     void *sockp) {
    auto &global_info = *static_cast<a6_3 *>(userp);
    switch (action) {
      case CURL_POLL_REMOVE:
        if (nullptr != sockp) {
          auto poll_closure = (libuv_poll_closure *)sockp;
          poll_closure->Stop();
          curl_multi_assign(global_info.multi, s, nullptr);
          libuv_poll_closure::AsyncDestroy(poll_closure);
        }
        break;
      case CURL_POLL_IN:
      case CURL_POLL_OUT:
      case CURL_POLL_INOUT:
      default: {
        int events = 0;
        /// 初始化 poll 的闭包，并传入 a6_3 的回调 socket_poll_cb
        auto poll_closure =
            nullptr != sockp
                ? (libuv_poll_closure *)sockp
                : libuv_poll_closure::Create(
                      *global_info.loop, s,
                      std::bind(&a6_3::socket_poll_cb, &global_info,
                                std::placeholders::_1, std::placeholders::_2,
                                std::placeholders::_3));
        curl_multi_assign(global_info.multi, s, (void *)poll_closure);
        if (action != CURL_POLL_IN) {
          events |= UV_WRITABLE;
        }
        if (action != CURL_POLL_OUT) {
          events |= UV_READABLE;
        }
        poll_closure->Start(events);
      } break;
    }
    return 0;
  }

  void check_multi_info() {
    CURLMsg *message = nullptr;
    int pending = 0;

    while ((message = curl_multi_info_read(multi, &pending)) != nullptr) {
      if (CURLMSG_DONE == message->msg) {
        msg_cb(message);
      }
    }
  }
};
#endif  // V2_CURL_MULTI_H__
