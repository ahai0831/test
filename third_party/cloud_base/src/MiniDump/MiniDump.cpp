// MiniDump.cpp: implementation of the CMiniDump class.
//
//////////////////////////////////////////////////////////////////////
#include "MiniDump.h"

#include <atlstr.h>
#include <tchar.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace cloud_base {

namespace {
// Patch for SetUnhandledExceptionFilter
const BYTE PatchBytes[5] = {0x33, 0xC0, 0xC2, 0x04, 0x00};

// Original bytes at the beginning of SetUnhandledExceptionFilter
BYTE OriginalBytes[5] = {0};

bool EnforceFilter(bool bEnforce);
bool WriteMemory(BYTE* pTarget, const BYTE* pSource, DWORD Size);

////////////////////////////////////////////////////////////////////////////////
// EnforceFilter function
//

bool EnforceFilter(bool bEnforce) {
  DWORD ErrCode = 0;

  // Obtain the address of SetUnhandledExceptionFilter

  HMODULE hLib = GetModuleHandle(_T("kernel32.dll"));

  if (hLib == NULL) {
    ErrCode = GetLastError();
    _ASSERTE(!_T("GetModuleHandle(kernel32.dll) failed."));
    return false;
  }

  BYTE* pTarget = (BYTE*)GetProcAddress(hLib, "SetUnhandledExceptionFilter");

  if (pTarget == 0) {
    ErrCode = GetLastError();
    _ASSERTE(!_T("GetProcAddress(SetUnhandledExceptionFilter) failed."));
    return false;
  }

  if (IsBadReadPtr(pTarget, sizeof(OriginalBytes))) {
    _ASSERTE(!_T("Target is unreadable."));
    return false;
  }

  if (bEnforce) {
    // Save the original contents of SetUnhandledExceptionFilter

    memcpy(OriginalBytes, pTarget, sizeof(OriginalBytes));

    // Patch SetUnhandledExceptionFilter

    if (!WriteMemory(pTarget, PatchBytes, sizeof(PatchBytes))) return false;

  } else {
    // Restore the original behavior of SetUnhandledExceptionFilter

    if (!WriteMemory(pTarget, OriginalBytes, sizeof(OriginalBytes)))
      return false;
  }

  // Success

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// WriteMemory function
//

bool WriteMemory(BYTE* pTarget, const BYTE* pSource, DWORD Size) {
  DWORD ErrCode = 0;

  // Check parameters

  if (pTarget == 0) {
    _ASSERTE(!_T("Target address is null."));
    return false;
  }

  if (pSource == 0) {
    _ASSERTE(!_T("Source address is null."));
    return false;
  }

  if (Size == 0) {
    _ASSERTE(!_T("Source size is null."));
    return false;
  }

  if (IsBadReadPtr(pSource, Size)) {
    _ASSERTE(!_T("Source is unreadable."));
    return false;
  }

  // Modify protection attributes of the target memory page

  DWORD OldProtect = 0;

  if (!VirtualProtect(pTarget, Size, PAGE_EXECUTE_READWRITE, &OldProtect)) {
    ErrCode = GetLastError();
    _ASSERTE(!_T("VirtualProtect() failed."));
    return false;
  }

  // Write memory

  memcpy(pTarget, pSource, Size);

  // Restore memory protection attributes of the target memory page

  DWORD Temp = 0;

  if (!VirtualProtect(pTarget, Size, OldProtect, &Temp)) {
    ErrCode = GetLastError();
    _ASSERTE(!_T("VirtualProtect() failed."));
    return false;
  }

  // Success

  return true;
}
};  // namespace

//转储类型 默认为全内存转储
MINIDUMP_TYPE CMiniDump::m_DumpType = MiniDumpWithFullMemory;
// dump文件名前缀部分 默认是"Program"
LPTSTR CMiniDump::m_lpstrPrefixName = NULL;
//错误报告程序文件名
LPTSTR CMiniDump::m_lpstrReportProgram = NULL;
//保存前一个顶层异常过滤
LPTOP_LEVEL_EXCEPTION_FILTER CMiniDump::m_lpPrevTopLevelFilter = NULL;
//存储转储文件的路径 默认为临时文件夹
LPTSTR CMiniDump::m_lpstrMiniDumpPath = NULL;
//单实例函数
CMiniDump* CMiniDump::Instance() {
  static CMiniDump* _Instance = new CMiniDump;
  return _Instance;
}

//////////////////////////////////////////////////////////////////////////
//内部函数,作为未处理异常的处理函数
// ExceptionInfo 有操作系统传入的未处理异常的信息
//////////////////////////////////////////////////////////////////////////
#define TIME_FORMAT_LEN 256
LONG WINAPI CMiniDump::ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo) {
  MINIDUMP_EXCEPTION_INFORMATION miniExcInfo;
  SYSTEMTIME st;
  LPTSTR lpParentPath;
  LPTSTR lpPrefixName;
  // dump文件后缀名
  LPTSTR lpPostfixName = _T(".dmp");
  TCHAR timeBuffer[TIME_FORMAT_LEN];
  TCHAR tmpPath[MAX_PATH];

  //获取当前系统本地时间
  GetLocalTime(&st);
  //通过获取的时间得到一个部分文件名
  _sntprintf(timeBuffer, TIME_FORMAT_LEN,
             _T("_Crash_Dump__Date__%d_%d_%d__Time_%d_%d_%d_%d"), st.wYear,
             st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
             st.wMilliseconds);
  //父目录
  if (m_lpstrMiniDumpPath) {
    lpParentPath = m_lpstrMiniDumpPath;
  } else {
    //没有设置默认为临时目录
    DWORD dwRequireLen = GetTempPath(MAX_PATH - 1, tmpPath);
    lpParentPath = tmpPath;
    // assert(dwRequireLen && dwRequireLen < MAX_PATH);
  }
  //文件前缀名 默认为"Program"
  if (m_lpstrPrefixName) {
    lpPrefixName = m_lpstrPrefixName;
  } else {
    lpPrefixName = _T("Program");
  }
  TCHAR Buffer[MAX_PATH];
  //文件名格式如下:
  // Parent\\Prefix|timeBuffer\\Prefix|timeBuffer|Postfix
  _tcscpy(Buffer, lpParentPath);
  if (!FolderExist(lpParentPath)) {
    //目录不存在就创建该目录
    CreateFolder(lpParentPath);
  }
  _tcscat(Buffer, _T("\\"));
  _tcscat(Buffer, lpPrefixName);
  _tcscat(Buffer, timeBuffer);
  _tcscat(Buffer, lpPostfixName);
  //创建dump文件
  HANDLE hFile = CreateFile(Buffer, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  //存储异常信息
  miniExcInfo.ThreadId = GetCurrentThreadId();
  miniExcInfo.ExceptionPointers = ExceptionInfo;
  miniExcInfo.ClientPointers = FALSE;
  //写dump文件
  MINIDUMP_TYPE nType =
      (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithHandleData |
                      MiniDumpWithUnloadedModules |
                      MiniDumpWithProcessThreadData);  // MiniDumpNormal
  //(MINIDUMP_TYPE)(0x00000001 | 0x00000002 | 0x00000004 | 0x00000100
  //|0x00000200 |0x00000800 | 0x00001000  );
  BOOL bOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                               hFile, nType, &miniExcInfo, NULL, NULL);
  CloseHandle(hFile);
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  if (m_lpstrReportProgram) {
    //创建错误报告进程通知客户做相应的处理 命令行为dump文件名
    CString cmdline = m_lpstrReportProgram;
    cmdline += _T(" -p ");
    cmdline += lpPrefixName;
    cmdline += timeBuffer;
    cmdline += lpPostfixName;
    // OutputDebugString(cmdline);
    CreateProcess(m_lpstrReportProgram, (LPTSTR)(LPCTSTR)cmdline, NULL, NULL,
                  FALSE, 0, NULL, NULL, &si, &pi);
  }
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  EnforceFilter(false);

  return EXCEPTION_EXECUTE_HANDLER;
  return EXCEPTION_CONTINUE_SEARCH;
}

//////////////////////////////////////////////////////////////////////////
//设置转储文件路径
//
//        lpstrPath 路径位置
//////////////////////////////////////////////////////////////////////////
void CMiniDump::SetMiniDumpPath(LPCTSTR lpstrPath) {
  if (IsBadReadPtr(lpstrPath, sizeof(TCHAR))) return;
  if (m_lpstrMiniDumpPath) delete[] m_lpstrMiniDumpPath;
  size_t dwLen = _tcslen(lpstrPath) + 1;
  m_lpstrMiniDumpPath = new TCHAR[dwLen * sizeof(TCHAR)];
  _tcscpy(m_lpstrMiniDumpPath, lpstrPath);
}

//////////////////////////////////////////////////////////////////////////
//开始捕获未处理异常
//////////////////////////////////////////////////////////////////////////
void CMiniDump::StartExceptionFilter() {
  m_lpPrevTopLevelFilter = SetUnhandledExceptionFilter(ExceptionFilter);

  EnforceFilter(true);
  // return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//设置错误报告程序
// lpstrReportProgram 错误报告程序文件名
//////////////////////////////////////////////////////////////////////////
void CMiniDump::SetReportProgram(LPCTSTR lpstrReportProgram) {
  if (IsBadReadPtr(lpstrReportProgram, sizeof(TCHAR))) return;
  if (m_lpstrReportProgram) delete[] m_lpstrReportProgram;
  size_t dwLen = _tcslen(lpstrReportProgram) + 1;
  m_lpstrReportProgram = new TCHAR[dwLen * sizeof(TCHAR)];
  _tcscpy(m_lpstrReportProgram, lpstrReportProgram);
}

//////////////////////////////////////////////////////////////////////////
//设置dump文件名前缀
// lpstrPrefixName 文件名前缀部分
//////////////////////////////////////////////////////////////////////////
void CMiniDump::SetMiniDumpPrefixName(LPCTSTR lpstrPrefixName) {
  if (IsBadReadPtr(lpstrPrefixName, sizeof(TCHAR))) return;
  if (m_lpstrPrefixName) delete[] m_lpstrPrefixName;
  size_t dwLen = _tcslen(lpstrPrefixName) + 1;
  m_lpstrPrefixName = new TCHAR[dwLen * sizeof(TCHAR)];
  _tcscpy(m_lpstrPrefixName, lpstrPrefixName);
}

//////////////////////////////////////////////////////////////////////////
//判断一个目录是否存在
//////////////////////////////////////////////////////////////////////////
BOOL CMiniDump::FolderExist(LPCTSTR strPath) {
  WIN32_FIND_DATA wfd;
  BOOL rValue = FALSE;
  HANDLE hFind = FindFirstFile(strPath, &wfd);
  if ((hFind != INVALID_HANDLE_VALUE) &&
      (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
    rValue = TRUE;
  }
  if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);
  return rValue;
}

//////////////////////////////////////////////////////////////////////////
//创建目录
//////////////////////////////////////////////////////////////////////////
BOOL CMiniDump::CreateFolder(LPCTSTR strPath) {
  SECURITY_ATTRIBUTES attrib;
  attrib.bInheritHandle = FALSE;
  attrib.lpSecurityDescriptor = NULL;
  attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
  //上面定义的属性可以省略。 直接return ::CreateDirectory( path, NULL); 即可
  return CreateDirectory(strPath, &attrib);
}

//////////////////////////////////////////////////////////////////////////
//设置转储类型,默认是全内存转储
//////////////////////////////////////////////////////////////////////////
void CMiniDump::SetMiniDumpType(MINIDUMP_TYPE DumpType) {
  m_DumpType = DumpType;
}
}  // namespace cloud_base
