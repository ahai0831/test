#include "SHA1Digest.h"

#include <iostream>
#include <memory>

#include "HashAlgorithm\HMAC_SHA1.h"

#define SHA1_DIGESTSIZE (20)

namespace cloud_base {
namespace hash_algorithm {
std::string GenerateSha1Digest(const std::string &ssKey,
                               const std::string &sData) {
  unsigned char digest[SHA1_DIGESTSIZE] = {0};
  if (ssKey.length()) {
    // cloud_base::CHMAC_SHA1 *a = new cloud_base::CHMAC_SHA1();
    /// use make_unique instead of new method
    auto a = std::make_unique<hash_algorithm::CHMAC_SHA1>();
    a->HMAC_SHA1((unsigned char *)sData.c_str(), (int)sData.length(),
                 (unsigned char *)ssKey.c_str(), (int)ssKey.length(),
                 (unsigned char *)digest);
  } else {
    // cloud_base::CSHA1 *b = new cloud_base::CSHA1();
    /// use make_unique instead of new method
    auto b = std::make_unique<hash_algorithm::CSHA1>();
    b->Update((unsigned char *)sData.c_str(), (unsigned int)sData.length());
    b->Final();
    b->GetHash((unsigned char *)digest);
  }
  char s[SHA1_DIGESTSIZE * 2 + 1] = {0};
  for (int i = 0; i < SHA1_DIGESTSIZE; i++) {
    sprintf(s + i * 2, "%02X", digest[i]);
  }
  return std::string(s);
}
}  // namespace hash_algorithm
}  // namespace cloud_base
