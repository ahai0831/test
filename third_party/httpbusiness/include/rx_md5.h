#pragma once
#ifndef RX_MD5_H
#define RX_MD5_H
//#ifdef _MSC_VER
//#pragma warning(disable : 4503)
//#endif
//#include <functional>
//#include <memory>
//
//#include <Assistant_v2.h>
//#include <rxcpp/rx.hpp>
#include <md5/md5.h>
#include <v2/thread.h>
#include <v2/tools.h>
#include <windows.h>

#include "rx_assistant.h"
#include "speed_counter.h"
namespace rx_assistant {
namespace md5 {
// httpbusiness::progress_notifier notify;

// struct cal_md5_factory{};

/// 计算MD5需要：
/// 同步计算MD5，阻塞式计算，无需进度回显
// 	std::string md5_sync(const std::string& file_path)
// 	{
// 		std::string res;
// 			/// 注册通知器
// 			//httpbusiness::progress_notifier notify;
// 		auto filepath_w = assistant::tools::utf8ToWstring(file_path);
// 		auto handle = CreateFileW(filepath_w.c_str(), GENERIC_READ,
// 			FILE_SHARE_READ, NULL, OPEN_EXISTING,
// 			FILE_FLAG_SEQUENTIAL_SCAN, NULL);
// 		if (INVALID_HANDLE_VALUE != handle)
// 		{
// 			LARGE_INTEGER file_size = { 0 };
// 			GetFileSizeEx(handle, &file_size);
// 			curl_transfercallback::ReadByMmap mmap(file_path.c_str(),
// file_size.QuadPart, 0, file_size.QuadPart); 			cloud_base::MD5 _md5; 			const
// uint64_t caled_size = mmap.granularity << 12; 			notify.total_number =
// file_size.QuadPart;
// 			//std::string res;
// 			notify.RegSubscription([](float v){printf("%.2f\n", v * 100);
// }, [&res](){printf("Fin!MD5:%s\n", res.c_str()); });
// 			/// 可以改成传lambda去计算
// 			while (mmap.done < mmap.length)
// 			{
// 				//_md5.update(mmap.fileview + mmap.viewoffset,
// 0);
// 				//auto ccc = [&_md5](void*data, uint64_t
// len){_md5.update((unsigned char *)data, len); };
// 				mmap.AccessDelegate(caled_size, [&_md5/*,
// &notify*/](void*data, uint64_t len){_md5.update((unsigned char *)data, len);
// notify.finished_number += len; });
// 				///// 一次性读取的不应太长
// 				//uint32_t input_length =
// static_cast<uint32_t>(caled_size + mmap.done < mmap.length ? caled_size :
// mmap.length - mmap.done);
// 				///// 一次性读16M
// 				//mmap.Reform(input_length);
// 				//_md5.update(mmap.fileview + mmap.viewoffset,
// input_length);
// 				//mmap.viewoffset += input_length;
// 				//mmap.done += input_length;
// 				//printf("%.2f\n", (float)(100.F * mmap.done /
// mmap.length));
// 			}
// 			_md5.finalize();
// 			res = _md5.hex_string();
// 			notify.finished_flag = true;
// 			CloseHandle(handle);
// 			handle = INVALID_HANDLE_VALUE;
// 		}
// 		return res;
// 	}

std::string md5_sync_calculate(const std::string &file_path) {
  std::string result;
  auto filepath_w = assistant::tools::utf8ToWstring(file_path);
  auto handle =
      CreateFileW(filepath_w.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                  OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (INVALID_HANDLE_VALUE != handle) {
    LARGE_INTEGER file_size = {0};
    GetFileSizeEx(handle, &file_size);
    curl_transfercallback::ReadByMmap mmap(
        file_path.c_str(), file_size.QuadPart, 0, file_size.QuadPart);
    cloud_base::MD5 _md5;
    const uint64_t caled_size = mmap.granularity << 12;
    /// 改成传lambda去计算
    while (mmap.done < mmap.length) {
      mmap.AccessDelegate(caled_size, [&_md5](void *data, uint64_t len) {
        _md5.update((unsigned char *)data, len);
      });
    }
    _md5.finalize();
    result = _md5.hex_string();
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
    struct a1_1_uv thread_model;
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

/// MD5计算队列，返回结果，支持回显进度

}  // namespace md5

}  // namespace rx_assistant
#endif  /// RX_MD5_H
