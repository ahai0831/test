#pragma once
#ifndef ASSISTANT_SAFECONTAINER_H__
#define ASSISTANT_SAFECONTAINER_H__

#include <functional>
#include <map>
#include <mutex>
#include <set>

namespace assistant {

/// 基于互斥量实现的线程安全Set
template <typename T>
struct safeset_closure {
 private:
  typedef std::set<T> SetType;
  typedef typename SetType::key_type Item;
  SetType set;
  std::mutex mutex;

 public:
  typedef std::function<void(const Item&)> ForeachCallback;

 public:
  safeset_closure() = default;
  void Put(const Item& key) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    set.emplace(key);
  }
  bool Delete(const Item& key) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    bool flag = false;
    auto iter = set.find(key);
    if (flag = (set.end() != iter)) {
      set.erase(iter);
    }
    return flag;
  }
  void ForeachDelegate(ForeachCallback delegate) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    std::for_each(set.begin(), set.end(), delegate);
  }
  bool empty() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    return set.empty();
  }
  typename SetType::size_type size() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    return set.size();
  }
  ~safeset_closure() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    /// 显式地清理并释放内存；令临时变量就地析构
    auto i = decltype(set)();
    set.swap(i);
  }

 private:
  /// 禁用其他隐式的构造函数
  safeset_closure(safeset_closure const&) = delete;
  safeset_closure& operator=(safeset_closure const&) = delete;
  safeset_closure(safeset_closure&&) = delete;
};

/// 基于互斥量实现的线程安全Map
template <typename K, typename V>
struct safemap_closure {
 private:
  typedef std::map<K, V> MapType;
  typedef typename MapType::key_type Key;
  typedef typename MapType::mapped_type Value;
  MapType map;
  std::mutex mutex;

 public:
  /// TODO: 考虑去掉const限制
  typedef std::function<void(const Value&)> FindCallback;
  typedef std::function<void(const Key&, const Value&)> ForeachCallback;

 public:
  safemap_closure() = default;
  /// Key已存在则更新，否则插入
  void Set(const Key& key, const Value& value) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    map[key] = value;
  }
  /// 直接插入，若Key已存在则插入失败，返回false
  bool Put(const Key& key, const Value& value) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    return map.emplace(key, value).second;
  }
  /// 查询Key对应的Value，失败则返回 false
  bool Get(const Key& key, Value& value) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    bool flag = false;
    auto iter = map.find(key);
    if (flag = (map.end() != iter)) {
      value = iter->second;
    }
    return flag;
  }
  /// 取出Key对应的Value，失败则返回 false
  bool Take(const Key& key, Value& value) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    bool flag = false;
    auto iter = map.find(key);
    if (flag = (map.end() != iter)) {
      value = iter->second;
      map.erase(iter);
    }
    return flag;
  }
  /// 删除Key对应的Value，失败则返回 false
  bool Delete(const Key& key) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    bool flag = false;
    auto iter = map.find(key);
    if (flag = (map.end() != iter)) {
      map.erase(iter);
    }
    return flag;
  }
  /// 查询Key对应的Value，利用Delegate处理Value；失败则什么也不做
  void FindDelegate(const Key& key, FindCallback delegate) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    auto iter = map.find(key);
    if (map.end() != iter) {
      delegate(iter->second);
    }
  }
  void ForeachDelegate(ForeachCallback delegate) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    for (const auto& x : map) {
      delegate(x.first, x.second);
    }
  }
  bool empty() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    return map.empty();
  }
  typename MapType::size_type size() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    return map.size();
  }
  ~safemap_closure() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    /// 显式地清理并释放内存；令临时变量就地析构
    auto i = decltype(map)();
    map.swap(i);
  }

 private:
  /// 禁用其他隐式的构造函数
  safemap_closure(safemap_closure const&) = delete;
  safemap_closure& operator=(safemap_closure const&) = delete;
  safemap_closure(safemap_closure&&) = delete;
};

}  // namespace assistant

#endif  // ASSISTANT_SAFECONTAINER_H__
