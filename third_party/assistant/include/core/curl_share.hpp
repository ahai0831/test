#pragma once
#ifndef _CORE_CURL_SHARE_H__
#define _CORE_CURL_SHARE_H__
#include <list>
#include <memory>
#ifdef ASSISTANT_USE_STD_MUTEX
#include <mutex>
#endif

#include <curl/curl.h>
#include <uv.h>

#include "tools/scopeguard.hpp"
namespace assistant {
namespace core {
/// 利用标准库锁机制定义的非递归锁闭包
#ifdef ASSISTANT_USE_STD_MUTEX
struct std_lock_closure {
 private:
  typedef std::mutex Lock;
  std::unique_ptr<Lock> mutex;

 public:
  explicit std_lock_closure() : mutex(std::make_unique<Lock>()) {}
  void lock() { mutex->lock(); }
  void unlock() { mutex->unlock(); }
  std_lock_closure(std_lock_closure &&lock) mutex(std::move(lock.mutex)) {}
  std_lock_closure(const std_lock_closure &) = delete;
  std_lock_closure &operator=(const std_lock_closure &) = delete;
};
#endif
/// 利用libuv的mutex定义的非递归锁闭包
/// 注意，必须使此闭包的内存分配于堆上
struct libuv_lock_closure {
 private:
  typedef uv_mutex_t Lock;
  Lock mutex;
  assistant::tools::scope_guard guard;

 public:
  explicit libuv_lock_closure()
      : mutex({0}), guard([this]() { uv_mutex_destroy(&mutex); }) {
    uv_mutex_init(&mutex);
  }
  void lock() { uv_mutex_lock(&mutex); }
  void unlock() { uv_mutex_unlock(&mutex); }

 private:
  /// 禁用移动构造、复制构造和=号操作符
  libuv_lock_closure(libuv_lock_closure &&) = delete;
  libuv_lock_closure(const libuv_lock_closure &) = delete;
  libuv_lock_closure &operator=(const libuv_lock_closure &) = delete;
};

struct libcurl_share_closure {
 private:
  CURLSH *share;
  std::unique_ptr<libuv_lock_closure[]> lock;
  void *data;
  assistant::tools::scope_guard guard;

 public:
  explicit libcurl_share_closure()
      : share(nullptr),
        lock(std::make_unique<libuv_lock_closure[]>(CURL_LOCK_DATA_LAST)),
        data(this),
        guard([this]() {
          curl_share_cleanup(share);
          share = nullptr;
        }) {
    share = curl_share_init();
    /// 为缩短连接的启动时间，指定需要利用DNS缓存，SSL会话缓存，连接缓存
    /// 对C/S架构的网络库而言，Cookies缓存和PSL缓存无意义
    curl_share_setopt(share, CURLSHOPT_UNSHARE, CURL_LOCK_DATA_COOKIE);
    curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
    curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
    curl_share_setopt(share, CURLSHOPT_UNSHARE, CURL_LOCK_DATA_PSL);
    /// 多线程下使用easy句柄传输并设置同一个share句柄，则需要加锁
    curl_share_setopt(share, CURLSHOPT_USERDATA, data);
    curl_share_setopt(share, CURLSHOPT_LOCKFUNC, lock_cb);
    curl_share_setopt(share, CURLSHOPT_UNLOCKFUNC, unlock_cb);
  }
  CURLSH *&get_share() { return share; }

 private:
  static void lock_cb(CURL *handle, curl_lock_data data,
                      curl_lock_access laccess, void *useptr) {
    if (nullptr != useptr) {
      auto &share_closure = *static_cast<libcurl_share_closure *>(useptr);
      share_closure.lock[data].lock();
    }
  }
  static void unlock_cb(CURL *handle, curl_lock_data data, void *useptr) {
    if (nullptr != useptr) {
      auto &share_closure = *static_cast<libcurl_share_closure *>(useptr);
      share_closure.lock[data].unlock();
    }
  }

 public:
  libcurl_share_closure(libcurl_share_closure &&) = delete;
  libcurl_share_closure(const libcurl_share_closure &) = delete;
  libcurl_share_closure &operator=(const libcurl_share_closure &) = delete;
};

/// 基于非线程安全容器，需要保证从同一个线程中调用
struct libcurl_share_container {
 private:
  typedef std::shared_ptr<libcurl_share_closure> Item;
  typedef std::list<Item> List;
  std::unique_ptr<List> _list;

 public:
  explicit libcurl_share_container() : _list(std::make_unique<List>()) {
    _list->emplace_front(std::make_shared<libcurl_share_closure>());
  }
  std::weak_ptr<libcurl_share_closure> get_first_weak() {
    std::weak_ptr<libcurl_share_closure> weak;
    if (!_list->empty() && nullptr != _list->front()) {
      weak = std::weak_ptr<libcurl_share_closure>(_list->front());
    }
    return weak;
  }
  void CleanRedundant() {
    if (_list->size() > 1) {
      /// 利用_list的首项构造新List，直接交换
      auto i = List{std::move(_list->front())};
      _list->swap(i);
    }
  }
  void AddNew() {
    _list->emplace_front(std::make_shared<libcurl_share_closure>());
  }
  libcurl_share_container(libcurl_share_container &&) = delete;
  libcurl_share_container(const libcurl_share_container &) = delete;
  libcurl_share_container &operator=(const libcurl_share_container &) = delete;
};
}  // namespace core
}  // namespace assistant

#endif  // _CORE_CURL_SHARE_H__
