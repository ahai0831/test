#pragma once
#ifndef _SHA1Digest_HH
#define _SHA1Digest_HH

#include <string>

namespace cloud_base {
namespace hash_algorithm {
// GenerateSha1Digest
std::string GenerateSha1Digest(const std::string &ssKey,
                               const std::string &sData);
}  // namespace hash_algorithm
}  // namespace cloud_base
#endif  //_SHA1Digest_HH
