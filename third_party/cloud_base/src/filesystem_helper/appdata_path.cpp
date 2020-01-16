#include "filesystem_helper.h"

#include <Shlobj.h>
using std::wstring;

namespace cloud_base {
namespace filesystem_helper {
bool get_appdata_path(std::wstring &appdata_path) {
  WCHAR tmpPath[MAX_PATH] = {'\0'};
  //  get the temp path of application
  //  SHGFP_TYPE_CURRENT (Retrieve the folder's current path.)
  HRESULT result = ::SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL,
                                      SHGFP_TYPE_CURRENT, tmpPath);
  appdata_path = (S_OK == result) ? tmpPath : L"";
  return !appdata_path.empty();
}
}  // namespace filesystem_helper
}  // namespace cloud_base
