#include "upload_file_mastercontrol.h"

#include <chrono>
#include <thread>

#include <json/json.h>

#include <rx_md5.hpp>

#include <tools/string_format.hpp>
using assistant::tools::string::StringFormat;

#include "enterprise_cloud/apis/comfirm_corp_upload_file_complete.h"
#include "enterprise_cloud/apis/create_corp_upload_file.h"
#include "enterprise_cloud/apis/get_corp_upload_status.h"
#include "enterprise_cloud/apis/upload_corp_file_data.h"

#include "enterprise_cloud/error_code/nderror.h"
using EnterpriseCloud::ErrorCode::nderr_infosecurityerrorcode;
using EnterpriseCloud::ErrorCode::nderr_permission_denied;

namespace EnterpriseCloud {
namespace UploadFileMasterControl {

UploadFileMasterControl::UploadFileMasterControl(
    std::shared_ptr<assistant::Assistant_v3>& assistant_ptr,
    const std::string& file_path, const std::string& corp_id,
    const std::string& parent_id, const std::string md5, int32_t file_source,
    const std::string coshare_id, int32_t is_log, const int32_t oper_type,
    const std::string upload_file_id)
    : assistant_weak_ptr_(assistant_ptr),
      md5_finish_size(0),
      upload_bytes_(0),
      file_size_(0),
      file_ptr_(nullptr),
      file_exists_(false),
      status_call_back_(nullptr),
      stop_flag_({false}),
      retry_count_ufd_all(0),
      retry_count_ufd_current(0) {
  file_path_ = file_path;
  corp_id_ = corp_id;
  parent_id_ = parent_id;
  md5_ = md5;
  file_source_ = file_source;
  coshare_id_ = coshare_id;
  is_log_ = is_log;
  oper_type_ = oper_type;
  upload_file_id_ = upload_file_id;
}

void UploadFileMasterControl::MD5CompleteCallback(const std::string& file_md5) {
  Json::Value msg;
  if (nullptr != status_call_back_) {
    auto md5_process = (float)((long double)md5_finish_size / file_size_);
    msg["stage"] = stage_code_0;
    msg["md5_process"] = md5_process;
    msg["md5"] = file_md5;
    status_call_back_(msg.toStyledString());
  }
  msg.clear();

  if (stop_flag_) {
    return;
  }
  do {
    if (md5_ != file_md5) {
      md5_ = file_md5;
      // 跳第一步
      if (!CRUFRequest()) {
        msg["stage"] = 102;
        break;
      }
      return;
    }
    if (!upload_file_id_.empty()) {
      // 跳第四步
      if (!GUSRequest()) {
        msg["stage"] = 402;
        break;
      }
    } else {
      // 跳第一步
      if (!CRUFRequest()) {
        msg["stage"] = 102;
        break;
      }
    }
    return;
  } while (false);
  if (nullptr != status_call_back_) {
    status_call_back_(msg.toStyledString());
  }
}

void UploadFileMasterControl::MD5ProcessCallback(int64_t size) {
  md5_finish_size += size;
  if (nullptr != status_call_back_) {
    auto md5_process = (float)((long double)md5_finish_size / file_size_);
    Json::Value msg;
    msg["stage"] = stage_code_0;
    msg["md5_process"] = md5_process;
    status_call_back_(msg.toStyledString());
  }
}

void UploadFileMasterControl::UploadSpeedCallback(uint64_t upload_speed) {
  if (stop_flag_) {
    upload_speedcounter.Stop();
    return;
  }
  if (nullptr != status_call_back_) {
    Json::Value msg;
    msg["stage"] = stage_code_300;
    msg["md5"] = md5_;
    msg["upload_speed"] = upload_speed;
    msg["upload_bytes"] = upload_bytes_.load();
    msg["upload_file_id"] = upload_file_id_;
    ;
    msg["file_size"] = file_size_;
    status_call_back_(msg.toStyledString());
  }
}

void UploadFileMasterControl::CRUFCallback(
    const assistant::HttpResponse_v1& res,
    const assistant::HttpRequest_v1& req) {
  std::string used_uuid;
  if (req.extends.Get("uuid", used_uuid)) {
    uuid_set_.Delete(used_uuid);
  }
  if (stop_flag_) {
    return;
  }

  Json::Value json_value;
  Json::Reader json_reader;
  do {
    std::string res_info;
    EnterpriseCloud::Apis::CreateUploadFile::HttpResponseDecode(res, req,
                                                                res_info);
    if (!json_reader.parse(res_info, json_value)) {
      json_value["stage"] = stage_code_101;
      break;
    }
    if (!json_value["isSuccess"].asBool()) {
      auto http_statuc_code = json_value["httpStatusCode"].asInt();
      if (4 == (http_statuc_code % 100)) {
        // 4xx错误
        auto int32_error_code = json_value["int32ErrorCode"].asInt();
        if (int32_error_code == nderr_infosecurityerrorcode &&
            1 > retry_info_cruf.retry_count_is) {
          // 重试一次
          if (!CRUFRequest()) {
            json_value["stage"] = stage_code_102;
            break;
          }
          retry_info_cruf.retry_count_is += 1;
          return;
        } else if (int32_error_code == nderr_permission_denied &&
                   1 > retry_info_cruf.retry_count_pd) {
          // 重试一次
          if (!CRUFRequest()) {
            json_value["stage"] = stage_code_102;
            break;
          }
          retry_info_cruf.retry_count_pd += 1;
          return;
        }
      } else if (5 == (http_statuc_code % 100) &&
                 retry_info_cruf.retry_count_max >
                     retry_info_cruf.retry_count_5xx) {
        // 5xx错误
        // 重试，沉睡时长一开始1.5s, 然后1.5的2次方，以此类推，重试最多10次
        auto retry_time = (int)std::pow(retry_info_cruf.retry_time_5xx,
                                        retry_info_cruf.retry_count_5xx);
        Sleep(retry_time);
        if (!CRUFRequest()) {
          json_value["stage"] = stage_code_102;
          break;
        }
        retry_info_cruf.retry_count_5xx += 1;
        return;
      } else if (601 == http_statuc_code &&
                 retry_info_cruf.retry_count_max >
                     retry_info_cruf.retry_count_601) {
        // 重试，沉睡时长一开始1.5s, 然后1.5的2次方，以此类推，重试最多10次
        auto retry_time = (int)std::pow(retry_info_cruf.retry_time_601,
                                        retry_info_cruf.retry_count_601);
        if (retry_time < json_value["waitingTime"].asInt()) {
          retry_time = json_value["waitingTime"].asInt();
        }
        Sleep(retry_time);
        if (!CRUFRequest()) {
          json_value["stage"] = stage_code_102;
          break;
        }
        retry_info_cruf.retry_count_601 += 1;
        return;
      }

      json_value["stage"] = stage_code_103;
      break;
    }

    retry_info_cruf.init();

    file_upload_url_ = json_value["fileUploadUrl"].asString();
    file_commit_url_ = json_value["fileCommitUrl"].asString();
    file_upload_size_ = json_value["size"].asInt64();
    file_exists_ = json_value["fileDataExists"].asBool();
    upload_file_id_ = json_value["uploadFileId"].asString();

    if (file_size_ == file_upload_size_ || file_exists_) {
      // 跳第四步
      if (!COUFRequest()) {
        json_value["stage"] = stage_code_402;
      }
      return;
    } else {
      // 跳第三步
      if (!UFDRequest()) {
        json_value["stage"] = stage_code_302;
        break;
      }
    }
    return;
  } while (false);
  status_call_back_(json_value.toStyledString());
}

void UploadFileMasterControl::GUSCallback(
    const assistant::HttpResponse_v1& res,
    const assistant::HttpRequest_v1& req) {
  std::string used_uuid;
  if (req.extends.Get("uuid", used_uuid)) {
    uuid_set_.Delete(used_uuid);
  }
  if (stop_flag_) {
    return;
  }

  Json::Value json_value;
  Json::Reader json_reader;
  do {
    std::string res_info;
    EnterpriseCloud::Apis::GetUploadFileStatus::HttpResponseDecode(res, req,
                                                                   res_info);
    if (!json_reader.parse(res_info, json_value)) {
      json_value["stage"] = stage_code_201;
      break;
    }
    if (!json_value["isSuccess"].asBool()) {
      auto http_statuc_code = json_value["httpStatusCode"].asInt();
      if (4 == (http_statuc_code % 100)) {
        auto int32_error_code = json_value["int32ErrorCode"].asInt();
        if (int32_error_code == nderr_infosecurityerrorcode &&
            1 > retry_info_gus.retry_count_is) {
          // 重试一次
          if (!GUSRequest()) {
            json_value["stage"] = stage_code_202;
            break;
          }
          retry_info_gus.retry_count_is += 1;
          return;
        } else if (int32_error_code == nderr_permission_denied &&
                   1 > retry_info_gus.retry_count_pd) {
          // 重试1次
          if (!GUSRequest()) {
            json_value["stage"] = stage_code_202;
            break;
          }
          retry_info_gus.retry_count_pd += 1;
          return;
        }
      } else if (5 == (http_statuc_code % 100) &&
                 retry_info_gus.retry_count_max >
                     retry_info_gus.retry_count_5xx) {
        auto retry_time = (int)std::pow(retry_info_gus.retry_time_5xx,
                                        retry_info_gus.retry_count_5xx);
        // 重试，最多重试10次
        Sleep(retry_time);
        if (!GUSRequest()) {
          json_value["stage"] = stage_code_202;
          break;
        }
        retry_info_gus.retry_count_5xx += 1;
        return;
      } else if (601 == http_statuc_code &&
                 retry_info_gus.retry_count_max >
                     retry_info_gus.retry_count_601) {
        // 重试，最多重试10次
        auto retry_time = (int)std::pow(retry_info_gus.retry_time_601,
                                        retry_info_gus.retry_count_601);
        if (retry_time < json_value["waitingTime"].asInt()) {
          retry_time = json_value["waitingTime"].asInt();
        }
        Sleep(retry_time);
        if (!GUSRequest()) {
          json_value["stage"] = stage_code_202;
          break;
        }
        retry_info_gus.retry_count_601 += 1;
        return;
      } else if (602 == http_statuc_code &&
                 retry_info_gus.retry_count_max >
                     retry_info_gus.retry_count_602) {
        // 跳第一步
        if (!CRUFRequest()) {
          json_value["stage"] = stage_code_102;
          break;
        }
        retry_info_gus.retry_count_602 += 1;
        return;
      }

      json_value["stage"] = stage_code_203;
      break;
    }

    retry_info_gus.init();

    file_upload_url_ = json_value["fileUploadUrl"].asString();
    file_commit_url_ = json_value["fileCommitUrl"].asString();
    file_upload_size_ = json_value["size"].asInt64();
    file_exists_ = json_value["fileDataExists"].asBool();
    upload_file_id_ = json_value["uploadFileId"].asString();

    if (file_size_ == file_upload_size_ || file_exists_) {
      // 跳第四步
      if (!COUFRequest()) {
        json_value["stage"] = stage_code_402;
      }
    } else {
      // 跳第三步
      if (!UFDRequest()) {
        json_value["stage"] = stage_code_302;
      }
    }

    return;
  } while (false);
  status_call_back_(json_value.toStyledString());
}

void UploadFileMasterControl::UFDCallback(
    const assistant::HttpResponse_v1& res,
    const assistant::HttpRequest_v1& req) {
  std::string used_uuid;
  if (req.extends.Get("uuid", used_uuid)) {
    uuid_set_.Delete(used_uuid);
  }
  if (stop_flag_) {
    return;
  }
  Json::Value json_value;
  Json::Reader json_reader;
  do {
    upload_speedcounter.Stop();

    std::string res_info;
    EnterpriseCloud::Apis::UploadFileData::HttpResponseDecode(res, req,
                                                              res_info);
    if (!json_reader.parse(res_info, json_value)) {
      json_value["stage"] = stage_code_301;
      break;
    }
    if (!json_value["isSuccess"].asBool()) {
      auto http_statuc_code = json_value["httpStatusCode"].asInt();
      if (retry_info_ufd.retry_count_max <= retry_info_ufd.retry_count_601) {
        json_value["stage"] = stage_code_303;
        break;
      }
      if (601 == http_statuc_code) {
        // 重试，最多重试10次
        auto retry_time = (int)std::pow(retry_info_ufd.retry_time_601,
                                        retry_info_ufd.retry_count_601);
        if (retry_time < json_value["waitingTime"].asInt()) {
          retry_time = json_value["waitingTime"].asInt();
        }
        Sleep(retry_time);
        if (!UFDRequest()) {
          json_value["stage"] = stage_code_302;
          break;
        }
        retry_info_ufd.retry_count_601 += 1;
        return;
      }

      if (10 > retry_count_ufd_current) {
        // 跳第一步，最多跳10次
        if (!CRUFRequest()) {
          json_value["stage"] = stage_code_102;
          break;
        }
        retry_count_ufd_all += 1;
        retry_count_ufd_current += 1;
        return;
      }
      json_value["stage"] = stage_code_303;
      break;
    }

    retry_info_ufd.init();

    // 跳第四步
    if (!COUFRequest()) {
      json_value["stage"] = stage_code_402;
      break;
    }
    return;
  } while (false);
  if (nullptr != status_call_back_) {
    status_call_back_(json_value.toStyledString());
  }
}

void UploadFileMasterControl::COUFCallback(
    const assistant::HttpResponse_v1& res,
    const assistant::HttpRequest_v1& req) {
  std::string used_uuid;
  if (req.extends.Get("uuid", used_uuid)) {
    uuid_set_.Delete(used_uuid);
  }
  if (stop_flag_) {
    return;
  }

  Json::Value json_value;
  Json::Reader json_reader;
  do {
    std::string res_info;
    EnterpriseCloud::Apis::ComfirmUploadFileComplete::HttpResponseDecode(
        res, req, res_info);
    if (!json_reader.parse(res_info, json_value)) {
      json_value["stage"] = stage_code_401;
      break;
    }
    if (!json_value["isSuccess"].asBool()) {
      auto http_statuc_code = json_value["httpStatusCode"].asInt();
      if (4 == (http_statuc_code % 100)) {
        auto int32_error_code = json_value["int32ErrorCode"].asInt();
        if (int32_error_code == nderr_infosecurityerrorcode &&
            1 > retry_info_couf.retry_count_is) {
          // 重试1次
          if (!COUFRequest()) {
            json_value["stage"] = stage_code_402;
            break;
          }
          retry_info_couf.retry_count_is += 1;
          return;
        } else if (int32_error_code == nderr_permission_denied &&
                   1 > retry_info_couf.retry_count_pd) {
          // 重试1次
          if (!COUFRequest()) {
            json_value["stage"] = stage_code_402;
            break;
          }
          retry_info_couf.retry_count_pd += 1;
          return;
        }
      } else if (5 == (http_statuc_code % 100) &&
                 retry_info_couf.retry_count_max >
                     retry_info_couf.retry_count_5xx) {
        // 重试，最多重试10次
        auto retry_time = (int)std::pow(retry_info_couf.retry_time_5xx,
                                        retry_info_couf.retry_count_5xx);
        Sleep(retry_time);
        if (!COUFRequest()) {
          json_value["stage"] = stage_code_402;
          break;
        }
        retry_info_couf.retry_count_5xx += 1;
        return;
      } else if (601 == http_statuc_code &&
                 retry_info_couf.retry_count_max >
                     retry_info_couf.retry_count_601) {
        // 重试，最多重试10次
        auto retry_time = (int)std::pow(retry_info_couf.retry_time_601,
                                        retry_info_couf.retry_count_601);
        if (retry_time < json_value["waitingTime"].asInt()) {
          retry_time = json_value["waitingTime"].asInt();
        }
        Sleep(retry_time);
        if (!COUFRequest()) {
          json_value["stage"] = stage_code_402;
          break;
        }
        retry_info_couf.retry_count_601 += 1;
        return;
      } else if (602 == http_statuc_code &&
                 retry_info_couf.retry_count_max >
                     retry_info_couf.retry_count_602) {
        // 跳第一步
        if (!CRUFRequest()) {
          json_value["stage"] = stage_code_102;
          break;
        }
        retry_info_couf.retry_count_602 += 1;
        return;
      } else if (600 == http_statuc_code) {
        // 跳第二步
        if (!GUSRequest()) {
          json_value["stage"] = stage_code_202;
          break;
        }
        return;
      }

      json_value["stage"] = stage_code_403;
      break;
    }

    retry_info_couf.init();

    // 上传完成
    json_value["stage"] = stage_code_400;
    json_value["fileExists"] = file_exists_;
  } while (false);
  if (nullptr != status_call_back_) {
    status_call_back_(json_value.toStyledString());
  }
}

bool UploadFileMasterControl::Start() {
  retry_info_cruf.init();
  retry_info_gus.init();
  retry_info_ufd.init();
  retry_info_couf.init();

  bool is_success = false;
  do {
    file_ptr_ = _fsopen(file_path_.c_str(), "r", _SH_DENYWR);
    if (nullptr == file_ptr_) {
      break;
    }

    if (_fseeki64(file_ptr_, 0L, SEEK_END) != 0) {
      break;
    }

    file_size_ = _ftelli64(file_ptr_);  // get file_size
    if (file_size_ == -1) {
      break;
    }

    if (file_size_ == 0) {
      //跳第一步
      md5_ = "D41D8CD98F00B204E9800998ECF8427E";
      if (!CRUFRequest()) {
        Json::Value json_value;
        json_value["stage"] = stage_code_102;
        if (nullptr != status_call_back_) {
          status_call_back_(json_value.toStyledString());
        }
        break;
      }
    } else {
      rx_assistant::md5::md5_async_factory::create(
          file_path_,
          std::bind(&UploadFileMasterControl::MD5CompleteCallback, this,
                    std::placeholders::_1),
          std::bind(&UploadFileMasterControl::MD5ProcessCallback, this,
                    std::placeholders::_1));
    }
    is_success = true;
  } while (false);
  return is_success;
}

void UploadFileMasterControl::Stop() {
  // TODO(sun): other code
  if (!stop_flag_.exchange(true)) {
    // 生成根据UUID批量停止连接的Req
    std::string uuids;
    auto GenerateUuidstr = [&uuids](const std::string& uuid) -> void {
      uuids += uuid + ";";
    };
    uuid_set_.ForeachDelegate(GenerateUuidstr);
    if (!uuids.empty()) {
      assistant::HttpRequest stop_req(
          assistant::HttpRequest::Opts::SPCECIALOPERATORS_STOPCONNECT);
      stop_req.extends.Set("uuids", uuids);
      auto assistant_ptr_ = assistant_weak_ptr_.lock();
      if (nullptr == assistant_ptr_) {
        return;
      }
      assistant_ptr_->AsyncHttpRequest(stop_req);
    }
  }
  fclose(file_ptr_);
  Json::Value msg;
  msg["stage"] = stage_code_1;
  status_call_back_(msg.toStyledString());
}

bool UploadFileMasterControl::CRUFRequest() {
  bool is_success = false;
  do {
    assistant::HttpRequest create_upload_file("");
    auto json_params =
        EnterpriseCloud::Apis::CreateUploadFile::JsonStringHelper(
            file_path_, atoll(corp_id_.c_str()), atoll(parent_id_.c_str()),
            md5_, file_source_, coshare_id_, is_log_);
    if (json_params.empty()) {
      break;
    }
    if (!EnterpriseCloud::Apis::CreateUploadFile::HttpRequestEncode(
            json_params, create_upload_file)) {
      break;
    }
    create_upload_file.solve_func =
        std::bind(&UploadFileMasterControl::CRUFCallback, this,
                  std::placeholders::_1, std::placeholders::_2);

    std::string unused_uuid = assistant::tools::uuid::generate();
    create_upload_file.extends.Set("uuid", unused_uuid);
    uuid_set_.Put(unused_uuid);
    auto assistant_ptr_ = assistant_weak_ptr_.lock();
    if (nullptr == assistant_ptr_) {
      break;
    }
    assistant_ptr_->AsyncHttpRequest(create_upload_file);

    is_success = true;
  } while (false);
  return is_success;
}

bool UploadFileMasterControl::GUSRequest() {
  bool is_success = false;
  do {
    assistant::HttpRequest get_upload_status("");
    auto json_params =
        EnterpriseCloud::Apis::GetUploadFileStatus::JsonStringHelper(
            atoll(upload_file_id_.c_str()), atoll(corp_id_.c_str()), is_log_);
    if (json_params.empty()) {
      break;
    }
    if (!EnterpriseCloud::Apis::GetUploadFileStatus::HttpRequestEncode(
            json_params, get_upload_status)) {
      break;
    }
    get_upload_status.solve_func =
        std::bind(&UploadFileMasterControl::GUSCallback, this,
                  std::placeholders::_1, std::placeholders::_2);

    std::string unused_uuid = assistant::tools::uuid::generate();
    get_upload_status.extends.Set("uuid", unused_uuid);
    uuid_set_.Put(unused_uuid);
    auto assistant_ptr_ = assistant_weak_ptr_.lock();
    if (nullptr == assistant_ptr_) {
      break;
    }
    assistant_ptr_->AsyncHttpRequest(get_upload_status);

    is_success = true;
  } while (false);
  return is_success;
}

bool UploadFileMasterControl::UFDRequest() {
  bool is_success = false;
  do {
    // 清零跳转第二步的次数
    if (upload_bytes_ >= file_upload_size_) {
      retry_count_ufd_current = 0;
    }

    upload_bytes_ = file_upload_size_;
    assistant::HttpRequest upload_file_data("");
    auto json_params = EnterpriseCloud::Apis::UploadFileData::JsonStringHelper(
        file_upload_url_, file_path_, atoll(upload_file_id_.c_str()),
        atoll(corp_id_.c_str()), file_source_, file_upload_size_,
        file_size_ - file_upload_size_, is_log_);
    if (json_params.empty()) {
      break;
    }
    if (!EnterpriseCloud::Apis::UploadFileData::HttpRequestEncode(
            json_params, upload_file_data)) {
      break;
    }

    upload_speedcounter.RegSubscription(
        std::bind(&UploadFileMasterControl::UploadSpeedCallback, this,
                  std::placeholders::_1),
        []() {});

    upload_file_data.solve_func =
        std::bind(&UploadFileMasterControl::UFDCallback, this,
                  std::placeholders::_1, std::placeholders::_2);
    upload_file_data.retval_func = [this](uint64_t value) {
      upload_speedcounter.Add(value);
      upload_bytes_ += value;
    };

    std::string unused_uuid = assistant::tools::uuid::generate();
    upload_file_data.extends.Set("uuid", unused_uuid);
    uuid_set_.Put(unused_uuid);
    auto assistant_ptr_ = assistant_weak_ptr_.lock();
    if (nullptr == assistant_ptr_) {
      break;
    }
    assistant_ptr_->AsyncHttpRequest(upload_file_data);

    is_success = true;
  } while (false);
  return is_success;
}

bool UploadFileMasterControl::COUFRequest() {
  bool is_success = false;
  do {
    assistant::HttpRequest comfirm_upload_file("");
    auto json_params =
        EnterpriseCloud::Apis::ComfirmUploadFileComplete::JsonStringHelper(
            file_commit_url_, atoll(upload_file_id_.c_str()),
            atoll(corp_id_.c_str()), file_source_, oper_type_, coshare_id_,
            is_log_);
    if (json_params.empty()) {
      break;
    }
    if (!EnterpriseCloud::Apis::ComfirmUploadFileComplete::HttpRequestEncode(
            json_params, comfirm_upload_file)) {
      break;
    }

    comfirm_upload_file.solve_func =
        std::bind(&UploadFileMasterControl::COUFCallback, this,
                  std::placeholders::_1, std::placeholders::_2);

    std::string unused_uuid = assistant::tools::uuid::generate();
    comfirm_upload_file.extends.Set("uuid", unused_uuid);
    uuid_set_.Put(unused_uuid);
    auto assistant_ptr_ = assistant_weak_ptr_.lock();
    if (nullptr == assistant_ptr_) {
      break;
    }
    assistant_ptr_->AsyncHttpRequest(comfirm_upload_file);

    is_success = true;
  } while (false);
  return is_success;
}

}  // namespace UploadFileMasterControl
}  // namespace EnterpriseCloud
