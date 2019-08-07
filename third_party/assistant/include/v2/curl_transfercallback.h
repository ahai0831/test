#pragma once
#ifndef V2_CURL_TRANSFERCALLBACK_H__
#define V2_CURL_TRANSFERCALLBACK_H__
#include <curl/curl.h>

#include "http_primitives.h"
#include "v2/tools.h"

namespace curl_transfercallback {
size_t headerCallback(char *ptr, size_t size, size_t nmemb, void *data) {
  auto total_size = size * nmemb;
  auto &response = *static_cast<assistant::HttpResponse *>(data);
  std::string line(ptr);
  auto iter = line.find(": ");
  if (iter != std::string::npos) {
    auto iter2 = line.find("\r\n", iter + 2);
    if (iter2 != std::string::npos) {
      response.headers.Set(line.substr(0, iter),
                           line.substr(iter + 2, iter2 - iter - 2));
    }
  }
  return total_size;
}
size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *data) {
  auto total_size = size * nmemb;
  auto &response = *static_cast<assistant::HttpResponse *>(data);
  // pre reserve the memory
  auto offset = response.body.size();
  response.body.reserve(total_size + offset);
  response.body.append((char *)ptr, total_size);
  return total_size;
}
int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                      curl_off_t ultotal, curl_off_t ulnow) {
  auto &response = *static_cast<assistant::HttpResponse *>(clientp);
  return response.stop_flag ? -1 : 0;
}
struct Headeronly {
  static size_t writeCallback(void *, size_t size, size_t nmemb, void *) {
    /// Do nothing!
    return size * nmemb;
  }
  static int progress_callback(void *, curl_off_t dltotal, curl_off_t dlnow,
                               curl_off_t, curl_off_t) {
    if (dltotal > 0 && dlnow < dltotal) {
      /// Once headers finished, break the transport.
      return -1;
    }
    return 0;
  }
};
struct WriteByAppend : public assistant::HttpResponse::CallbackBase {
 public:
  bool count_size;
  FILE *file;

 public:
  /// 避免Create，直接构造。避免默认构造、复制构造、隐式构造、=操作符
  WriteByAppend(const char *filepath, bool destroy)
      : count_size(false), file(nullptr) {
    /// 初始化file句柄
    auto w_filepath = assistant::tools::utf8ToWstring(filepath);
    if (!w_filepath.empty()) {
      if (destroy) {
        /// 即使存在旧文件也销毁；以二进制方式打开；
        /// 指定为顺序访问而优化磁盘缓存
        file = _wfopen(w_filepath.c_str(), L"wbS");
      } else {
        /// 附加到旧文件；以二进制方式打开；
        /// 指定为顺序访问而优化磁盘缓存
        file = _wfopen(w_filepath.c_str(), L"abS");
      }
    }
    /// 要求目标文件必须已存在，若Valid为false，则必须Destroy一次
    /// 避免部分资源泄露
    if (!Valid()) {
      Destroy();
    }
  }
  virtual bool Valid() const override final { return nullptr != file; }
  virtual void Destroy() override final {
    if (nullptr != file) {
      fclose(file);
      file = nullptr;
    }
  }
  static size_t Callback(void *ptr, size_t size, size_t nmemb, void *data) {
    auto &response = *static_cast<assistant::HttpResponse *>(data);
    auto &cb = static_cast<WriteByAppend &>(*response.transfer_callback);
	return cb.Returnvalue(fwrite(ptr, size, nmemb, cb.file));
  }

 private:
  WriteByAppend() = delete;
  WriteByAppend(WriteByAppend const &) = delete;
  WriteByAppend &operator=(WriteByAppend const &) = delete;
  WriteByAppend(WriteByAppend &&) = delete;
};

struct WriteByFile : public assistant::HttpResponse::CallbackBase {
 public:
  bool count_size;
  const int64_t filesize;
  const int64_t begin_offset;
  const int64_t length;
  FILE *file;

 private:
  int64_t done;

 public:
  WriteByFile(const char *filepath, int64_t size, int64_t begin, int64_t len)
      : count_size(false),
        filesize(size),
        begin_offset(begin),
        length(len),
        file(nullptr),
        done(0) {
    do {
      auto w_filepath = assistant::tools::utf8ToWstring(filepath);
      if (w_filepath.empty()) {
        break;
      }
      /// 初始化file句柄
      /// 要求文件必须已经存在；以二进制方式打开；
      /// 指定为顺序访问而优化磁盘缓存
      file = _wfopen(w_filepath.c_str(), L"r+bS");
    } while (false);
    /// 要求可成功移动file文件句柄到begin指向的位置
    if (!Valid() || 0 != _fseeki64(file, begin_offset, SEEK_SET)) {
      Destroy();
    }
  }
  virtual bool Valid() const override final {
    /// 要求目标文件必须已存在
    /// begin_offset小于filesize，且begin_offset+length-1小于filesize
    return (nullptr != file) && (begin_offset < filesize) &&
           (begin_offset + length - 1 >= filesize);
  }
  virtual void Destroy() override final {
    if (nullptr != file) {
      fclose(file);
      file = nullptr;
    }
  }
  static size_t Callback(void *ptr, size_t size, size_t nmemb, void *data) {
    auto &response = *static_cast<assistant::HttpResponse *>(data);
    auto &cb = static_cast<WriteByFile &>(*response.transfer_callback);
    auto total_size = size * nmemb;
    size_t write_size = 0;
	if (static_cast<uint64_t>(cb.done) + total_size <= static_cast<uint64_t>(cb.length)) {
      write_size = fwrite(ptr, size, nmemb, cb.file);
      cb.done += total_size;
    } else if (cb.done < cb.length) {
      /// 这种情况下将发生截断；
      /// 都已经发生内存块的截断了，这里即使发生数值的截断，也不会有更坏的结果了
      write_size = fwrite(ptr, (1 << 0), static_cast<size_t>(cb.length - cb.done), cb.file);
      cb.done += write_size;
    }
    return cb.Returnvalue(write_size);
  }

 private:
  WriteByFile() = delete;
  WriteByFile(WriteByFile const &) = delete;
  WriteByFile &operator=(WriteByFile const &) = delete;
  WriteByFile(WriteByFile &&) = delete;
};

struct WriteByMmap : public assistant::HttpResponse::CallbackBase {
 public:
  bool count_size;
  /// x64下DWORD对应的类型是UL，应拆出两个DWORD，此处需要兼容
  const uint64_t filesize;
  const uint64_t begin_offset;
  /// x64下SIZE_T对应的类型是ULL，此处需要兼容
  const uint64_t length;
  /// Mmap needed in Win32
  HANDLE file;
  HANDLE filemapping;
  DWORD granularity;
  PBYTE fileview;
  /// 极端情况下，viewlength即filesize，所以别想了，直接兼容ULL吧
  uint64_t viewlength;
  /// 将一片view视作一块可写内存，viewoffset即已写内容的长度
  uint64_t viewoffset;

 private:
  /// 在length这个值对应文件实体中，已写内容的长度，兼容ULL
  uint64_t done;
  const int32_t power_of_multiple;

 protected:
 public:
  WriteByMmap(const char *filepath, int64_t size, int64_t begin, int64_t len)
      : count_size(false),
        filesize(size),
        begin_offset(begin),
        length(len),
        file(INVALID_HANDLE_VALUE),
        filemapping(NULL),
        granularity(65536UL),
        fileview(nullptr),
        viewlength(0),
        viewoffset(0),
        done(0),
        power_of_multiple(8) {
    do {
      /// 校验输入的参数
      /// 内存映射打开文件视图长度应大于0
      /// 需要满足：0<=begin_offset<filesize, 且0<length<filesize-begin_offset+1
      if (begin_offset < 0 || begin_offset >= filesize || length <= 0 ||
          begin_offset + length - 1 >= filesize) {
        break;
      }
      auto w_filepath = assistant::tools::utf8ToWstring(filepath);
      if (w_filepath.empty()) {
        break;
      }
      /// 初始化文件内核对象句柄
      /// 可读可写，共享读写；不存在则创建，存在依然成功；
      /// 指定为顺序访问而优化磁盘缓存
      file = CreateFileW(w_filepath.c_str(), GENERIC_WRITE | GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                         FILE_FLAG_SEQUENTIAL_SCAN, NULL);
      if (INVALID_HANDLE_VALUE == file) {
        break;
      }
      /// 初始化文件映射内核对象句柄
      filemapping = CreateFileMappingW(
          file, NULL, PAGE_READWRITE, static_cast<DWORD>(filesize >> 32),
          static_cast<DWORD>(filesize & 0xFFFFFFFF), NULL);
      if (NULL == filemapping) {
        break;
      }
      /// 获取文件分配颗粒大小
      _SYSTEM_INFO info = {0};
      GetSystemInfo(&info);
      if (0 < info.dwAllocationGranularity) {
        granularity = info.dwAllocationGranularity;
      }
      /// 尝试初始化文件（传0意味着暂时无需求，如果第一、第二级尝试都失败，直接失败）
      auto result = Reform(0);
    } while (false);
    /// 要求尝试关联文件视图成功
    if (!Valid()) {
      Destroy();
    }
  }
  /// 输入：本次需要写入的字节数（需要提前与 length比较，截断越界的字节数）
  /// 输出：可供写入的字节数，成功的情况（含无需重整）下应大于或等于输入值
  /// 失败的情况下返回0
  uint64_t Reform(uint64_t needed_length) {
    uint64_t flag = 0ULL;
    do {
      /// 检查：viewoffset+needed_length<=viewlength
      if (viewoffset + needed_length > viewlength) {
        /// 成功：无需重分配
        flag = viewlength - viewoffset;
        break;
      }
      if (INVALID_HANDLE_VALUE == file) {
        break;
      }
      if (NULL == filemapping) {
        break;
      }
      if (0 >= granularity) {
        break;
      }
      /// 尝试前，若有已经打开的文件视图，需清理
      if (nullptr != fileview) {
        UnmapViewOfFile(fileview);
        fileview = nullptr;
        viewlength = 0;
        viewoffset = 0;
      }
      /// 起始偏移，需要向前对齐字节数
      auto view_align = (begin_offset + done) % granularity;
      /// 起始偏移 = (begin_offset + done) - 向下取整align个字节
      auto view_begin = begin_offset + done - view_align;
      /// 第一次尝试，打开足够写(length-done)的view
      /// 范围(align+):[begin_offset+done, begin_offset+length)
	  /// x64下SIZE_T即为ULL，x86下SIZE_T虽为UL，但ULL对此无意义，截断无影响
	  SIZE_T view_length_1 = static_cast<decltype(view_length_1)>(length - done + view_align);
      fileview = (PBYTE)MapViewOfFile(
          filemapping, FILE_MAP_ALL_ACCESS,
          static_cast<DWORD>(view_begin >> 32),
          static_cast<DWORD>(view_begin & 0xFFFFFFFF), view_length_1);
      if (nullptr != fileview) {
        viewlength = view_length_1;
        viewoffset = view_align;
        /// 成功，第一次分配
        flag = viewlength - viewoffset;
        break;
      }
      /// 第二次尝试，打开足够写(64K*2^8)的view
      /// 前提条件，比第一次的范围小，但大于或等于needed_length
      /// 范围(align+):[begin_offset+done, begin_offset+done+(64K*2^8))
      auto length_suggest = granularity << power_of_multiple;
	  /// x64下SIZE_T即为ULL，x86下SIZE_T虽为UL，但ULL对此无意义，截断无影响
	  SIZE_T view_length_2 = static_cast<decltype(view_length_2)>(length_suggest + view_align);
      if (view_length_2 < view_length_1 && length_suggest >= needed_length) {
        fileview = (PBYTE)MapViewOfFile(
            filemapping, FILE_MAP_ALL_ACCESS,
            static_cast<DWORD>(view_begin >> 32),
            static_cast<DWORD>(view_begin & 0xFFFFFFFF), view_length_2);
      }
      if (nullptr != fileview) {
        viewlength = view_length_2;
        viewoffset = view_align;
        /// 成功，第二次分配
        flag = viewlength - viewoffset;
        break;
      }
      /// 第三次尝试：打开足够写needed_length的view
      /// 前提条件，needed_length>0
      /// 范围(align+):[begin_offset+done, begin_offset+done+needed_length)
	  /// x64下SIZE_T即为ULL，x86下SIZE_T虽为UL，但ULL对此无意义，截断无影响
	  SIZE_T view_length_3 = static_cast<decltype(view_length_3)>(needed_length + view_align);
      if (needed_length > 0) {
        fileview = (PBYTE)MapViewOfFile(
            filemapping, FILE_MAP_ALL_ACCESS,
            static_cast<DWORD>(view_begin >> 32),
			static_cast<DWORD>(view_begin & 0xFFFFFFFF), view_length_3);
      }
      if (nullptr != fileview) {
        viewlength = view_length_3;
        viewoffset = view_align;
        /// 成功，第二次分配
        flag = viewlength - viewoffset;
        break;
      }
    } while (false);
    return flag;
  }
  virtual bool Valid() const override final { return nullptr != fileview; }
  virtual void Destroy() override final {
    if (nullptr != fileview) {
      UnmapViewOfFile(fileview);
      fileview = nullptr;
      viewlength = 0;
      viewoffset = 0;
    }
    if (NULL != filemapping) {
      CloseHandle(filemapping);
      filemapping = NULL;
    }
    if (INVALID_HANDLE_VALUE != file) {
      CloseHandle(file);
      file = INVALID_HANDLE_VALUE;
    }
  }
  static size_t Callback(void *ptr, size_t size, size_t nmemb, void *data) {
    auto &response = *static_cast<assistant::HttpResponse *>(data);
    auto &cb = static_cast<WriteByMmap &>(*response.transfer_callback);
    auto total_size = size * nmemb;
	/// 极限情况下，needed_length即write_size，理论上需兼容为ULL
	/// 但x86下，ULL对此值无意义，不会溢出，直接截断无影响
	size_t write_size = 0;
	/// cb.length - cb.done，有这样一处用法，故理论上需兼容为ULL
	/// 但x86下，ULL对此值无意义，不会溢出，直接截断无影响
	size_t needed_length = 0;
    if (cb.done + total_size <= cb.length) {
      needed_length = total_size;
    } else if (cb.done < cb.length) {
      /// 这种情况下将发生截断；
		needed_length = static_cast<decltype(needed_length)>(cb.length - cb.done);
    }
    auto result = cb.Reform(needed_length);
    if (result >= needed_length) {
      /// 为了避免读写内存时的的异常（页面错误），应加上结构化异常处理
      __try {
        memcpy(cb.fileview + cb.viewoffset, ptr, needed_length);
        cb.viewoffset += needed_length;
        cb.done += needed_length;
        write_size = needed_length;
      } __except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR
                      ? EXCEPTION_EXECUTE_HANDLER
                      : EXCEPTION_CONTINUE_SEARCH) {
      }
    }
    return cb.Returnvalue(write_size);
  }

 private:
  WriteByMmap() = delete;
  WriteByMmap(WriteByMmap const &) = delete;
  WriteByMmap &operator=(WriteByMmap const &) = delete;
  WriteByMmap(WriteByMmap &&) = delete;
};

}  // namespace curl_transfercallback

#endif  // V2_CURL_TRANSFERCALLBACK_H__
