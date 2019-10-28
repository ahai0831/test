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
  return 0;
}
