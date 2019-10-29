#include <iostream>
#include <string>
#include <vector>

#include <Assistant_v2.h>
#include <filesystem_helper\filesystem_helper.h>

int main() {
  // std::wstring dir_path = L"C:/\/\//\/\/\/\/\/\/\/\//\/";// failed
  // std::wstring dir_path = L"C:/";  // successful
  std::wstring dir_path = L"C:\\Users\\NOL\\Desktop\\test";  // successful
  // std::wstring dir_path = L"C:/Users\\NOL\\Desktop\\test";// successful
  // std::wstring dir_path = L"C:/test";// successful
  // std::wstring dir_path = L"C://Users//NOL//Desktop//test";// successful
  // std::wstring dir_path = L"C:/Users/\NOL/\Desktop/\test";// failed
  bool flag =
      cloud_base::filesystem_helper::guarantee_directory_exists(dir_path);
  std::wstring tmpPath;
  bool tpFlag = cloud_base::filesystem_helper::get_appdata_path(tmpPath);
  //////////////////////////////////////////////
  if (flag == true) {
    printf("path exists or create it successful\n");
  } else {
    printf("create path or write data to it failed\n");
  }
  std::wcout << "appdataPath:" << tmpPath << std::endl;
  //////////////////////////////////////////////
  std::vector<std::wstring> test_vec;
  std::wstring test_path(L"C:\\Users\\NOL\\Desktop");
  std::wstring test_suffix(L"TXT");
  cloud_base::filesystem_helper::get_file_list(test_path, test_suffix,
                                               test_vec);
  for (const auto &out_str : test_vec) {
    std::string temp_str = assistant::tools::wstringToAnsi(out_str);
    std::cout << temp_str << std::endl;
  }

  /**********************file_helper test****************************/
  std::cout << std::endl << std::endl << "file infomation:" << std::endl;
  std::wstring path_test = L"C:/Users/NOL/Desktop\\spencer";
  std::string file_date;
  std::wstring file_name;
  uint64_t file_size = 0;
  cloud_base::filesystem_helper::GetFileName(path_test, file_name);
  cloud_base::filesystem_helper::GetFileSize(path_test, file_size);
  cloud_base::filesystem_helper::GetFileLastChange(path_test, file_date);

  std::string str_file_name = assistant::tools::wstringToAnsi(file_name);
  std::cout << "file name:" << str_file_name << std::endl;

  printf("file size:%d bytes\n", file_size);

  std::cout << "file modify time:" << file_date << std::endl;
  /**********************file_helper test2****************************/
  std::cout << std::endl
            << std::endl
            << "another file infomation:" << std::endl;
  std::wstring path_test2 = L"C:\\Users\\NOL\\Desktop\\test.txt";
  std::string file_date2;
  std::wstring file_name2;
  uint64_t file_size2 = 0;
  cloud_base::filesystem_helper::GetFileName(path_test2, file_name2);
  cloud_base::filesystem_helper::GetFileSize(path_test2, file_size2);
  cloud_base::filesystem_helper::GetFileLastChange(path_test2, file_date2);

  std::string str_file_name2 = assistant::tools::wstringToAnsi(file_name2);
  std::cout << "file name:" << str_file_name2 << std::endl;

  printf("file size:%d bytes\n", file_size2);

  std::cout << "file modify time:" << file_date2 << std::endl;

  return 0;
}
