#pragma once
#ifndef AUTOBACKUP_BACKUP_HELPER_H
#define AUTOBACKUP_BACKUP_HELPER_H

#include <cinttypes>

#include <memory>
#include <string>

#include <sqlite3.h>

namespace EnterpriseCloud {
namespace Sqlite3Helper {
namespace AutobackupBackupHelper {

class AutobackupBackupHelper {
 public:
  // 创建文件夹备份信息数据库，如果数据库没有创建和
  // 创建upload_seccess_table(文件上传成功),
  // uploading_files_table(文件正在上传),
  // already_scan_table(已扫描的文件夹)三张表来保存元数据。
  // 目前只建了upload_seccess_table,uploading_files_table表,already_scan_table表未建立。
  // 成功则返回指向一个不为nullptr的unique指针，用于对这个数据库的操作，
  // 失败则返回nullptr的unique指针
  // 参数说明：
  // appdata_apth: AppdataPath
  // user_id: 用户的user id
  // corp_id: 用户的corp id
  // space_type: 空间类型，不能为空
  // cloud_path: 备份文件夹云端路径，不能为空
  // local_path: 备份文件夹本地路径，不能为空
  //
  // upload_seccess_table的结构：
  // file_local_path: [text，字符串]，已完成备份的文件的本地路径，不能为空
  // file_cloud_path: [text，字符串]，已完成备份的文件的云端路径，不能为空
  // md5: [text，字符串]，已完成备份的文件的md5，不能为空
  // is_backup: [boolean，实际存储为int], 是否已备份, 默认为true，可以为空
  // last_change_date: [datatime, 实际存储格式为字符串],
  // 文件的最后修改时间，格式为YYYY-MM-DD HH:MM:SS.SSS，默认为空
  // cloud_parent_folder_id:[integer], 对于云端的parentfolderid
  // cloud_file_id:[integer], 对应云盘的fileid
  // extrends: [text，字符串],预留的扩展字段
  // 主键(file_local_path)
  //
  // uploading_files_table的结构
  // file_local_path：[text，字符串]，正在上传文件的本地路径，不能为空
  // file_cloud_path：[text，字符串]，正在上传文件的云端路径，不能为空
  // upload_file_id：[text，字符串]，正在上传文件的uploadfileid，不能为空
  // coshare_id [text，字符串]，正在上传的文件的conshare id，可以传入空字符串""
  // cloud_parent_folder_id：[integer], 正在上传的文件的父文件夹的id，不能为空
  // file_source：[integer], 文件夹类型：
  // 1-企业空间
  // 2-协作文件夹
  // 3-工作空间
  // 主键(file_local_path)
  static std::unique_ptr<AutobackupBackupHelper> Create(
      std::string appdata_path, int64_t user_id, int64_t corp_id,
      int32_t space_type, std::string cloud_path, std::string local_path);

  // 根据主键(upload_seccess_table)来向upload_seccess_table表插入信息，file_path和md5必须非空。
  // 成功返回空字符串，失败返回错误信息。
  // 参数说明：
  // file_local_path: 已完成备份的文件的本地路径，不能为空
  // file_cloud_path: 已完成备份的文件的云端路径，不能为空
  // md5: 已完成备份的文件的md5，不能为空
  // is_backup: 是否已备份, 默认为true，可以为空
  // last_change_date: 文件的最后修改时间，格式为YYYY-MM-DD
  // HH:MM:SS.SSS，默认为空 cloud_parent_folder_id: 对于云端的parentfolderid
  // cloud_file_id: 对应云盘的fileid
  // extrends: 预留的扩展字段
  std::string InsertToUSTable(std::string file_local_path,
                              std::string file_cloud_path, std::string md5,
                              bool is_backup = true,
                              std::string last_change_date = "",
                              int64_t cloud_parent_folder_id = 0,
                              int64_t cloud_file_id = 0,
                              std::string extrends = "");

  // 根据主键(upload_seccess_table)来向upload_seccess_table表更新表的信息，file_path和md5必须非空。
  // 成功返回空字符串，失败返回错误信息。
  // 参数说明：
  // file_local_path: 已完成备份的文件的本地路径，不能为空
  // file_cloud_path: 已完成备份的文件的云端路径，不能为空
  // md5: 已完成备份的文件的md5，不能为空
  // is_backup: 是否已备份, 默认为true，可以为空
  // last_change_date: 文件的最后修改时间，格式为YYYY-MM-DD
  // HH:MM:SS.SSS，默认为空 cloud_parent_folder_id: 对于云端的parentfolderid
  // cloud_file_id: 对应云盘的fileid
  // extrends: 预留的扩展字段
  std::string UpdateUSTable(std::string file_local_path,
                            std::string file_cloud_path, std::string md5,
                            bool is_backup = true,
                            std::string last_change_date = "",
                            int64_t cloud_parent_folder_id = 0,
                            int64_t cloud_file_id = 0,
                            std::string extrends = "");

  // 根据主键(upload_seccess_table)从upload_seccess_table表中查询其余字段的信息
  // 成功返回json字符串，失败返回字符串"-1"，查询不到信息返回字符串"null"。
  // 参数说明：
  // file_local_path: 已完成备份的文件的本地路径，不能为空
  // 返回的json字段：
  // file_cloud_path: 已完成备份的文件的云端路径，字符串类型
  // md5: 已完成备份的文件的md5，字符串类型
  // is_backup: 是否已备份, int类型
  // last_change_date: 文件的最后修改时间，格式为YYYY-MM-DD
  // HH:MM:SS.SSS，字符串类型 cloud_parent_folder_id:
  // 对于云端的parentfolderid，int64字符串类型 cloud_file_id:
  // 对应云盘的fileid，int64类型 extrends: 预留的扩展字段，字符串类型
  std::string QueryFromUSTable(std::string file_local_path);

  // 根据主键(file_local_path)来向uploading_files_table表插入信息。
  // 成功返回空字符串，失败返回错误信息。
  // 传入参数说明：
  // file_local_path：正在上传文件的本地路径，不能为空
  // file_cloud_path：正在上传文件的云端路径，不能为空
  // upload_file_id：正在上传文件的uploadfileid，不能为空
  // coshare_id：正在上传的文件的conshare id，可以传入空字符串""
  // cloud_parent_folder_id：正在上传的文件的父文件夹的id，不能为空
  // file_source：文件夹类型：
  // 1-企业空间
  // 2-协作文件夹
  // 3-工作空间
  std::string InsertToUFTable(std::string file_local_path,
                              std::string file_cloud_path,
                              std::string upload_file_id,
                              std::string coshare_id,
                              int64_t cloud_parent_folder_id,
                              int32_t file_source);

  // 根据主键(file_local_path)来对uploading_files_table表更新信息。
  // 成功返回空字符串，失败返回错误信息。
  // 传入参数说明：
  // file_local_path：正在上传文件的本地路径，不能为空
  // file_cloud_path：正在上传文件的云端路径，不能为空
  // upload_file_id：正在上传文件的uploadfileid，不能为空
  // coshare_id：正在上传的文件的conshare id，可以传入空字符串""
  // cloud_parent_folder_id：正在上传的文件的父文件夹的id，不能为空
  // file_source：文件夹类型：
  // 1-企业空间
  // 2-协作文件夹
  // 3-工作空间
  std::string UpdateUFTable(std::string file_local_path,
                            std::string file_cloud_path,
                            std::string upload_file_id, std::string coshare_id,
                            int64_t cloud_parent_folder_id,
                            int32_t file_source);

  // 根据主键(file_local_path)来在uploading_files_table表中查询信息。
  // 成功返回json字符串，失败返回字符串"-1"，查询不到信息返回字符串"null"。
  // 返回的json字符串字段：
  // file_local_path：正在上传文件的本地路径，不能为空
  // file_cloud_path：正在上传文件的云端路径，字符串类型
  // upload_file_id：正在上传文件的uploadfileid，字符串类型
  // coshare_id：正在上传的文件的conshare id，字符串类型
  // cloud_parent_folder_id：正在上传的文件的父文件夹的id，int64_t类型
  // file_source：文件夹类型，int类型
  std::string QueryFromUFTable();

  // 根据主键(file_local_path)来在uploading_files_table表中查询信息。
  // 成功返回json字符串，失败返回字符串"-1"，查询不到信息返回字符串"null"。
  // 传入参数说明：
  // file_local_path：正在上传文件的本地路径，不能为空
  // 返回的json字符串字段：
  // file_cloud_path：正在上传文件的云端路径，字符串类型
  // upload_file_id：正在上传文件的uploadfileid，字符串类型
  // coshare_id：正在上传的文件的conshare id，字符串类型
  // cloud_parent_folder_id：正在上传的文件的父文件夹的id，int64_t类型
  // file_source：文件夹类型，int类型
  std::string QueryFromUFTable(std::string file_local_path);

  // 根据主键(file_local_path)来在uploading_files_table表中删除信息
  // 成功返回空字符串，失败返回错误信息。
  // 传入参数说明：
  // file_local_path：正在上传文件的本地路径，不能为空
  std::string DeleteFromUFTable(std::string file_local_path);

  ~AutobackupBackupHelper();

 protected:
  sqlite3 *GetSqlite3Handle();

  AutobackupBackupHelper(std::string appdata_path, int64_t user_id,
                         int64_t corp_id, int32_t space_type,
                         std::string cloud_path, std::string local_path);

  AutobackupBackupHelper() = delete;
  AutobackupBackupHelper(const AutobackupBackupHelper &) = delete;
  AutobackupBackupHelper(AutobackupBackupHelper &&) = delete;
  AutobackupBackupHelper &operator=(const AutobackupBackupHelper &) = delete;

 private:
  sqlite3 *sqlite3_handle_;
};

}  // namespace AutobackupBackupHelper
}  // namespace Sqlite3Helper
}  // namespace EnterpriseCloud

#endif
