#include <cinttypes>

#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>

#include "enterprise_cloud/sqlite3_helper/autobackup_meatadata_helper/autobackup_meatadata_helper.h"
using EnterpriseCloud::Sqlite3Helper::AutobackupMetadataHelper::
    AutobackupMetadataHelper;

#include "enterprise_cloud/sqlite3_helper/autobackup_backup_helper/autobackup_backup_helper.h"
using EnterpriseCloud::Sqlite3Helper::AutobackupBackupHelper::
    AutobackupBackupHelper;

int main() {
  std::string appdata_path = "E:\\Desktop\\appdata\\";
  int64_t user_id = 123;
  int64_t corp_id = 321;

  // 测试元数据数据库的接口
  std::cout << "======test metadata table======" << std::endl;
  int32_t space_type = 1;
  std::string cloud_path = "\\backup\\test\\";
  std::string local_path = "E:\\Desktop\\backup_test\\";

  auto test_ptr =
      AutobackupMetadataHelper::Create(appdata_path, user_id, corp_id);
  if (test_ptr == nullptr) {
    std::cout << "test1 : create database fail." << std::endl;
  }

  std::string errmsg, query_result;
  // 插入数据1
  errmsg = test_ptr->Insert(space_type, cloud_path, local_path, "3234324");
  if (!errmsg.empty()) {
    std::cout << "insert data fail: " << errmsg << std::endl;
  }
  // 插入数据2
  errmsg = test_ptr->Insert(space_type + 1, cloud_path, local_path, "3234324");
  if (!errmsg.empty()) {
    std::cout << "insert data fail: " << errmsg << std::endl;
  }
  // 更新数据
  errmsg = test_ptr->Update(space_type, cloud_path, local_path, "32343232");
  if (!errmsg.empty()) {
    std::cout << "test1 : update data fail: " << errmsg << std::endl;
  }
  // 查询表中数据
  query_result = test_ptr->Query();
  if (query_result.empty()) {
    std::cout << "test1 : query data fail. " << std::endl;
  } else if (query_result == "-1") {
    std::cout << "test1 : query data fail, no data." << std::endl;
  } else {
    Json::Value query_result_json;
    Json::Reader json_reader;
    json_reader.parse(query_result, query_result_json);
    auto list = query_result_json.getMemberNames();
    for (auto &iter : list) {
      std::cout << iter << ": " << query_result_json[iter] << std::endl;
    }
  }
  // 根据主键查询数据
  query_result = test_ptr->Query(space_type, cloud_path, local_path);
  if (query_result.empty()) {
    std::cout << "test1 : query data fail. " << std::endl;
  } else if (query_result == "-1") {
    std::cout << "test1 : query data fail, no data." << std::endl;
  } else {
    Json::Value query_result_json;
    Json::Reader json_reader;
    json_reader.parse(query_result, query_result_json);
    auto list = query_result_json.getMemberNames();
    for (auto &iter : list) {
      std::cout << iter << ": " << query_result_json[iter] << std::endl;
    }
  }
  // 根据主键删除数据
  errmsg = test_ptr->Delete(space_type, cloud_path, local_path);
  if (!errmsg.empty()) {
    std::cout << "test1 : delete data fail: " << errmsg << std::endl;
  }

  // 测试备份完成三张表的接口，目前只有备份完成和正在上传的两张表的接口
  auto test_ptr_2 = AutobackupBackupHelper::Create(
      appdata_path, user_id, corp_id, space_type, cloud_path, local_path);
  if (test_ptr_2 == nullptr) {
    std::cout << "test2 : create database fail." << std::endl;
  }

  // 测试备份完成表的接口，表(upload_seccess_table)的缩写是USTable
  std::cout << "======test USTable======" << std::endl;

  std::string file_local_path = "E:\\Desktop\\backup_test\\test.txt";
  std::string file_cloud_path = "\\backup\\test\\test.txt";
  std::string file_md5 = "4599b9be704277ee700f1295aee0c2e8";
  std::string file_last_change_time = "2019-11-08 16:45:11.019";
  // 插入数据1
  std::string errmsg_2, query_result_2;
  errmsg_2 = test_ptr_2->InsertToUSTable(file_local_path, file_cloud_path,
                                         file_md5, true, file_last_change_time);
  if (!errmsg_2.empty()) {
    std::cout << "test2 : insert data to USTable fail: " << errmsg_2
              << std::endl;
  }
  // 更新数据
  errmsg_2 = test_ptr_2->UpdateUSTable(file_local_path, file_cloud_path,
                                       file_md5, false, file_last_change_time);
  if (!errmsg_2.empty()) {
    std::cout << "test2 : update USTable data fail: " << errmsg_2 << std::endl;
  }
  // 根据主键查询数据
  query_result_2 = test_ptr_2->QueryFromUSTable(file_local_path);
  if (query_result_2.empty()) {
    std::cout << "test2 : query data from USTable fail. " << std::endl;
  } else if (query_result_2 == "-1") {
    std::cout << "test2 : query data from USTable fail, no data." << std::endl;
  } else {
    Json::Value query_result_json;
    Json::Reader json_reader;
    json_reader.parse(query_result_2, query_result_json);
    auto list = query_result_json.getMemberNames();
    for (auto &iter : list) {
      std::cout << iter << ": " << query_result_json[iter] << std::endl;
    }
  }

  // 测试正在上传表的接口，表(uploading_files_table)的缩写是UFTable
  std::cout << "======test UFTable======" << std::endl;
  int64_t file_size = 44;
  std::string upload_file_id = "234234234";
  int64_t parent_id = 23424343;
  int32_t file_source = 1;
  // 插入数据1
  errmsg_2 = test_ptr_2->InsertToUFTable(file_local_path, file_cloud_path,
                                         file_size, file_md5, upload_file_id,
                                         "", parent_id, file_source);
  if (!errmsg_2.empty()) {
    std::cout << "test2 : insert data to UFTable fail: " << errmsg_2
              << std::endl;
  }
  // 插入数据2
  errmsg_2 = test_ptr_2->InsertToUFTable(
      file_local_path + "test2", file_cloud_path, file_size, file_md5,
      upload_file_id, "", parent_id, file_source);
  if (!errmsg_2.empty()) {
    std::cout << "test2 : insert data to UFTable fail: " << errmsg_2
              << std::endl;
  }
  // 更新数据
  errmsg_2 = test_ptr_2->UpdateUFTable(file_local_path, file_cloud_path,
                                       file_size, file_md5, upload_file_id,
                                       "234324", parent_id, file_source);
  if (!errmsg_2.empty()) {
    std::cout << "test2 : update UFTable data fail: " << errmsg_2 << std::endl;
  }
  // 查询表中全部数据
  query_result_2 = test_ptr_2->QueryFromUFTable();
  if (query_result_2.empty()) {
    std::cout << "test2 : query data from USTable fail. " << std::endl;
  } else if (query_result_2 == "-1") {
    std::cout << "test2 : query data from USTable fail, no data." << std::endl;
  } else {
    Json::Value query_result_json;
    Json::Reader json_reader;
    json_reader.parse(query_result_2, query_result_json);
    auto list = query_result_json.getMemberNames();
    for (auto &iter : list) {
      std::cout << iter << ": " << query_result_json[iter] << std::endl;
    }
  }
  // 根据主键查询数据
  query_result_2 = test_ptr_2->QueryFromUFTable(file_local_path);
  if (query_result_2.empty()) {
    std::cout << "test2 : query data from USTable fail. " << std::endl;
  } else if (query_result_2 == "-1") {
    std::cout << "test2 : query data from USTable fail, no data." << std::endl;
  } else {
    Json::Value query_result_json;
    Json::Reader json_reader;
    json_reader.parse(query_result_2, query_result_json);
    auto list = query_result_json.getMemberNames();
    for (auto &iter : list) {
      std::cout << iter << ": " << query_result_json[iter] << std::endl;
    }
  }
  // 根据主键删除数据
  errmsg_2 = test_ptr_2->DeleteFromUFTable(file_local_path);
  if (!errmsg.empty()) {
    std::cout << "test1 : delete data from USTable fail: " << errmsg
              << std::endl;
  }
  return 0;
}
