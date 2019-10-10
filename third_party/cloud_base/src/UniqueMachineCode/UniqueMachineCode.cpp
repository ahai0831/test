#include "UniqueMachineCode.h"

#include <cinttypes>
#include <iostream>

#include <Wbemidl.h>
#include <comdef.h>
#include <intrin.h>

#include "HashAlgorithm\SHA1Digest.h"

#define SHA1_DIGESTSIZE (20)

namespace {
// sha1 encode test key
const char *key = "1234567890ABCDEF";
}  // namespace

namespace cloud_base {
namespace unique_machine_code {
// get Mac address
std::string get_mac_address(void) {
  char mac[200];
  NCB ncb;
  typedef struct _ASTAT_ {
    ADAPTER_STATUS adapt;
    NAME_BUFFER NameBuff[30];
  } ASTAT, *PASTAT;

  ASTAT Adapter;

  typedef struct _LANA_ENUM {
    UCHAR length;
    UCHAR lana[MAX_LANA];
  } LANA_ENUM;

  LANA_ENUM lana_enum;
  UCHAR uRetCode;
  memset(&ncb, 0, sizeof(ncb));
  memset(&lana_enum, 0, sizeof(lana_enum));
  ncb.ncb_command = NCBENUM;
  ncb.ncb_buffer = (unsigned char *)&lana_enum;
  ncb.ncb_length = sizeof(LANA_ENUM);
  uRetCode = Netbios(&ncb);

  if (uRetCode != NRC_GOODRET) return "";

  for (int lana = 0; (lana < lana_enum.length) && (uRetCode == NRC_GOODRET);
       lana++) {
    ncb.ncb_command = NCBRESET;
    ncb.ncb_lana_num = lana_enum.lana[lana];
    uRetCode = Netbios(&ncb);
  }

  if (uRetCode != NRC_GOODRET) return "";

  memset(&ncb, 0, sizeof(ncb));
  ncb.ncb_command = NCBASTAT;
  ncb.ncb_lana_num = lana_enum.lana[0];
  strcpy((char *)ncb.ncb_callname, "*");
  ncb.ncb_buffer = (unsigned char *)&Adapter;
  ncb.ncb_length = sizeof(Adapter);
  uRetCode = Netbios(&ncb);

  if (uRetCode != NRC_GOODRET) return "";

  sprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X",
          Adapter.adapt.adapter_address[0], Adapter.adapt.adapter_address[1],
          Adapter.adapt.adapter_address[2], Adapter.adapt.adapter_address[3],
          Adapter.adapt.adapter_address[4], Adapter.adapt.adapter_address[5]);
  std::string strMac = mac;
  return strMac;
}

// get disk serial
std::string get_disk_serial(void) {
  std::string result;
  HRESULT hres = S_FALSE;

  IWbemLocator *pLoc = nullptr;
  IWbemServices *pSvc = nullptr;
  IEnumWbemClassObject *pEnumerator = nullptr;
  IWbemClassObject *pclsObj = nullptr;
  /// use do-while(false)
  do {
    /// Initialize COM
    hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (S_OK != hres) {
      break;
    }
    /// Obtain the initial locator to WMI
    CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                     IID_IWbemLocator, (LPVOID *)&pLoc);

    if (nullptr == pLoc) {
      break;
    }
    /// Connect to WMI through the IWbemLocator::ConnectServer method
    pLoc->ConnectServer(L"ROOT\\CimV2",  // Object path of WMI namespace
                        NULL,            // User name. NULL = current user
                        NULL,            // User password. NULL = current
                        0,               // Locale. NULL indicates current
                        WBEM_FLAG_CONNECT_USE_MAX_WAIT,  // Security flags.
                        0,     // Authority (e.g. Kerberos)
                        0,     // Context object
                        &pSvc  // pointer to IWbemServices proxy
    );

    if (nullptr == pSvc) {
      break;
    }
    /// Set security levels on the proxy
    auto proxyblanket_result =
        CoSetProxyBlanket(pSvc,                    // Indicates the proxy to set
                          RPC_C_AUTHN_WINNT,       // RPC_C_AUTHN_xxx
                          RPC_C_AUTHZ_NONE,        // RPC_C_AUTHZ_xxx
                          NULL,                    // Server principal name
                          RPC_C_AUTHN_LEVEL_CALL,  // RPC_C_AUTHN_LEVEL_xxx
                          RPC_C_IMP_LEVEL_IMPERSONATE,  // RPC_C_IMP_LEVEL_xxx
                          NULL,                         // client identity
                          EOAC_NONE                     // proxy capabilities
        );
    if (S_OK != proxyblanket_result) {
      break;
    }

    /// Use the IWbemServices pointer to make requests of WMI
    /// Microsoft docs says that IWbemServices::ExecQuery method's
    /// Minimum supported client is Windows Vista
    auto exec_query_result =
        pSvc->ExecQuery(L"WQL", bstr_t("SELECT * FROM Win32_PhysicalMedia"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL, &pEnumerator);

    if (S_OK != exec_query_result) {
      break;
    }

    /// Get the data from the query
    if (nullptr == pEnumerator) {
      break;
    }
    /// Microsoft docs says that IEnumWbemClassObject::Next method's
    /// Minimum supported client is Windows Vista
    ULONG uReturn = 0;
    HRESULT next_result =
        pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
    if (S_OK != next_result) {
      break;
    }

    if (0 == uReturn) {
      break;
    }

    /// Get the value of the Name property
    /// Microsoft docs says that IEnumWbemClassObject::Get method's
    /// Minimum supported client is Windows Vista
    VARIANT vtProp;
    HRESULT get_result = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
    if (SUCCEEDED(get_result) && (V_VT(&vtProp) == VT_BSTR)) {
      std::wstring wstrDiskNum = vtProp.bstrVal;
      result.resize(wstrDiskNum.size());
      for (decltype(wstrDiskNum.size()) i = 0; i < wstrDiskNum.size(); ++i) {
        result[i] = static_cast<char>(wstrDiskNum[i]);
      }
    }
    /// Cleanup
    VariantClear(&vtProp);

  } while (false);

  if (nullptr != pclsObj) {
    pclsObj->Release();
    pclsObj = nullptr;
  }
  if (nullptr != pEnumerator) {
    pEnumerator->Release();
    pEnumerator = nullptr;
  }
  if (nullptr == pSvc) {
    pSvc->Release();
    pSvc = nullptr;
  }
  if (nullptr != pLoc) {
    pLoc->Release();
    pLoc = nullptr;
  }

  if (S_FALSE != hres) {
    // Uninitialize
    CoUninitialize();
    hres = S_FALSE;
  }
  return result;
}

// get Cpuid
void getcpuid(unsigned int CPUInfo[4], unsigned int InfoType) {
#if defined(__GNUC__)  // GCC
  __cpuid(InfoType, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#elif defined(_MSC_VER)  // MSVC
#if _MSC_VER >= 1400     // VC2005≤≈÷ß≥÷__cpuid
  __cpuid((int *)(void *)CPUInfo, (int)InfoType);
#else
  getcpuidex(CPUInfo, InfoType, 0);
#endif
#endif  // #if defined(__GNUC__)
}

// get strCpuId
std::string get_cpu_id(void) {
  std::string strs;
  unsigned int dwBuf[4] = {0};
  getcpuid(dwBuf, 0);
  for (const auto &x : dwBuf) {
    strs += std::to_string(x);
  }
  return strs;
}

// GenerateUniqueMachineCode
std::string generate_unique_machine_code(void) {
  std::string strMacAddress = get_mac_address();
  std::string strDiskNum = get_disk_serial();
  std::string strCpuId = get_cpu_id();
  std::string reData = "MacAddress=" + strMacAddress + "&CpuId=" + strCpuId +
                       "&DiskNum=" + strDiskNum;  // UniqueMachineCode
  std::string enResult = hash_algorithm::GenerateSha1Digest(key, reData);
  return enResult;
}

}  // namespace unique_machine_code
}  // namespace cloud_base