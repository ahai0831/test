#pragma once
#ifndef HTTPPRIMITIVES_H__
#define HTTPPRIMITIVES_H__
#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#pragma warning(disable : 4996)
#endif
#include <cstdint>

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace assistant {
namespace details {
inline int stricmp(const char* _Str1, const char* _Str2) {
#ifdef _WIN32
  return ::stricmp(_Str1, _Str2);
#else
  /// ��ʱ����Ӧ�ڴ�ƽ̨��ʵ��
  return strcasecmp(_Str1, _Str2);
#endif
}

}  // namespace details

struct StringMap {
 private:
  typedef std::string StringType;
  typedef StringType HeaderKey;
  typedef StringType HeaderValue;
  struct MapCompare {
    bool operator()(const HeaderKey& left, const HeaderKey& right) const {
      return details::stricmp(left.c_str(), right.c_str()) < 0;
    }
  };
  typedef std::map<HeaderKey, HeaderValue, MapCompare> HeaderMap;

  HeaderMap _str_map;

 public:
  explicit StringMap() = default;
  StringMap(std::initializer_list<HeaderMap::value_type> l) : _str_map(l) {}
  bool Get(const HeaderMap::key_type& header_key,
           HeaderMap::mapped_type& header_value) const {
    auto iter = _str_map.find(header_key);
    bool success_flag = false;
    if (success_flag = (_str_map.end() != iter)) {
      header_value = iter->second;
    }
    return success_flag;
  }
  void Set(const HeaderMap::key_type& header_key,
           const HeaderMap::mapped_type& header_value) {
    _str_map[header_key] = header_value;
  }
  bool Delete(const HeaderMap::key_type& header_key) {
    auto iter = _str_map.find(header_key);
    bool success_flag = false;
    if (success_flag = (_str_map.end() != iter)) {
      _str_map.erase(iter);
    }
    return success_flag;
  }
  HeaderMap::mapped_type Get(const HeaderMap::key_type& header_key) const {
    HeaderMap::mapped_type header_value;
    Get(header_key, header_value);
    return header_value;
  }
  void Clear() {
    auto i = decltype(_str_map)();
    _str_map.swap(i);
  }
  HeaderMap::iterator begin() { return _str_map.begin(); }
  HeaderMap::const_iterator begin() const { return _str_map.begin(); }
  HeaderMap::iterator end() { return _str_map.end(); }
  HeaderMap::const_iterator end() const { return _str_map.end(); }
  bool empty() const { return _str_map.empty(); }
  HeaderMap::size_type size() const { return _str_map.size(); }
  StringType extends_str() const {
    StringType extends_str;
    for (const auto& x : _str_map) {
      extends_str.append(x.first + ": " + x.second + ";");
    }
    // ���һ��';'�ַ�����Ҫ��
    if (!extends_str.empty()) {
      extends_str.erase(extends_str.end() - 1);
    }
    return extends_str;
  }
  StringType headers_str() const {
    StringType headers_str;
    for (const auto& x : _str_map) {
      headers_str.append(x.first + ": " + x.second + "\r\n");
    }
    return headers_str;
  }
  /// �ƶ����췽��
  StringMap(StringMap&& map) { swap(map); };
  void swap(StringMap& _Right) { _str_map.swap(_Right._str_map); }
  StringMap(const StringMap&) = default;
  StringMap& operator=(const StringMap&) = default;
};

typedef struct HttpRequest_v1 HttpRequest;
typedef struct HttpResponse_v1 HttpResponse;
struct HttpRequest_v1 {
  typedef std::function<void(struct HttpResponse_v1&, struct HttpRequest_v1&)>
      HttpResolveFunc;
  typedef std::function<void(size_t)> TransCbRetvalCallback;
  typedef enum SpcecialOperators {
    SPCECIALOPERATORS_NORMAL = 0,
    SPCECIALOPERATORS_CLEARCACHE,
    SPCECIALOPERATORS_STOPCONNECT,
    SPCECIALOPERATORS_LIMITDOWNLOADSPEED,
    SPCECIALOPERATORS_LIMITUPLOADSPEED,
  } Opts;
  static SpcecialOperators StringToSpcecialOperators(
      const std::string& special_operator) {
    SpcecialOperators s = SPCECIALOPERATORS_NORMAL;
    do {
      if (special_operator.empty()) {
        break;
      }
      if (details::stricmp(special_operator.c_str(), "StopConnect") == 0) {
        s = SPCECIALOPERATORS_STOPCONNECT;
        break;
      }
      if (details::stricmp(special_operator.c_str(), "LimitDownloadSpeed") ==
          0) {
        s = SPCECIALOPERATORS_LIMITDOWNLOADSPEED;
        break;
      }
      if (details::stricmp(special_operator.c_str(), "LimitUploadSpeed") == 0) {
        s = SPCECIALOPERATORS_LIMITUPLOADSPEED;
        break;
      }
      if (details::stricmp(special_operator.c_str(), "ClearCache") == 0) {
        s = SPCECIALOPERATORS_CLEARCACHE;
        break;
      }
    } while (false);
    return s;
  }
  std::string method;
  std::string url;
  StringMap headers;
  std::string body;
  HttpResolveFunc solve_func;
  StringMap extends;
  TransCbRetvalCallback retval_func;
  HttpRequest_v1(const std::string& _method, const std::string& _url,
                 const StringMap& _headers, const std::string& _body,
                 HttpResolveFunc _solve_func, const StringMap& _extends)
      : method(_method),
        url(_url),
        headers(_headers),
        body(_body),
        solve_func(std::move(_solve_func)),
        extends(_extends) {}
  HttpRequest_v1(const std::string& _method, const std::string& _url,
                 const std::string& _body)
      : method(_method), url(_url), body(_body) {}
  explicit HttpRequest_v1(const std::string& _url) : method("GET"), url(_url){};
  explicit HttpRequest_v1(Opts opts) {
    switch (opts) {
      case SPCECIALOPERATORS_STOPCONNECT:
        extends.Set("SpcecialOperators", "StopConnect");
        break;
      case SPCECIALOPERATORS_CLEARCACHE:
        extends.Set("SpcecialOperators", "ClearCache");
        break;
      case SPCECIALOPERATORS_LIMITDOWNLOADSPEED:
        extends.Set("SpcecialOperators", "LimitDownloadSpeed");
        break;
      case SPCECIALOPERATORS_LIMITUPLOADSPEED:
        extends.Set("SpcecialOperators", "LimitUploadSpeed");
        break;
      default:
        break;
    }
  };
  /// = ��������������solve_func�ص���
  HttpRequest_v1& operator=(HttpRequest_v1 const& req) {
    method = req.method;
    url = req.url;
    headers = req.headers;
    body = req.body;
    extends = req.extends;
    return *this;
  };
  /// �����ƹ��췽��������ȿ����������ͬ��HttpRequest
  HttpRequest_v1(HttpRequest_v1 const& req) {
    *this = req;
    this->solve_func = req.solve_func;
    this->retval_func = req.retval_func;
  };
  /// �ƶ����췽��
  HttpRequest_v1(HttpRequest_v1&& req) {
    method.swap(req.method);
    url.swap(req.url);
    headers.swap(req.headers);
    body.swap(req.body);
    solve_func.swap(req.solve_func);
    extends.swap(req.extends);
    retval_func.swap(req.retval_func);
  };

 private:
  /// ����Ĭ�Ϲ��췽��
  HttpRequest_v1() = delete;
};
struct HttpResponse_v1 {
  struct CallbackBase {
   public:
    HttpRequest::TransCbRetvalCallback retval_callback;
    /// Destroy����������Դ����Ӧ�����ظ����ٵĸ�����
    virtual void Destroy(){};
    /// Vaild�����ж�����Ĺ����Ƿ���Ч������Ծ����ָ��ȣ�
    virtual bool Valid() const { return false; };
    /// ���������д�˷�����ֱ���ü���
    virtual size_t Returnvalue(size_t value) const final {
      (nullptr != retval_callback) ? retval_callback(value) : (void)(0);
      return value;
    };
    /// ֧��Ĭ�Ϲ��췽��
    CallbackBase() = default;
    /// �Ի��࣬��������������ʽ������Դ�������ⲿд�����
    virtual ~CallbackBase() {
      if (Valid()) {
        Destroy();
      }
    };

   private:
    /// �����ƶ����졢���ƹ��졢=���������ط���
    CallbackBase(CallbackBase const&) = delete;
    CallbackBase& operator=(CallbackBase const&) = delete;
    CallbackBase(CallbackBase&&) = delete;
  };
  int32_t status_code;
  std::string effective_url;
  StringMap headers;
  std::string body;
  StringMap extends;
  void* data;
  std::unique_ptr<CallbackBase> transfer_callback;
  std::atomic_bool stop_flag;
  /// ֧��Ĭ�Ϲ��췽��
  HttpResponse_v1()
      : status_code(0), data(static_cast<void*>(this)), stop_flag({false}) {}
  /// = ������������swap��дԭ��ȿ�����Ϊ��
  /// ��֤HttpResponse����һ��
  HttpResponse_v1& operator=(HttpResponse_v1 const& res) {
    auto& cast_res = const_cast<HttpResponse_v1&>(res);
    std::swap(status_code, cast_res.status_code);
    effective_url.swap(cast_res.effective_url);
    headers.swap(cast_res.headers);
    body.swap(cast_res.body);
    extends.swap(cast_res.extends);
    data = static_cast<void*>(this);
    return *this;
  };
  /// �ƶ����췽��
  HttpResponse_v1(HttpResponse_v1&& res)
      : status_code(0), data(static_cast<void*>(this)), stop_flag({false}) {
    *this = res;
  };
  /// ���ƹ��췽��
  HttpResponse_v1(const HttpResponse_v1& res) { *this = res; };

 private:
};
}  // namespace assistant
#endif  // HTTPPRIMITIVES_H__
