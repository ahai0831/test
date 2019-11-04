﻿#pragma once
#ifndef RX_MD5_H
#define RX_MD5_H

#include <HashAlgorithm/md5.h>
#include <core/readwrite_callback.hpp>
#include <core/uv_thread.hpp>
#include <tools/string_convert.hpp>

#include <windows.h>

// #include "rx_assistant.h"
// #include "speed_counter.h"
namespace rx_assistant {
namespace md5 {

typedef std::function<void(void *, uint64_t)> mmap_md5_callback;
static void mmap_md5_access_callback(void *memory, uint64_t len,
                                     void *callback_data) {
  auto &callback = *(mmap_md5_callback *)callback_data;
  callback(memory, len);
}
std::string md5_sync_calculate(const std::string &file_path) {
  std::string result;
  auto filepath_w = assistant::tools::string::utf8ToWstring(file_path);
  //// TODO: 需要改成获取文件元数据
  auto handle = CreateFileW(filepath_w.c_str(), 0, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (INVALID_HANDLE_VALUE != handle) {
    LARGE_INTEGER file_size = {0};
    GetFileSizeEx(handle, &file_size);
    auto readonly_mmap =
        std::make_unique<assistant::core::readwrite::ReadwriteByMmap>(
            file_path.c_str(), file_size.QuadPart, 0, file_size.QuadPart,
            static_cast<assistant::core::readwrite::ReadwriteByMmap::RW>(
                assistant::core::readwrite::ReadwriteByMmap::need_read |
                assistant::core::readwrite::ReadwriteByMmap::open_exists |
                assistant::core::readwrite::ReadwriteByMmap::shared_read));
    cloud_base::hash_algorithm::MD5 _md5;
    const uint64_t caled_size = readonly_mmap->granularity
                                << readonly_mmap->power_of_multiple;
    /// 记录已计算的mmap长度
    uint64_t calculate_done = 0;
    /// 初始化内存映射读取MD5所需回调
    mmap_md5_callback read_function = [&_md5, &calculate_done](void *data,
                                                               uint64_t len) {
      /// 由于业务流程，不会一次性传入超过uint32_t最大值的块长度，进行截断无风险
      _md5.update(static_cast<unsigned char *>(data),
                  static_cast<uint32_t>(len));
      calculate_done += static_cast<uint32_t>(len);
    };
    /// 改成传lambda去计算
    while (calculate_done < readonly_mmap->length) {
      readonly_mmap->AccessDelegate(caled_size, mmap_md5_access_callback,
                                    &read_function);
    }
    _md5.finalize();
    result = _md5.hex_string();
    readonly_mmap->Destroy();
    CloseHandle(handle);
    handle = INVALID_HANDLE_VALUE;
  }
  return result;
}

/// 异步计算MD5，仅需要结果，无需进度回显
struct md5_async_factory {
 private:
  struct md5_async {
    typedef std::function<void(const std::string &)> Callback;

   private:
    struct assistant::core::libuv_threadmodel thread_model;
    std::string _path;
    Callback _cb;

   public:
    md5_async(const std::string &filepath, Callback callback)
        : _path(filepath), _cb(callback) {}
    ~md5_async() { thread_model.ThreadJoin(); }
    void async_calculate() {
      thread_model.ThreadCreate(async_calculate_work, (void *)this);
    }

   private:
    static void async_calculate_work(void *data) {
      if (nullptr != data) {
        md5_async &async = *static_cast<md5_async *>(data);
        auto result = md5_sync_calculate(async._path);
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
    auto u = std::make_unique<md5_async>(filepath, callback);
    u->async_calculate();
    return u;
  }
};

/// TODO: MD5计算队列，返回结果，支持回显进度

}  // namespace md5

}  // namespace rx_assistant
#endif  /// RX_MD5_H
