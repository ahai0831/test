// MiniDump.h: interface for the CMiniDump class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#ifndef MINIDUMP_MINIDUMP_H__
#define MINIDUMP_MINIDUMP_H__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <DbgHelp.h>

namespace cloud_base {

class CMiniDump {
 public:
  //设置转储类型,默认是全内存转储
  static void SetMiniDumpType(MINIDUMP_TYPE);
  //设置dump文件名前缀
  static void SetMiniDumpPrefixName(LPCTSTR);
  //设置错误报告程序
  static void SetReportProgram(LPCTSTR);
  //开始捕获未处理异常
  static void StartExceptionFilter();
  //设置转储文件路径
  static void SetMiniDumpPath(LPCTSTR);
  static CMiniDump* Instance();
  virtual ~CMiniDump() = default;
  CMiniDump() = default;

 private:
  //创建目录
  static BOOL CreateFolder(LPCTSTR);
  //判断一个目录是否存在
  static BOOL FolderExist(LPCTSTR);
  static LPTOP_LEVEL_EXCEPTION_FILTER m_lpPrevTopLevelFilter;
  static LPTSTR m_lpstrMiniDumpPath;
  static LPTSTR m_lpstrReportProgram;
  static LPTSTR m_lpstrPrefixName;
  static MINIDUMP_TYPE m_DumpType;
  //内部函数,作为未处理异常的处理函数
  static LONG WINAPI ExceptionFilter(PEXCEPTION_POINTERS);
};
}  // namespace cloud_base

#endif  // MINIDUMP_MINIDUMP_H__
