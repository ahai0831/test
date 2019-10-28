#include "rand.h"

#include <cinttypes>
#include <random>
namespace {
//  set rand seed
std::random_device rd_seed;
std::mt19937_64 mt64(rd_seed());
}  // namespace
namespace cloud_base {
namespace rand {
uint64_t GetRand() {
  // generates number in the range (0,18446744073709551615)
  std::uniform_int_distribution<uint64_t> distribution;
  return distribution(mt64);
}

uint64_t GetRand(uint64_t max_num) {
  // generates number in the range (0,max_num)
  std::uniform_int_distribution<uint64_t> distribution(0, max_num);
  return distribution(mt64);
}

uint64_t GetRand(uint64_t min_num, uint64_t max_num) {
  // generates number in the range (min_num, max_num)
  std::uniform_int_distribution<uint64_t> distribution(min_num, max_num);
  return distribution(mt64);
}
}  // namespace rand
}  // namespace cloud_base
