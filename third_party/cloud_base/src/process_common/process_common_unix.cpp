//
//  process_common_unix.cpp
//  ExampleMac
//
//  Created by cocoDevil on 2020/2/18.
//  Copyright © 2020 cocoDevil. All rights reserved.
//

#include "process_common_unix.h"
#include "Process-C-Interface.h"
#include <CoreFoundation/CFBundle.h>

namespace cloud_base {
namespace process_common_unix {
    
bool GetCurrentApplicationDataPath(std::string &appdata_path){
    std::string tempPath;
    c_get_appdata_path(tempPath);
    if (tempPath.empty()) {
        appdata_path = "";
        return false;
    }else{
        appdata_path = tempPath.c_str();
        return true;
    }
}

std::string GetCurrentProcessVersion() {
  CFBundleRef ref = CFBundleGetMainBundle();
  // 构建版本
  int build_version = CFBundleGetVersionNumber(ref);
  CFDictionaryRef dict = CFBundleGetInfoDictionary(ref);
  // 版本名
  CFStringRef key_name = CFStringCreateWithCString(
      CFAllocatorGetDefault(), "CFBundleExecutable", kCFStringEncodingUTF8);
  CFStringRef value_name =
      (CFStringRef)CFDictionaryGetValue(dict, (void *)key_name);
  // 版本号
  CFStringRef key_version = CFStringCreateWithCString(
      CFAllocatorGetDefault(), "CFBundleShortVersionString",
      kCFStringEncodingUTF8);

  // 拼接版本号，并做cstring转换。不做拼接直接返回版本号
  // CFMutableStringRef version =
  //     CFStringCreateMutable(CFAllocatorGetDefault(), 20);
  // CFStringAppend(version, value_name);
  // CFStringAppendCString(version, " ", kCFStringEncodingUTF8);
  // CFStringAppend(version, value_version);
  char buf[1025];
  memset(buf, 0, 1025);
  if (CFDictionaryContainsValue(dict, key_version)){
    CFStringRef value_version = (CFStringRef)CFDictionaryGetValue(dict, (void *)key_version);
    CFStringGetCString(value_version, buf, 1024, kCFStringEncodingUTF8);
  }
  return buf;
}

bool get_log_path(std::string &log_path){
    std::string logPath;
    c_get_log_path(logPath);
    if (logPath.empty()) {
        log_path = "";
        return false;
    }else{
        log_path = logPath.c_str();
        return true;
    }
}

}  // namespace process_common_unix
}  // namespace cloud_base
