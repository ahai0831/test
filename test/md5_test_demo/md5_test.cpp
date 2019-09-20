/**
 * @brief cloud_base md5 test.
 * @date 2019-09-20
 *
 * @copyright Copyright (c) 2019
 *
 */
#include <cinttypes>
#include <iostream>
#include <string>

#include <md5/md5.h>
int main(void) {
  std::string str = "md5_test";
  cloud_base::MD5 md5;
  md5.update((unsigned char*)str.data(), static_cast<uint32_t>(str.size()));
  md5.finalize();
  auto dis = md5.hex_digest();

  if (nullptr != dis) {
    printf("%s\n", dis);
    delete dis;
    dis = nullptr;
  }

  cloud_base::MD5 md5_2(str);
  auto md5_2_str = md5_2.hex_string();
  printf("%s\n", md5_2_str.c_str());

  return 0;
}
