#include "RequestEncode.h"
#include <time.h>
#include "HMAC_SHA1.h"
#include "SHA1.h"

using namespace std;

string GetAPIHost() { return "api-b.cloud.189.cn"; }

string GenerateSignature(const std::string &sKey, const std::string &sSecret,
                         const std::string &operate, const std::string &reqURL,
                         const std::string &reqDate) {
  std::string sData = "SessionKey=%s&Operate=%s&RequestURI=%s&Date=%s";
  return GenerateSha1Digest(sSecret, sData);
}

string GenerateRequestData() {
  time_t t;
  time(&t);
  struct tm *gmt = gmtime(&t);
  if (gmt) {
    char buff[128] = {0};
    setlocale(LC_TIME, "C");
    size_t len =
        strftime(buff, sizeof(buff) - 1, "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return std::string(buff);
  }
  return "";
}

string GenerateSha1Digest(const std::string &ssKey, const std::string &sData) {
  unsigned char digest[SHA1_DIGESTSIZE] = {0};
  if (ssKey.length()) {
    CHMAC_SHA1 *a = new CHMAC_SHA1();
    a->HMAC_SHA1((BYTE *)ssKey.c_str(), (int)ssKey.length(),
                 (BYTE *)sData.c_str(), (int)sData.length(), (BYTE *)digest);
  } else {
    CSHA1 *b = new CSHA1();
    b->Update((BYTE *)sData.c_str(), (unsigned int)sData.length());
    b->Final();
    b->GetHash((BYTE *)digest);
  }
  char s[SHA1_DIGESTSIZE * 2 + 1] = {0};
  for (int i = 0; i < SHA1_DIGESTSIZE; i++) {
    sprintf(s + i * 2, "%02x", digest[i]);
  }
  return std::string(s);
}

string Encode_DownloadUrl(ReqPara_GetDownloadURL *reqPara) {
  char buffer[256] = {'\0'};
  _snprintf(buffer, 255,
            "%s?fileId=%s&dt=%d&corpId=%s&flag=%d&coshareId=%s&clientType=%s&"
            "version=%s&rand=%s",
            reqPara->strUrl, reqPara->strFileID, reqPara->nDt,
            reqPara->strCorpId, reqPara->nFlag, reqPara->strShareFolderID);
  return (string)buffer;
}
