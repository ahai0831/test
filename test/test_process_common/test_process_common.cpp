#include <iostream>
#include <string>

#include <process_common/process_common_helper.h>

int main() {
//  std::string pVersion =
//	  cloud_base::process_common_helper::GetCurrentApplicationVersion();

  std::string appDataPath;
  cloud_base::process_common_helper::GetCurrentApplicationDataPath(appDataPath);

//  printf("process_version:  %s\n", pVersion.c_str());
  printf("appdata_path:  %s\n\n", appDataPath.c_str());
  return 0;
}
