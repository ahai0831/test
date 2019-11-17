#include <gtest/gtest.h>

#include <HashAlgorithm\md5.h>

TEST(Md5, Md5Test)

{
  std::string str = "md5_test";
  cloud_base::hash_algorithm::MD5 md5;
  md5.update((unsigned char*)str.data(), static_cast<uint32_t>(str.size()));
  md5.finalize();
  auto dis = md5.hex_digest();
  if (nullptr != dis) {
    printf("%s\n", dis);
    delete dis;
    dis = nullptr;
  }
  cloud_base::hash_algorithm::MD5 md5_2(str);
  auto md5_2_str = md5_2.hex_string();
  printf("%s\n", md5_2_str.c_str());
}
