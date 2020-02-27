#pragma once
#ifndef RX_UV_FS
#define RX_UV_FS
#ifdef _MSC_VER
//#pragma warning(disable : 4503)
#endif
#include <functional>
#include <memory>
#include <mutex>

#include <Assistant_v3.hpp>

#include <rxcpp/rx.hpp>

namespace rx_uv_fs {

/// Provide a uv_loop run in thread that would join while destruction
struct uv_loop_with_thread {
 public:
  typedef std::function<void(uv_loop_t&)> uv_todo_wrapper;
  uv_loop_with_thread()
      : guard([this]() {
          uv_loop.LoopStop();
          thread_model.ThreadJoin();
        }),
        uv_wakeup(uv_loop.thread_loop, [this]() {
          auto& loop = uv_loop.thread_loop;
          auto item = uv_todo_queue.Dequeue();
          do {
            if (nullptr == item) {
              break;
            }
            const auto& wrapper_function = *item;
            if (nullptr == wrapper_function) {
              break;
            }
            wrapper_function(loop);
          } while (false);
        }) {
    thread_model.ThreadCreate(thread_worker, &uv_loop);
  }
  void Do(uv_todo_wrapper& wrapper) {
    uv_todo_queue.Enqueue(std::make_unique<uv_todo_wrapper>(wrapper));
    uv_wakeup.Notify();
  }

 private:
  assistant::core::libuv_threadmodel thread_model;
  assistant::core::thread_loop_closure uv_loop;
  assistant::tools::safequeue_closure<uv_todo_wrapper> uv_todo_queue;
  assistant::core::libuv_async_closure uv_wakeup;
  assistant::tools::scope_guard guard;

  static void thread_worker(void* uv_loop_ptr) {
    auto& uv_loop =
        *static_cast<assistant::core::thread_loop_closure*>(uv_loop_ptr);
    return uv_loop.LoopRun();
  }
  /// TODO: 禁用移动、复制构造和=号操作符
};

namespace uv_fs_stat {
/// 入参内的int32_t为0x4000，代表传入的路径是有效的文件夹
typedef int32_t Type;
typedef std::function<void(Type)> Callback;
namespace details {
inline void stat_callback(uv_fs_t* req) {
  /// req必然非空指针
  const auto& mode = req->statbuf.st_mode;
  Type result{0};
  if (mode & S_IFREG) {
    printf("%s isfile\n", req->path);
    result = static_cast<Type>(S_IFREG);
  }
  if (mode & S_IFDIR) {
    printf("%s isdir\n", req->path);
    result = static_cast<Type>(S_IFDIR);
  }
  do {
    if (nullptr == req->data) {
      break;
    }
    const auto& callback = *static_cast<Callback*>(req->data);
    if (nullptr == callback) {
      break;
    }
    callback(result);
    delete static_cast<Callback*>(req->data);
    req->data = nullptr;
  } while (false);

  if (nullptr != req) {
    uv_fs_req_cleanup(req);
    delete req;
  }
}
}  // namespace details
inline void uv_fs_stat_wrapper(uv_loop_t& loop, const std::string& path,
                               Callback callback) {
  uv_fs_t* req = new (std::nothrow) uv_fs_t({0});
  auto callback_ptr = new (std::nothrow) Callback(callback);
  req->data = static_cast<void*>(callback_ptr);
  ::uv_fs_stat(&loop, req, path.c_str(), details::stat_callback);
}
}  // namespace uv_fs_stat

namespace uv_fs_scandir {
typedef struct {
  std::vector<std::string> files;
  std::vector<std::string> dirs;
} Type;
typedef std::function<void(Type)> Callback;
namespace details {
inline void scandir_callback(uv_fs_t* req) {
  do {
    if (nullptr == req->data) {
      break;
    }
    const auto& callback = *static_cast<Callback*>(req->data);
    if (nullptr == callback) {
      break;
    }
    /// 从事件循环中被调用的req必然非空指针、野指针
    Type result;
    for (uv_dirent_t dirent{nullptr};;) {
      const auto scandir_next_result = uv_fs_scandir_next(req, &dirent);
      if (UV_EOF == scandir_next_result || scandir_next_result < 0) {
        break;
      }
      if (dirent.type == UV_DIRENT_DIR) {
        printf("%s is a dir.\n", dirent.name);
        result.dirs.emplace_back(dirent.name);
      } else if (dirent.type == UV_DIRENT_FILE) {
        printf("%s is a file.\n", dirent.name);
        result.files.emplace_back(dirent.name);
      } else {
        printf("%s Unknown type.\n", dirent.name);
      }
    }
    callback(result);
    delete static_cast<Callback*>(req->data);
    req->data = nullptr;
  } while (false);

  if (nullptr != req) {
    uv_fs_req_cleanup(req);
    delete req;
  }
}
}  // namespace details
inline void uv_fs_scandir_wrapper(uv_loop_t& loop, const std::string& path,
                                  Callback callback) {
  uv_fs_t* req = new (std::nothrow) uv_fs_t({0});
  auto callback_ptr = new (std::nothrow) Callback(callback);
  req->data = static_cast<void*>(callback_ptr);
  ::uv_fs_scandir(&loop, req, path.c_str(), 0, details::scandir_callback);
}
}  // namespace uv_fs_scandir

namespace rx_uv_fs_factory {
inline rxcpp::observable<uv_fs_stat::Type> Stat(
    std::weak_ptr<uv_loop_with_thread> thread, const std::string& path) {
  return rxcpp::observable<>::create<uv_fs_stat::Type>(
      [thread, path](rxcpp::subscriber<uv_fs_stat::Type> s) -> void {
        auto thread_ptr = thread.lock();
        if (s.is_subscribed() && nullptr != thread_ptr) {
          /// 必须以值捕获的方式，保持rxcpp::subscriber<XXX>生命周期
          uv_fs_stat::Callback subscriber_func =
              [s](uv_fs_stat::Type result) -> void {
            s.on_next(result);
            s.on_completed();
          };

          rx_uv_fs::uv_loop_with_thread::uv_todo_wrapper stat_task =
              std::bind(uv_fs_stat::uv_fs_stat_wrapper, std::placeholders::_1,
                        path, subscriber_func);
          thread_ptr->Do(stat_task);
        } else if (s.is_subscribed()) {
          /// 发射一个负面的响应，保证完备
          s.on_next(0);
          s.on_completed();
        }
      });
}

inline rxcpp::observable<uv_fs_scandir::Type> Scandir(
    std::weak_ptr<uv_loop_with_thread> thread, const std::string& path) {
  return rxcpp::observable<>::create<uv_fs_scandir::Type>(
      [thread, path](rxcpp::subscriber<uv_fs_scandir::Type> s) -> void {
        auto thread_ptr = thread.lock();
        if (s.is_subscribed() && nullptr != thread_ptr) {
          /// 必须以值捕获的方式，保持rxcpp::subscriber<XXX>生命周期
          uv_fs_scandir::Callback subscriber_func =
              [s](uv_fs_scandir::Type result) -> void {
            s.on_next(result);
            s.on_completed();
          };

          rx_uv_fs::uv_loop_with_thread::uv_todo_wrapper scandir_task =
              std::bind(uv_fs_scandir::uv_fs_scandir_wrapper,
                        std::placeholders::_1, path, subscriber_func);
          thread_ptr->Do(scandir_task);
        } else if (s.is_subscribed()) {
          /// 发射一个负面的响应，保证完备
          s.on_next(uv_fs_scandir::Type());
          s.on_completed();
        }
      });
}
}  // namespace rx_uv_fs_factory
}  // namespace rx_uv_fs

#endif  /// RX_UV_FS
