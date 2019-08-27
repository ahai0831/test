#include <chrono>
#include <new>
#include <thread>
#include <tuple>

#include <Assistant_v2.h>
#include <slicedownload_mastercontrol.h>
#include "electron-dll-proj.h"

struct Assist_Type {
  int64_t nLocalAddr;
  typedef std::tuple<std::unique_ptr<std::string>,
                     std::unique_ptr<SlicedownloadMastercontrol>>
      DLMapValue;
  typedef std::string DLMapKey;
  typedef std::map<DLMapKey, DLMapValue> DLMap;
  DLMap downloadInfo;

  typedef std::tuple<std::unique_ptr<std::string>, std::string> RegMapValue;
  typedef std::string RegMapKey;
  typedef std::map<RegMapKey, RegMapValue> RegMap;
  RegMap regisStrings;

  std::shared_ptr<assistant::Assistant_v2> pAssist;
  Assist_Type()
      : nLocalAddr(0LL), pAssist(std::make_shared<assistant::Assistant_v2>()) {
    nLocalAddr = (int64_t)(this);
  }
  /// 复制构造、默认构造、移动构造、=号操作符
  /// 要么禁用掉，要么显式写清
 private:
  Assist_Type(Assist_Type const &) = delete;
  Assist_Type &operator=(Assist_Type const &) = delete;
  Assist_Type(Assist_Type &&) = delete;
};

extern "C" int64_t CreateAst() {
  auto ast_ptr = new (std::nothrow) Assist_Type();
  return ast_ptr->nLocalAddr;
}

extern "C" char *StartSliceDownload(char *sDownloadURL, char *sDownloadPath,
                                    int64_t nAst) {
  auto downloadid_ptr =
      std::make_unique<std::string>(std::move(assistant::uuid::generate()));
  char *flag = nullptr;
  if (nullptr != sDownloadPath && nullptr != sDownloadURL &&
      strlen(sDownloadPath) > 0 && strlen(sDownloadURL) > 0) {
    auto ast_ptr = (Assist_Type *)nAst;
    auto downloadctrl_ptr =
        std::make_unique<SlicedownloadMastercontrol>(ast_ptr->pAssist);
    flag = const_cast<char *>(downloadid_ptr->data());

    downloadctrl_ptr->AsyncProcess(sDownloadURL, sDownloadPath);
    auto piter = ast_ptr->downloadInfo.emplace(
        std::string(flag), std::make_tuple(std::move(downloadid_ptr),
                                           std::move(downloadctrl_ptr)));
  }
  return flag;
}

extern "C" char *RegisterSliceDownloadSubscription(
    f4download::OnNext fNext, f4download::OnComplete fComplete,
    char *sDownloadID, int64_t nAst) {
  char *flag = nullptr;
  do {
    if (nullptr == sDownloadID) {
      break;
    }
    const std::string download_uuid(sDownloadID);
    Assist_Type &ast = *(Assist_Type *)nAst;
    auto iter = ast.downloadInfo.find(download_uuid);
    if (ast.downloadInfo.end() == iter) {
      break;
    }
    /// 取出下载总控
    SlicedownloadMastercontrol &slicedownload = *std::get<1>(iter->second);
    /// 注册订阅
    /// uint64_t -> const char *适配器
    auto next_lambda = [fNext, &slicedownload](uint64_t v) -> void {
      /// 临时解决方案 TODO: 替换为使用json库进行json字符串生成
      char buffer[256] = {'\0'};
      /// 临时解决方案：拼接所需json字符串
      _snprintf(buffer, 255,
                "{\"smooth_speed\":\"%" PRIu64 "\",\"total_length\":\"%" PRIu64
                "\",\"completed_length\":\"%" PRIu64 "\",\"in_progress\":%s}",
                v, slicedownload.total_length.load(),
                slicedownload.processed_bytes.load(),
                slicedownload.inprocess_flag ? "true" : "false");
      fNext(buffer);
    };
    auto subscription_uuid = std::make_unique<std::string>(
        slicedownload.RegSpeedCallback(next_lambda, fComplete));
    /// 保存订阅的uuid
    flag = const_cast<char *>(subscription_uuid->data());
    ast.regisStrings.emplace(
        flag, std::make_tuple(std::move(subscription_uuid), download_uuid));
  } while (false);
  return flag;
}

extern "C" bool CancelSubscription(char *sRegisterID, int64_t nAst) {
  bool bSucceed = false;
  auto ptrA_v2 = (Assist_Type *)nAst;
  auto pIter = ptrA_v2->regisStrings.find(sRegisterID);
  if (pIter != ptrA_v2->regisStrings.end()) {
    Assist_Type::RegMapValue &tuple_strings = pIter->second;
    auto &sDownloadID = std::get<1>(tuple_strings);
    auto pIter_downloadid = ptrA_v2->downloadInfo.find(sDownloadID);
    if (pIter_downloadid != ptrA_v2->downloadInfo.end()) {
      auto &downloadCtrl = std::get<1>(pIter_downloadid->second);
      if (nullptr != downloadCtrl) {
      }
    }
  }
  return bSucceed;
}

extern "C" void StopDownload(char *sDownloadID, int64_t nAst) {
  auto ptrA_v2 = (Assist_Type *)nAst;
  auto pIter = ptrA_v2->downloadInfo.find(sDownloadID);
  if (pIter != ptrA_v2->downloadInfo.end()) {
    auto &downloadCtrl = std::get<1>(pIter->second);
    auto another_thread = std::thread([&downloadCtrl]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
      downloadCtrl->AsyncStop();
    });
    if (another_thread.joinable()) {
      another_thread.join();
    }
  }
}

extern "C" void DestroyAst(int64_t nAst) {
  auto ast_ptr = (Assist_Type *)nAst;
  delete ast_ptr;
}
