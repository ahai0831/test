/**
 * @file sqlite_test_demo.cpp
 * @brief test sqlite3 static lib.
 * @date 2019-10-11
 *
 * @copyright Copyright (c) 2019
 *
 */
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include <sqlite3.h>

int test_callback(void *ptr, int columns_number, char **cloumn_text,
                  char **cloumn_name) {
  auto pttr = (std::vector<std::string> *)ptr;
  for (int i = 0; i < columns_number; i++) {
    pttr->push_back(cloumn_text[i]);
  }
  return 0;
}

int main() {
  std::cout << "hello sqlite3!" << std::endl;

  sqlite3 *sql_handle = nullptr;
  if (sqlite3_open("sqlite3_test_database.db", &sql_handle)) {
    std::cerr << "open database fail." << std::endl;
  }

  char *errmsg = nullptr;
  // create table
  sqlite3_exec(sql_handle,
               "create table test_sqlite(No integer, TestNum varchar(8));",
               nullptr, nullptr, &errmsg);
  if (nullptr != errmsg) {
    std::cerr << "create table fail: " << errmsg << std::endl;
  }
  // insert data into table
  sqlite3_exec(sql_handle,
               "insert into test_sqlite(No, TestNum) values(0,'00');"
               "insert into test_sqlite(No, TestNum) values(1,'');"
               "insert into test_sqlite(No, TestNum) values(2,'02');",
               nullptr, nullptr, &errmsg);
  if (nullptr != errmsg) {
    std::cerr << "insert data fail: " << errmsg << std::endl;
  }
  // update table
  sqlite3_exec(sql_handle, "update test_sqlite set TestNum='01' where No=1;",
               nullptr, nullptr, &errmsg);
  if (nullptr != errmsg) {
    std::cerr << "update data fail: " << errmsg << std::endl;
  }
  // query data from table
  std::vector<std::string> table_data;
  sqlite3_exec(sql_handle, "select * from test_sqlite", test_callback,
               &table_data, &errmsg);
  if (nullptr != errmsg) {
    std::cerr << "query data fail: " << errmsg << std::endl;
  }
  const int col_num = 2;
  const size_t row_num = table_data.size() / col_num;
  std::cout << std::left << "-------------table data-------------" << std::endl;
  std::cout << std::setw(4) << "No" << std::setw(8) << "TestNum" << std::endl;
  for (size_t i = 0; i < row_num; i++) {
    for (int j = 0; j < col_num; j++) {
      std::cout << std::setw(4) << table_data[(i * col_num) + j];
    }
    std::cout << std::endl;
  }
  std::cout << std::right << "------------------------------------"
            << std::endl;

  sqlite3_exec(sql_handle, "drop table test_sqlite;", nullptr, nullptr,
               &errmsg);
  sqlite3_free(errmsg);
  sqlite3_close(sql_handle);
  return 0;
}
