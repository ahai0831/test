#include "autobackup_meatadata_helper.h"

#include <json/json.h>

#include <tools/string_format.hpp>
using assistant::tools::string::StringFormat;

#include <tools/string_convert.hpp>
using assistant::tools::string::utf8ToWstring;

#include <filesystem_helper/filesystem_helper.h>

namespace EnterpriseCloud {
namespace Sqlite3Helper {
namespace AutobackupMetadataHelper {

std::unique_ptr<AutobackupMetadataHelper> AutobackupMetadataHelper::Create(
    std::string appdata_path, int64_t user_id, int64_t corp_id) {
  auto database_ptr =
      new AutobackupMetadataHelper(appdata_path, user_id, corp_id);
  if (nullptr == database_ptr->GetSqlite3Handle()) {
    database_ptr = nullptr;
  }
  return std::unique_ptr<AutobackupMetadataHelper>(database_ptr);
}

std::string AutobackupMetadataHelper::Insert(
    int32_t space_type, std::string cloud_path, std::string local_path,
    std::string cloud_folder_id, bool is_backup /*=true*/,
    std::string coshare_id /*=""*/, std::string work_time /*=""*/,
    std::string extrends /*=""*/) {
  char *errmsg = nullptr;
  std::string error_msg;
  do {
    if (sqlite3_handle_ == nullptr || cloud_path.empty() ||
        local_path.empty() || cloud_folder_id.empty()) {
      break;
    }
    std::string sql_string = StringFormat(
        "INSERT INTO metadata_table"
        "(space_type,cloud_path,local_path,cloud_folder_id,is_backup,coshare_"
        "id,work_time,"
        "extrends)"
        " VALUES(%s,'%s','%s','%s',%s,'%s','%s','%s');",
        std::to_string(space_type).c_str(), cloud_path.c_str(),
        local_path.c_str(), cloud_folder_id.c_str(),
        std::to_string((int)is_backup).c_str(), coshare_id.c_str(),
        work_time.c_str(), extrends.c_str());
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

std::string AutobackupMetadataHelper::Update(
    int32_t space_type, std::string cloud_path, std::string local_path,
    std::string cloud_folder_id, bool is_backup /*= true*/,
    std::string coshare_id /*= ""*/, std::string work_time /*= ""*/,
    std::string extrends /*= ""*/) {
  char *errmsg = nullptr;
  std::string error_msg;
  do {
    if (sqlite3_handle_ == nullptr || cloud_path.empty() ||
        local_path.empty()) {
      break;
    }
    std::string sql_string = StringFormat(
        "UPDATE metadata_table "
        "SET cloud_folder_id='%s', is_backup=%s, coshare_id='%s', "
        "work_time='%s', extrends='%s' "
        "WHERE space_type=%s and cloud_path='%s' and local_path='%s';",
        cloud_folder_id.c_str(), std::to_string(is_backup).c_str(),
        coshare_id.c_str(), work_time.c_str(), extrends.c_str(),
        std::to_string(space_type).c_str(), cloud_path.c_str(),
        local_path.c_str());
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

std::string AutobackupMetadataHelper::Query() {
  std::string result;
  Json::Value result_json;
  do {
    if (sqlite3_handle_ == nullptr) {
      break;
    }
    std::string sql_string = "SELECT * FROM metadata_table;";
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
        result_json_temp["space_type"] = sqlite3_column_int(stmt, 0);
        result_json_temp["cloud_path"] = (char *)sqlite3_column_text(stmt, 1);
        result_json_temp["local_path"] = (char *)sqlite3_column_text(stmt, 2);
        result_json_temp["cloud_folder_id"] =
            (char *)sqlite3_column_text(stmt, 3);
        result_json_temp["is_backup"] = sqlite3_column_int(stmt, 4);
        result_json_temp["coshare_id"] = (char *)sqlite3_column_text(stmt, 5);
        result_json_temp["work_time"] = (char *)sqlite3_column_text(stmt, 6);
        result_json_temp["extrends"] = (char *)sqlite3_column_text(stmt, 7);

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

std::string AutobackupMetadataHelper::Query(int32_t space_type,
                                            std::string cloud_path,
                                            std::string local_path) {
  std::string result;
  Json::Value result_json;
  do {
    if (sqlite3_handle_ == nullptr || cloud_path.empty() ||
        local_path.empty()) {
      break;
    }
    std::string sql_string = StringFormat(
        "SELECT cloud_folder_id,is_backup,coshare_id,work_time,extrends FROM "
        "metadata_table "
        "WHERE space_type=%s and cloud_path='%s' and local_path='%s';",
        std::to_string(space_type).c_str(), cloud_path.c_str(),
        local_path.c_str());
    sqlite3_stmt *stmt;
    if (SQLITE_OK != sqlite3_prepare_v2(sqlite3_handle_, sql_string.c_str(), -1,
                                        &stmt, nullptr)) {
      break;
    }

    int ret;
    ret = sqlite3_step(stmt);
    if (SQLITE_ROW == ret) {
      result_json["cloud_folder_id"] = (char *)sqlite3_column_text(stmt, 0);
      result_json["is_backup"] = sqlite3_column_int(stmt, 1);
      result_json["coshare_id"] = (char *)sqlite3_column_text(stmt, 2);
      result_json["work_time"] = (char *)sqlite3_column_text(stmt, 3);
      result_json["extrends"] = (char *)sqlite3_column_text(stmt, 4);
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

std::string AutobackupMetadataHelper::Delete(int32_t space_type,
                                             std::string cloud_path,
                                             std::string local_path) {
  char *errmsg = nullptr;
  std::string error_msg;
  do {
    if (sqlite3_handle_ == nullptr || cloud_path.empty() ||
        local_path.empty()) {
      break;
    }
    std::string sql_string = StringFormat(
        "DELETE FROM metadata_table "
        "WHERE space_type=%s and cloud_path='%s' and local_path='%s';",
        std::to_string(space_type).c_str(), cloud_path.c_str(),
        local_path.c_str());
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

AutobackupMetadataHelper::~AutobackupMetadataHelper() {
  if (nullptr != sqlite3_handle_) {
    sqlite3_close(sqlite3_handle_);
  }
}

sqlite3 *AutobackupMetadataHelper::GetSqlite3Handle() {
  return sqlite3_handle_;
}

AutobackupMetadataHelper::AutobackupMetadataHelper(std::string appdata_path,
                                                   int64_t user_id,
                                                   int64_t corp_id) {
  sqlite3_handle_ = nullptr;
  do {
    if (appdata_path.empty()) {
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
    db_path.append("metadata_database.db");
    // open database
    if (sqlite3_open(db_path.c_str(), &sqlite3_handle_)) {
      break;
    }
    // check table
    sqlite3_stmt *stmt = nullptr;
    std::string query_string =
        "SELECT * FROM sqlite_master WHERE type='table' and "
        "name='metadata_table';";
    int ret = sqlite3_prepare_v2(sqlite3_handle_, query_string.c_str(), -1,
                                 &stmt, nullptr);
    if (ret != SQLITE_OK) {
      sqlite3_close(sqlite3_handle_);
      sqlite3_handle_ = nullptr;
      break;
    }
    // tabel exist
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      break;
    }
    // tabel no exist, create it.
    sqlite3_finalize(stmt);
    query_string =
        "CREATE TABLE metadata_table"
        "("
        "space_type INTEGER NOT NULL,"
        "cloud_path TEXT NOT NULL,"
        "local_path TEXT NOT NULL,"
        "cloud_folder_id TEXT NOT NULL,"
        "is_backup BOOLEAN DEFAULT 1,"
        "coshare_id TEXT DEFAULT '',"
        "work_time DATETIME DEFAULT '',"
        "extrends TEXT DEFAULT '',"
        "PRIMARY KEY(space_type,cloud_path,local_path)"
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
  } while (false);
}

}  // namespace AutobackupMetadataHelper
}  // namespace Sqlite3Helper
}  // namespace EnterpriseCloud
