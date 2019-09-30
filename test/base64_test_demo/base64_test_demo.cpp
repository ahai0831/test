/**
 * @brief cloud_base base64 test.
 * @date 2019-09-23
 * @copyright Copyright (c) 2019
 */

#include <iostream>

#include <base64/base64.h>

int main(void) {
  ///
  const char* str1 = "sfsfsdfsdfsdf3545640QSX";
  auto str1_res1 = cloud_base::Base64Encode(str1);
  auto str1_res2 = cloud_base::Base64Decode(str1_res1);

  const char str2[] = {'a', 'b', 'c', '\0', '1', '2', '3'};
  const char str3[] = {'a', 'b', 'c', '\0', '\0', '1', '2', '3'};
  const char str4[] = {'a', 'b', 'c', '\0', '\0', '\0', '1', '2', '3'};
  const char str5[] = {'a', 'b', 'c', '\0', '\0', '\0', '\0', '1', '2', '3'};

  std::string string2;
  string2.append(str2, sizeof(str2));
  auto str2_res1 = cloud_base::Base64Encode(string2);
  auto str2_res2 = cloud_base::Base64Decode(str2_res1);
  auto str2_res3 = cloud_base::Base64Decode_raw(str2_res1);

  std::string string3;
  string3.append(str3, sizeof(str3));
  auto str3_res1 = cloud_base::Base64Encode(string3);
  auto str3_res2 = cloud_base::Base64Decode(str3_res1);

  std::string string4;
  string4.append(str4, sizeof(str4));
  auto str4_res1 = cloud_base::Base64Encode(string4);
  auto str4_res2 = cloud_base::Base64Decode(str4_res1);

  std::string string5;
  string5.append(str5, sizeof(str5));
  auto str5_res1 = cloud_base::Base64Encode(string5);
  auto str5_res2 = cloud_base::Base64Decode(str5_res1);

  return 0;
}