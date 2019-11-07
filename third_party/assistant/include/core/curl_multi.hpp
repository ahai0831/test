#pragma once
#ifndef _CORE_CURL_MULTI_H__
#define _CORE_CURL_MULTI_H__
#include <cassert>
#include <functional>

#include <curl/curl.h>

#include "core/curl_easy.hpp"
#include "core/uv_poll.hpp"
#include "core/uv_timer.hpp"
#include "tools/scopeguard.hpp"
namespace assistant {
namespace core {
/// 前置声明 libcurl_multi_closure 和 curl_multi ，以通过编译 2019.10.30
struct libcurl_multi_closure;
namespace curl_multi {
static void BindEasy(easy_mapping_easyclosure::Value &easy_closure,
                     libcurl_multi_closure &multi);
static easy_mapping_easyclosure::Value UnbindEasy(
    const easy_mapping_easyclosure::Key &easy_handle,
    libcurl_multi_closure &multi);
static void ClearMultiStack(libcurl_multi_closure &multi);
static void SolveSpeedLimit(const easy_mapping_easyclosure::Value &easy_closure,
                            libcurl_multi_closure &multi);
}  // namespace curl_multi
/// 保存multi句柄以及相应easy映射关系的闭包
struct libcurl_multi_closure {
 public:
  CURLM *multi;
  easy_mapping_easyclosure easy;
  bool frozen;

 private:
  /// 接收到 ClearMultiStack 指令后，此标志位置为true
  /// 此标志位为true，则不再绑定 easy 到 multi
  assistant::tools::scope_guard guard;

 public:
  libcurl_multi_closure()
      : multi(nullptr), frozen(false), guard([this]() {
          //           assert(easy.empty());
          curl_multi::ClearMultiStack(*this);
          curl_multi_cleanup(multi);
          multi = nullptr;
        }) {
    multi = curl_multi_init();
  }
  libcurl_multi_closure(libcurl_multi_closure &&) = delete;
  libcurl_multi_closure(const libcurl_multi_closure &) = delete;
  libcurl_multi_closure &operator=(const libcurl_multi_closure &) = delete;
};

namespace curl_multi {
void BindEasy(easy_mapping_easyclosure::Value &easy_closure,
              libcurl_multi_closure &multi) {
  if (!multi.frozen) {
    auto &easy_handle = easy_closure->get_easy();
    multi.easy.Put(easy_handle, easy_closure);
    curl_multi_add_handle(multi.multi, easy_handle);
  }
}
easy_mapping_easyclosure::Value UnbindEasy(
    const easy_mapping_easyclosure::Key &easy_handle,
    libcurl_multi_closure &multi) {
  easy_mapping_easyclosure::Value value;
  value = multi.easy.Get(easy_handle);
  curl_multi_remove_handle(multi.multi, value->get_easy());
  return value;
}
void ClearMultiStack(libcurl_multi_closure &multi) {
  multi.frozen = true;
  for (const auto &x : multi.easy) {
    curl_multi_remove_handle(multi.multi, x.first);
  }
  multi.easy.Clear();
}
void SolveSpeedLimit(const easy_mapping_easyclosure::Value &easy_closure,
                     libcurl_multi_closure &multi) {
  auto &easy_handle = easy_closure->get_easy();
  if (multi.easy.Exists(easy_handle)) {
    if (easy_closure->operation ==
        assistant::core::libcurl_easy_closure::SpecialOpt::LimitDownloadSpeed) {
      curl_multi_remove_handle(multi.multi, easy_handle);
      auto ddd = curl_easy_setopt(easy_handle, CURLOPT_MAX_RECV_SPEED_LARGE,
                                  easy_closure->speedlimit);
      curl_multi_add_handle(multi.multi, easy_handle);
    } else if (easy_closure->operation ==
               assistant::core::libcurl_easy_closure::SpecialOpt::
                   LimitUploadSpeed) {
      curl_multi_remove_handle(multi.multi, easy_handle);
      curl_easy_setopt(easy_handle, CURLOPT_MAX_SEND_SPEED_LARGE,
                       easy_closure->speedlimit);
      curl_multi_add_handle(multi.multi, easy_handle);
    }
  }
}
}  // namespace curl_multi

/// 提供multi socket 流程所需的全局信息闭包，并支持配置multi选项
struct libcurl_multi_socket_closure {
 private:
  typedef std::function<void(CURLMsg *message)> Callback;
  /// 保存multi句柄，需配置；multi socket全流程必须用到
  CURLM *multi;
  /// 保存事件循环句柄，用于初始化定时器句柄，以及poll句柄
  uv_loop_t *loop;
  /// 保存multi_socket流程中libuv使用的定时器句柄
  assistant::core::libuv_timer_closure socket_timer;
  Callback msg_cb;

 public:
  libcurl_multi_socket_closure(CURLM *multi_handle, uv_loop_t *loop_handle,
                               Callback cb)
      : multi(multi_handle),
        loop(loop_handle),
        socket_timer(
            *loop_handle,
            std::bind(&libcurl_multi_socket_closure::socket_timer_cb, this)),
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
    auto &global_info = *static_cast<libcurl_multi_socket_closure *>(userp);
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
    auto &global_info = *static_cast<libcurl_multi_socket_closure *>(userp);
    switch (action) {
      case CURL_POLL_REMOVE:
        if (nullptr != sockp) {
          auto poll_closure = (assistant::core::libuv_poll_closure *)sockp;
          poll_closure->Stop();
          curl_multi_assign(global_info.multi, s, nullptr);
          assistant::core::libuv_poll_closure::AsyncDestroy(poll_closure);
        }
        break;
      case CURL_POLL_IN:
      case CURL_POLL_OUT:
      case CURL_POLL_INOUT:
      default: {
        int events = 0;
        /// 初始化 poll 的闭包，并传入 libcurl_multi_socket_closure 的回调
        /// socket_poll_cb
        auto poll_closure =
            nullptr != sockp
                ? (assistant::core::libuv_poll_closure *)sockp
                : assistant::core::libuv_poll_closure::Create(
                      *global_info.loop, s,
                      std::bind(&libcurl_multi_socket_closure::socket_poll_cb,
                                &global_info, std::placeholders::_1,
                                std::placeholders::_2, std::placeholders::_3));
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
}  // namespace core
}  // namespace assistant
#endif  // _CORE_CURL_MULTI_H__
