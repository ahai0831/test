#include <iostream>

#include "restful_common\jsoncpp_helper\jsoncpp_helper.hpp"
int main() {
  /** test data
  {
  "test_string":"test_string",
  "test_int":-123,
  "test_uint":78,
  "test_int64_t":-8223372036854775808,
  "test_uint64_t":8223372036854775808,
  "test_bool":true,
  "test_double": 123.34
  }
  */
  // initializa json::value
  const char* str =
      "{\"test_string\": \"test_string\",\"test_int\": "
      "-123,\"test_uint\":78,\"test_int64_t\":-8223372036854775808,\"test_"
      "uint64_t\":8223372036854775808,\"test_bool\":\"true\",\"test_double\":"
      "123.34}";
  Json::CharReaderBuilder b;
  Json::CharReader* reader(b.newCharReader());
  Json::Value root;
  JSONCPP_STRING errs;
  bool reader_res = reader->parse(str, str + std::strlen(str), &root, &errs);

  //  test
  if (reader_res && errs.size() == 0) {
    std::string resStr =
        restful_common::jsoncpp_helper::GetString(root["test_string"]);
    bool resBool = restful_common::jsoncpp_helper::GetBoolc;
    int resInt = restful_common::jsoncpp_helper::GetInt(root["test_int"]);
    uint32_t resUint =
        restful_common::jsoncpp_helper::GetUint(root["test_uint"]);
    double resDdouble =
        restful_common::jsoncpp_helper::GetDouble(root["test_double"]);
    int64_t resInt64 =
        restful_common::jsoncpp_helper::GetInt64(root["test_int64_t"]);
    uint64_t resUint64 =
        restful_common::jsoncpp_helper::GetUint64(root["test_uint64_t"]);
    delete reader;

    //  out infomation
    std::cout << "string: " << resStr << std::endl;
    std::cout << "bool: " << resBool << std::endl;
    std::cout << "int32: " << resInt << std::endl;
    std::cout << "unsigned int32: " << resUint << std::endl;
    std::cout << "double: " << resDdouble << std::endl;
    std::cout << "int64: " << resInt64 << std::endl;
    std::cout << "unsigned int64: " << resUint64 << std::endl;
  }
  return 0;
}
