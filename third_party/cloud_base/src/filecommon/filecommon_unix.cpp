#include "filecommon_unix.h"
// mac
// #include <CoreFoundation/CFBundle.h>
#include <dirent.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <vector>

// macaddress use
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>

using namespace std;

namespace cloud_base {
namespace filecommon_unix {

// get file name
bool GetFileName(const std::string file_path, std::string &file_name) {
  file_name.clear();
  do {
    //  judge the path
    if (file_path.empty()) {
      break;
    }

    //  '/' change to "\\"
    std::string temp_path = file_path;
    std::replace(temp_path.begin(), temp_path.end(), '/', '\\');

    //  get file name
    if (temp_path.back() == '\\') {
      size_t pos = temp_path.rfind('\\', temp_path.length() - 2);
      if (pos == temp_path.npos) {
        break;
      }
      file_name = temp_path.substr(pos + 1, temp_path.length() - pos - 2);
    } else {
      size_t pos = temp_path.rfind('\\');
      if (pos == temp_path.npos) {
        break;
      }
      file_name = temp_path.substr(pos + 1);
    }
  } while (false);
  return !file_name.empty();
}

// 判断文件夹是否存在，不存在就创建
bool guarantee_directory_exists(const std::string &dir_path) {
  bool result = false;
  do {
    // 1.路径为空，break
    if (dir_path.empty()) {
      break;
    }
    // 2.路径已存在，返回true，break
    DIR *dir;
    if ((dir = opendir(dir_path.c_str())) != NULL) {
      result = true;
      closedir(dir);
      break;
    }
    // 3.路径不存在，逐级创建路径
    // 分割字符串
    std::vector<std::string> vec;
    size_t tmp_pos = 0;
    while (tmp_pos != dir_path.npos) {
      tmp_pos = dir_path.find('/', tmp_pos + 1);
      std::string temp_path = dir_path.substr(0, tmp_pos);
      vec.emplace_back(temp_path);
    }

    // 逐级创建路径
    //      S_IRWXU  00700权限，代表该文件所有者拥有读，写和执行操作的权限
    //      S_IRUSR(S_IREAD) 00400权限，代表该文件所有者拥有可读的权限
    //      S_IWUSR(S_IWRITE) 00200权限，代表该文件所有者拥有可写的权限
    //      S_IXUSR(S_IEXEC) 00100权限，代表该文件所有者拥有执行的权限
    //      S_IRWXG 00070权限，代表该文件用户组拥有读，写和执行操作的权限
    //      S_IRGRP 00040权限，代表该文件用户组拥有可读的权限
    //      S_IWGRP 00020权限，代表该文件用户组拥有可写的权限
    //      S_IXGRP 00010权限，代表该文件用户组拥有执行的权限
    //      S_IRWXO 00007权限，代表其他用户拥有读，写和执行操作的权限
    //      S_IROTH 00004权限，代表其他用户拥有可读的权限
    //      S_IWOTH 00002权限，代表其他用户拥有可写的权限
    //      S_IXOTH 00001权限，代表其他用户拥有执行的权限
    bool mkdir_flag = false;
    for (const auto &path : vec) {
      DIR *dir;
      if ((dir = opendir(path.c_str())) == NULL) {
        int res = mkdir(path.c_str(), 00700);
        if (0 != res) {
          break;
        }
      } else {
        closedir(dir);
      }
    }
    DIR *last_dir;
    if ((last_dir = opendir(dir_path.c_str())) == NULL) {
      break;
    }
    result = true;
  } while (false);
  return result;
}

// get file.suffix files in folder
typedef vector<string> FilesList;
bool get_file_list(const string &dirPath, const string &suffix,
                   FilesList &vec) {
  bool get_result = false;
  // judge string
  if (dirPath.empty() || suffix.empty()) {
    return get_result;
  }
  // folder must exists
  if (access(dirPath.c_str(), 0) == -1) {
    return get_result;
  }
  // get suffix file in dir
  int return_code;
  DIR *dir;
  struct dirent entry;
  struct dirent *res;
  char *sourceDir = new char[dirPath.size() + 4];
  strcpy(sourceDir, dirPath.c_str());
  // cout << sourceDir << endl;
  if ((dir = opendir(sourceDir)) != NULL) {
    for (return_code = readdir_r(dir, &entry, &res);
         res != NULL && return_code == 0;
         return_code = readdir_r(dir, &entry, &res)) {
      if (entry.d_type != DT_DIR) { /* save to file */
        // cout << "a" << endl;
        string sFile = entry.d_name;
        //        int iPos = sFile.find.find_last_of(".");
        int iPos = (int)sFile.find_last_of(".");
        if ((iPos == string::npos) ||
            (strcmp(suffix.c_str(), sFile.substr(iPos).c_str()))) {
          continue;
        }
        vec.push_back(entry.d_name);
      }
    }
    get_result = true;
    closedir(dir);
  }
  return get_result;
}

/* src为文件，target为文件夹 */
void cpFile(char *src, char *target) {
  FILE *srcH = NULL;
  FILE *outH = NULL;
  char buffer[1024] = "";
  char path[1024] = "";

  snprintf(path, sizeof(path), "%s/%s", target, src);

  srcH = fopen(src, "rb"); /* rb表示打开二进制 */
  if (srcH == NULL) {
    perror("fopen");
    return;
  }
  outH = fopen(path, "wb+");
  if (outH == NULL) {
    perror("fopen");
    return;
  }

  while (fread(buffer, 1, 1024, srcH) != 0) {
    fwrite(buffer, 1, 1024, outH);
    /* 清空buffer中的内容 */
    memset(buffer, 0, sizeof(buffer));
  }
  fclose(outH);
  fclose(srcH);
}

// get fileSize
bool GetFileSize(const std::string &file_path, uint64_t &file_size) {
  struct stat st;
  if (stat(file_path.c_str(), &st) != 0) {
    return false;
  }
  file_size = st.st_size;
  return true;
}

/// 字符串
std::string DatetimeToString(time_t time) {
  tm *tm_ = localtime(&time);  // 将time_t格式转换为tm结构体
  int year, month, day, hour, minute, second;  // 定义时间的各个int临时变量。
  year =
      tm_->tm_year +
      1900;  // 临时变量，年，由于tm结构体存储的是从1900年开始的时间，所以临时变量int为tm_year加上1900。
  month =
      tm_->tm_mon +
      1;  // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1。
  day = tm_->tm_mday;    // 临时变量，日。
  hour = tm_->tm_hour;   // 临时变量，时。
  minute = tm_->tm_min;  // 临时变量，分。
  second = tm_->tm_sec;  // 临时变量，秒。
  char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3],
      secondStr[3];                  // 定义时间的各个char*变量。
  sprintf(yearStr, "%d", year);      // 年。
  sprintf(monthStr, "%d", month);    // 月。
  sprintf(dayStr, "%d", day);        // 日。
  sprintf(hourStr, "%d", hour);      // 时。
  sprintf(minuteStr, "%d", minute);  // 分。
  if (minuteStr[1] ==
      '\0') {  // 如果分为一位，如5，则需要转换字符串为两位，如05。
    minuteStr[2] = '\0';
    minuteStr[1] = minuteStr[0];
    minuteStr[0] = '0';
  }
  sprintf(secondStr, "%d", second);  // 秒。
  if (secondStr[1] ==
      '\0') {  // 如果秒为一位，如5，则需要转换字符串为两位，如05。
    secondStr[2] = '\0';
    secondStr[1] = secondStr[0];
    secondStr[0] = '0';
  }
  char s[20];  // 定义总日期时间char*变量。
  sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr,
          secondStr);  // 将年月日时分秒合并。
  string str(
      s);  // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。
  return str;  // 返回转换日期时间后的string变量。
}

// 获取文件
bool GetFileLastChange(const char *file_path, std::string &file_modify_date) {
  struct stat buf;
  FILE *pFile;
  pFile = fopen(file_path, "r");
  int fd = fileno(pFile);
  fstat(fd, &buf);
  time_t t = buf.st_mtime;
  // struct tm lt;
  // localtime_r(&t, &lt);
  // char timbuf[80];
  // 处理为时间字符串
  file_modify_date = DatetimeToString(t);
  fclose(pFile);
  return !file_modify_date.empty();
}

// std::string GetCurrentProcessVersion() {
//   CFBundleRef ref = CFBundleGetMainBundle();
//   // 构建版本
//   int build_version = CFBundleGetVersionNumber(ref);
//   CFDictionaryRef dict = CFBundleGetInfoDictionary(ref);
//   // 版本名
//   CFStringRef key_name = CFStringCreateWithCString(
//       CFAllocatorGetDefault(), "CFBundleExecutable",
//       kCFStringEncodingUTF8);
//   CFStringRef value_name =
//       (CFStringRef)CFDictionaryGetValue(dict, (void *)key_name);
//   // 版本号
//   CFStringRef key_version = CFStringCreateWithCString(
//       CFAllocatorGetDefault(), "CFBundleShortVersionString",
//       kCFStringEncodingUTF8);
//   CFStringRef value_version =
//       (CFStringRef)CFDictionaryGetValue(dict, (void *)key_version);

//   // 拼接版本号，并做cstring转换。不做拼接直接返回版本号
//   // CFMutableStringRef version =
//   //     CFStringCreateMutable(CFAllocatorGetDefault(), 20);
//   // CFStringAppend(version, value_name);
//   // CFStringAppendCString(version, " ", kCFStringEncodingUTF8);
//   // CFStringAppend(version, value_version);
//   char buf[1025];
//   memset(buf, 0, 1025);
//   CFStringGetCString(value_version, buf, 1024, kCFStringEncodingUTF8);
//   return buf;
// }

// 3、获取mac地址
static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices);
static kern_return_t GetMACAddress(io_iterator_t intfIterator,
                                   UInt8 *MACAddress, UInt8 bufferSize);
std::string get_mac_address() {
  // struct ifreq ifr;
  // struct ifconf ifc;
  // char buf[1024];
  // int success = 0;

  // int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  // if (sock == -1) { /* handle error*/
  // };

  // ifc.ifc_len = sizeof(buf);
  // ifc.ifc_buf = buf;
  // if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */
  // }

  // struct ifreq *it = ifc.ifc_req;
  // const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct
  // ifreq));

  // for (; it != end; ++it) {
  //   strcpy(ifr.ifr_name, it->ifr_name);
  //   if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
  //     if (!(ifr.ifr_flags & IFF_LOOPBACK)) {  // don't count loopback
  //       if (ioctl(sock, SIOCGIFDSTADDR, &ifr) == 0) {
  //         success = 1;
  //         break;
  //       }
  //     }
  //   } else { /* handle error */
  //   }
  // }
  // unsigned char mac_address[6];
  // if (success) memcpy(mac_address, ifr.ifr_ifru.ifru_data, 6);
  // string str = (char *)mac_address;
  // return str;

  string address = "";
  kern_return_t kernResult = KERN_SUCCESS;
  io_iterator_t intfIterator;
  UInt8 MACAddress[kIOEthernetAddressSize];
  kernResult = FindEthernetInterfaces(&intfIterator);
  if (KERN_SUCCESS != kernResult) {
    printf("FindEthernetInterfaces returned 0x%08x\n", kernResult);
    //    return (char *)kernResult;
    return std::to_string(kernResult);
  } else {
    kernResult = GetMACAddress(intfIterator, MACAddress, sizeof(MACAddress));
    if (KERN_SUCCESS != kernResult) {
      printf("GetMACAddress returned 0x%08x\n", kernResult);
      return std::to_string(kernResult);
      //      return (char *)kernResult;
    } else {
      printf(
          "This system's built-in MAC address is "
          "%02x:%02x:%02x:%02x:%02x:%02x.\n",
          MACAddress[0], MACAddress[1], MACAddress[2], MACAddress[3],
          MACAddress[4], MACAddress[5]);
    }
  }
  std::string MACstring;
  for (size_t i = 0; i < 6; ++i) {
    MACstring += std::to_string(MACAddress[i]);
    if (i != 5) MACstring += ":";
  }
  (void)IOObjectRelease(intfIterator);  // Release the iterator.
  return MACstring;
}

// Returns an iterator containing the primary (built-in) Ethernet interface.
// The caller is responsible for releasing the iterator after the caller is
// done with it.
static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices) {
  kern_return_t kernResult;
  CFMutableDictionaryRef matchingDict;
  CFMutableDictionaryRef propertyMatchDict;
  // Ethernet interfaces are instances of class kIOEthernetInterfaceClass.
  // IOServiceMatching is a convenience function to create a dictionary with
  // the key kIOProviderClassKey and the specified value.
  matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);
  // Note that another option here would be:
  // matchingDict = IOBSDMatching("en0");
  // but en0: isn't necessarily the primary interface, especially on systems
  // with multiple Ethernet ports.
  if (NULL == matchingDict) {
    printf("IOServiceMatching returned a NULL dictionary.\n");
  } else {
    // Each IONetworkInterface object has a Boolean property with the key
    // kIOPrimaryInterface. Only the primary (built-in) interface has this
    // property set to TRUE.

    // IOServiceGetMatchingServices uses the default matching criteria defined
    // by IOService. This considers only the following properties plus any
    // family-specific matching in this order of precedence (see
    // IOService::passiveMatch):
    //
    // kIOProviderClassKey (IOServiceMatching)
    // kIONameMatchKey (IOServiceNameMatching)
    // kIOPropertyMatchKey
    // kIOPathMatchKey
    // kIOMatchedServiceCountKey
    // family-specific matching
    // kIOBSDNameKey (IOBSDNameMatching)
    // kIOLocationMatchKey

    // The IONetworkingFamily does not define any family-specific matching.
    // This means that in order to have IOServiceGetMatchingServices consider
    // the kIOPrimaryInterface property, we must add that property to a
    // separate dictionary and then add that to our matching dictionary
    // specifying kIOPropertyMatchKey.

    propertyMatchDict = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    if (NULL == propertyMatchDict) {
      printf("CFDictionaryCreateMutable returned a NULL dictionary.\n");
    } else {
      // Set the value in the dictionary of the property with the given key,
      // or add the key to the dictionary if it doesn't exist. This call
      // retains the value object passed in.
      CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface),
                           kCFBooleanTrue);

      // Now add the dictionary containing the matching value for
      // kIOPrimaryInterface to our main matching dictionary. This call will
      // retain propertyMatchDict, so we can release our reference on
      // propertyMatchDict after adding it to matchingDict.
      CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey),
                           propertyMatchDict);
      CFRelease(propertyMatchDict);
    }
  }

  // IOServiceGetMatchingServices retains the returned iterator, so release
  // the iterator when we're done with it. IOServiceGetMatchingServices also
  // consumes a reference on the matching dictionary so we don't need to
  // release the dictionary explicitly.
  kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict,
                                            matchingServices);
  if (KERN_SUCCESS != kernResult) {
    printf("IOServiceGetMatchingServices returned 0x%08x\n", kernResult);
  }

  return kernResult;
}

// Given an iterator across a set of Ethernet interfaces, return the MAC
// address of the last one. If no interfaces are found the MAC address is set
// to an empty string. In this sample the iterator should contain just the
// primary interface.
static kern_return_t GetMACAddress(io_iterator_t intfIterator,
                                   UInt8 *MACAddress, UInt8 bufferSize) {
  io_object_t intfService;
  io_object_t controllerService;
  kern_return_t kernResult = KERN_FAILURE;

  // Make sure the caller provided enough buffer space. Protect against buffer
  // overflow problems.
  if (bufferSize < kIOEthernetAddressSize) {
    return kernResult;
  }

  // Initialize the returned address
  bzero(MACAddress, bufferSize);

  // IOIteratorNext retains the returned object, so release it when we're done
  // with it.
  while ((intfService = IOIteratorNext(intfIterator))) {
    CFTypeRef MACAddressAsCFData;

    // IONetworkControllers can't be found directly by the
    // IOServiceGetMatchingServices call, since they are hardware nubs and do
    // not participate in driver matching. In other words, registerService()
    // is never called on them. So we've found the IONetworkInterface and will
    // get its parent controller by asking for it specifically.

    // IORegistryEntryGetParentEntry retains the returned object, so release
    // it when we're done with it.
    kernResult = IORegistryEntryGetParentEntry(intfService, kIOServicePlane,
                                               &controllerService);

    if (KERN_SUCCESS != kernResult) {
      printf("IORegistryEntryGetParentEntry returned 0x%08x\n", kernResult);
    } else {
      // Retrieve the MAC address property from the I/O Registry in the form
      // of a CFData
      MACAddressAsCFData = IORegistryEntryCreateCFProperty(
          controllerService, CFSTR(kIOMACAddress), kCFAllocatorDefault, 0);
      if (MACAddressAsCFData) {
        CFShow(MACAddressAsCFData);
        // for display purposes only; output goes
        // to stderr
        // Get the raw bytes of the MAC address from the CFData
        CFDataGetBytes((CFDataRef)MACAddressAsCFData,
                       CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
        CFRelease(MACAddressAsCFData);
      }
      // Done with the parent Ethernet controller object so we release it.
      (void)IOObjectRelease(controllerService);
    }
    // Done with the Ethernet interface object so we release it.
    (void)IOObjectRelease(intfService);
  }
  return kernResult;
}
}  // namespace filecommon_unix
}  // namespace cloud_base
