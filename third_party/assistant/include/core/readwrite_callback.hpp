#pragma once
#ifndef CORE_READWRITE_CALLBACK_H__
#define CORE_READWRITE_CALLBACK_H__
#include <curl/curl.h>

#include "http_primitives.h"
#include "tools/string_convert.hpp"
namespace assistant {
namespace core {
namespace readwrite {

namespace details {
/// Take care: _Filename should be utf-8 in WIN32
inline FILE *fopen(const char *_Filename, const char *_Mode) {
#ifdef WIN32
  FILE *file = nullptr;
  do {
    auto w_filepath = assistant::tools::string::utf8ToWstring(_Filename);
    if (w_filepath.empty()) {
      break;
    }
    auto w_mode = assistant::tools::string::utf8ToWstring(_Mode);
    w_mode.append(L"S");
    file = _wfopen(w_filepath.c_str(), w_mode.c_str());
  } while (false);
  return file;
#else
  return std::fopen(_Filename, _Mode);
#endif
}

inline int fseek(FILE *_File, int64_t _Offset, int _Origin) {
#ifdef WIN32
  return _fseeki64(_File, _Offset, _Origin);
#else
  return fseeko(_File, _Offset, _Origin);
#endif
}
}  // namespace details

static size_t headerCallback(char *ptr, size_t size, size_t nmemb, void *data) {
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
static size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *data) {
  auto total_size = size * nmemb;
  auto &response = *static_cast<assistant::HttpResponse *>(data);
  // pre reserve the memory
  auto offset = response.body.size();
  response.body.reserve(total_size + offset);
  response.body.append((char *)ptr, total_size);
  return total_size;
}
static int progress_callback(void *clientp, curl_off_t dltotal,
                             curl_off_t dlnow, curl_off_t ultotal,
                             curl_off_t ulnow) {
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
    //     auto w_filepath = assistant::tools::string::utf8ToWstring(filepath);
    //     if (!w_filepath.empty()) {
    //       if (destroy) {
    //         /// 即使存在旧文件也销毁；以二进制方式打开；
    //         /// 指定为顺序访问而优化磁盘缓存
    //         file = _wfopen(w_filepath.c_str(), L"wbS");
    //       } else {
    //         /// 附加到旧文件；以二进制方式打开；
    //         /// 指定为顺序访问而优化磁盘缓存
    //         file = _wfopen(w_filepath.c_str(), L"abS");
    //       }
    //     }
    if (destroy) {
      file = details::fopen(filepath, "wb");
    } else {
      file = details::fopen(filepath, "ab");
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
      //       auto w_filepath =
      //       assistant::tools::string::utf8ToWstring(filepath); if
      //       (w_filepath.empty()) {
      //         break;
      //       }
      /// 初始化file句柄
      /// 要求文件必须已经存在；以二进制方式打开；
      /// 指定为顺序访问而优化磁盘缓存
      //       file = _wfopen(w_filepath.c_str(), L"r+bS");
      file = details::fopen(filepath, "r+b");
    } while (false);
    /// 要求可成功移动file文件句柄到begin指向的位置
    if (!Valid() || 0 != details::fseek(file, begin_offset, SEEK_SET)) {
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
    if (static_cast<uint64_t>(cb.done) + total_size <=
        static_cast<uint64_t>(cb.length)) {
      write_size = fwrite(ptr, size, nmemb, cb.file);
      cb.done += total_size;
    } else if (cb.done < cb.length) {
      /// 这种情况下将发生截断；
      /// 都已经发生内存块的截断了，这里即使发生数值的截断，也不会有更坏的结果了
      write_size = fwrite(ptr, (1 << 0),
                          static_cast<size_t>(cb.length - cb.done), cb.file);
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

struct ReadwriteByMmap : public assistant::HttpResponse::CallbackBase {
#ifdef WIN32
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
  const int32_t power_of_multiple;
  PBYTE fileview;
  /// 极端情况下，viewlength即filesize，所以别想了，直接兼容ULL吧
  uint64_t viewlength;
  /// 将一片view视作一块可读内存，viewoffset即已读内容的长度
  uint64_t viewoffset;

 private:
  /// 在length这个值对应文件实体中，已写内容的长度，兼容ULL
  uint64_t done;
  /// 在构造函数中初始化以下值，供win内核句柄使用
  const DWORD dwDesiredAccess_file;
  const DWORD dwShareMode;
  const DWORD dwCreationDisposition;
  const DWORD flProtect;
  const DWORD dwDesiredAccess_map;

 protected:
 public:
  enum RW{
      none = 0x0000,
      need_read = 0x0001,
      shared_read = 0x0002,
      need_write = 0x0004,
      shared_write = 0x0008,
      /// 仅当文件已存在，才打开
      open_exists = 0x0010,
      /// 文件存在则打开，文件不存在则创建
      open_always = 0x0020,
  };
  virtual ~ReadwriteByMmap() {
    if (Valid()) {
      Destroy();
    }
  };
  ReadwriteByMmap(const char *filepath, int64_t size, int64_t begin,
                  int64_t len, RW rw)
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
        power_of_multiple(8),
        dwDesiredAccess_file((rw & need_read ? GENERIC_READ : NULL) |
                             (rw & need_write ? GENERIC_WRITE : NULL)),
        dwShareMode((rw & shared_read ? FILE_SHARE_READ : NULL) |
                    (rw & shared_write ? FILE_SHARE_WRITE : NULL)),
        dwCreationDisposition(
            (rw & open_always ? OPEN_ALWAYS
                              : (rw & open_exists ? OPEN_EXISTING : NULL))),
        flProtect(rw & need_write ? PAGE_READWRITE : PAGE_READONLY),
        dwDesiredAccess_map(rw & need_write ? FILE_MAP_ALL_ACCESS
                                            : FILE_MAP_READ) {
    do {
      /// 校验输入的参数
      /// 内存映射打开文件视图长度应大于0
      /// 需要满足：0<=begin_offset<filesize, 且0<length<filesize-begin_offset+1
      if (begin_offset < 0 || begin_offset >= filesize || length <= 0 ||
          begin_offset + length - 1 >= filesize) {
        break;
      }
      auto w_filepath = assistant::tools::string::utf8ToWstring(filepath);
      if (w_filepath.empty()) {
        break;
      }
      /// 初始化文件内核对象句柄
      /// 仅可读，不可写；仅共享读，禁止共享写；不存在则报错，存在依然成功；
      /// 指定为顺序访问而优化磁盘缓存
      file = CreateFileW(w_filepath.c_str(), dwDesiredAccess_file, dwShareMode,
                         NULL, dwCreationDisposition, FILE_FLAG_SEQUENTIAL_SCAN,
                         NULL);
      if (INVALID_HANDLE_VALUE == file) {
        break;
      }
      /// 初始化文件映射内核对象句柄
      filemapping = CreateFileMappingW(
          file, NULL, flProtect, static_cast<DWORD>(filesize >> 32),
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
  /// 输入：本次需要读取的字节数（需要提前与 length比较，截断越界的字节数）
  /// 输出：可供读取的字节数，成功的情况（含无需重整）下应大于或等于输入值
  /// 失败的情况下返回0
  uint64_t Reform(uint64_t needed_length) {
    uint64_t flag = 0ULL;
    do {
      /// 检查：viewoffset+needed_length<=viewlength
      /// 满足条件则认为，可从已经分配的页面中直接取出来。前提条件：viewlength大于0；
      if (viewoffset + needed_length <= viewlength && viewlength > 0) {
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
      SIZE_T view_length_1 =
          static_cast<decltype(view_length_1)>(length - done + view_align);
      if (0 == length - done + view_align - view_length_1) {
        /// 保证view_length_1未被截断
        fileview = (PBYTE)MapViewOfFile(
            filemapping, dwDesiredAccess_map,
            static_cast<DWORD>(view_begin >> 32),
            static_cast<DWORD>(view_begin & 0xFFFFFFFF), view_length_1);
        if (nullptr != fileview) {
          viewlength = view_length_1;
          viewoffset = view_align;
          /// 成功，第一次分配
          flag = viewlength - viewoffset;
          break;
        }
      }
      /// 第二次尝试，打开足够写(64K*2^8)的view
      /// 前提条件，比第一次的范围小，但大于或等于needed_length
      /// 范围(align+):[begin_offset+done, begin_offset+done+(64K*2^8))
      auto length_suggest = granularity << power_of_multiple;
      /// x64下SIZE_T即为ULL，x86下SIZE_T虽为UL，但ULL对此无意义，截断无影响
      SIZE_T view_length_2 =
          static_cast<decltype(view_length_2)>(length_suggest + view_align);
      if (view_length_2 < view_length_1 && length_suggest >= needed_length) {
        fileview = (PBYTE)MapViewOfFile(
            filemapping, dwDesiredAccess_map,
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
      SIZE_T view_length_3 =
          static_cast<decltype(view_length_3)>(needed_length + view_align);
      if (needed_length > 0) {
        fileview = (PBYTE)MapViewOfFile(
            filemapping, dwDesiredAccess_map,
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
    /// 对于用于“写入”的内存映射，存在这样一种情况：
    /// 独占式打开一个文件，但"预分配"的物理空间未用尽
    /// 这种情况下需要对磁盘文件进行截断，以避免浪费空间
    if (INVALID_HANDLE_VALUE != file &&
        (dwDesiredAccess_file & GENERIC_WRITE) &&
        !(dwShareMode & FILE_SHARE_WRITE) && filesize == length &&
        0 == begin_offset && done < length) {
      LARGE_INTEGER final_filesize = {0};
      final_filesize.QuadPart = done;
      if (0 != SetFilePointerEx(file, final_filesize, nullptr, FILE_BEGIN)) {
        /// 可选流程：失败不应有副作用
        SetEndOfFile(file);
      }
    }
    if (INVALID_HANDLE_VALUE != file) {
      CloseHandle(file);
      file = INVALID_HANDLE_VALUE;
    }
  }
  static size_t WriteCallback(void *ptr, size_t size, size_t nmemb,
                              void *data) {
    auto &response = *static_cast<assistant::HttpResponse *>(data);
    auto &cb = static_cast<ReadwriteByMmap &>(*response.transfer_callback);
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
  static size_t ReadCallback(void *ptr, size_t size, size_t nmemb, void *data) {
    auto &response = *static_cast<assistant::HttpResponse *>(data);
    auto &cb = static_cast<ReadwriteByMmap &>(*response.transfer_callback);
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
        memcpy(ptr, cb.fileview + cb.viewoffset, needed_length);
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
  /// 为了避免读写内存时的的异常（页面错误），callback内应加上结构化异常处理
  virtual void AccessDelegate(const uint64_t need_data_length,
                              void (*callback)(void *, uint64_t, void *),
                              void *callback_data) final {
    auto total_size = need_data_length;
    /// 极限情况下，needed_length即write_size，理论上需兼容为ULL
    /// 但x86下，ULL对此值无意义，不会溢出，直接截断无影响
    /// size_t write_size = 0;
    /// cb.length - cb.done，有这样一处用法，故理论上需兼容为ULL
    /// 但x86下，ULL对此值无意义，不会溢出，直接截断无影响
    uint64_t needed_length = 0;
    if (this->done + total_size <= this->length) {
      needed_length = total_size;
    } else if (this->done < this->length) {
      /// 这种情况下将发生截断；
      needed_length =
          static_cast<decltype(needed_length)>(this->length - this->done);
    }
    auto result = this->Reform(needed_length);
    if (result >= needed_length) {
      /// 为了避免读写内存时的的异常（页面错误），应加上结构化异常处理
      __try {
        callback(this->fileview + this->viewoffset, needed_length,
                 callback_data);
        this->viewoffset += needed_length;
      } __except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR
                      ? EXCEPTION_EXECUTE_HANDLER
                      : EXCEPTION_CONTINUE_SEARCH) {
      }
      this->done += needed_length;
    }
    // return cb.Returnvalue(write_size);
  }
  /// TODO: 加上这样一个重载:
  /// 将会返回访问的字节数（最好统一转发此函数，以避免逻辑不统一）
  //   ///
  //   为了避免读写内存时的的异常（页面错误），callback内应加上结构化异常处理
  //   virtual uint64_t AccessDelegate(const uint64_t need_data_length,
  // 	  uint64_t(*callback)(void *, uint64_t, void *),
  // 	  void *callback_data) final
#else
  enum RW {
    none = 0x0000,
    need_read = 0x0001,
    shared_read = 0x0002,
    need_write = 0x0004,
    shared_write = 0x0008,
    /// 仅当文件已存在，才打开
    open_exists = 0x0010,
    /// 文件存在则打开，文件不存在则创建
    open_always = 0x0020,
  };
  ReadwriteByMmap(const char *filepath, int64_t size, int64_t begin,
                  int64_t len, RW rw) {}
  virtual ~ReadwriteByMmap() {
    if (Valid()) {
      Destroy();
    }
  };
  uint64_t Reform(uint64_t needed_length) { return 0; }
  virtual bool Valid() const override final { return false; }
  static size_t WriteCallback(void *ptr, size_t size, size_t nmemb,
                              void *data) {
    return 0;
  }
  static size_t ReadCallback(void *ptr, size_t size, size_t nmemb, void *data) {
    return 0;
  }
  virtual void AccessDelegate(const uint64_t need_data_length,
                              void (*callback)(void *, uint64_t, void *),
                              void *callback_data) final {}
#endif
 private:
  ReadwriteByMmap() = delete;
  ReadwriteByMmap(ReadwriteByMmap const &) = delete;
  ReadwriteByMmap &operator=(ReadwriteByMmap const &) = delete;
  ReadwriteByMmap(ReadwriteByMmap &&) = delete;
};

}  // namespace readwrite
}  // namespace core
}  // namespace assistant

#endif  // CORE_READWRITE_CALLBACK_H__
