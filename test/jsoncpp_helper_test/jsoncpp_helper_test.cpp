#include <iostream>

#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

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
    bool resBool = restful_common::jsoncpp_helper::GetBool(root["test_bool"]);
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

  // test reader helper and writer helper
  std::cout << std::endl << "test reader helper" << std::endl;
  Json::Value test_reader_helper;
  if (restful_common::jsoncpp_helper::ReaderHelper(str, test_reader_helper)) {
    std::string testStr = restful_common::jsoncpp_helper::GetString(
        test_reader_helper["test_string"]);
    bool testBool = restful_common::jsoncpp_helper::GetBool(
        test_reader_helper["test_bool"]);
    int testInt =
        restful_common::jsoncpp_helper::GetInt(test_reader_helper["test_int"]);
    uint32_t testUint = restful_common::jsoncpp_helper::GetUint(
        test_reader_helper["test_uint"]);
    double testDdouble = restful_common::jsoncpp_helper::GetDouble(
        test_reader_helper["test_double"]);
    int64_t testInt64 = restful_common::jsoncpp_helper::GetInt64(
        test_reader_helper["test_int64_t"]);
    uint64_t testUint64 = restful_common::jsoncpp_helper::GetUint64(
        test_reader_helper["test_uint64_t"]);
    std::cout << "string: " << testStr << std::endl;
    std::cout << "bool: " << testBool << std::endl;
    std::cout << "int32: " << testInt << std::endl;
    std::cout << "unsigned int32: " << testUint << std::endl;
    std::cout << "double: " << testDdouble << std::endl;
    std::cout << "int64: " << testInt64 << std::endl;
    std::cout << "unsigned int64: " << testUint64 << std::endl;
  }

  std::cout << std::endl << "test writer helper" << std::endl;
  std::string test_writer_helper_str;
  restful_common::jsoncpp_helper::WriterHelper(test_reader_helper,
                                               test_writer_helper_str);
  std::cout << test_writer_helper_str << std::endl;
  std::cout << restful_common::jsoncpp_helper::WriterHelper(test_reader_helper)
            << std::endl;
  return 0;
}
