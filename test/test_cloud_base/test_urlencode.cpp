#include <gtest\gtest.h>

#include <UrlEncode\UrlEncode.h>
#include <tools\string_convert.hpp>

TEST(UrlEncode, Rfc3986_1)

{
  std::wstring unencode_url_1 = L"URLencode123测试";
  std::string unencode_url_1_out =
      assistant::tools::string::wstringToAnsi(unencode_url_1);
  std::string temp_str_1 =
      assistant::tools::string::wstringToUtf8(unencode_url_1);
  std::string encode_url_1 =
      cloud_base::url_encode::rfc3986::url_encode(temp_str_1);
  std::string decode_url_1 =
      cloud_base::url_encode::rfc3986::url_decode(encode_url_1);
  std::wstring temp_wstr_1 =
      assistant::tools::string::utf8ToWstring(decode_url_1);
  std::string temp_str_1_out =
      assistant::tools::string::wstringToAnsi(temp_wstr_1);

  printf("编码前:%s\n", unencode_url_1_out.c_str());
  printf("编码后:%s\n", encode_url_1.c_str());
  printf("解码后:%s\n", temp_str_1_out.c_str());
}

TEST(UrlEncode, Rfc3986_2)

{
  std::wstring unencode_url_2 = L"URLencode 123测试";
  std::string unencode_url_2_out =
      assistant::tools::string::wstringToAnsi(unencode_url_2);
  std::string temp_str_2 =
      assistant::tools::string::wstringToUtf8(unencode_url_2);
  std::string encode_url_2 =
      cloud_base::url_encode::rfc3986::url_encode(temp_str_2);
  std::string decode_url_2 =
      cloud_base::url_encode::rfc3986::url_decode(encode_url_2);
  std::wstring temp_wstr_2 =
      assistant::tools::string::utf8ToWstring(decode_url_2);
  std::string temp_str_2_out =
      assistant::tools::string::wstringToAnsi(temp_wstr_2);

  printf("编码前:%s\n", unencode_url_2_out.c_str());
  printf("编码后:%s\n", encode_url_2.c_str());
  printf("解码后:%s\n", temp_str_2_out.c_str());
}

TEST(UrlEncode, Rfc3986_3)

{
  std::wstring unencode_url_3 = L"URL/./-/_/~/+/()/encode 123测试";
  std::string unencode_url_3_out =
      assistant::tools::string::wstringToAnsi(unencode_url_3);
  std::string temp_str_3 =
      assistant::tools::string::wstringToUtf8(unencode_url_3);
  std::string encode_url_3 =
      cloud_base::url_encode::rfc3986::url_encode(temp_str_3);
  std::string decode_url_3 =
      cloud_base::url_encode::rfc3986::url_decode(encode_url_3);
  std::wstring temp_wstr_3 =
      assistant::tools::string::utf8ToWstring(decode_url_3);
  std::string temp_str_3_out =
      assistant::tools::string::wstringToAnsi(temp_wstr_3);

  printf("编码前:%s\n", unencode_url_3_out.c_str());
  printf("编码后:%s\n", encode_url_3.c_str());
  printf("解码后:%s\n", temp_str_3_out.c_str());
}

TEST(UrlEncode, http_post_form_1)

{
  std::wstring unencode_url_4 = L"URLencode123测试";
  std::string unencode_url_4_out =
      assistant::tools::string::wstringToAnsi(unencode_url_4);
  std::string temp_str_4 =
      assistant::tools::string::wstringToUtf8(unencode_url_4);
  std::string encode_url_4 =
      cloud_base::url_encode::http_post_form::url_encode(temp_str_4);
  std::string decode_url_4 =
      cloud_base::url_encode::http_post_form::url_decode(encode_url_4);
  std::wstring temp_wstr_4 =
      assistant::tools::string::utf8ToWstring(decode_url_4);
  std::string temp_str_4_out =
      assistant::tools::string::wstringToAnsi(temp_wstr_4);

  printf("编码前:%s\n", unencode_url_4_out.c_str());
  printf("编码后:%s\n", encode_url_4.c_str());
  printf("解码后:%s\n", temp_str_4_out.c_str());
}

TEST(UrlEncode, http_post_form_2)

{
  std::wstring unencode_url_5 = L"URLencode 123测试";
  std::string unencode_url_5_out =
      assistant::tools::string::wstringToAnsi(unencode_url_5);
  std::string temp_str_5 =
      assistant::tools::string::wstringToUtf8(unencode_url_5);
  std::string encode_url_5 =
      cloud_base::url_encode::http_post_form::url_encode(temp_str_5);
  std::string decode_url_5 =
      cloud_base::url_encode::http_post_form::url_decode(encode_url_5);
  std::wstring temp_wstr_5 =
      assistant::tools::string::utf8ToWstring(decode_url_5);
  std::string temp_str_5_out =
      assistant::tools::string::wstringToAnsi(temp_wstr_5);

  printf("编码前:%s\n", unencode_url_5_out.c_str());
  printf("编码后:%s\n", encode_url_5.c_str());
  printf("解码后:%s\n", temp_str_5_out.c_str());
}

TEST(UrlEncode, http_post_form_3)

{
  std::wstring unencode_url_6 = L"URL/./-/_/~/+/()/encode 123测试";
  std::string unencode_url_6_out =
      assistant::tools::string::wstringToAnsi(unencode_url_6);
  std::string temp_str_6 =
      assistant::tools::string::wstringToUtf8(unencode_url_6);
  std::string encode_url_6 =
      cloud_base::url_encode::http_post_form::url_encode(temp_str_6);
  std::string decode_url_6 =
      cloud_base::url_encode::http_post_form::url_decode(encode_url_6);
  std::wstring temp_wstr_6 =
      assistant::tools::string::utf8ToWstring(decode_url_6);
  std::string temp_str_6_out =
      assistant::tools::string::wstringToAnsi(temp_wstr_6);

  printf("编码前:%s\n", unencode_url_6_out.c_str());
  printf("编码后:%s\n", encode_url_6.c_str());
  printf("解码后:%s\n", temp_str_6_out.c_str());
}

TEST(UrlEncode, space_test)

{
  std::string encode_url_7 = "+%20+%20+%20+%20";
  std::string decode_url_7 =
      cloud_base::url_encode::http_post_form::url_decode(encode_url_7);
  printf("编码后:%s\n", encode_url_7.c_str());
  printf("解码后:%s\n", decode_url_7.c_str());
}
