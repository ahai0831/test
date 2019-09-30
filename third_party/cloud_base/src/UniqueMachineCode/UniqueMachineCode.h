#pragma once
#ifndef _MachineInfo_HH
#define _MachineInfo_HH

#include <string>

namespace cloud_base {
namespace unique_machine_code {
// get Mac address
std::string get_mac_address(void);
// get CpuId
std::string get_cpu_id(void);
// get disk serial number
std::string get_disk_serial(void);
// generate unique machine code
std::string generate_unique_machine_code(void);
}  // namespace unique_machine_code
}  // namespace cloud_base
#endif  //_MachineInfo_HH