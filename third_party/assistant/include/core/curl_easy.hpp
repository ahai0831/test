#pragma once
#ifndef _CORE_CURL_EASY_H__
#define _CORE_CURL_EASY_H__
#include <map>
#include <memory>

#include <curl/curl.h>

#include "core/curl_share.hpp"
#include "core/readwrite_callback.hpp"
#include "http_primitives.h"
#include "tools/safequeue.hpp"
#include "tools/scopeguard.hpp"

namespace assistant {
namespace core {
/// Curl简单句柄数据闭包
/// 封装Curl简单句柄操作

struct libcurl_easy_closure {
  enum SpecialOpt {
    None = 0,
    ClearMultiStack,
    RefreshShareHandle,
    LimitDownloadSpeed,
    LimitUploadSpeed,
    Last,
  };

 private:
  CURL *easy_handle;
  curl_slist *slist;

 public:
  CURLcode result;
  /// 特殊的构造函数对此值进行初始化
  const SpecialOpt operation;
  const curl_off_t speedlimit;
  std::weak_ptr<libcurl_share_closure> share_handle;

 private:
  assistant::tools::scope_guard guard;

 public:
  libcurl_easy_closure()
      : easy_handle(nullptr),
        slist(nullptr),
        result(CURLcode(-1)),
        operation(SpecialOpt(0)),
        guard([this]() {
          if (nullptr != easy_handle) {
            curl_easy_cleanup(easy_handle);
            easy_handle = nullptr;
          }
          if (nullptr != slist) {
            curl_slist_free_all(slist);
            slist = nullptr;
          }
        }),
        speedlimit(0) {
    easy_handle = curl_easy_init();
  }
  explicit libcurl_easy_closure(const SpecialOpt opt)
      : easy_handle(nullptr),
        slist(nullptr),
        result(CURLcode(-1)),
        operation(opt),
        guard([this]() {
          if (nullptr != slist) {
            curl_slist_free_all(slist);
            slist = nullptr;
          }
        }),
        speedlimit(0) {}
  explicit libcurl_easy_closure(const SpecialOpt opt, CURL *const limited_easy,
                                const curl_off_t limit)
      : easy_handle(limited_easy),
        slist(nullptr),
        result(CURLcode(-1)),
        operation(opt),
        guard([this]() {
          if (nullptr != slist) {
            curl_slist_free_all(slist);
            slist = nullptr;
          }
        }),
        speedlimit(limit) {}
  decltype(easy_handle) &get_easy() { return easy_handle; }
  decltype(slist) &get_slist() { return slist; }
  void slist_append(const char *str) { slist = curl_slist_append(slist, str); }
  void reset() {
    if (nullptr != easy_handle) {
      curl_easy_reset(easy_handle);
    }
    if (nullptr != slist) {
      curl_slist_free_all(slist);
      slist = nullptr;
    }
    result = CURLcode(0);
    share_handle.reset();
  }
  /// 禁用移动构造、复制构造、=号操作符
  libcurl_easy_closure(libcurl_easy_closure &&) = delete;
  libcurl_easy_closure(const libcurl_easy_closure &) = delete;
  libcurl_easy_closure &operator=(const libcurl_easy_closure &) = delete;
};
/// 重用 libcurl_easy_closure 容器
/// 用于存取 libcurl_easy_closure (Unique_ptr)的闭包
/// 保存即将开始传输的 libcurl_easy_closure 的容器
/// 用于存取 libcurl_easy_closure (Unique_ptr)的闭包
typedef assistant::tools::safequeue_closure<libcurl_easy_closure>
    libcurl_easy_safequeue;

namespace curl_easy {
/// 包裹Curl简单句柄, Request, Response的类，便于传参和调用
struct assistant_request_response {
  std::unique_ptr<assistant::core::libcurl_easy_closure> easy;
  std::unique_ptr<assistant::HttpRequest> request;
  std::unique_ptr<assistant::HttpResponse> response;
  CURL *easy_handle;
  assistant_request_response() = default;
  assistant_request_response(decltype(easy) &easy_closure,
                             decltype(request) &http_request,
                             decltype(response) &http_response)
      : easy(std::move(easy_closure)),
        request(std::move(http_request)),
        response(std::move(http_response)),
        easy_handle(easy->get_easy()) {}
  /// 禁用复制构造和=号操作符
  assistant_request_response(assistant_request_response &&v)
      : easy(std::move(v.easy)),
        request(std::move(v.request)),
        response(std::move(v.response)),
        easy_handle(easy->get_easy()) {}
  /// 禁用复制构造和=号操作符
  assistant_request_response(const assistant_request_response &) = delete;
  assistant_request_response &operator=(const assistant_request_response &) =
      delete;
};
//////////////////////////////////////////////////////////////////////////
/// 此工具方法，根据传入的 HttpRequest 对easy句柄进行配置
/// 并将特定回调对象绑定在 HttpResponse 中。
static void ConfigEasyHandle(const assistant::HttpRequest &request,
                             assistant::HttpResponse &response,
                             libcurl_easy_closure &easy_closure) {
  auto &req = request;
  auto &res = response;
  auto &easy_handle = easy_closure.get_easy();
  /// 处理通用的HTTP选项、扩展
  //////////////////////////////////////////////////////////////////////////
  /// 配置访问的URL
  curl_easy_setopt(easy_handle, CURLOPT_URL, req.url.c_str());
  /// 配置http请求的headers
  for (auto const &x : req.headers) {
    if (!x.first.empty()) {
      /// libcurl: 如果需要header:后为空，则以;终止
      auto &&header =
          x.second.empty() ? x.first + ";" : x.first + ": " + x.second;
      easy_closure.slist_append(header.c_str());
    }
  }
  auto &&slist = easy_closure.get_slist();
  if (nullptr != slist) {
    curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, slist);
  }
  /// config range (Curl would solve it)
  const auto kRange = req.extends.Get("range");
  if (!kRange.empty()) {
    curl_easy_setopt(easy_handle, CURLOPT_RANGE, kRange.c_str());
  }
  /// config proxy
  const auto kProxyStr = req.extends.Get("proxy");
  if (!kProxyStr.empty()) {
    curl_easy_setopt(easy_handle, CURLOPT_PROXY, kProxyStr.c_str());
  }
  /// config share handle
  const auto kShareClosure = easy_closure.share_handle.lock();
  if (nullptr != kShareClosure) {
    curl_easy_setopt(easy_handle, CURLOPT_SHARE, kShareClosure->get_share());
  }
  /// use c-ares
  curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1L);
  /// config some timeout
  /// abort if connect cost more than 10 seconds
  curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT_MS, 10000L);
  /// abort if slower than 1 bytes/sec during 60(default) seconds
  curl_easy_setopt(easy_handle, CURLOPT_LOW_SPEED_LIMIT, 1L);
  const int32_t kTransferTimeout =
      strtol(req.extends.Get("transfer_timeout").c_str(), nullptr, 0);
  curl_easy_setopt(easy_handle, CURLOPT_LOW_SPEED_TIME,
                   0 == kTransferTimeout ? 60L : kTransferTimeout);

  /// By default: Don't verify server's name and certificate
  curl_easy_setopt(easy_handle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(easy_handle, CURLOPT_SSL_VERIFYHOST, 0L);
  /// By default: Don't verify (https)proxy's name and certificate
  curl_easy_setopt(easy_handle, CURLOPT_PROXY_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(easy_handle, CURLOPT_PROXY_SSL_VERIFYHOST, 0L);
  /// Use large buffer
  curl_easy_setopt(easy_handle, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);

  /// 处理libcurl所需回调
  //////////////////////////////////////////////////////////////////////////
  /// 获取各回调使用的内存指针（相应的HttpResponse*）
  void *const &response_callback_data = res.data;
  curl_easy_setopt(easy_handle, CURLOPT_PRIVATE, response_callback_data);
  /// 禁用curl的默认回显型进度回调
  curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION,
                   assistant::core::readwrite::progress_callback);
  curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, response_callback_data);
  /// 处理HTTP响应头回调
  curl_easy_setopt(easy_handle, CURLOPT_HEADERFUNCTION,
                   assistant::core::readwrite::headerCallback);
  curl_easy_setopt(easy_handle, CURLOPT_HEADERDATA, response_callback_data);
  /// 处理HTTP响应载荷回调（对特定GET方法，需定制响应回调时，可能会再次配置）
  curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION,
                   assistant::core::readwrite::writeCallback);
  curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, response_callback_data);
  /// 处理 header_only 扩展
  /// 语义：仅接收此请求的响应的headers，无需保存响应
  const bool kHeaderOnly = req.extends.Get("header_only").compare("true") == 0;
  if (kHeaderOnly) {
    /// Headeronly进度回调，不接受任何非完整数据（可能存在完整数据，无需打断）
    curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION,
                     assistant::core::readwrite::Headeronly::progress_callback);
    curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, response_callback_data);
    /// Headeronly响应载荷回调，不保存任何响应
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION,
                     assistant::core::readwrite::Headeronly::writeCallback);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, response_callback_data);
  }

  /// 处理方法特定的HTTP Method所需的扩展
  //////////////////////////////////////////////////////////////////////////
  enum RequestHttpMethod {
    kUnsupport = -1,
    kHttpGet = 65536,
    kHttpPost,
    kHttpPut,
    kHttpLast,  /// should not be uesd
  };
  auto GetHttpMethod =
      [](const decltype(req.method) &method) -> RequestHttpMethod {
    RequestHttpMethod r = kUnsupport;
    do {
      if (details::stricmp(method.c_str(), "GET") == 0) {
        r = kHttpGet;
        break;
      }
	  if (details::stricmp(method.c_str(), "POST") == 0) {
        r = kHttpPost;
        break;
      }
	  if (details::stricmp(method.c_str(), "PUT") == 0) {
        r = kHttpPut;
        break;
      }
    } while (false);
    return r;
  };
  const auto kHttpMethod = GetHttpMethod(req.method);
  /// 根据特定的HTTP方法，进行不同处理
  switch (kHttpMethod) {
      /// 处理GET类型的请求，根据扩展字段，绑定内存映射、C文件句柄或C文件句柄（追加型）回调
      //////////////////////////////////////////////////////////////////////////
    case kHttpGet:
      do {
        const auto kDownloadFilepath = req.extends.Get("download_filepath");
        if (kDownloadFilepath.empty()) {
          /// 那就不用管了，不是下载到文件
          break;
        }
        const curl_off_t kLimitSpeed =
            strtoll(req.extends.Get("speed_limit").c_str(), nullptr, 0);
        if (kLimitSpeed > 0) {
          curl_easy_setopt(easy_handle, CURLOPT_MAX_RECV_SPEED_LARGE,
                           kLimitSpeed);
        }
        const auto kDownloadFilesize =
            strtoll(req.extends.Get("download_filesize").c_str(), nullptr, 0);
        const auto kDownloadOffset =
            strtoll(req.extends.Get("download_offset").c_str(), nullptr, 0);
        const auto kDownloadLength =
            strtoll(req.extends.Get("download_length").c_str(), nullptr, 0);
        /// 尝试使用内存映射（可读可写，共享读写，不存在则创建）
        {
          const auto kDownloadMmapParam = static_cast<
              assistant::core::readwrite::ReadwriteByMmap::RW>(
              assistant::core::readwrite::ReadwriteByMmap::RW::need_read |
              assistant::core::readwrite::ReadwriteByMmap::RW::need_write |
              assistant::core::readwrite::ReadwriteByMmap::RW::shared_read |
              assistant::core::readwrite::ReadwriteByMmap::RW::shared_write |
              assistant::core::readwrite::ReadwriteByMmap::RW::open_always);
          auto write_cbdata_mmap_ptr =
              std::make_unique<assistant::core::readwrite::ReadwriteByMmap>(
                  kDownloadFilepath.c_str(), kDownloadFilesize, kDownloadOffset,
                  kDownloadLength, kDownloadMmapParam);
          if (nullptr != write_cbdata_mmap_ptr &&
              write_cbdata_mmap_ptr->Valid()) {
            curl_easy_setopt(
                easy_handle, CURLOPT_WRITEFUNCTION,
                assistant::core::readwrite::ReadwriteByMmap::WriteCallback);
            res.transfer_callback = std::move(write_cbdata_mmap_ptr);
            res.transfer_callback->retval_callback = std::move(req.retval_func);
            curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA,
                             response_callback_data);
            /// 使用内存映射，无需再尝试降级
            break;
          }
          /// 尝试使用C文件句柄
          {
            auto writecb_data_ptr =
                std::make_unique<assistant::core::readwrite::WriteByFile>(
                    kDownloadFilepath.c_str(), kDownloadFilesize,
                    kDownloadOffset, kDownloadLength);
            if (nullptr != writecb_data_ptr && writecb_data_ptr->Valid()) {
              curl_easy_setopt(
                  easy_handle, CURLOPT_WRITEFUNCTION,
                  assistant::core::readwrite::WriteByFile::Callback);
              res.transfer_callback = std::move(writecb_data_ptr);
              res.transfer_callback->retval_callback =
                  std::move(req.retval_func);
              curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA,
                               response_callback_data);
              /// 使用C文件句柄，无需再尝试降级
              break;
            }
          }
        }
        /// 尝试使用C文件句柄（追加型）
        {
          const auto kDestroy =
              req.extends.Get("download_append").compare("true") != 0;
          auto writecb_data_ptr =
              std::make_unique<assistant::core::readwrite::WriteByAppend>(
                  kDownloadFilepath.c_str(), kDestroy);
          if (nullptr != writecb_data_ptr && writecb_data_ptr->Valid()) {
            curl_easy_setopt(
                easy_handle, CURLOPT_WRITEFUNCTION,
                assistant::core::readwrite::WriteByAppend::Callback);
            res.transfer_callback = std::move(writecb_data_ptr);
            res.transfer_callback->retval_callback = std::move(req.retval_func);
            curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA,
                             response_callback_data);
            /// 使用C文件句柄（追加），无需再尝试降级
            break;
          }
        }

      } while (false);
      break;  /// case kHttpGet
      /// 处理POST类型的请求，直接从body中读取数据即可
      //////////////////////////////////////////////////////////////////////////
    case kHttpPost:
      do {
        const curl_off_t kPostFieldSize = req.body.size();
        curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE_LARGE,
                         kPostFieldSize);
        curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, req.body.c_str());
      } while (false);
      break;  /// case kHttpPost
              /// 处理PUT类型的请求，优先绑定读取型回调，若无，直接从body中读取数据即可
              //////////////////////////////////////////////////////////////////////////
    case kHttpPut:
      do {
        /// Todo: 增加Chunk上传文件所需的读回调处理
        /// 由于场景，暂时无需支持Chunk传输，故须指定待上传实体的大小（单位:字节）

        /// 指定回调
        const auto kUploadFilepath = req.extends.Get("upload_filepath");
        if (!kUploadFilepath.empty()) {
          /// 设置读回调
          const auto kUploadFilesize =
              strtoll(req.extends.Get("upload_filesize").c_str(), nullptr, 0);
          const auto kUploadOffset =
              strtoll(req.extends.Get("upload_offset").c_str(), nullptr, 0);
          const auto kUploadLength =
              strtoll(req.extends.Get("upload_length").c_str(), nullptr, 0);
 #ifdef _WIN32
          const auto kUploadMmapParam =
              static_cast<assistant::core::readwrite::ReadwriteByMmap::RW>(
                  assistant::core::readwrite::ReadwriteByMmap::RW::need_read |
                  assistant::core::readwrite::ReadwriteByMmap::RW::shared_read |
                  assistant::core::readwrite::ReadwriteByMmap::RW::open_exists);
          auto read_cbdata_mmap_ptr =
              std::make_unique<assistant::core::readwrite::ReadwriteByMmap>(
                  kUploadFilepath.c_str(), kUploadFilesize, kUploadOffset,
                  kUploadLength, kUploadMmapParam);
                  
          if (nullptr != read_cbdata_mmap_ptr &&
              read_cbdata_mmap_ptr->Valid()) {
            curl_easy_setopt(easy_handle, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(
                easy_handle, CURLOPT_READFUNCTION,
                assistant::core::readwrite::ReadwriteByMmap::ReadCallback);
            curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE_LARGE,
                             kUploadLength);
            res.transfer_callback = std::move(read_cbdata_mmap_ptr);
            res.transfer_callback->retval_callback = std::move(req.retval_func);
            curl_easy_setopt(easy_handle, CURLOPT_READDATA,
                             response_callback_data);
          }
#else
            auto read_cbdata_file_ptr =
            std::make_unique<assistant::core::readwrite::ReadByFile>(
                kUploadFilepath.c_str(), kUploadFilesize, kUploadOffset,
                kUploadLength);

        if (nullptr != read_cbdata_file_ptr &&
                read_cbdata_file_ptr->Valid()) {
              curl_easy_setopt(easy_handle, CURLOPT_UPLOAD, 1L);
              curl_easy_setopt(
                  easy_handle, CURLOPT_READFUNCTION,
                  assistant::core::readwrite::ReadByFile::Callback);
              curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE_LARGE,
                               kUploadLength);
              res.transfer_callback = std::move(read_cbdata_file_ptr);
              res.transfer_callback->retval_callback = std::move(req.retval_func);
              curl_easy_setopt(easy_handle, CURLOPT_READDATA,
                               response_callback_data);
            }
#endif

        } else {
          /// 从Body中读取数据
          const curl_off_t kDataLength = req.body.size();
          /// Ref:
          /// https://stackoverflow.com/questions/14010346/should-i-set-curlopt-upload-when-i-post-file-with-curl-in-php
          /// The idea behind CURLOPT_UPLOAD is to tell curl to use PUT method,
          /// add some common file uploading headers for that such as Expect:
          /// 100-continue header and use chunked encoding to upload a file of
          /// unknown size if you are using HTTP/1.1
          curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, req.body.c_str());
          curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE_LARGE,
                           kDataLength);
          curl_easy_setopt(easy_handle, CURLOPT_CUSTOMREQUEST, "PUT");
        }
        /// 默认地，PUT请求，未指定header中此字段的情况下，应为：Content-Type:
        /// application/octet-stream 所以应带上此处理
        /// 2019.11.4 额外处理：
        /// 由于CURLOPT_POSTFIELDS实质上会使libcurl自动带上：Content-Type:
        /// application/x-www-form-urlencoded
        /// 但PUT请求，未指定header中此字段的情况下，应为：Content-Type:
        /// application/octet-stream 所以应带上此处理
        /// 此处把上述两个逻辑分支归并
        if (req.headers.Get("Content-Type").empty()) {
          auto &&header = "Content-Type: application/octet-stream";
          easy_closure.slist_append(header);
          auto &&slist = easy_closure.get_slist();
          if (nullptr != slist) {
            curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, slist);
          }
        }

      } while (false);
      break;  /// case kHttpPut
    default:
      break;
  }
}
static void ResolveEasyHandle(libcurl_easy_closure &easy,
                              assistant::HttpResponse &response) {
  auto &res = response;
  auto &easy_handle = easy.get_easy();
  /// 尽快销毁写回调对象，以将文件写入磁盘
  if (nullptr != res.transfer_callback) {
    res.transfer_callback->Destroy();
    res.transfer_callback = nullptr;
  }
  /// check and save CURLcode
  if (CURLE_OK != easy.result) {
    res.extends.Set("CURLcode", std::to_string(easy.result));
  }
  /// pickup infos from easy handle
  long res_code = 0;
  curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &res_code);
  res.status_code = res_code;
  char *url = nullptr;
  curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &url);
  if (nullptr != url) {
    res.effective_url = url;
  }
  /// 从响应的Content-Length读取的值，-1为未知
  curl_off_t content_length_download = -1;
  curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                    &content_length_download);
  res.extends.Set("content_length_download",
                  std::to_string(content_length_download));
  /// 响应的载荷的实际长度
  curl_off_t size_download = -1;
  curl_easy_getinfo(easy_handle, CURLINFO_SIZE_DOWNLOAD_T, &size_download);
  res.extends.Set("size_download", std::to_string(size_download));
  /// 下载的平均速度，单位是：bytes/second
  curl_off_t speed_download = 0;
  curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD_T, &speed_download);
  if (0 != speed_download) {
    res.extends.Set("speed_download", std::to_string(speed_download));
  }
  /// 从请求的Content-Length读取的值，-1为未知
  curl_off_t content_length_upload = -1;
  curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_UPLOAD_T,
                    &content_length_upload);
  res.extends.Set("content_length_upload",
                  std::to_string(content_length_upload));
  /// 请求的载荷的实际长度
  curl_off_t size_upload = -1;
  curl_easy_getinfo(easy_handle, CURLINFO_SIZE_UPLOAD_T, &size_upload);
  res.extends.Set("size_upload", std::to_string(size_upload));
  /// 取重定向URL
  if (3 == res_code / 100) {
    char *redirect_url = nullptr;
    curl_easy_getinfo(easy_handle, CURLINFO_REDIRECT_URL, &redirect_url);
    if (nullptr != redirect_url) {
      res.extends.Set("redirect_url", redirect_url);
    } else {
      /// 存在接收响应不完整的情况，Headers中的Location字段保存了完整的重定向URL
      std::string header_location;
      if (res.headers.Get("Location", header_location) &&
          !header_location.empty()) {
        res.extends.Set("redirect_url", header_location);
      }
    }
  }

  /// 解析206对应的content-range
  if (206 == res_code) {
    std::string content_range;
    if (res.headers.Get("Content-Range", content_range) &&
        !content_range.empty()) {
      auto iter1 = content_range.find("bytes ");
      if (std::string::npos != iter1) {
        auto iter2 = content_range.find("/", iter1);
        if (std::string::npos != iter2) {
          res.extends.Set("content_range_range",
                          content_range.substr(iter1 + 6, iter2 - iter1 - 6));
          res.extends.Set("content_range_tatal",
                          content_range.substr(
                              iter2 + 1, content_range.size() - iter2 - 1));
        }
      }
    }
  }
}
}  // namespace curl_easy

/// Curl简单句柄与任意类型的映射方法闭包
template <typename T>
struct libcurl_easy_mapping_container {
  typedef CURL *Key;
  typedef T Value;

 private:
  typedef std::map<Key, Value> Map;
  typedef typename Map::iterator iterator;
  typedef typename Map::const_iterator const_iterator;
  Map map;

 public:
  /// 非线程安全方法，应保证在 同一线程调用
  void Put(const Key &key, Value &value) { map.emplace(key, std::move(value)); }
  Value Get(const Key &key) {
    Value value;
    auto iter = map.find(key);
    if (map.end() != iter) {
      value = std::move(iter->second);
      map.erase(iter);
    }
    return value;
  }
  void Clear() {
    auto i = decltype(map)();
    map.swap(i);
  }
  /// 返回是否存在特定的Key
  bool Exists(const Key &key) {
    auto iter = map.find(key);
    return map.end() != iter;
  }
  iterator begin() { return map.begin(); }
  const_iterator begin() const { return map.begin(); }
  iterator end() { return map.end(); }
  const_iterator end() const { return map.end(); }
  bool empty() const { return map.empty(); }
};
/// Curl简单句柄与 assistant_request_response 闭包的映射方法闭包
typedef libcurl_easy_mapping_container<
    std::shared_ptr<curl_easy::assistant_request_response>>
    easy_mapping_assistantclosure;

/// Curl简单句柄与 libcurl_easy_closure 闭包的映射方法闭包
typedef libcurl_easy_mapping_container<std::unique_ptr<libcurl_easy_closure>>
    easy_mapping_easyclosure;
}  // namespace core
}  // namespace assistant
#endif  // _CORE_CURL_EASY_H__
