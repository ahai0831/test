#pragma once
#ifndef FILECOMMON_WIN_H__
#define FILECOMMON_WIN_H__
#include <cinttypes>
#include <string>
#include <vector>
namespace cloud_base {
namespace filecommon_win {
//  input a path and then Determine if the path exists or not
//  1.if the path exist and write a temp file to it successfully,return true;
//  2.if the path not exist,create it and then write a temp file to it,return
//  true if create and write.
//  3.if the path not exist and create it or write a temp file to it
//  failed,return false.
bool guarantee_directory_exists(const std::wstring& dir_path);
//  create a input path,return true if create successfully,return false if
//  failed
bool create_dir(const std::wstring& dir);
//  get the application temp path and write it to reference
//  parameter(appdata_path)
bool get_appdata_path(std::wstring& appdata_path);
//  input a path(dirPath) and a file type(suffix),get all files in the path
//  match the file type then write them to the reference parameter(vec)
bool get_file_list(const std::wstring& dirPath, const std::wstring& suffix,
                   std::vector<std::wstring>& vec);
//  input a file path,get and write the file name to the reference
//  parameter(file_name)
//  return true if get file name successful,return false if failed
bool GetFileName(const std::wstring& file_path, std::wstring& file_name);
//  input a file path,get and write the file size to the reference
//  parameter(file_size)
//  return true if get size name successful,return false if failed
bool GetFileSize(const std::wstring& file_path, uint64_t& file_size);
//  input a file path,get and write the last change date to the reference
//  parameter(file_modify_data)
//  return true if get file last modify date successful,return false if failed
bool GetFileLastChange(const std::wstring& file_path,
                       std::string& file_modify_date);
}  // namespace filecommon_win
}  // namespace cloud_base
#endif  // FILECOMMON_WIN_H__
