/**
 * @brief cloud_base UniqueMachineCode test.
 * @date 2019-09-23
 * @copyright Copyright (c) 2019
 */
#include <iostream>

#include <UniqueMachineCode\UniqueMachineCode.h>

int main(void) {
  std::string testMacAddress =
      cloud_base::unique_machine_code::get_mac_address();
  std::string testDiskNum = cloud_base::unique_machine_code::get_disk_serial();
  std::string testCpuId = cloud_base::unique_machine_code::get_cpu_id();
  printf("MacAddress:%s\n", testMacAddress.c_str());
  printf("DiskNum:%s\n", testDiskNum.c_str());
  printf("CpuId:%s\n", testCpuId.c_str());
  std::string testMachineCode =
      cloud_base::unique_machine_code::generate_unique_machine_code();  // test
  // data
  printf("MahcineCode:%s\n", testMachineCode.c_str());
  return 0;
}
