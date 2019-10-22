/**
 * @brief process_version test.
 * @date 2019-10-18
 *
 * @copyright Copyright (c) 2019
 *
 */
#include <iostream>
#include <string>

#include "process_version\process_version.h"
int main() {
  std::string proVersion =
      cloud_base::process_version::GetCurrentProcessVersion();
  std::cout << "process version:" << proVersion << std::endl;
  return 0;
}

