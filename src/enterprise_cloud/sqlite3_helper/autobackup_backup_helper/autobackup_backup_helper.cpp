#include "autobackup_backup_helper.h"

#include <json/json.h>

#include <tools/string_format.hpp>
using assistant::tools::string::StringFormat;

#include <tools/string_convert.hpp>
using assistant::tools::string::utf8ToWstring;

#include <HashAlgorithm/md5.h>
#include <filesystem_helper/filesystem_helper.h>

namespace EnterpriseCloud {
namespace Sqlite3Helper {
namespace AutobackupBackupHelper {

std::unique_ptr<AutobackupBackupHelper> AutobackupBackupHelper::Create(
    std::string appdata_path, int64_t user_id, int64_t corp_id,
    int32_t space_type, std::string cloud_path, std::string local_path) {
  auto database_ptr = new AutobackupBackupHelper(
      appdata_path, user_id, corp_id, space_type, cloud_path, local_path);
  if (nullptr == database_ptr->GetSqlite3Handle()) {
    database_ptr == nullptr;
  }
  return std::unique_ptr<AutobackupBackupHelper>(database_ptr);
}

std::string AutobackupBackupHelper::InsertToUSTable(
    std::string file_local_path, std::string file_cloud_path, std::string md5,
    bool is_backup /*=true*/, std::string last_change_date /*=""*/,
    int64_t cloud_parent_folder_id /*=0*/, int64_t cloud_file_id /*=0*/,
    std::string extrends /*=""*/) {
  char *errmsg = nullptr;
  std::string error_msg;
  do {
    if (sqlite3_handle_ == nullptr || file_local_path.empty() ||
        file_cloud_path.empty() || md5.empty()) {
      error_msg = "params error.";
      break;
    }
    std::string sql_string = StringFormat(
        "INSERT INTO upload_seccess_table"
        "(file_local_path,file_cloud_path,md5,is_backup,last_change_date,"
        "cloud_parent_folder_id,"
        "cloud_file_id,"
        "extrends)"
        " VALUES('%s','%s','%s',%s,'%s',%s,%s,'%s');",
        file_local_path.c_str(), file_cloud_path.c_str(), md5.c_str(),
        std::to_string(is_backup).c_str(), last_change_date.c_str(),
        std::to_string(cloud_parent_folder_id).c_str(),
        std::to_string(cloud_file_id).c_str(), extrends.c_str());
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "BEGIN TRANSACTION", nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, sql_string.c_str(), nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      sqlite3_exec(sqlite3_handle_, "ROLLBACK TRANSACTION", nullptr, nullptr,
                   nullptr);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "COMMIT TRANSACTION",
                                  nullptr, nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
  } while (false);

  return error_msg;
}

std::string AutobackupBackupHelper::UpdateUSTable(
    std::string file_local_path, std::string file_cloud_path, std::string md5,
    bool is_backup /*= true*/, std::string last_change_date /*= ""*/,
    int64_t cloud_parent_folder_id /*= 0*/, int64_t cloud_file_id /*= 0*/,
    std::string extrends /*= ""*/) {
  char *errmsg = nullptr;
  std::string error_msg;
  do {
    if (sqlite3_handle_ == nullptr || file_local_path.empty() ||
        file_cloud_path.empty() || md5.empty()) {
      error_msg = "params error.";
      break;
    }
    std::string sql_string = StringFormat(
        "UPDATE upload_seccess_table "
        "SET "
        "file_cloud_path='%s',"
        "md5='%s',is_backup=%s,last_change_date='%s',cloud_parent_folder_id=%s,"
        "cloud_file_id=%s,"
        "extrends='%s' "
        "WHERE file_local_path='%s';",
        file_cloud_path.c_str(), md5.c_str(), std::to_string(is_backup).c_str(),
        last_change_date.c_str(),
        std::to_string(cloud_parent_folder_id).c_str(),
        std::to_string(cloud_file_id).c_str(), extrends.c_str(),
        file_local_path.c_str());
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "BEGIN TRANSACTION", nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, sql_string.c_str(), nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      sqlite3_exec(sqlite3_handle_, "ROLLBACK TRANSACTION", nullptr, nullptr,
                   nullptr);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "COMMIT TRANSACTION",
                                  nullptr, nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
  } while (false);

  return error_msg;
}

std::string AutobackupBackupHelper::QueryFromUSTable(
    std::string file_local_path) {
  std::string result;
  Json::Value result_json;
  do {
    if (sqlite3_handle_ == nullptr || file_local_path.empty()) {
      break;
    }
    std::string sql_string = StringFormat(
        "SELECT "
        "file_cloud_path, "
        "md5,is_backup,last_change_date,cloud_parent_folder_id,cloud_file_id,"
        "extrends FROM upload_seccess_table "
        "WHERE file_local_path='%s';",
        file_local_path.c_str());
    sqlite3_stmt *stmt;
    if (SQLITE_OK != sqlite3_prepare_v2(sqlite3_handle_, sql_string.c_str(), -1,
                                        &stmt, nullptr)) {
      break;
    }

    int ret;
    ret = sqlite3_step(stmt);
    if (SQLITE_ROW == ret) {
      result_json["file_cloud_path"] = (char *)sqlite3_column_text(stmt, 0);
      result_json["md5"] = (char *)sqlite3_column_text(stmt, 1);
      result_json["is_backup"] = sqlite3_column_int(stmt, 2);
      result_json["last_change_date"] = (char *)sqlite3_column_text(stmt, 3);
      result_json["cloud_parent_folder_id"] = sqlite3_column_int64(stmt, 4);
      result_json["cloud_file_id"] = sqlite3_column_int64(stmt, 5);
      result_json["extrends"] = (char *)sqlite3_column_text(stmt, 5);
    } else if (SQLITE_DONE == ret) {
      result = "-1";
      break;
    } else {
      break;
    }
    result = result_json.toStyledString();
  } while (false);
  return result;
}

std::string AutobackupBackupHelper::InsertToUFTable(
    std::string file_local_path, std::string file_cloud_path,
    std::string upload_file_id, std::string coshare_id,
    int64_t cloud_parent_folder_id, int32_t file_source) {
  char *errmsg = nullptr;
  std::string error_msg;
  do {
    if (sqlite3_handle_ == nullptr || file_local_path.empty() ||
        file_cloud_path.empty() || upload_file_id.empty()) {
      error_msg = "params error.";
      break;
    }
    std::string sql_string = StringFormat(
        "INSERT INTO uploading_files_table"
        "(file_local_path,file_cloud_path,upload_file_id,coshare_id,"
        "cloud_parent_folder_id,"
        "file_source)"
        " VALUES('%s','%s','%s','%s',%s,%s);",
        file_local_path.c_str(), file_cloud_path.c_str(),
        upload_file_id.c_str(), coshare_id.c_str(),
        std::to_string(cloud_parent_folder_id).c_str(),
        std::to_string(file_source).c_str());
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "BEGIN TRANSACTION", nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, sql_string.c_str(), nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      sqlite3_exec(sqlite3_handle_, "ROLLBACK TRANSACTION", nullptr, nullptr,
                   nullptr);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "COMMIT TRANSACTION",
                                  nullptr, nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
  } while (false);

  return error_msg;
}

std::string AutobackupBackupHelper::UpdateUFTable(
    std::string file_local_path, std::string file_cloud_path,
    std::string upload_file_id, std::string coshare_id,
    int64_t cloud_parent_folder_id, int32_t file_source) {
  char *errmsg = nullptr;
  std::string error_msg;
  do {
    if (sqlite3_handle_ == nullptr || file_local_path.empty() ||
        file_cloud_path.empty() || upload_file_id.empty()) {
      error_msg = "params error.";
      break;
    }
    std::string sql_string = StringFormat(
        "UPDATE uploading_files_table "
        "SET file_cloud_path='%s',upload_file_id='%s',coshare_id='%s', "
        "cloud_parent_folder_id=%s, "
        "file_source=%s "
        "WHERE file_local_path='%s';",
        file_cloud_path.c_str(), upload_file_id.c_str(), coshare_id.c_str(),
        std::to_string(cloud_parent_folder_id).c_str(),
        std::to_string(file_source).c_str(), file_local_path.c_str());
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "BEGIN TRANSACTION", nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, sql_string.c_str(), nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      sqlite3_exec(sqlite3_handle_, "ROLLBACK TRANSACTION", nullptr, nullptr,
                   nullptr);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "COMMIT TRANSACTION",
                                  nullptr, nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
  } while (false);

  return error_msg;
}

std::string AutobackupBackupHelper::QueryFromUFTable() {
  std::string result;
  Json::Value result_json;
  do {
    if (sqlite3_handle_ == nullptr) {
      break;
    }
    std::string sql_string = "SELECT * FROM uploading_files_table;";
    sqlite3_stmt *stmt;
    if (SQLITE_OK != sqlite3_prepare_v2(sqlite3_handle_, sql_string.c_str(), -1,
                                        &stmt, nullptr)) {
      break;
    }

    int ret;
    int result_count = 0;
    bool while_flag = false;
    Json::Value result_json_temp;
    while (true) {
      ret = sqlite3_step(stmt);
      if (SQLITE_ROW == ret) {
        result_json_temp["file_local_path"] =
            (char *)sqlite3_column_text(stmt, 0);
        result_json_temp["file_cloud_path"] =
            (char *)sqlite3_column_text(stmt, 1);
        result_json_temp["upload_file_id"] =
            (char *)sqlite3_column_text(stmt, 2);
        result_json_temp["coshare_id"] = (char *)sqlite3_column_text(stmt, 3);
        result_json_temp["cloud_parent_folder_id"] =
            sqlite3_column_int64(stmt, 4);
        result_json_temp["file_source"] = sqlite3_column_int(stmt, 5);

        result_json[std::to_string(result_count)] = result_json_temp;
        result_json_temp.clear();
        result_count++;
      } else if (SQLITE_DONE == ret) {
        if (result_count == 0) {
          result = "null";
          while_flag = true;
        }
        break;
      } else {
        result = "-1";
        while_flag = true;
        break;
      }
    }
    if (while_flag) {
      break;
    }
    result = result_json.toStyledString();
  } while (false);
  return result;
}

std::string AutobackupBackupHelper::QueryFromUFTable(
    std::string file_local_path) {
  std::string result;
  Json::Value result_json;
  do {
    if (sqlite3_handle_ == nullptr || file_local_path.empty()) {
      break;
    }
    std::string sql_string = StringFormat(
        "SELECT "
        "file_cloud_path,upload_file_id,coshare_id,"
        "cloud_parent_folder_id,"
        "file_source FROM uploading_files_table "
        "WHERE file_local_path='%s';",
        file_local_path.c_str());
    sqlite3_stmt *stmt;
    if (SQLITE_OK != sqlite3_prepare_v2(sqlite3_handle_, sql_string.c_str(), -1,
                                        &stmt, nullptr)) {
      break;
    }

    int ret;
    ret = sqlite3_step(stmt);
    if (SQLITE_ROW == ret) {
      result_json["file_cloud_path"] = (char *)sqlite3_column_text(stmt, 0);
      result_json["upload_file_id"] = (char *)sqlite3_column_text(stmt, 1);
      result_json["coshare_id"] = (char *)sqlite3_column_text(stmt, 2);
      result_json["cloud_parent_folder_id"] = sqlite3_column_int64(stmt, 3);
      result_json["file_source"] = sqlite3_column_int(stmt, 4);
    } else if (SQLITE_DONE == ret) {
      result = "-1";
      break;
    } else {
      break;
    }
    result = result_json.toStyledString();
  } while (false);
  return result;
}

std::string AutobackupBackupHelper::DeleteFromUFTable(
    std::string file_local_path) {
  char *errmsg = nullptr;
  std::string error_msg;
  do {
    if (sqlite3_handle_ == nullptr || file_local_path.empty()) {
      break;
    }
    std::string sql_string = StringFormat(
        "DELETE FROM uploading_files_table "
        "WHERE file_local_path='%s';",
        file_local_path.c_str());
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "BEGIN TRANSACTION", nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, sql_string.c_str(), nullptr,
                                  nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      sqlite3_exec(sqlite3_handle_, "ROLLBACK TRANSACTION", nullptr, nullptr,
                   nullptr);
      break;
    }
    if (SQLITE_OK != sqlite3_exec(sqlite3_handle_, "COMMIT TRANSACTION",
                                  nullptr, nullptr, &errmsg)) {
      error_msg = errmsg;
      sqlite3_free(errmsg);
      break;
    }
  } while (false);

  return error_msg;
}

AutobackupBackupHelper::~AutobackupBackupHelper() {
  if (nullptr != sqlite3_handle_) {
    sqlite3_close(sqlite3_handle_);
  }
}

sqlite3 *AutobackupBackupHelper::GetSqlite3Handle() { return sqlite3_handle_; }

AutobackupBackupHelper::AutobackupBackupHelper(std::string appdata_path,
                                               int64_t user_id, int64_t corp_id,
                                               int32_t space_type,
                                               std::string cloud_path,
                                               std::string local_path) {
  sqlite3_handle_ = nullptr;
  do {
    if (appdata_path.empty() || cloud_path.empty() || local_path.empty()) {
      break;
    }
    if (appdata_path.back() != '\\') {
      appdata_path.append("\\");
    }
    std::string db_path = StringFormat(R"(%s%s\%s\)", appdata_path.c_str(),
                                       std::to_string(user_id).c_str(),
                                       std::to_string(corp_id).c_str());
    if (!cloud_base::filesystem_helper::create_dir(utf8ToWstring(db_path))) {
      break;
    }

    cloud_base::hash_algorithm::MD5 md5_(
        std::to_string(user_id) + std::to_string(corp_id) +
        std::to_string(space_type) + cloud_path + local_path);

    db_path.append(StringFormat("backup_%s.db", md5_.hex_string().c_str()));

    // 打开数据库，没有则创建
    if (sqlite3_open(db_path.c_str(), &sqlite3_handle_)) {
      break;
    }
    // 检查upload_seccess_tabel表是否存在
    sqlite3_stmt *stmt = nullptr;
    std::string query_string =
        "SELECT * FROM sqlite_master WHERE type='table' and "
        "name='upload_seccess_table';";
    int ret = sqlite3_prepare_v2(sqlite3_handle_, query_string.c_str(), -1,
                                 &stmt, nullptr);
    if (ret != SQLITE_OK) {
      sqlite3_close(sqlite3_handle_);
      sqlite3_handle_ = nullptr;
      break;
    }
    // table no exist, create it.
    if (sqlite3_step(stmt) == SQLITE_DONE) {
      sqlite3_finalize(stmt);
      query_string =
          "CREATE TABLE upload_seccess_table"
          "("
          "file_local_path TEXT NOT NULL,"
          "file_cloud_path TEXT NOT NULL,"
          "md5 TEXT NOT NULL,"
          "is_backup BOOLEAN DEFAULT 1,"
          "last_change_date DATETIME DEFAULT '',"
          "cloud_parent_folder_id INTEGER DEFAULT 0,"
          "cloud_file_id INTEGER DEFAULT 0,"
          "extrends TEXT DEFAULT '',"
          "PRIMARY KEY(file_local_path)"
          "); ";
      ret = sqlite3_prepare_v2(sqlite3_handle_, query_string.c_str(), -1, &stmt,
                               nullptr);
      if (ret != SQLITE_OK) {
        sqlite3_close(sqlite3_handle_);
        sqlite3_handle_ = nullptr;
        break;
      }
      if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_close(sqlite3_handle_);
        sqlite3_handle_ = nullptr;
        break;
      }
    }
    // 检查uploading_files_table表是否存在
    sqlite3_finalize(stmt);
    query_string =
        "SELECT * FROM sqlite_master WHERE type='table' and "
        "name='uploading_files_table';";
    ret = sqlite3_prepare_v2(sqlite3_handle_, query_string.c_str(), -1, &stmt,
                             nullptr);
    if (ret != SQLITE_OK) {
      sqlite3_close(sqlite3_handle_);
      sqlite3_handle_ = nullptr;
      break;
    }
    // table no exist, create it.
    if (sqlite3_step(stmt) == SQLITE_DONE) {
      sqlite3_finalize(stmt);
      query_string =
          "CREATE TABLE uploading_files_table"
          "("
          "file_local_path TEXT NOT NULL,"
          "file_cloud_path TEXT NOT NULL,"
          "upload_file_id TEXT NOT NULL,"
          "coshare_id TEXT NOT NULL,"
          "cloud_parent_folder_id INTEGER NOT NULL,"
          "file_source INTEGER NOT NULL,"
          "PRIMARY KEY(file_local_path)"
          "); ";
      ret = sqlite3_prepare_v2(sqlite3_handle_, query_string.c_str(), -1, &stmt,
                               nullptr);
      if (ret != SQLITE_OK) {
        sqlite3_close(sqlite3_handle_);
        sqlite3_handle_ = nullptr;
        break;
      }
      if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_close(sqlite3_handle_);
        sqlite3_handle_ = nullptr;
        break;
      }
    }

  } while (false);
}

}  // namespace AutobackupBackupHelper
}  // namespace Sqlite3Helper
}  // namespace EnterpriseCloud
