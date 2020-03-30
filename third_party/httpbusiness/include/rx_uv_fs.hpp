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

#include "rx_md5.hpp"

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
    auto tempWarpper = std::make_unique<uv_todo_wrapper>(wrapper);
    uv_todo_queue.Enqueue(tempWarpper);
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
typedef struct uv_fs_scandir_type__ {
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

namespace uv_fs_mkdir {
/// 入参内的int32_t为0，代表传入的路径mkdir操作成功
typedef int32_t Type;
typedef std::function<void(Type)> Callback;
namespace details {
inline void mkdir_callback(uv_fs_t* req) {
  /// req必然非空指针
  Type result{static_cast<Type>(req->result)};

  if (0 == result) {
    printf("mkdir: %s succeed\n", req->path);
  } else {
    printf("mkdir: %s failed\n", req->path);
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
inline void uv_fs_mkdir_wrapper(uv_loop_t& loop, const std::string& path,
                                Callback callback) {
  uv_fs_t* req = new (std::nothrow) uv_fs_t({0});
  auto callback_ptr = new (std::nothrow) Callback(callback);
  req->data = static_cast<void*>(callback_ptr);
  int mode = 0;
#ifdef _WIN32
  /// Do nothing, for libuv ignore mode in Windows.
#else
  mode = S_IRWXU | S_IRWXG | S_IRWXO;
#endif
  ::uv_fs_mkdir(&loop, req, path.c_str(), mode, details::mkdir_callback);
}
}  // namespace uv_fs_mkdir

namespace uv_fs_rename {
/// 入参内的bool为true，代表rename操作成功
typedef bool Type;
typedef std::function<void(Type)> Callback;
namespace details {
inline void rename_callback(uv_fs_t* req) {
  /// req必然非空指针
  Type result{false};
  result = 0 == req->result;

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
inline void uv_fs_rename_wrapper(uv_loop_t& loop, const std::string& path,
                                 const std::string& new_path,
                                 Callback callback) {
  uv_fs_t* req = new (std::nothrow) uv_fs_t({0});
  auto callback_ptr = new (std::nothrow) Callback(callback);
  req->data = static_cast<void*>(callback_ptr);

  ::uv_fs_rename(&loop, req, path.c_str(), new_path.c_str(),
                 details::rename_callback);
}
}  // namespace uv_fs_rename

/// 在此增加基于uv的事件循环线程进行同步阻塞式计算文件MD5的流程
/// 注意MD5的计算过程，将持续占用指定uv的线程，有异于其他fs_xxx
namespace uv_sync_md5 {
/// 入参内的std::string代表MD5，非空字符串代表计算完成
typedef std::string Type;
/// std::function<bool()>为形式。一旦返回值为false，立即打断计算过程
typedef rx_assistant::md5::CheckStopCallback CheckStopCallback;
typedef std::function<void(Type)> Callback;

inline void uv_sync_md5_wrapper(uv_loop_t&, const std::string& path,
                                const CheckStopCallback& stop_cb,
                                Callback callback) {
  const auto result = rx_assistant::md5::md5_sync_calculate_with_process(
      path, -1, -1, rx_assistant::rx_md5::details::DefaultMd5ProcessCallback,
      stop_cb);
  callback(result);
}
}  // namespace uv_sync_md5

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

inline rxcpp::observable<uv_fs_mkdir::Type> Mkdir(
    std::weak_ptr<uv_loop_with_thread> thread, const std::string& path) {
  return rxcpp::observable<>::create<uv_fs_mkdir::Type>(
      [thread, path](rxcpp::subscriber<uv_fs_mkdir::Type> s) -> void {
        auto thread_ptr = thread.lock();
        if (s.is_subscribed() && nullptr != thread_ptr) {
          /// 必须以值捕获的方式，保持rxcpp::subscriber<XXX>生命周期
          uv_fs_mkdir::Callback subscriber_func =
              [s](uv_fs_mkdir::Type result) -> void {
            s.on_next(result);
            s.on_completed();
          };

          rx_uv_fs::uv_loop_with_thread::uv_todo_wrapper stat_task =
              std::bind(uv_fs_mkdir::uv_fs_mkdir_wrapper, std::placeholders::_1,
                        path, subscriber_func);
          thread_ptr->Do(stat_task);
        } else if (s.is_subscribed()) {
          /// 发射一个负面的响应，保证完备
          s.on_next(0);
          s.on_completed();
        }
      });
}

inline rxcpp::observable<uv_fs_rename::Type> Rename(
    std::weak_ptr<uv_loop_with_thread> thread, const std::string& path,
    const std::string& new_path) {
  return rxcpp::observable<>::create<uv_fs_rename::Type>(
      [thread, path,
       new_path](rxcpp::subscriber<uv_fs_rename::Type> s) -> void {
        auto thread_ptr = thread.lock();
        if (s.is_subscribed() && nullptr != thread_ptr) {
          /// 必须以值捕获的方式，保持rxcpp::subscriber<XXX>生命周期
          uv_fs_rename::Callback subscriber_func =
              [s](uv_fs_rename::Type result) -> void {
            s.on_next(result);
            s.on_completed();
          };

          rx_uv_fs::uv_loop_with_thread::uv_todo_wrapper rename_task =
              std::bind(uv_fs_rename::uv_fs_rename_wrapper,
                        std::placeholders::_1, path, new_path, subscriber_func);
          thread_ptr->Do(rename_task);
        } else if (s.is_subscribed()) {
          /// 发射一个负面的响应，保证完备
          s.on_next(false);
          s.on_completed();
        }
      });
}

inline rxcpp::observable<uv_sync_md5::Type> Md5(
    std::weak_ptr<uv_loop_with_thread> thread, const std::string& path,
    const uv_sync_md5::CheckStopCallback& stop_cb) {
  return rxcpp::observable<>::create<uv_sync_md5::Type>(
      [thread, path, stop_cb](rxcpp::subscriber<uv_sync_md5::Type> s) -> void {
        auto thread_ptr = thread.lock();
        if (s.is_subscribed() && nullptr != thread_ptr) {
          /// 必须以值捕获的方式，保持rxcpp::subscriber<XXX>生命周期
          uv_sync_md5::Callback subscriber_func =
              [s](uv_sync_md5::Type result) -> void {
            s.on_next(result);
            s.on_completed();
          };

          rx_uv_fs::uv_loop_with_thread::uv_todo_wrapper md5_task =
              std::bind(uv_sync_md5::uv_sync_md5_wrapper, std::placeholders::_1,
                        path, stop_cb, subscriber_func);
          thread_ptr->Do(md5_task);
        } else if (s.is_subscribed()) {
          /// 发射一个负面的响应，保证完备
          s.on_next(uv_sync_md5::Type());
          s.on_completed();
        }
      });
}

}  // namespace rx_uv_fs_factory
}  // namespace rx_uv_fs

#endif  /// RX_UV_FS
