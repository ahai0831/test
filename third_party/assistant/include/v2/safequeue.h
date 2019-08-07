#pragma once
#ifndef V2_SAFEQUEUE_H__
#define V2_SAFEQUEUE_H__
#include <memory>

#include <concurrentqueue.h>

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

 private:
  /// 禁用其他隐式的构造函数
  safequeue_closure(safequeue_closure const &) = delete;
  safequeue_closure &operator=(safequeue_closure const &) = delete;
  safequeue_closure(safequeue_closure &&) = delete;
};
#endif  // V2_SAFEQUEUE_H__
