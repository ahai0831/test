#pragma once
#ifndef FILESYSTEM__FILESYSTEM_H__
#define FILESYSTEM__FILESYSTEM_H__
#include <string>
#include <vector>
namespace cloud_base {
namespace filesystem_helper {
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
}  // namespace filesystem_helper
}  // namespace cloud_base
#endif  // FILESYSTEM__FILESYSTEM_H__
