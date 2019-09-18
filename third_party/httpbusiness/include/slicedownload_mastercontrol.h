#pragma once
#ifndef SLICEDOWNLOADMASTERCONTROL
#define SLICEDOWNLOADMASTERCONTROL
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif

#include <atomic>
#include <future>
#include <memory>
#include <numeric>
#include <tuple>

#include <Assistant_v2.h>
#include <v2/safecontainer.h>
#include <v2/uuid.h>
#include <rxcpp/rx.hpp>

#include "rx_assistant.h"
#include "speed_counter.h"

struct SlicedownloadMastercontrol {
 private:
  std::weak_ptr<assistant::Assistant_v2> netframe;
  // 	std::atomic<std::string> direct_url;
  /// TODO: 考虑到direct_url涉及到多线程读写，后续需利用读写锁封装
  std::string direct_url;
  std::string file_path;

 public:
  /// 此文件实际字节数
  std::atomic_int64_t total_length;
  /// 传输实际进度的字节数(可能回滚)
  std::atomic_int64_t processed_bytes;
  /// 标志位: 传输阶段是否已实际开始
  /// worker数量
  std::atomic_int32_t current_worker;
  //////////////////////////////////////////////////////////////////////////
  /// 错误处理，此任务遇到的累计总错误数
  std::atomic_int32_t total_error_count;
  /// 错误处理，当前错误计数器，可重置
  std::atomic_int32_t error_count;

 private:
  safequeue_closure<std::tuple<int64_t, int64_t>> slice_queue;
  std::promise<void> fin_signal;
  /// 标记此对象AsyncProcess()被调用，避免重复调用
  std::atomic_bool processing_flag;
  /// 集合：各worker对应http请求的uuid，用于停止传输、限速等
  assistant::safeset_closure<std::string> uuid_set;
  /// 标记外部调用AsyncStop()以请求连接停止，不应发起新连接
  std::atomic_bool stop_flag;
  /// 分片下载计速器
  httpbusiness::speed_counter slicedl_speedcounter;

 public:
  explicit SlicedownloadMastercontrol(
      std::shared_ptr<assistant::Assistant_v2>& assistant_v2)
      : netframe(assistant_v2),
        total_length(0),
        processed_bytes(0),
        current_worker(0),
        processing_flag({false}),
        stop_flag({false}),
        total_error_count(0),
        error_count(0) {}
  /// 阻塞式处理分片下载流程
  /// 应仅被调用一次，后续调用无效
  void AsyncProcess(const std::string& url, const std::string& filepath) {
    do {
      if (processing_flag.exchange(true)) {
        break;
      }
      if (url.empty()) {
        break;
      }
      if (filepath.empty()) {
        break;
      }
      if (!direct_url.empty()) {
        break;
      }
      /// 检测netframe是否依然存活，网络库的对象若已死则无需继续
      auto assistant_v2 = netframe.lock();
      if (nullptr == assistant_v2) {
        break;
      }
      /// 重定向，直到获取到非30X的URL
      file_path = filepath;
      /// TODO: 长字符串的原子性操作，有问题
      direct_url = url;

      /// 首个httprequest
      rx_assistant::rx_assistant_factory ast_factory(assistant_v2);
      assistant::HttpRequest first_req(url);
      first_req.extends.Set("range", "0-0");
      first_req.extends.Set("header_only", "true");
      auto first_req_obs = ast_factory.create(first_req);
      auto delayed_iterate_req_callback =
          [this](const rx_assistant::HttpResult& result,
                 assistant::HttpRequest& req,
                 int32_t& delayed_milliseconds) -> bool {
        bool flag = false;
        const int32_t error_count_limits = 10;
        do {
          /// 取curlcode
          int32_t code = 0;
          std::string str_CURLcode;
          if (result.res.extends.Get("CURLcode", str_CURLcode)) {
            code = atol(str_CURLcode.c_str());
            /// 由于Set("header_only", "true"); code可能为42(0x2a)
            if (0x2a == code) {
              code = 0;
            }
          }
          /// 3xx 继续重定向
          if (0 == code && 3 == result.res.status_code / 100) {
            req.extends.Set("range", "0-0");
            req.extends.Set("header_only", "true");
            req.url = result.res.extends.Get("redirect_url");
            flag = true;
            /// 重置当时的错误计数器
            error_count = 0;
            break;
          }
          /// 2xx 调用链终点：无需继续处理
          if (0 == code && 2 == result.res.status_code / 100) {
            flag = false;
            /// 重置当时的错误计数器
            error_count = 0;
            break;
          }
          //////////////////////////////////////////////////////////////////////////
          /// 未处理的情况，均视为发生错误，增加错误计数
          total_error_count += 1;
		  const auto currernt_error_count = error_count.fetch_add(1) + 1;
          /// 未到调用链终点，则应delay一段时间后重试
		  if (currernt_error_count <= error_count_limits) {
            req = assistant::HttpRequest(result.req);
            delayed_milliseconds =
				static_cast<int32_t>(pow(1.5f, currernt_error_count - 1)) * 1000;
            flag = true;
            break;
          }
          /// error_count 超过限制作为调用链的终点
          flag = false;
          break;

        } while (false);
        return flag;
      };

      auto iterate_req_obs = ast_factory.iterator_with_delay_result(
          first_req_obs, delayed_iterate_req_callback,
          slicedl_speedcounter.speed_counter_thread());

      /// 处理分片下载assistant::HttpRequest(const HttpResult&)
      auto solve_206_callback = [this](const rx_assistant::HttpResult& result) {
        /// begin_lambda: solve_206_callback

        /// TODO: 非3xx以及200，需要进行异常处理
        /// TODO: 若200代表不可分片的流程，单连接下载
        /// 若206代表可分片，从header中提取可分片的总长度，用于初始化
        direct_url = result.res.effective_url;
        auto content_range_tatal =
            result.res.extends.Get("content_range_tatal");
        const int64_t current_total_length =
            _atoi64(content_range_tatal.c_str());
        total_length.store(current_total_length);
        /// TODO: test code, to be removed.
        const int32_t max_worker_num = 3;
        const int32_t max_slice_size = 3 << 20;

        /// TODO: 应细分：对分片策略：
        /// 初始worker数3，初始分片大小3M
        /// 1.
        /// 若分片数超过最大worker数的33倍以上，分片大小自动翻番;分片大小上限为48M
        /// 2. 改进：根据16-4-1策略，前76.2%部分，自动合并为worker数量*1片；
        /// 剩余的~前95.2%部分，自动合并为worker数量*2片；

        /// 基于分片策略，生成各个分片
        int64_t current_sliced_size = 0;
        do {
          auto range1 = current_sliced_size;
          auto range2 = range1 + max_slice_size < current_total_length
                            ? range1 + max_slice_size - 1
                            : current_total_length - 1;
          auto tuple = std::make_tuple(range1, range2);
          auto i = std::make_unique<decltype(tuple)>(tuple);
          slice_queue.Enqueue(i);
          current_sliced_size = range2 + 1;
        } while (current_sliced_size < current_total_length);
        /// 初始化worker
        /// 初始化期间，无条件初始化，直到达到上限
        do {
          if (stop_flag) {
            if (0 == current_worker) {
              slicedl_speedcounter.finished_flag = true;
            }
            break;
          }
          auto item = slice_queue.Dequeue();
          if (nullptr != item) {
            ++current_worker;
            worker_awake(std::get<0>(*item), std::get<1>(*item));
          } else {
            break;
          }
        } while (current_worker < max_worker_num);
        /// TODO: 若有expire流程，需要增加到期更新URL流程
      };  /// end_lambda: solve_206_callback

      iterate_req_obs.subscribe(solve_206_callback);
    } while (false);
  }
  /// 阻塞式等待流程结束
  void SyncWait() {
    if (processing_flag) {
      auto future = fin_signal.get_future();
      future.wait();
    }
  }
  ~SlicedownloadMastercontrol() = default;
  /// TODO: 可查询结果，若未传完，则返回传输进行中，若已传完，则返回结果

  /// 注册异步回调
  /// 返回uuid
  std::string RegSpeedCallback(
      httpbusiness::speed_counter::OnnextCb onnext_cb,
      httpbusiness::speed_counter::OncompleteCb oncomplete_cb) {
    return slicedl_speedcounter.RegSubscription(onnext_cb, oncomplete_cb);
  }
  /// 根据uuid取消特定订阅的方法，返回值为true代表取消订阅成功
  bool UnregSpeedCallback(const std::string& uuid) {
    return slicedl_speedcounter.UnregSubscription(uuid);
  }

  /// 异步使此任务停止传输
  /// 可在其他线程中调用
  void AsyncStop() {
    if (!stop_flag.exchange(true)) {
      /// 生成根据UUID批量停止连接的Req
      std::string uuids;
      auto GenerateUuidstr = [&uuids](const std::string& uuid) -> void {
        uuids += uuid + ";";
      };
      uuid_set.ForeachDelegate(GenerateUuidstr);
      if (!uuids.empty()) {
        assistant::HttpRequest stop_req(
            assistant::HttpRequest::Opts::SPCECIALOPERATORS_STOPCONNECT);
        stop_req.extends.Set("uuids", uuids);
        auto assistant_v2 = netframe.lock();
        if (nullptr != assistant_v2) {
          assistant_v2->AsyncHttpRequest(stop_req);
        }
      }
    }
  }

 private:
  void worker_awake(int64_t range1, int64_t range2) {
    assistant::HttpRequest req(direct_url);
    req.extends.Set("range",
                    std::to_string(range1) + "-" + std::to_string(range2));
    req.extends.Set("download_filepath", file_path);
    req.extends.Set("download_filesize", std::to_string(total_length));
    req.extends.Set("download_offset", std::to_string(range1));
    req.extends.Set("download_length", std::to_string(range2 - range1 + 1));
    /// TODO: 对传输类请求，应缩短建立连接后的超时时间到10S以内
    /// 避免长时间占着坑位 2019.8.4

    /// process, uuid , for potential stop
    std::string unused_uuid = assistant::uuid::generate();
    req.extends.Set("uuid", unused_uuid);
    uuid_set.Put(unused_uuid);
    req.solve_func = std::bind(&SlicedownloadMastercontrol::done_callback, this,
                               std::placeholders::_1, std::placeholders::_2);
    req.retval_func = [this](uint64_t value) {
      slicedl_speedcounter.finished_bytes += value;
      processed_bytes += value;
    };
    auto assistant_v2 = netframe.lock();
    if (nullptr != assistant_v2) {
      assistant_v2->AsyncHttpRequest(req);
    }
  }
  void done_callback(const assistant::HttpResponse_v1& res,
                     const assistant::HttpRequest_v1& req) {
    /// remove uuid
    std::string used_uuid;
    if (req.extends.Get("uuid", used_uuid)) {
      uuid_set.Delete(used_uuid);
    }
    decltype(slice_queue.Dequeue()) item = nullptr;
    /// TODO: 应在此处检查此次传输是否按照要求成功完成
    /// 若失败，理应对未完成传输的部分，发起重试
    /// 为统一，应将未完成的信息，放进队列；
    /// 取分片时，优先从此队列中取出待重试信息。
    /// 2019.8.4

    /// stop_flag为true时，不再发起新的连接
    if (!stop_flag) {
      item = slice_queue.Dequeue();
    }
    if (nullptr != item) {
      worker_awake(std::get<0>(*item), std::get<1>(*item));
    } else {
      /// worker不应继续工作
      if (--current_worker == 0) {
        /// 计速流程停止
        slicedl_speedcounter.finished_flag = true;
        /// 同步型总控对应的处理
        fin_signal.set_value();
      }
    }
  }
};

#endif  /// SLICEDOWNLOADMASTERCONTROL
