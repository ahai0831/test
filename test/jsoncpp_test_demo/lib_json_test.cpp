/**
 * @file lib_json_test.cpp
 * @brief test jsoncpp.
 * @date 2019-09-11
 *
 * @copyright Copyright (c) 2019
 *
 */

/** an example of JSON data:

// example
{
    // comment
    "json_name" : "jsoncpp_test",
    

    // comment
    "array_test" : [
        "element1",
        "element2",
        "element3"
        ],
        


    // comment
    "object_test" : { "name" : "小明明", "edge": 6, "man": true }
}

 */

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <json/json.h>

int main(void) {
  Json::Value
      root;  // starts as "null"; will contain the root value after parsing
  std::cin >>
      root;  // support iostream, input the example json and Ctrl + z end input.
  std::cout << root << std::endl;

  std::cout
      << "===============================1==============================\n";

  // ***read data test***
  std::cout << root["json_name"].asString() << std::endl;   // std::string
  std::cout << root["json_name"].asCString() << std::endl;  // char *

  // array
  Json::Value array_test = root["array_test"];
  for (const auto &i : array_test) {
    std::cout << i.asString() << std::endl;
  }
  // object
  Json::Value object_test = root["object_test"];
  std::cout << "name: " << object_test["name"].asString() << std::endl;
  std::cout << "edge: " << object_test["edge"].asString() << std::endl;
  std::cout << "man: " << object_test["man"].asString() << std::ends
            << object_test["man"].asBool() << std::endl;

  std::cout
      << "===============================2==============================\n";

  // ***write data test***
  Json::Value value_test;

  value_test["string_test1"] = std::string("string_test1");
  value_test["string_test2"] = "string_test2";
  // array
  value_test["array_test"].append("array_test1");
  value_test["array_test"].append("array_test2");
  // object
  value_test["root"] = root;
  std::cout << value_test << std::endl;

  std::cout
      << "===============================3==============================\n";

  // input and output will be same as using asString and asCString
  // although jsoncpp save Chinese with Unicode.
  std::cout << root["object_test"]["name"].asString() << std::endl;
  std::cout << root["object_test"]["name"].asCString() << std::endl;
  std::cout << root["object_test"]["name"] << std::endl;

  std::cout
      << "===============================4==============================\n";

  std::string str = "{\"fileName\":\"文件名\"}";
  // ***jsoncpp old api for read and write string, and use old api will have
  // warning. =_=*** You should be careful that Chinese is save with Unicode in
  // json file string. read
  Json::Reader reader;
  reader.parse(str, root);
  std::cout << root << std::endl;

  // write
  // this no format, no space.
  Json::FastWriter fast_writer;
  std::string json_file = fast_writer.write(root);
  std::cout << json_file << std::endl;

  // this format, it will add \n and space.
  Json::StyledWriter style_writer;
  json_file = style_writer.write(root);
  std::cout << json_file << std::endl;

  // it also have another method to get a json file string
  // but it's a bit awful, this string have \n and \t.
  json_file = root.toStyledString();
  std::cout << json_file << std::endl;

  std::cout
      << "===============================5==============================\n";

  // ***jsoncpp new api for read and write string.***
  // read
  Json::CharReaderBuilder crb;
  std::unique_ptr<Json::CharReader> read_ptr(crb.newCharReader());
  read_ptr->parse(str.c_str(), str.c_str() + str.length(), &root, NULL);

  // write
  Json::StreamWriterBuilder swb;
  swb["indentation"] =
      "";  // not format, default is \t, same as toStyledString().
  std::unique_ptr<Json::StreamWriter> write_ptr(swb.newStreamWriter());

  std::ostringstream oss;
  write_ptr->write(root, &oss);
  std::cout << oss.str() << std::endl;

  return 0;
}
