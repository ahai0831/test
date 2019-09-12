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
  std::string strSessKey;  ///< ��¼�ỰKey
  std::string strSign;     ///< ����ǩ��
  std::string strDate;     ///< �������ں�ʱ��
  std::string strFileID;   ///< �ļ�ID
  int nFlag;
  std::string strCorpId;         ///< ��ҵ��ID
  long nDt;                      ///< ��������
  std::string strShareFolderID;  ///< �����ļ���ID
  std::string strRange;
  std::string strVersion;     //����汾��Ϣ��
  std::string strClientType;  //����ͻ������ͣ�
  std::string strRandom;      //�����������
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
