#pragma once
#ifndef V2_CURL_EASY_H__
#define V2_CURL_EASY_H__
#include <map>
#include <memory>

#include <curl/curl.h>

#include "http_primitives.h"
#include "v2/curl_share.h"
#include "v2/curl_transfercallback.h"
#include "v2/safequeue.h"

/// Curl简单句柄数据闭包
/// 封装Curl简单句柄操作
enum SpecialOpt {
  None = 0,
  ClearMultiStack,
  RefreshShareHandle,
  Last,
};

struct a3_1 {
 private:
  CURL *easy_handle;
  curl_slist *slist;

 public:
  CURLcode result;
  /// 特殊的构造函数对此值进行初始化
  const SpecialOpt operation;
  std::weak_ptr<a7_1> share_handle;

 private:
  scopeguard_internal::ScopeGuard guard;

 public:
  a3_1()
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
        }) {
    easy_handle = curl_easy_init();
  }
  a3_1(SpecialOpt opt)
      : easy_handle(nullptr),
        slist(nullptr),
        result(CURLcode(-1)),
        operation(opt),
        guard([this]() {
          if (nullptr != slist) {
            curl_slist_free_all(slist);
            slist = nullptr;
          }
        }) {}
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
};
/// 重用 a3_1 容器
/// 用于存取 a3_1 (Unique_ptr)的闭包
typedef safequeue_closure<a3_1> a3_5;
/// 保存即将开始传输的 a3_1 的容器
/// 用于存取 a3_1 (Unique_ptr)的闭包
typedef safequeue_closure<a3_1> a3_6;

/// Curl简单句柄工具方法闭包
/// 封装Curl简单句柄的设定和提取的工具方法
/// 作为包裹Curl, Request, Response的类
struct a3_2 {
  std::unique_ptr<a3_1> easy;
  std::unique_ptr<assistant::HttpRequest> request;
  std::unique_ptr<assistant::HttpResponse> response;
  static void SetEasyInfo(decltype(request) &request,
                          decltype(response) &response, decltype(easy) &easy) {
    auto &req = *request.get();
    auto &res = *response.get();
    auto &easy_handle = easy->get_easy();
    curl_easy_setopt(easy_handle, CURLOPT_URL, req.url.c_str());
    printf("Do request: %s\n", req.url.c_str());
    /// config some write callback of easy handle transfer
    void *response_callback_data = res.data;
    curl_easy_setopt(easy_handle, CURLOPT_PRIVATE, response_callback_data);
    curl_easy_setopt(easy_handle, CURLOPT_HEADERFUNCTION,
                     curl_transfercallback::headerCallback);
    curl_easy_setopt(easy_handle, CURLOPT_HEADERDATA, response_callback_data);
    /// Solve write callback
    do {
      std::string header_only;
      if (req.extends.Get("header_only", header_only) &&
          0 == header_only.compare("true")) {
        curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION,
                         curl_transfercallback::Headeronly::writeCallback);
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA,
                         response_callback_data);
        /// 使用header_only的写回调，后续无需再尝试
        break;
      }
      std::string download_length;
      std::string download_offset;
      std::string download_filesize;
      std::string download_filepath;
      std::string download_append;
      if (req.extends.Get("download_length", download_length) &&
          req.extends.Get("download_offset", download_offset) &&
          req.extends.Get("download_filesize", download_filesize) &&
          req.extends.Get("download_filepath", download_filepath)) {
        int64_t size = atoll(download_filesize.c_str());
        int64_t begin = atoll(download_offset.c_str());
        int64_t len = atoll(download_length.c_str());
        /// 尝试使用内存映射
        auto write_cbdata_mmap_ptr =
            std::make_unique<curl_transfercallback::WriteByMmap>(
                download_filepath.c_str(), size, begin, len);
        if (nullptr != write_cbdata_mmap_ptr &&
            write_cbdata_mmap_ptr->Valid()) {
          curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION,
                           curl_transfercallback::WriteByMmap::Callback);
          res.transfer_callback = std::move(write_cbdata_mmap_ptr);
          res.transfer_callback->retval_callback = std::move(req.retval_func);
          curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA,
                           response_callback_data);
          /// 使用内存映射，无需再尝试降级
          break;
        }
        /// 尝试使用C文件句柄
        auto writecb_data_ptr =
            std::make_unique<curl_transfercallback::WriteByFile>(
                download_filepath.c_str(), size, begin, len);
        if (nullptr != writecb_data_ptr && writecb_data_ptr->Valid()) {
          curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION,
                           curl_transfercallback::WriteByFile::Callback);
          res.transfer_callback = std::move(writecb_data_ptr);
          res.transfer_callback->retval_callback = std::move(req.retval_func);
          curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA,
                           response_callback_data);
          /// 使用C文件句柄，无需再尝试降级
          break;
        }
      }
      if (req.extends.Get("download_filepath", download_filepath)) {
        download_append = req.extends.Get("download_append");
        bool destroy = 0 != download_append.compare("true");
        auto writecb_data_ptr =
            std::make_unique<curl_transfercallback::WriteByAppend>(
                download_filepath.c_str(), destroy);
        if (nullptr != writecb_data_ptr && writecb_data_ptr->Valid()) {
          curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION,
                           curl_transfercallback::WriteByAppend::Callback);
          res.transfer_callback = std::move(writecb_data_ptr);
          res.transfer_callback->retval_callback = std::move(req.retval_func);
          curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA,
                           response_callback_data);
          /// 使用C文件句柄（追加），无需再尝试降级
          break;
        }
      }
      /// 最终方案，仍是写入到body容器（内存）中
      curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION,
                       curl_transfercallback::writeCallback);
      curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, response_callback_data);
    } while (false);
    /// Solve progress callback
    do {
      curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L);
      std::string header_only;
      if (req.extends.Get("header_only", header_only) &&
          0 == header_only.compare("true")) {
        curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION,
                         curl_transfercallback::Headeronly::progress_callback);
        curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA,
                         response_callback_data);
        /// 使用header_only的写回调，后续无需再尝试
        break;
      }
      curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION,
                       curl_transfercallback::progress_callback);
      curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA,
                       response_callback_data);

    } while (false);
    /// config http request headers
    for (auto const &x : req.headers) {
      /// if nothing to the right side of the colon
      if (!x.first.empty()) {
        if (x.second.empty()) {
          easy->slist_append(std::string(x.first + ";").c_str());
        } else {
          easy->slist_append(std::string(x.first + ": " + x.second).c_str());
        }
      }
    }
    auto &slist = easy->get_slist();
    if (nullptr != slist) {
      curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, slist);
    }
    /// config range (Curl would solve it)
    auto range = req.extends.Get("range");
    if (!range.empty()) {
      curl_easy_setopt(easy_handle, CURLOPT_RANGE, range.c_str());
    }
    /// config proxy
    auto proxy_str = req.extends.Get("proxy");
    if (!proxy_str.empty()) {
      curl_easy_setopt(easy_handle, CURLOPT_PROXY, proxy_str.c_str());
    }
    /// config share handle
    if (auto share_closure = easy->share_handle.lock()) {
      curl_easy_setopt(easy_handle, CURLOPT_SHARE, share_closure->get_share());
    }
    /// config some timeout
    curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1L);
    /// abort if connect cost more than 10 seconds
    curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT_MS, 10000L);
    /// abort if slower than 1 bytes/sec during 60 seconds
    curl_easy_setopt(easy_handle, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_LOW_SPEED_TIME, 60L);
    /// By default: Don't verify server's name and certificate
    curl_easy_setopt(easy_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(easy_handle, CURLOPT_SSL_VERIFYHOST, 0L);
    /// By default: Don't verify (https)proxy's name and certificate
    curl_easy_setopt(easy_handle, CURLOPT_PROXY_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(easy_handle, CURLOPT_PROXY_SSL_VERIFYHOST, 0L);
    /// Use large buffer
    curl_easy_setopt(easy_handle, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);
  }
  static void GetEasyInfo(decltype(easy) &easy, decltype(response) &response) {
    auto &res = *response.get();
    auto &easy_handle = easy->get_easy();
    /// 尽快销毁写回调对象，以将文件写入磁盘
    if (nullptr != res.transfer_callback) {
      res.transfer_callback->Destroy();
      res.transfer_callback = nullptr;
    }

    /// pickup infos from easy handle
    long res_code;
    curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &res_code);
    res.status_code = res_code;
    char *url = nullptr;
    curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &url);
    if (nullptr != url) {
      res.effective_url = url;
    }
    /// 从响应的Content-Length读取的值，-1为未知
    curl_off_t content_length_download;
    curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                      &content_length_download);
    res.extends.Set("content_length_download",
                    std::to_string(content_length_download));
    /// 下载的平均速度，单位是：bytes/second
    curl_off_t speed_download;
    curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD_T, &speed_download);
    if (0 != speed_download) {
      res.extends.Set("speed_download", std::to_string(speed_download));
    }
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
};

/// Curl简单句柄与任意类型的映射方法闭包
template <typename T>
struct easy_map_closure {
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
  void Clear() { map.swap(decltype(map)()); }
  iterator begin() { return map.begin(); }
  const_iterator begin() const { return map.begin(); }
  iterator end() { return map.end(); }
  const_iterator end() const { return map.end(); }
  bool empty() const { return map.empty(); }
};
/// Curl简单句柄与 a3_2 闭包的映射方法闭包
typedef easy_map_closure<std::shared_ptr<a3_2>> a3_7;
/// Curl简单句柄与 a3_1 闭包的映射方法闭包
typedef easy_map_closure<std::unique_ptr<a3_1>> a3_8;
#endif  // V2_CURL_EASY_H__
