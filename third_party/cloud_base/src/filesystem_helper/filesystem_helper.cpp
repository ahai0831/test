#include "filesystem_helper.h"

#include <io.h>

#include <algorithm>

#include <Windows.h>
using std::wstring;
namespace cloud_base {
namespace filesystem_helper {
bool guarantee_directory_exists(const std::wstring &dir_path) {
  bool result_flag = false;
  std::wstring temp_path(dir_path);
  do {
    /// judge empty
    if (temp_path.empty()) {
      break;
    }

    //  determine the "\\" end of str
    if (temp_path.back() != '\\') {
      temp_path += L"\\";
    }

    //  judge if the path is effective or not
    bool invalid_character_flag = true;
    for (auto &tmp : temp_path) {
      if (tmp == '/') {
        tmp = '\\';
      }
      if (tmp == '>' || tmp == '<' || tmp == '|' || tmp == '*' || tmp == '?' ||
          tmp == '"') {
        invalid_character_flag = false;
        break;
      }
    }
    // judge invalid_character
    if (!invalid_character_flag) {
      break;
    }

    //  path not exsit,create it //
    if (_waccess(temp_path.c_str(), 0)) {
      bool creatFlag = create_dir(temp_path.c_str());
      if (!creatFlag) {
        break;
      }
    }

    temp_path += L"tmp.txt";
    //  write a temporary file
    //  "w+",can write and read
    //  "D",temp file flag,will delete it at last
    //  Reject other processes Read and write access to the file
    FILE *fp = _wfsopen(temp_path.c_str(), L"wD+", _SH_DENYRW);
    if (nullptr == fp) {
      break;
    }

    int write_result = fputs("this is a test file.", fp);
    if (write_result == EOF) {
      break;
    }

    //  judge if close fp successfully or not
    int close_result = fclose(fp);
    if (close_result == EOF) {
      break;
    }

    fp = nullptr;
    result_flag = true;
  } while (false);

  //  return result
  return result_flag;
}
bool create_dir(const std::wstring &dir) {
  bool create_result = false;
  wstring temp_dir(dir);
  do {
    /// judge empty
    if (temp_dir.empty()) {
      break;
    }

    // judge if the path exsit or not
    if (!_waccess(temp_dir.c_str(), 0)) {
      //  path exist,return true
      create_result = true;
      break;
    }

    //  determine the "\\" end of str
    if (temp_dir.back() != '\\') {
      temp_dir += L"\\";
    }

    //  judge if the path is effective or not
    bool invalid_character_flag = true;
    for (auto &tmp : temp_dir) {
      if (tmp == '/') {
        tmp = '\\';
      }
      if (tmp == '>' || tmp == '<' || tmp == '|' || tmp == '*' || tmp == '?' ||
          tmp == '"') {
        invalid_character_flag = false;
        break;
      }
    }

    // judge invalid_character
    if (!invalid_character_flag) {
      break;
    }

    std::vector<std::wstring> vec;
    // pos initialization
    size_t tmp_pos = 0;

    //   Split string
    while (tmp_pos != temp_dir.npos) {
      tmp_pos = temp_dir.find('\\', tmp_pos + 1);
      std::wstring temp_path = temp_dir.substr(0, tmp_pos);
      vec.emplace_back(temp_path);
    }

    //  create path
    bool mkdir_flag = true;
    for (const auto &path : vec) {
      if (_waccess(path.c_str(), 0)) {
        int res = _wmkdir(path.c_str());
        if (res == -1) {
          mkdir_flag = false;
          break;
        }
      }
    }
    // judge mkdir_flag;
    if (!mkdir_flag) {
      break;
    }

    // judge if create path successful or not
    create_result = true;
  } while (false);

  return create_result;
}

bool get_file_list(const std::wstring &dirPath, const std::wstring &suffix,
                   std::vector<std::wstring> &vec) {
  bool get_result = false;
  intptr_t handle = -1;
  do {
    // if dirPath or suffix is empty,return false.
    if (dirPath.empty() || suffix.empty()) {
      break;
    }

    //  '/' change to "\\"
    std::wstring inPath(dirPath);
    std::replace(inPath.begin(), inPath.end(), '/', '\\');

    //  determine the "\\" end of str
    if (inPath.back() != '\\') {
      inPath += L"\\";
    }

    //  judge if the path exists or not
    if (_waccess(inPath.c_str(), 0)) {
      break;
    }

    std::wstring match_path = inPath + L"*.*";  //  Match all files
    _wfinddata_t fileInfo;

    if ((handle = _wfindfirst(match_path.c_str(), &fileInfo)) == -1) {
      break;
    }

    //  match all files those end of Specific file suffix
    do {
      //  get the file suffix
      std::wstring fileName(fileInfo.name);
      std::wstring subsuffix;

      //  suffix is empty if file name is a folder
      size_t pos = fileName.rfind(L".");
      subsuffix = (pos == fileName.npos) ? L"" : fileName.substr(pos + 1);
      //  match the file end of Specific file suffix
      //  push it to vector if success;
      int cmp_result = _wcsicmp(suffix.c_str(), subsuffix.c_str());

      //  judge if it is directory or not
      if (cmp_result == 0 && fileInfo.attrib != _A_SUBDIR) {
        vec.emplace_back(fileName);
      }

    } while (_wfindnext(handle, &fileInfo) == 0);

    get_result = true;
  } while (false);

  if (handle != -1) {
    _findclose(handle);
    handle = -1;
  }

  return get_result;
}

bool GetFileName(const std::wstring &file_path, std::wstring &file_name) {
  do {
    file_name.clear();

    //  judge the path
    if (file_path.empty()) {
      break;
    }

    //  '/' change to "\\"
    std::wstring temp_path = file_path;
    std::replace(temp_path.begin(), temp_path.end(), '/', '\\');

    //  get file name
    if (temp_path.back() == '\\') {
      size_t pos = temp_path.rfind('\\', temp_path.length() - 2);
      if (pos == temp_path.npos) {
        break;
      }
      file_name = temp_path.substr(pos + 1, temp_path.length() - pos - 2);
    } else {
      size_t pos = temp_path.rfind('\\');
      if (pos == temp_path.npos) {
        break;
      }
      file_name = temp_path.substr(pos + 1);
    }
  } while (false);
  return !file_name.empty();
}

bool GetFileSize(const std::wstring &file_path, uint64_t &file_size) {
  bool get_size_result = false;
  FILE *fp = nullptr;
  do {
    //  judge the path
    if (file_path.empty()) {
      break;
    }

    //  "w+",can write and read
    //  _SH_DENYRW,Reject other processes Read and write access to the file
    fp = _wfsopen(file_path.c_str(), L"r", _SH_DENYRW);
    if (nullptr == fp) {
      break;
    }

    if (_fseeki64(fp, 0L, SEEK_END) != 0) {
      break;
    }

    file_size = _ftelli64(fp);  // get file_size
    if (file_size == -1) {
      break;
    }

    get_size_result = true;
  } while (false);

  if (nullptr != fp) {
    fclose(fp);
    fp = nullptr;
  }
  return get_size_result;
}

bool GetFileLastChange(const std::wstring &file_path,
                       std::string &file_modify_date) {
  //  get latest modify Timestamp passed from 01/01/00:00:00 1970 UTC)
  do {
    file_modify_date.clear();

    //  judge the path
    if (file_path.empty()) {
      break;
    }
    char ModifyTime[30] = {'\0'};

    //  打开文件以获取文件属性
    HANDLE hFile = CreateFileW(
        file_path.c_str(),
        0,  // 即使拒绝GENERIC_READ访问，应用程序也可以查询某些元数据，例如文件，目录或设备属性
        0,  // 不共享读写
        NULL,                   // default security
        OPEN_EXISTING,          // existing file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
      break;
    }

    //  获取文件信息
    FILETIME ftModify;
    if (!GetFileTime(hFile, nullptr, nullptr, &ftModify)) {
      break;
    }

    //  初始化stLocal
    SYSTEMTIME stLocal = {0};
    //  数据类型转换
    if (!FileTimeToSystemTime(&ftModify, &stLocal)) {
      break;
    }

    //  格式化字符串
    sprintf(ModifyTime, "%04u-%02u-%02u %02u:%02u:%02u", stLocal.wYear,
            stLocal.wMonth, stLocal.wDay, stLocal.wHour + 8, stLocal.wMinute,
            stLocal.wSecond);  //  文件修改时间
    file_modify_date = ModifyTime;

  } while (false);
  return !file_modify_date.empty();
}
}  // namespace filesystem_helper
}  // namespace cloud_base
