/**
 * @brief cloud_base::rand.h
 * @date 2019-10-18
 *
 * @copyright Copyright 2019
 *
 */
#ifndef RAND_RAND_H__
#define RAND_RAND_H__
#include <cinttypes>
namespace cloud_base {
namespace rand {
uint64_t GetRand();
uint64_t GetRand(uint64_t max_num);
uint64_t GetRand(uint64_t min_num, uint64_t max_num);
}  // namespace rand
}  // namespace cloud_base
#endif  // RAND_RAND_H__
