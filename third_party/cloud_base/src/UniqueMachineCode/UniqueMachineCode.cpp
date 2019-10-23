#include "UniqueMachineCode.h"

#include <comdef.h>
#include <intrin.h>

#include <cinttypes>
#include <iostream>
#include <memory>

#include "HashAlgorithm\SHA1Digest.h"

namespace {
typedef struct _ASTAT_ {
  ADAPTER_STATUS adapt;
  NAME_BUFFER NameBuff[30];
} ASTAT;
}  // namespace

namespace cloud_base {
namespace unique_machine_code {

// get Mac address
std::string get_mac_address() {
  char mac[200] = {'\0'};
  NCB ncb = {0};
  ASTAT Adapter = {0};
  LANA_ENUM lana_enum = {0};
  UCHAR uRetCode = NRC_GOODRET;
  std::string strMac;
  do {
    ncb.ncb_command = NCBENUM;
    ncb.ncb_buffer = (unsigned char *)&lana_enum;
    ncb.ncb_length = sizeof(LANA_ENUM);
    uRetCode = Netbios(&ncb);

    if (uRetCode != NRC_GOODRET) {
      break;
    }

    for (int lana = 0; lana < lana_enum.length; lana++) {
      ncb.ncb_command = NCBRESET;
      ncb.ncb_lana_num = lana_enum.lana[lana];
      uRetCode = Netbios(&ncb);
    }

    if (uRetCode != NRC_GOODRET) {
      break;
    }

    ncb.ncb_command = NCBASTAT;
    ncb.ncb_lana_num = lana_enum.lana[0];
    strcpy((char *)ncb.ncb_callname, "*");
    ncb.ncb_buffer = (unsigned char *)&Adapter;
    ncb.ncb_length = sizeof(Adapter);
    uRetCode = Netbios(&ncb);

    if (uRetCode != NRC_GOODRET) {
      break;
    }

    sprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X",
            Adapter.adapt.adapter_address[0], Adapter.adapt.adapter_address[1],
            Adapter.adapt.adapter_address[2], Adapter.adapt.adapter_address[3],
            Adapter.adapt.adapter_address[4], Adapter.adapt.adapter_address[5]);
    strMac = mac;
  } while (false);
  return strMac;
}

// get disk serial
std::string get_disk_serial() {
  HANDLE hPhysicalDrive = INVALID_HANDLE_VALUE;
  ULONG ulSerialLen = 0;
  char serialNumber[1024] = {'\0'};
  std::string strSerialNumber;
  /// use do-while(false)
  do {
    hPhysicalDrive = CreateFileA(R"(\\.\PhysicalDrive0)", 0,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                 OPEN_EXISTING, 0, NULL);
    if (hPhysicalDrive == INVALID_HANDLE_VALUE) {
      break;
    }
    STORAGE_PROPERTY_QUERY query = {static_cast<STORAGE_PROPERTY_ID>(0)};
    DWORD cbBytesReturned = 0;
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    STORAGE_DESCRIPTOR_HEADER pDevDescHeader = {0};
    auto header_result = DeviceIoControl(
        hPhysicalDrive, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
        &pDevDescHeader, sizeof(pDevDescHeader), &cbBytesReturned, NULL);

    if (header_result == 0) {
      break;
    }

    auto disk_property_buffer = std::make_unique<char[]>(pDevDescHeader.Size);

    if (nullptr == disk_property_buffer) {
      break;
    }
    void *buffer_ptr = &(disk_property_buffer[0]);
    auto device_result = DeviceIoControl(
        hPhysicalDrive, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
        buffer_ptr, pDevDescHeader.Size, &cbBytesReturned, NULL);

    if (device_result == 0) {
      break;
    }

    STORAGE_DEVICE_DESCRIPTOR &descriptor =
        *static_cast<STORAGE_DEVICE_DESCRIPTOR *>(buffer_ptr);

    if (descriptor.SerialNumberOffset == 0) {
      break;
    }
    _snscanf(((const char *)&descriptor + descriptor.SerialNumberOffset),
             (descriptor.Size - descriptor.SerialNumberOffset), "%1023s",
             serialNumber);
    // in windows xp sp3(virtual machine),it may cause serialnumber is "3030..."
    const auto sn_size = strlen(serialNumber);
    if (sn_size % 2 == 0) {
      decltype(strlen("")) i = 0;
      for (; i < sn_size; i += 2) {
        if (strncmp(serialNumber + i, "30", 2) != 0) {
          break;
        }
      }
      if (sn_size == i) {
        break;
      }
    }
    strSerialNumber = serialNumber;
  } while (false);

  if (hPhysicalDrive != INVALID_HANDLE_VALUE) {
    CloseHandle(hPhysicalDrive);
    hPhysicalDrive = INVALID_HANDLE_VALUE;
  }
  return strSerialNumber;
}

// get Cpuid
void getcpuid(unsigned int CPUInfo[4], unsigned int InfoType) {
#if defined(__GNUC__)  // GCC
  __cpuid(InfoType, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#elif defined(_MSC_VER)  // MSVC
#if _MSC_VER >= 1400     // VC2005???¡ì??__cpuid
  __cpuid((int *)(void *)CPUInfo, (int)InfoType);
#else
  getcpuidex(CPUInfo, InfoType, 0);
#endif
#endif  // #if defined(__GNUC__)
}

// get strCpuId
std::string get_cpu_id() {
  std::string strs;
  unsigned int dwBuf[4] = {0};
  getcpuid(dwBuf, 0);
  for (const auto &x : dwBuf) {
    strs += std::to_string(x);
  }
  return strs;
}

// GenerateUniqueMachineCode
std::string generate_unique_machine_code() {
  std::string strMacAddress = get_mac_address();
  std::string strDiskNum = get_disk_serial();
  std::string strCpuId = get_cpu_id();
  std::string reData = "MacAddress=" + strMacAddress + "&CpuId=" + strCpuId +
                       "&DiskNum=" + strDiskNum;  // UniqueMachineCode
  const char key[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9',
                      '0', 'A', 'B', 'C', 'D', 'E', 'F', '\0'};
  std::string strKey = key;
  std::string enResult = hash_algorithm::GenerateSha1Digest(strKey, reData);
  return enResult;
}

}  // namespace unique_machine_code
}  // namespace cloud_base
