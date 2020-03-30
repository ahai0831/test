#pragma once
#ifndef RX_MD5_H
#define RX_MD5_H

#include <rxcpp/rx.hpp>

#include <HashAlgorithm/md5.h>
#include <core/readwrite_callback.hpp>
#include <core/uv_thread.hpp>
#include <tools/string_convert.hpp>

/// 改为依赖闭包，避免直接依赖平台特定的系统级API
// #include <windows.h>
#include <filecommon/filecommon_helper.h>
#include <filesystem_helper/filesystem_helper.h>

namespace rx_assistant {
namespace md5 {

namespace details {
typedef std::function<void(void *, uint64_t)> mmap_md5_callback;
inline void mmap_md5_access_callback(void *memory, uint64_t len,
                                     void *callback_data) {
  auto &callback = *(mmap_md5_callback *)callback_data;
  callback(memory, len);
}

}  // namespace details
/// 由于硬件级异常导致的问题，会导致计算的内容长度和实际内容长度不一致
/// 从而迫使返回一个失败的结果

/// int64_t 代表了 md5 计算的进度回调，传入值为每次update的内存块的长度
typedef std::function<void(int64_t)> Md5ProcessCallback;
/// 返回值代表了外部一旦传入false时，立即停止
typedef std::function<bool()> CheckStopCallback;
inline std::string md5_sync_calculate_with_process(
    const std::string &file_path, const int64_t range_left,
    const int64_t range_right, Md5ProcessCallback process_cb,
    CheckStopCallback checkstop_cb) {
  /// 若文件为0字节，依然会返回MD5；
  /// 若传入的range超出，返回空字符串
  /// 若文件打开内存映射失败，返回空字符串
  /// 若中途取消计算，返回空字符串
  /// 若文件读取完毕，计算的完整字节数与文件大小不一致，返回空字符串
  std::string result;
#ifdef _WIN32
  std::unique_ptr<assistant::core::readwrite::ReadwriteByMmap> readonly_mmap;
#else
  std::unique_ptr<assistant::core::readwrite::ReadByFile> readonly_file;
#endif
  do {

    /// 改成获取文件元数据
    uint64_t file_size = 0;

    const auto file_exist =
        cloud_base::file_common::GetFileSize(file_path, file_size);
    if (!file_exist) {
      break;
    }
    /// 校验文件range的规则
    ///  0<=range_left<=range_right<=file_size-1
    if ((-1 != range_left && -1 != range_right &&
         (range_left < 0 || range_right < range_left ||
          file_size - 1 < static_cast<uint64_t>(range_right)))) {
      break;
    }
    /// 需要计算的内容的长度，在range_left和range_right均为-1时，认为是计算整个文件；
    const uint64_t range_begin = -1 != range_left ? range_left : 0;
    const uint64_t range_length =
        -1 != range_left ? range_right - range_left + 1 : file_size;
    /// 若传入的range非法，则返回空字符串；
    /// 若传入的range合法，但range的长度为0，依然会返回这段MD5（等同于一个0字节的文件）
    cloud_base::hash_algorithm::MD5 _md5;
    if (0 == range_length) {
      _md5.finalize();
      result = _md5.hex_string();
      break;
    }
    /// 记录已计算的mmap长度
    uint64_t calculate_done = 0;
#ifdef _WIN32
    /// 文件存在，且大小大于0，才调用Mmap去读文件算MD5
    const auto mmap_type =
        static_cast<assistant::core::readwrite::ReadwriteByMmap::RW>(
            assistant::core::readwrite::ReadwriteByMmap::need_read |
            assistant::core::readwrite::ReadwriteByMmap::open_exists |
            assistant::core::readwrite::ReadwriteByMmap::shared_read);
    readonly_mmap =
        std::make_unique<assistant::core::readwrite::ReadwriteByMmap>(
            file_path.c_str(), file_size, range_begin, range_length, mmap_type);
    if (nullptr == readonly_mmap || !readonly_mmap->Valid()) {
      break;
    }
    /// 开启内存映射成功，才计算MD5
    const uint64_t caled_size = readonly_mmap->granularity
                                << readonly_mmap->power_of_multiple;
    /// 初始化内存映射读取MD5所需回调
    details::mmap_md5_callback read_function =
        [&_md5, &calculate_done, process_cb](void *data, uint64_t len) {
          /// 由于业务流程，不会一次性传入超过uint32_t最大值的块长度，进行截断无风险
          _md5.update(static_cast<unsigned char *>(data),
                      static_cast<uint32_t>(len));
          calculate_done += static_cast<uint32_t>(len);
          process_cb(static_cast<uint32_t>(len));
        };
    /// 改成传lambda去计算
    while (calculate_done < readonly_mmap->length && checkstop_cb()) {
      readonly_mmap->AccessDelegate(
          caled_size, details::mmap_md5_access_callback, &read_function);
    }
#else
    readonly_file = std::make_unique<assistant::core::readwrite::ReadByFile>(
        file_path.c_str(), file_size, range_begin, range_length);
    if (nullptr == readonly_file || !readonly_file->Valid()) {
      break;
    }
    const uint64_t caled_size = 4194304;  // 4MB
    auto file_buffer = std::make_unique<unsigned char[]>(caled_size);
    if (0 == caled_size || nullptr == file_buffer) {
      break;
    }
    void *const &fileview = static_cast<void *>(&file_buffer[0]);
    while (calculate_done < readonly_file->length && checkstop_cb()) {
      /// TODO: 需要完成所需读取逻辑
      auto readed_size = readonly_file->SelfCallback(
          fileview, (1 << 0), caled_size, &*readonly_file);
      _md5.update(static_cast<unsigned char *>(fileview),
                  static_cast<uint32_t>(readed_size));
      calculate_done += static_cast<uint32_t>(readed_size);
      process_cb(static_cast<uint32_t>(readed_size));
    }
#endif
    _md5.finalize();
    /// 仅当读取的数据总长度===文件长度；支持range
    /// 计算MD5成功
    if (range_length == calculate_done) {
      result = _md5.hex_string();
    }
  } while (false);
#ifdef _WIN32
  if (nullptr != readonly_mmap) {
    readonly_mmap->Destroy();
  }
#else
  if (nullptr != readonly_file) {
    readonly_file->Destroy();
  }
#endif
  return result;
}
/// 异步计算MD5，仅需要结果，无需进度回显
struct md5_async_factory {
 private:
  static const void DefaultMd5ProcessCallback(int64_t) {}
  static const bool DefaultCheckStopCallback() { return true; }

  struct md5_async {
    typedef std::function<void(const std::string &)> Callback;

   private:
    struct assistant::core::libuv_threadmodel thread_model;
    std::string _path;
    Callback _cb;
    Md5ProcessCallback process_cb;

   public:
    md5_async(const std::string &filepath, Callback callback)
        : _path(filepath), _cb(callback) {}
    md5_async(const std::string &filepath, Callback callback,
              Md5ProcessCallback md5_process_callback)
        : _path(filepath), _cb(callback), process_cb(md5_process_callback) {}
    ~md5_async() { thread_model.ThreadJoin(); }
    void async_calculate() {
      thread_model.ThreadCreate(async_calculate_work, (void *)this);
    }
    /// TODO: 加上终止的控制方法
   private:
    static void async_calculate_work(void *data) {
      if (nullptr != data) {
        md5_async &async = *static_cast<md5_async *>(data);
        std::string result;
        if (nullptr == async.process_cb) {
          result = md5_sync_calculate_with_process(async._path, -1, -1,
                                                   DefaultMd5ProcessCallback,
                                                   DefaultCheckStopCallback);
        } else {
          result = md5_sync_calculate_with_process(
              async._path, -1, -1, async.process_cb, DefaultCheckStopCallback);
        }
        async._cb(result);
      }
    }
    md5_async() = delete;
    md5_async(md5_async const &) = delete;
    md5_async &operator=(md5_async const &) = delete;
    md5_async(md5_async &&) = delete;
  };

 public:
  static std::unique_ptr<md5_async> create(const std::string &filepath,
                                           md5_async::Callback callback) {
    auto u = std::make_unique<md5_async>(filepath, callback,
                                         DefaultMd5ProcessCallback);
    u->async_calculate();
    return u;
  }
  static std::unique_ptr<md5_async> create(
      const std::string &filepath, md5_async::Callback callback,
      Md5ProcessCallback md5_process_callback) {
    auto u =
        std::make_unique<md5_async>(filepath, callback, md5_process_callback);
    u->async_calculate();
    return u;
  }
};

/// TODO: MD5计算队列，返回结果，支持回显进度

}  // namespace md5
/// 返回MD5计算结果的数据源，线程运行模型使用新线程并detach()进行，纯异步
namespace rx_md5 {
namespace details {
inline const void DefaultMd5ProcessCallback(int64_t) {}
inline const bool DefaultCheckStopCallback() { return true; }
}  // namespace details
/// 暂时无需进度
inline rxcpp::observable<std::string> create(
    const std::string &file_path, const int64_t range_left,
    const int64_t range_right, md5::Md5ProcessCallback process_cb,
    md5::CheckStopCallback checkstop_cb) {
  return rxcpp::observable<>::create<std::string>(
      [file_path, range_left, range_right, process_cb,
       checkstop_cb](rxcpp::subscriber<std::string> s) -> void {
        std::function<void(void)> md5_work_function = [s, file_path, range_left,
                                                       range_right, process_cb,
                                                       checkstop_cb]() -> void {
          if (s.is_subscribed()) {
            auto md5 = rx_assistant::md5::md5_sync_calculate_with_process(
                file_path, range_left, range_right, process_cb, checkstop_cb);
            s.on_next(md5);
          }
          s.on_completed();
        };
        /// 以detach的方式在新线程中进行MD5计算
        auto async = std::thread(md5_work_function);
        async.detach();
      });
}

inline rxcpp::observable<std::string> create(const std::string &file_path,
                                             const int64_t range_left,
                                             const int64_t range_right) {
  return create(file_path, range_left, range_right,
                details::DefaultMd5ProcessCallback,
                details::DefaultCheckStopCallback);
}
inline rxcpp::observable<std::string> create(const std::string &file_path) {
  return create(file_path, -1, -1, details::DefaultMd5ProcessCallback,
                details::DefaultCheckStopCallback);
}
inline rxcpp::observable<std::string> create(
    const std::string &file_path, const int64_t range_left,
    const int64_t range_right, md5::CheckStopCallback check_stop) {
  return create(file_path, range_left, range_right,
                details::DefaultMd5ProcessCallback, check_stop);
}
inline rxcpp::observable<std::string> create(
    const std::string &file_path, md5::CheckStopCallback check_stop) {
  return create(file_path, -1, -1, details::DefaultMd5ProcessCallback,
                check_stop);
}
}  // namespace rx_md5

}  // namespace rx_assistant
#endif  /// RX_MD5_H
