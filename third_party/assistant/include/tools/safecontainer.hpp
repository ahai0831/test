#pragma once
#ifndef TOOLS_SAFECONTAINER_H__
#define TOOLS_SAFECONTAINER_H__

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <set>

namespace assistant {
namespace tools {
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

/// 基于互斥量实现的线程安全Vector
template <typename T>
struct safevector_closure {
 private:
  typedef std::vector<T> VectorType;
  typedef typename VectorType::value_type Item;
  VectorType vec;
  std::mutex mutex;

 public:
  typedef std::function<void(const Item&)> ForeachCallback;

 public:
  safevector_closure() = default;
  void Put(const Item& key) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    vec.emplace_back(key);
  }
  bool Delete(const Item& key) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    bool flag = false;
    auto iter = vec.find(key);
    if (flag = (vec.end() != iter)) {
      vec.erase(iter);
    }
    return flag;
  }
  void ForeachDelegate(ForeachCallback delegate) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    std::for_each(vec.begin(), vec.end(), delegate);
  }
  bool empty() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    return vec.empty();
  }
  typename VectorType::size_type size() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    return vec.size();
  }
  ~safevector_closure() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    /// 显式地清理并释放内存；令临时变量就地析构
    auto i = decltype(vec)();
    vec.swap(i);
  }

 private:
  /// 禁用其他隐式的构造函数
  safevector_closure(safevector_closure const&) = delete;
  safevector_closure& operator=(safevector_closure const&) = delete;
  safevector_closure(safevector_closure&&) = delete;
};

/// 基于无锁和std::shard_ptr实现的无锁只读型Vector
/// 适用场景：无竞争写入行为，高频率读取行为
/// 内部行为：每次Add()，会重新生成一个只读的Vector
/// 每次获取WeakPtr()，会取得一个弱指针包裹的const Vector
/// 调用者不应突破const 的限制
/// 若有竞争写入行为，不会产生错误，但写入的结果不可预料
/// 每次Clear()，会将对应的内容置空
/// empty()判断对应的vector是否为空，或本身无内容
/// size()返回
template <typename T>
struct lockfree_vector_closure {
 private:
  typedef std::vector<T> VectorType;
  typedef typename VectorType::value_type Item;
  /// 必须保证对此对象的读写均为原子操作
  std::shared_ptr<const VectorType> vec_ptr;

 public:
  lockfree_vector_closure() = default;
  ~lockfree_vector_closure() = default;
  void Add(const Item& key) {
    VectorType vec{key};
    auto pre_ptr = std::atomic_load(&vec_ptr);
    if (nullptr != pre_ptr && !pre_ptr->empty()) {
      vec.reserve(1 + pre_ptr->size());
      vec.insert(vec.end(), pre_ptr->begin(), pre_ptr->end());
    }
    auto next_ptr = std::make_shared<const VectorType>(std::move(vec));
    std::atomic_exchange(&vec_ptr, next_ptr);
  }
  std::weak_ptr<const VectorType> WeakPtr() {
    return std::atomic_load(&vec_ptr);
  }
  void Clear() {
    decltype(vec_ptr) next_ptr = nullptr;
    std::atomic_exchange(&vec_ptr, next_ptr);
  }
  bool empty() {
    auto pre_ptr = std::atomic_load(&vec_ptr);
    return nullptr == pre_ptr || pre_ptr->empty();
  }
  typename VectorType::size_type size() {
    auto pre_ptr = std::atomic_load(&vec_ptr);
    return nullptr == pre_ptr ? 0 : pre_ptr->size();
  }

  /// 支持移动构造
  lockfree_vector_closure(lockfree_vector_closure&& vec) {
    std::atomic_exchange(&vec.vec_ptr, this->vec_ptr);
  }
  /// 禁用其他隐式的构造函数
  lockfree_vector_closure(lockfree_vector_closure const&) = delete;
  lockfree_vector_closure& operator=(lockfree_vector_closure const&) = delete;
};

/// TODO: 增加以无锁实现的无锁无竞争写只读型String
/// 对特定字符串的线程安全实现，频繁读取使用mutex影响性能

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
  bool Emplace(Key& key, Value& value) {
    std::lock_guard<decltype(mutex)> lock(mutex);
    return map.emplace(std::move(key), std::move(value)).second;
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
}  // namespace tools
}  // namespace assistant

#endif  // TOOLS_SAFECONTAINER_H__
