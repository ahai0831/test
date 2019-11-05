#pragma once
#ifndef _CORE_UV_THREAD_H__
#define _CORE_UV_THREAD_H__
#include <memory>
#include <thread>
#include <tuple>

#ifdef ASSISTANT_USE_STD_THREAD
#include <ThreadPool.h>
#endif  // ASSISTANT_USE_STD_THREAD
#include <uv.h>

namespace assistant {
namespace core {
typedef void (*thread_worker_cb)(void *data);

#ifdef ASSISTANT_USE_STD_THREAD
/// 线程模型（用于执行事件循环）
struct threadmodel {
 private:
  std::unique_ptr<std::thread> thread;
  /// TODO: 增加once_flag
  /// TODO: 禁用线程模型的移动构造、复制构造和赋值运算符
 public:
  void ThreadCreate(thread_worker_cb worker, void *data) {
    if (nullptr == thread) {
      thread = std::make_unique<std::thread>(worker, data);
    }
  }
  void ThreadJoin() {
    if (nullptr != thread && thread->joinable()) {
      thread->join();
    }
  }
};
#endif  // ASSISTANT_USE_STD_THREAD

/// 可作为标准库线程模型的替代品
/// libuv提供的线程（仅可被join）
/// 一旦不可靠可降级到标准库线程
struct libuv_threadmodel {
 private:
  uv_thread_t thread;

 public:
  /// TODO: 增加对线程模型的避免重入处理
  /// TODO: 显式指定禁用线程模型的移动构造、复制构造和=号运算符
  libuv_threadmodel() : thread((void *)(0)){};
  void ThreadCreate(thread_worker_cb worker, void *data) {
    uv_thread_create(&thread, worker, data);
  }
  void ThreadJoin() { uv_thread_join(&thread); }
};

#ifdef ASSISTANT_USE_STD_THREAD
/// threadpool_closure
struct threadpool_closure {
 private:
  ThreadPool pool;
  typedef std::function<void(void *)> Callback;
  typedef void *Data;

 public:
  threadpool_closure() : pool(4) {}
  void Run(Callback worker, Data data) { pool.enqueue(worker, data); }
};
#endif  // ASSISTANT_USE_STD_THREAD

/// libuv_threadpool_closure
/// 由于存在对wrapper的内存分配和释放
/// 对Run与after_work_cb的调用应在同一线程
struct libuv_threadpool_closure {
 private:
  const uv_loop_t *loop;
  typedef std::function<void(void *)> Callback;
  typedef void *Data;
  /// 优化为结构体，优化性能
  typedef std::tuple<uv_work_t, Callback, Data> threadpool_wrapper;

 public:
  libuv_threadpool_closure(const uv_loop_t &uv_loop) : loop(&uv_loop) {}
  static void work_cb(uv_work_t *handle) {
    auto wrapper = static_cast<threadpool_wrapper *>(handle->data);
    Callback &cb = std::get<1>(*wrapper);
    Data &cb_data = std::get<2>(*wrapper);
    cb(cb_data);
  }
  static void after_work_cb(uv_work_t *handle, int status) {
    auto wrapper = static_cast<threadpool_wrapper *>(handle->data);
    uv_work_t &work_handle = std::get<0>(*wrapper);
    work_handle.data = nullptr;
    delete wrapper;
  }
  void Run(Callback worker, Data data) {
    auto wrapper = new (std::nothrow) threadpool_wrapper();
    uv_work_t &work_handle = std::get<0>(*wrapper);
    Callback &cb = std::get<1>(*wrapper);
    Data &cb_data = std::get<2>(*wrapper);
    work_handle.data = wrapper;
    cb = worker;
    cb_data = data;
    uv_queue_work(const_cast<uv_loop_t *>(loop), &work_handle, work_cb,
                  after_work_cb);
  }
};
}  // namespace core
}  // namespace assistant

#endif  // _CORE_UV_THREAD_H__
