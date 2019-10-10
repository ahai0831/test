/**
 * @brief AES test.
 * @date 2019-09-23
 *
 * @copyright Copyright (c) 2019
 *
 */
#include <iostream>
#include <sstream>

#include <AES/AES.h>
using std::string;
string strToHex(std::string str);
int main() {
  std::string Data("spencerspenjfdksljflspencerfdsagafd");
  std::string key("1234567890ABCDEF");
  std::string iv("fedcba0987654321");
  std::string encrpt_re = cloud_base::aes_cbc_encryt(Data, key, iv);
  std::string final_re = strToHex(encrpt_re);
  printf("\nencrypt result :%s\n\n", final_re.c_str());
  ////////////////////////////////////////////////
  std::string decrypt_re = cloud_base::aes_cbc_decryt(encrpt_re, key, iv);
  printf("decrypt result :%s\n\n", decrypt_re.c_str());
  return 0;
}

std::string strToHex(std::string str) {
  const std::string hex = "0123456789abcdef";
  std::stringstream ss;
  for (std::string::size_type i = 0; i < str.size(); ++i)
    ss << hex[(unsigned char)str[i] >> 4] << hex[(unsigned char)str[i] & 0xf];
  return ss.str();
}
