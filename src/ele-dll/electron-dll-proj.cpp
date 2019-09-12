#include <time.h>
#include <chrono>
#include <new>
#include <thread>
#include <tuple>

#include <slicedownload_mastercontrol.h>
#include "RequestEncode.h"
#include "electron-dll-proj.h"

class Session {
 public:
  Session(const char *sKey, const char *sSecret);
  virtual ~Session();
  std::string GetSKey() { return m_sSSKey; }
  std::string GetSSecret() { return m_sSSSecret; }
  void SetSKey(std::string sKey) { m_sSSKey = sKey; }
  void SetSSecret(std::string sSecret) { m_sSSSecret = sSecret; }

 private:
  std::string m_sSSKey;
  std::string m_sSSSecret;
};

Session::Session(const char *sKey, const char *sSecret)
    : m_sSSKey(sKey), m_sSSSecret(sSecret) {}

Session::~Session() {}

struct Assist_Type {
  int64_t nLocalAddr;
  typedef std::tuple<std::unique_ptr<std::string>,
                     std::unique_ptr<SlicedownloadMastercontrol>>
      DLMapValue;
  typedef std::string DLMapKey;
  typedef assistant::safemap_closure<DLMapKey, DLMapValue> SafeDLMap;
  SafeDLMap downloadInfo;

  typedef std::tuple<std::unique_ptr<std::string>, std::string> RegMapValue;
  typedef std::string RegMapKey;
  typedef assistant::safemap_closure<RegMapKey, RegMapValue> SafeRegMap;
  SafeRegMap regisStrings;

  std::shared_ptr<assistant::Assistant_v2> pAssist;
  Assist_Type()
      : nLocalAddr(0LL), pAssist(std::make_shared<assistant::Assistant_v2>()) {
    nLocalAddr = (int64_t)(this);
  }
  std::shared_ptr<Session> m_session;
  std::shared_ptr<std::string> m_version;
  /// 复制构造、默认构造、移动构造、=号操作符
  /// 要么禁用掉，要么显式写清
 private:
  Assist_Type(Assist_Type const &) = delete;
  Assist_Type &operator=(Assist_Type const &) = delete;
  Assist_Type(Assist_Type &&) = delete;
};

extern "C" int64_t CreateAst(const char *sSessionInfo) {
  auto ast_ptr = new (std::nothrow) Assist_Type();
  // get session from sessioninfo;
  std::string sKey, sSecret, sVersion;
  sKey = sSecret = sVersion = "";
  ast_ptr->m_session = std::make_shared<Session>(sKey.c_str(), sSecret.c_str());
  ast_ptr->m_version = std::make_shared<std::string>(sVersion);
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
    ast_ptr->downloadInfo.Emplace(std::string(flag),
                                  std::make_tuple(std::move(downloadid_ptr),
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
    std::unique_ptr<std::string> subscription_uuid;
    auto solve_slicedownload_callback =
        [fNext, fComplete,
         &subscription_uuid](const Assist_Type::DLMapValue &mapV) {
          /// 取出下载总控
          SlicedownloadMastercontrol &slicedownload = *std::get<1>(mapV);
          /// 注册订阅
          /// uint64_t -> const char *适配器
          auto next_lambda = [fNext, &slicedownload](uint64_t v) -> void {
            /// 临时解决方案 TODO: 替换为使用json库进行json字符串生成
            char buffer[256] = {'\0'};
            /// 临时解决方案：拼接所需json字符串
            _snprintf(
                buffer, 255,
                "{\"smooth_speed\":\"%" PRIu64 "\",\"total_length\":\"%" PRIu64
                "\",\"completed_length\":\"%" PRIu64 "\",\"in_progress\":%s}",
                v, slicedownload.total_length.load(),
                slicedownload.processed_bytes.load(),
                slicedownload.inprocess_flag ? "true" : "false");
            fNext(buffer);
          };
          subscription_uuid = std::make_unique<std::string>(
              slicedownload.RegSpeedCallback(next_lambda, fComplete));
        };
    ast.downloadInfo.FindDelegate(download_uuid, solve_slicedownload_callback);
    /// 保存订阅的uuid
    flag = const_cast<char *>(subscription_uuid->data());
    ast.regisStrings.Emplace(
        std::string(flag),
        std::make_tuple(std::move(subscription_uuid), download_uuid));
  } while (false);
  return flag;
}

extern "C" bool CancelSubscription(char *sRegisterID, int64_t nAst) {
  bool bSucceed = false;
  auto ptrA_v2 = (Assist_Type *)nAst;
  const std::string reg_uuid(sRegisterID);
  std::string download_uuid;
  auto solve_slicedownload_callback =
      [&bSucceed, &reg_uuid](const Assist_Type::DLMapValue &mapV) {
        // get slicedownload master control;
        SlicedownloadMastercontrol &downloadCtrl = *std::get<1>(mapV);
        // unregister whats the register id points to;
        bSucceed = downloadCtrl.UnregSpeedCallback(reg_uuid);
      };
  auto solve_register_callback =
      [&ptrA_v2, &download_uuid, &solve_slicedownload_callback](
          const Assist_Type::RegMapValue &tuple_strings) {
        // get download uuid;
        download_uuid = std::get<1>(tuple_strings);
        ptrA_v2->downloadInfo.FindDelegate(download_uuid,
                                           solve_slicedownload_callback);
      };
  ptrA_v2->regisStrings.FindDelegate(reg_uuid, solve_register_callback);
  if (bSucceed) {
    ptrA_v2->regisStrings.Delete(reg_uuid);
  }
  return bSucceed;
}

extern "C" void StopDownload(char *sDownloadID, int64_t nAst) {
  auto ptrA_v2 = (Assist_Type *)nAst;
  auto solve_slicedownload_callback = [](const Assist_Type::DLMapValue &mapV) {
    SlicedownloadMastercontrol &downloadCtrl = *std::get<1>(mapV);
    downloadCtrl.AsyncStop();
  };
  ptrA_v2->downloadInfo.FindDelegate(sDownloadID, solve_slicedownload_callback);
}

extern "C" void DestroyAst(int64_t nAst) {
  auto ast_ptr = (Assist_Type *)nAst;
  delete ast_ptr;
}

extern "C" char *DoDownload(const char *sFileInfo, f4download::OnNext fNext,
                            f4download::OnComplete fComplete, int64_t nAst) {
  auto ptrA_v2 = (Assist_Type *)nAst;
  std::string sessionKey, signature, data_;
  std::string fileID, corpID, coSharedID, sDownloadTemPath;
  std::string clientType, version, nRandom;
  long dt, ispreview;
  clientType = "CORPPC";
  if ((dt == 2) && (coSharedID.empty())) {
    fComplete();
    return "";
  }
  data_ = GenerateRequestData();
  signature = GenerateSignature(ptrA_v2->m_session->GetSKey(),
                                ptrA_v2->m_session->GetSSecret(), "GET",
                                "/api/getFileDownloadUrl.action", data_);
  version = ptrA_v2->m_version->data();
  auto nCurTime = ::GetCurrentTime();
  nRandom = std::to_string(rand()) + "_" + std::to_string(nCurTime);
  ReqPara_GetDownloadURL *reqParameter_url = new ReqPara_GetDownloadURL();
  reqParameter_url->strUrl = GetAPIHost() + "/api/getFileDownloadUrl.action";
  reqParameter_url->strCorpId = corpID;
  reqParameter_url->strClientType = clientType;
  reqParameter_url->nDt = dt;
  reqParameter_url->nFlag = ispreview;
  reqParameter_url->strVersion = version;
  reqParameter_url->strRandom = nRandom;
  // encode url & header;
  auto reqUrl = Encode_DownloadUrl(reqParameter_url);
  // get download url;
  assistant::HttpRequest req(reqUrl);
  req.headers.Set("SessionKey", sessionKey);
  req.headers.Set("Signature", signature);
  req.headers.Set("Date", data_);
  // 	reqUrl.solve_func=
  // 	if (nullptr != ptrA_v2->pAssist)
  // 	{
  // 		ptrA_v2->pAssist->AsyncHttpRequest(reqUrl);
  // 	}
  //
  std::string err_code = "0";
  // get the error code of getdownloadurl;
  return "";
}

extern "C" void GetFinalResult(OnFinal fResult, const char *sDownloadID,
                               int64_t nAst) {
  auto ptrA_v2 = (Assist_Type *)nAst;

  //获取文件下载结果信息；
}

extern "C" void CheckDLFile(const char *sFileInfo,
                            f4download::OnCheck fOnCheck) {}
