#pragma once
#ifndef AUTOBACKUP_DATABASE_HELPER_H
#define AUTOBACKUP_DATABASE_HELPER_H

#include <cinttypes>

#include <memory>
#include <string>

#include <sqlite3.h>

namespace EnterpriseCloud {
namespace Sqlite3Helper {
namespace AutobackupMetadataHelper {

class AutobackupMetadataHelper {
 public:
  // 创建元数据数据库，如果数据库没有则创建元数据数据库和
  // 创建metadata_table表来保存元数据。
  // 成功则返回指向一个unique指针，用于对这个数据库的操作，
  // 失败则返回nullptr
  // 参数说明：
  // appdata_apth: AppdataPath
  // user_id: 用户的user id
  // corp_id: 用户的corp id
  // metadata_table的结构：
  // space_type: [integer], 空间类型，不能为空
  // cloud_path: [text，字符串], 备份文件夹云端路径，不能为空
  // local_path: [text，字符串], 备份文件夹本地路径，不能为空
  // cloud_folder_id: [text，字符串], 云端文件夹的folderid，不能为空
  // is_backup:[boolean，实际存储为int], 是否备份，默认值为true
  // coshare_id:[text，字符串], coshareid，默认为字符串
  // work_time: [datatime, 实际存储格式为字符串], 工作时间段，格式为YYYY-MM-DD
  // HH:MM:SS.SSS，默认为空 extrends:[text，字符串], 预留的扩展字段
  // 主键(space_type, cloud_path, local_path)
  static std::unique_ptr<AutobackupMetadataHelper> Create(
      std::string appdata_path, int64_t user_id, int64_t corp_id);

  // 根据主键(space_type, cloud_path,local_path)来插入信息
  // 成功返回空字符串，失败返回错误信息。
  // 参数说明：
  // space_type: 空间类型，不能为空
  // cloud_path: 备份文件夹云端路径，不能为空
  // local_path: 备份文件夹本地路径，不能为空
  // cloud_folder_id: 云端文件夹的folderid，不能为空
  // is_backup: 是否备份，默认值为true
  // coshare_id: coshareid，默认为空
  // work_time: 工作时间段，格式为YYYY-MM-DD HH:MM:SS.SSS，默认为空
  // extrends: 预留的扩展字段
  std::string Insert(int32_t space_type, std::string cloud_path,
                     std::string local_path, std::string cloud_folder_id,
                     bool is_backup = true, std::string coshare_id = "",
                     std::string work_time = "", std::string extrends = "");

  // 根据主键(space_type, cloud_path, local_path)来更新信息。
  // 成功返回空字符串，失败返回错误信息。
  // 参数说明：
  // space_type: 空间类型，不能为空
  // cloud_path: 备份文件夹云端路径，不能为空
  // local_path: 备份文件夹本地路径，不能为空
  // cloud_folder_id: 云端文件夹的folderid，不能为空
  // is_backup: 是否备份，默认值为true
  // coshare_id: coshareid，默认为空
  // work_time: 工作时间段，格式为YYYY-MM-DD HH:MM:SS.SSS，默认为空
  // extrends: 预留的扩展字段
  std::string Update(int32_t space_type, std::string cloud_path,
                     std::string local_path, std::string cloud_folder_id,
                     bool is_backup = true, std::string coshare_id = "",
                     std::string work_time = "", std::string extrends = "");

  // 根据主键(space_type, cloud_path,local_path)来查询其余字段的信息
  // 成功返回json字符串，失败返回字符串"-1"，查询不到信息返回字符串"null"。
  // 返回的json包含字段：
  // space_type: 空间类型，不能为空
  // cloud_path: 备份文件夹云端路径，不能为空
  // local_path: 备份文件夹本地路径，不能为空
  // cloud_folder_id: 云端文件夹的folderid，字符串类型
  // is_backup: 是否备份，int类型
  // coshare_id: coshareid，字符串类型
  // work_time: 工作时间段，格式为YYYY-MM-DD HH:MM:SS.SSS，字符串类型
  // extrends: 预留的扩展字段，字符串类型
  std::string Query();

  // 根据主键(space_type, cloud_path,local_path)来查询其余字段的信息
  // 成功返回json字符串，失败返回字符串"-1"，查询不到信息返回字符串"null"。
  // 参数说明：
  // space_type: 空间类型，不能为空
  // cloud_path: 备份文件夹云端路径，不能为空
  // local_path: 备份文件夹本地路径，不能为空
  // 返回的json包含字段：
  // cloud_folder_id: 云端文件夹的folderid，字符串类型
  // is_backup: 是否备份，int类型
  // coshare_id: coshareid，字符串类型
  // work_time: 工作时间段，格式为YYYY-MM-DD HH:MM:SS.SSS，字符串类型
  // extrends: 预留的扩展字段，字符串类型
  std::string Query(int32_t space_type, std::string cloud_path,
                    std::string local_path);

  // 根据主键(space_type, cloud_path,local_path)来删除信息
  // 成功返回空字符串，失败返回错误信息。
  // 参数说明：
  // space_type: 空间类型，不能为空
  // cloud_path: 备份文件夹云端路径，不能为空
  // local_path: 备份文件夹本地路径，不能为空
  std::string Delete(int32_t space_type, std::string cloud_path,
                     std::string local_path);

  //////////////////////////////////////////////////////////////////////////
  /// 以下为全局备份信息表的函数
  //////////////////////////////////////////////////////////////////////////

  // 插入
  std::string InsertToBITable(std::string backup_time, int32_t backup_status,
                              std::string extrends = "");
  // 根据主键backup_time更新
  std::string UpdateBITable(std::string backup_time, int32_t backup_status,
                            std::string extrends = "");
  // 查询表内的全部数据
  std::string QueryFormBITable();
  // 根据主键backup_time查询数据
  std::string QueryFormBITable(std::string backup_time);
  // 根据数据backup_time删除数据
  std::string DeleteFormBITable(std::string backup_time);

  ~AutobackupMetadataHelper();

 protected:
  sqlite3 *GetSqlite3Handle();

  AutobackupMetadataHelper(std::string appdata_path, int64_t user_id,
                           int64_t corp_id);

  AutobackupMetadataHelper() = delete;
  AutobackupMetadataHelper(const AutobackupMetadataHelper &) = delete;
  AutobackupMetadataHelper(AutobackupMetadataHelper &&) = delete;
  AutobackupMetadataHelper &operator=(const AutobackupMetadataHelper &) =
      delete;

 private:
  sqlite3 *sqlite3_handle_;
};

}  // namespace AutobackupMetadataHelper
}  // namespace Sqlite3Helper
}  // namespace EnterpriseCloud

#endif
