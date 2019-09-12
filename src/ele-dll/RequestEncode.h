#include <string>

// The first PRF to try when mounting
#define FIRST_PRF_ID 1

// Hash algorithms (pseudorandom functions).
enum {
  RIPEMD160 = FIRST_PRF_ID,
#ifndef TC_WINDOWS_BOOT
  SHA512,
  WHIRLPOOL,
  SHA1,  // Deprecated/legacy
#endif
  HASH_ENUM_END_ID
};

// The last PRF to try when mounting and also the number of implemented PRFs
#define LAST_PRF_ID (HASH_ENUM_END_ID - 1)

#define RIPEMD160_BLOCKSIZE 64
#define RIPEMD160_DIGESTSIZE 20

#define SHA1_BLOCKSIZE 64
#define SHA1_DIGESTSIZE 20

#define SHA512_BLOCKSIZE 128
#define SHA512_DIGESTSIZE 64

#define WHIRLPOOL_BLOCKSIZE 64
#define WHIRLPOOL_DIGESTSIZE 64

#define MAX_DIGESTSIZE WHIRLPOOL_DIGESTSIZE

#define DEFAULT_HASH_ALGORITHM FIRST_PRF_ID
#define DEFAULT_HASH_ALGORITHM_BOOT RIPEMD160

struct ReqPara_GetDownloadURL {
  ReqPara_GetDownloadURL() : nFlag(0), nDt(0) {}
  std::string strUrl;
  std::string strSessKey;  ///< 登录会话Key
  std::string strSign;     ///< 请求签名
  std::string strDate;     ///< 请求日期和时间
  std::string strFileID;   ///< 文件ID
  int nFlag;
  std::string strCorpId;         ///< 企业云ID
  long nDt;                      ///< 下载类型
  std::string strShareFolderID;  ///< 共享文件夹ID
  std::string strRange;
  std::string strVersion;     //加入版本信息；
  std::string strClientType;  //加入客户端类型；
  std::string strRandom;      //加入随机数；
};

// generate signature
std::string GenerateSha1Digest(const std::string& ssKey,
                               const std::string& sData);
std::string GenerateSignature(const std::string& sKey,
                              const std::string& sSecret,
                              const std::string& operate,
                              const std::string& reqURL,
                              const std::string& reqDate);
std::string GenerateRequestData();

// host;
std::string GetAPIHost();

// encode url;
std::string Encode_DownloadUrl(ReqPara_GetDownloadURL* reqPara);
