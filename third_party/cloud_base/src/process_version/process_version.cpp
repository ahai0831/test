#include "process_version.h"

#include <wchar.h>
#include <memory>

#include <Windows.h>
namespace cloud_base {
namespace process_version {
#ifdef _WIN32
std::string GetCurrentProcessVersion() {
  std::string result = "";
  do {
    /********************test lpFilename*************************/
    // WCHAR lpFilename[MAX_PATH] = {L"F:\\ecloud\\eCloud.exe"};

    // get filename
    WCHAR lpFilename[MAX_PATH] = {'\0'};
    // Retrieves the fully qualified path for the file that contains the
    // specified module the first parameter is a handle to the loaded module
    // whose path is being requested. If this parameter is NULL,
    // GetModuleFileName retrieves the path of the executable file of the
    // current process.
    auto filename_len = GetModuleFileNameW(NULL, lpFilename, MAX_PATH);
    if (0 == filename_len && (MAX_PATH >= filename_len)) {
      break;
    }

    // Get the length of the information in the file version
    // Determines whether the operating system can retrieve version information
    // for a specified file. If version information is available,
    // GetFileVersionInfoSize returns the size, in bytes, of that information.
    auto fileversioninfo_size = GetFileVersionInfoSizeW(lpFilename, nullptr);
    if (0 == fileversioninfo_size) {
      break;
    }

    // Allocate the required memory
    auto fileversioninfo_date_memory =
        std::make_unique<unsigned char[]>(fileversioninfo_size);
    if (nullptr == fileversioninfo_date_memory) {
      break;
    }

    //  Get file version information
    //  Retrieves version information for the specified file.
    if (!GetFileVersionInfoW(lpFilename, NULL, fileversioninfo_size,
                             (LPVOID)fileversioninfo_date_memory.get())) {
      break;
    }

    //  Parse file version information
    VS_FIXEDFILEINFO* versioninfo_ptr = nullptr;
    UINT uLen = 0UL;
    LPCWSTR lpSubBlock = L"\\";
    // Retrieves specified version information from the specified
    // version-information resource. To retrieve the appropriate resource,
    // before you call VerQueryValue, you must first call the
    // GetFileVersionInfoSize function, and then the GetFileVersionInfo
    // function.
    if (!VerQueryValueW((LPCVOID)fileversioninfo_date_memory.get(), lpSubBlock,
                        (LPVOID*)&versioninfo_ptr, &uLen)) {
      break;
    }

    //  Get the product version in the file version information(only the first
    //  two)
    WORD product_version[4] = {0};
    product_version[0] = HIWORD(versioninfo_ptr->dwProductVersionMS);
    product_version[1] = LOWORD(versioninfo_ptr->dwProductVersionMS);
    product_version[2] = HIWORD(versioninfo_ptr->dwProductVersionLS);
    product_version[3] = LOWORD(versioninfo_ptr->dwProductVersionLS);
    result = std::to_string(product_version[0]) + "." +
             std::to_string(product_version[1]);

  } while (false);

  return result;
}
#endif
}  // namespace process_version
}  // namespace cloud_base
