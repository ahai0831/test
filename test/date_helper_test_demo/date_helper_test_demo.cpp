/**
 * @brief cloud_base date_helper.
 * @date 2019-10-18
 * @copyright Copyright (c) 2019
 */
#include <iostream>

#include <DateHelper\DateHelper.h>

int main() {
  std::string test_1 = cloud_base::date_helper::get_time_stamp();
  printf("%s\n", test_1.c_str());
  std::string test_2 = cloud_base::date_helper::get_gmt_time_stamp();
  printf("%s\n", test_2.c_str());
  uint64_t test_3 = cloud_base::date_helper::get_second_time_stamp();
  printf("%" PRIu64 "\n", test_3);
  uint64_t test_4 = cloud_base::date_helper::get_millisecond_time_stamp();
  printf("%" PRIu64 "\n", test_4);
  return 0;
}
