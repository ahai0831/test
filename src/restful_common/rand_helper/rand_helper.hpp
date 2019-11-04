/**
 * @brief restful_common::rand_helper .hpp
 * @date 2019-10-18
 *
 * @copyright Copyright 2019
 *
 */
#ifndef RANDHELPER_RANDHELPER_H__
#define RANDHELPER_RANDHELPER_H__
#include <cinttypes>
#include <random>
#include <string>

#include <rand\rand.h>
namespace restful_common {
namespace rand_helper {
//  generate a rand string as xxxx_xxxxxxxx
static std::string GetRandString() {
  int four_rand = static_cast<int>(cloud_base::rand::GetRand(1000, 10000));
  int eight_rand =
      static_cast<int>(cloud_base::rand::GetRand(10000000, 100000000));
  char temp_str[20] = {"0"};
  sprintf(temp_str, "%d_%d", four_rand, eight_rand);
  return std::string(temp_str);
}
}  // namespace rand_helper
}  // namespace restful_common
#endif  // JSON_JSON_H__
