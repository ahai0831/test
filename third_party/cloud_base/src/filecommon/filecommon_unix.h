#pragma once
#include <cinttypes>
#include <string>
#include <vector>
namespace cloud_base {
namespace filecommon_unix {

//  input a file path,get and write the file name to the reference
//  parameter(file_name)
//  return true if get file name successful,return false if failed
bool GetFileName(const std::string file_path, std::string& file_name);

// input a file path.get file last change time
bool GetFileLastChange(const char* file_path, std::string& file_modify_date);

// input a file path.
bool GetFileSize(const std::string& file_path, uint64_t& file_size);

//  input a path and then Determine if the path exists or not
//  1.if the path exist and write a temp file to it successfully,return true;
//  2.if the path not exist,create it and then write a temp file to it,return
//  true if create and write.
//  3.if the path not exist and create it or write a temp file to it
//  failed,return false.
bool guarantee_directory_exists(const std::string& dir_path);

//  input a path(dirPath) and a file type(suffix),get all files in the path
//  match the file type then write them to the reference parameter(vec)
bool get_file_list(const std::string& dirPath, const std::string& suffix,
                   std::vector<std::string>& vec);

// Get current process version
// return the version infomation if success
// return "" if failed
// 移动到process_common_unix
// std::string GetCurrentProcessVersion();

// get Mac address
std::string get_mac_address(void);
}  // namespace filecommon_unix
}  // namespace cloud_base
