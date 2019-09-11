/**
 * @file lib_json_test.cpp
 * @brief code example of jsoncpp from https://github.com/open-source-parsers/jsoncpp/wiki
 * @date 2019-09-11
 * 
 * @copyright Copyright (c) 2019
 * 
 */

/** an example of JSON data:

// Configuration options
{
    // Default encoding for text
    "my-encoding" : "UTF-8",
    
    // Plug-ins loaded at start-up
    "my-plug-ins" : [
        "python",
        "c++",
        "ruby"
        ],
        
    // Tab indent size
    "my-indent" : { "length" : 3, "use_space": true }
}

 */

#include <json/json.h>
#include <iostream>

int main(void){
  Json::Value root;   // starts as "null"; will contain the root value after parsing
  std::cin >> root;

  // Get the value of the member of root named 'my-encoding', return 'UTF-32' if there is no
  // such member.
  std::string my_encoding = root.get("my-encoding", "UTF-32").asString();

  // Get the value of the member of root named 'my-plug-ins'; return a 'null' value if
  // there is no such member.
  const Json::Value my_plugins = root["my-plug-ins"];

  // Make a new JSON document with the new configuration. Preserve original comments.
  std::cout << root << "\n";
  return 0;
}
