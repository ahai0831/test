#include "log_system.h"

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <fcntl.h>
#include <pthread.h>
#endif

#include <DateHelper/DateHelper.h>
#include <filecommon/filecommon_helper.h>
#include <process_common/process_common_helper.h>
#include <cinttypes>
#include <core/readwrite_callback.hpp>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>
#include <tools/string_format.hpp>

namespace {
inline std::string Datetime() {
  return cloud_base::date_helper::get_time_stamp();
}
inline uint32_t Threadid() {
#ifdef _WIN32
  /// ref:
  /// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getcurrentthreadid
  return static_cast<uint32_t>(GetCurrentThreadId());
#elif __linux__
  /// ref: https://linux.die.net/man/2/gettid
  return static_cast<uint32_t>(gettid());
#else
  /// ref:
  /// https://stackoverflow.com/questions/28188258/how-do-i-get-the-current-pthread-id
  /// ref: http://manpagez.com/man/3/pthread_threadid_np/
  uint64_t tid = 0;
  pthread_threadid_np(NULL, &tid);
  return static_cast<uint32_t>(tid);
#endif
}

FILE* out = nullptr;
bool initfile_success = false;
std::once_flag initout_flag;
/// Don't use this struct outside.
/// Leave it here.
static struct InitOutOnce {
  /// Do nothing.
  InitOutOnce() = default;
  ~InitOutOnce() {
    if (initfile_success && nullptr != out) {
      fclose(out);
      out = nullptr;
    }
  }
} _init_out;

inline void InitOut() {
  std::call_once(initout_flag, []() {
    std::string log_name = assistant::tools::string::StringFormat(
        "logs-%s-%" PRIu64 ".log", "upload_dll",
        cloud_base::date_helper::get_millisecond_time_stamp());
    std::string appDataPath;
    cloud_base::process_common_helper::GetCurrentApplicationDataPath(
        appDataPath);
    std::string log_file_name;
#ifdef _WIN32
    std::string log_save_path = appDataPath + "\\" + "logs";
    bool isPathExist =
        cloud_base::file_common::guarantee_directory_exists(log_save_path);
    if (isPathExist) {
      log_file_name = log_save_path + "\\" + log_name;
    }
#else
        std::string log_save_path = appDataPath + '/' + "logs/nativeLogs";
        bool isPathExist =
            cloud_base::file_common::guarantee_directory_exists(log_save_path);
        if (isPathExist) {
           log_file_name = log_save_path + "/" + log_name;
        }
#endif
    out =
        assistant::core::readwrite::details::fopen(log_file_name.c_str(), "w");
    if (nullptr != out) {
      initfile_success = true;
    }
    if (!initfile_success) {
      out = stdout;
    }
  });
}  // namespace
}  // namespace

namespace general_restful_sdk_ast {
namespace log_system {
namespace details {
inline void Log(const char* type, const char* log_str) {
  InitOut();
  /// 获取日期时间、当前线程号
  fprintf(out, "[%s][%lu][%s] %s\n", Datetime().c_str(), Threadid(), type,
          log_str);
}
inline void LogInfo(const char* log_str) { return Log("INFO", log_str); }

}  // namespace details

void LogInfo(const char* format, ...) {
  std::string result;
  va_list args;
  va_start(args, format);
  auto size_1 = vsnprintf(nullptr, 0, format, args);
  va_end(args);
  result.resize(size_1 + 1);
  // Notice that only when this returned value is non - negative and less than
  // n, the string has been completely written.
  va_start(args, format);
  auto size_2 = vsnprintf((char*)result.data(), size_1 + 1, format, args);
  va_end(args);
  result.resize(size_2);
  va_end(args);
  details::LogInfo(result.c_str());
}
}  // namespace log_system
}  // namespace general_restful_sdk_ast
