#pragma once
#ifndef V2_CURL_SHARE_H__
#define V2_CURL_SHARE_H__
#include <list>
#include <memory>
#include <mutex>

#include <curl/curl.h>
#include <scopeguard.h>
#include <uv.h>

/// 利用标准库锁机制定义的非递归锁闭包
struct a7_2 {
 private:
  typedef std::mutex Lock;
  std::unique_ptr<Lock> mutex;

 public:
  explicit a7_2() : mutex(std::make_unique<Lock>()) {}
  void lock() { mutex->lock(); }
  void unlock() { mutex->unlock(); }
};

/// 利用libuv的mutex定义的非递归锁闭包
/// 注意，必须使此闭包的内存分配于堆上
struct a7_2_uv {
 private:
  typedef uv_mutex_t Lock;
  Lock mutex;
  scopeguard_internal::ScopeGuard guard;

 public:
  explicit a7_2_uv()
      : mutex({0}), guard([this]() { uv_mutex_destroy(&mutex); }) {
    uv_mutex_init(&mutex);
  }
  void lock() { uv_mutex_lock(&mutex); }
  void unlock() { uv_mutex_unlock(&mutex); }
};

struct a7_1 {
 private:
  CURLSH *share;
  std::unique_ptr<a7_2_uv[]> lock;
  void *data;
  scopeguard_internal::ScopeGuard guard;

 public:
  explicit a7_1()
      : share(nullptr),
        lock(std::make_unique<a7_2_uv[]>(CURL_LOCK_DATA_LAST)),
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
      auto &share_closure = *static_cast<a7_1 *>(useptr);
      share_closure.lock[data].lock();
    }
  }
  static void unlock_cb(CURL *handle, curl_lock_data data, void *useptr) {
    if (nullptr != useptr) {
      auto &share_closure = *static_cast<a7_1 *>(useptr);
      share_closure.lock[data].unlock();
    }
  }
};

struct a7_3 {
 private:
  typedef std::shared_ptr<a7_1> Item;
  typedef std::list<Item> List;
  std::unique_ptr<List> _list;

 public:
  explicit a7_3() : _list(std::make_unique<List>()) {
    _list->emplace_front(std::make_shared<a7_1>());
  }
  std::weak_ptr<a7_1> get_first_weak() {
    std::weak_ptr<a7_1> weak;
    if (!_list->empty() && nullptr != _list->front()) {
      weak = std::weak_ptr<a7_1>(_list->front());
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
  void AddNew() { _list->emplace_front(std::make_shared<a7_1>()); }
};

#endif  // V2_CURL_SHARE_H__
