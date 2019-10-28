#include <iostream>

#include <rand\rand.h>

#include "restful_common\rand_helper\rand_helper.hpp"
int main() {
  uint64_t minNum = 400;
  uint64_t maxNum = 445;
  uint64_t result1 = cloud_base::rand::GetRand();
  printf("rand1:%" PRIu64 "\n", result1);

  uint64_t result2 = cloud_base::rand::GetRand(maxNum);
  printf("rand2:%" PRIu64 "\n", result2);

  uint64_t result3 = cloud_base::rand::GetRand(minNum, maxNum);
  printf("rand3:%" PRIu64 "\n", result3);

  ////////////////////////
  std::string randStr = restful_common::rand_helper::GetRandString();
  std::cout << "randStr: " << randStr << std::endl;
  return 0;
}
