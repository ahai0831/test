#pragma once
#ifndef UPLOAD_FILE_MASTERCONTROL_H
#define UPLOAD_FILE_MASTERCONTROL_H

#include <cinttypes>
#include <cstdio>

#include <functional>
#include <memory>
#include <string>

#include <Assistant_v3.hpp>
#include <speed_counter.hpp>

namespace {
typedef std::function<void(std::string)> STATUSCALLBACK;

typedef struct RI {
  // 信安检查异常重试次数
  int retry_count_is;
  // 权限不足重试次数
  int retry_count_pd;
  // 最大重试次数
  const int retry_count_max = 10;
  // http status code 5xx
  int retry_count_5xx;
  const int retry_time_5xx = 1500;
  // http status code 601
  int retry_count_601;
  const int retry_time_601 = 1500;

  void init() {
    retry_count_is = 0;
    retry_count_pd = 0;
    retry_count_5xx = 0;
    retry_count_601 = 0;
  }

} RetryInfo;

}  // namespace

namespace EnterpriseCloud {
namespace UploadFileMasterControl {
class UploadFileMasterControl {
 public:
  UploadFileMasterControl(
      std::shared_ptr<assistant::Assistant_v3>& assistant_ptr,
      const std::string& file_path, const std::string& corp_id,
      const std::string& parent_id, const std::string md5, int32_t file_source,
      const std::string coshare_id, int32_t is_log, const int32_t oper_type,
      const std::string upload_file_id);

  ~UploadFileMasterControl() { fclose(file_ptr_); }

  // 开始上传文件
  // 开始成功返回true, 失败返回false。
  bool Start();

  // 停止请求，md5计算目前不能停止。
  void Stop();

  // 主控状态的回调
  // 返回一个json字符串，必含一个stage字段，int类型
  // stage: 0
  // md5计算，包含的其他字段有
  // md5_process: [float] md5计算进度
  // md5: [string] 计算出的md5值，只有计算完成才会包含
  // stage: 1
  // 调用Stop()发出的stage
  // stage: 300
  // 上传文件中，包含的字段有
  // md5: [string] 计算出的md5值
  // upload_speed: [uint64_t] 上传的速度，单位为 bytes/s
  // upload_bytes: [int64_t] 实际已经上传的字节数，包括已上传的加本次上传的数据
  // upload_file_id_: [string] 上传文件的upload file id;
  // file_size: [int64_t] 整个上传文件的大小
  // stage: 400
  // 上传完成，包含的字段有
  // fileSize,[int64_t], 表明文件大小
  // fileId,[string], 表明文件id
  // createTime,[string], 表明文件创建日期,格式为YYYY-MM-DD hh:mm:ss
  // rev,[int64_t], 表明文件版本号
  // md5,[string], 表明文件的MD5码
  // fileName,[string], 文件名称
  //
  // 其他stage都是发生率错误。
  void StatusCallback(STATUSCALLBACK status_call_back) {
    status_call_back_ = status_call_back;
  }

  // 实际已经上传的字节数
  // 包括已上传的加本次上传的数据
  // 有可能回滚
  std::atomic_int64_t upload_bytes_;

  // 整个上传的文件大小
  int64_t file_size_;

  // 上传文件的md5
  std::string md5_;
  // 上传文件的upload file id;
  std::string upload_file_id_;

 protected:
  UploadFileMasterControl() = delete;

  // md5计算全部完成回调函数
  void MD5CompleteCallback(const std::string&);
  // md5计算已完成回调函数
  void MD5ProcessCallback(int64_t size);

  // 上传速度的回调函数
  void UploadSpeedCallback(uint64_t);

  // create upload file的请求函数
  bool CRUFRequest();
  // create upload file的回调函数
  void CRUFCallback(const assistant::HttpResponse_v1& res,
                    const assistant::HttpRequest_v1& req);

  // get upload status的请求函数
  bool GUSRequest();
  // get upload status的回调函数
  void GUSCallback(const assistant::HttpResponse_v1& res,
                   const assistant::HttpRequest_v1& req);

  // upload file data的请求函数
  bool UFDRequest();
  // upload file data的回调函数
  void UFDCallback(const assistant::HttpResponse_v1& res,
                   const assistant::HttpRequest_v1& req);

  // comfirm upload file的请求函数
  bool COUFRequest();
  // comfirm upload file的回调函数
  void COUFCallback(const assistant::HttpResponse_v1& res,
                    const assistant::HttpRequest_v1& req);

 private:
  // 阶段码
  // 10x为创建续传请求
  // 20x为获取上传文件状态、
  // 30x为上传文件数据
  // 40x为确认文件上传完成
  //
  // xx0为请求和响应正常
  // xx1为解析json参数时出错
  // xx2为创建请求出错
  // xx3为请求失败
  const int stage_code_0 = 0;  // 计算md5
  const int stage_code_1 = 1;  // stop
  const int stage_code_100 = 100;
  const int stage_code_101 = 101;
  const int stage_code_102 = 102;
  const int stage_code_103 = 103;
  const int stage_code_200 = 200;
  const int stage_code_201 = 201;
  const int stage_code_202 = 202;
  const int stage_code_203 = 203;
  const int stage_code_300 = 300;
  const int stage_code_301 = 301;
  const int stage_code_302 = 302;
  const int stage_code_303 = 303;
  const int stage_code_400 = 400;
  const int stage_code_401 = 401;
  const int stage_code_402 = 402;
  const int stage_code_403 = 403;

  // 需要传入的参数
  std::string file_path_;
  std::string corp_id_;
  std::string parent_id_;
  int32_t file_source_;
  std::string coshare_id_;
  int32_t is_log_;
  int32_t oper_type_;

  // 内部存储的参数
  std::string file_upload_url_;
  std::string file_commit_url_;
  int64_t file_upload_size_;
  bool file_exists_;

  // md5计算相关
  int64_t md5_finish_size;

  // 主控状态回调
  STATUSCALLBACK status_call_back_;

  // 文件上传计速器
  httpbusiness::speed_counter_with_stop upload_speedcounter;

  // 持有的要上传的文件指针，共享读，拒绝写。
  FILE* file_ptr_;

  // assistant指针
  std::weak_ptr<assistant::Assistant_v3> assistant_weak_ptr_;

  // 集合：各worker对应http请求的uuid，用于停止传输、限速等
  assistant::tools::safeset_closure<std::string> uuid_set_;

  // 标记外部调用Stop()以请求连接停止，不应发起新连接
  std::atomic_bool stop_flag_;

  // create upload file的重试信息
  RetryInfo retry_info_cruf;
  // get upload status的重试信息
  RetryInfo retry_info_gus;
  // upload file data的重试信息
  RetryInfo retry_info_ufd;
  int retry_count_ufd_all;      // 总的跳转到get upload status的次数
  int retry_count_ufd_current;  // 当前跳转到get upload status的次数
  // comfirm upload file的重试信息
  RetryInfo retry_info_couf;
};
}  // namespace UploadFileMasterControl
}  // namespace EnterpriseCloud

#endif
