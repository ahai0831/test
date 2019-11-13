#pragma once
#ifndef TOOLS_SAFEQUEUE_H__
#define TOOLS_SAFEQUEUE_H__
#include <memory>

#include <concurrentqueue.h>
namespace assistant {
namespace tools {
/// 用于存取T(Unique_ptr)的闭包
template <typename T>
struct safequeue_closure {
 private:
  typedef std::unique_ptr<T> Item;
  typedef moodycamel::ConcurrentQueue<Item> Queue;
  std::unique_ptr<Queue> _queue;

 public:
  safequeue_closure() : _queue(std::make_unique<Queue>()) {}
  /// Should use std::move
  void Enqueue(Item &item) { _queue->enqueue(std::move(item)); }
  /// Should compare with nullptr
  Item Dequeue() {
    Item item;
    _queue->try_dequeue(item);
    return item;
  }
  size_t Size() { return _queue->size_approx(); }

 private:
  /// 禁用其他隐式的构造函数
  safequeue_closure(safequeue_closure const &) = delete;
  safequeue_closure &operator=(safequeue_closure const &) = delete;
  safequeue_closure(safequeue_closure &&) = delete;
};
}  // namespace tools
}  // namespace assistant

#endif  // TOOLS_SAFEQUEUE_H__
