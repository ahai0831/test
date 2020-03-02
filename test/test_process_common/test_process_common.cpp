#include <iostream>
#include <string>

#include <process_common/process_common_helper.h>

int main() {
  std::string appVersion =
      cloud_base::process_common_helper::GetCurrentApplicationVersion();

  std::string appName =
      cloud_base::process_common_helper::GetCurrentApplicationName();

  std::string appDataPath;
  cloud_base::process_common_helper::GetCurrentApplicationDataPath(appDataPath);

  printf("process_version:  %s\n", appVersion.c_str());
  printf("app_name:  %s\n", appName.c_str());
  printf("appdata_path:  %s\n\n", appDataPath.c_str());
  return 0;
}
