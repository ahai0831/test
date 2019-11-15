#include <gtest/gtest.h>

#include <AES\AES.h>

std::string strToHex(std::string str) {
  const std::string hex = "0123456789abcdef";
  std::stringstream ss;
  for (std::string::size_type i = 0; i < str.size(); ++i)
    ss << hex[(unsigned char)str[i] >> 4] << hex[(unsigned char)str[i] & 0xf];
  return ss.str();
}

TEST(Aes, AesTest)

{
  std::string Data("spencerspenjfdksljflspencerfdsagafd");
  std::string key("1234567890ABCDEF");
  std::string iv("fedcba0987654321");
  std::string encrpt_re = cloud_base::aes_cbc_encryt(Data, key, iv);
  std::string final_re = strToHex(encrpt_re);
  printf("nencrypt result:%s\n", final_re.c_str());
  std::string decrypt_re = cloud_base::aes_cbc_decryt(encrpt_re, key, iv);
  printf("decrypt result:%s\n", decrypt_re.c_str());
}
