// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__E30B2003_188B_4EB4_AB99_3F3734D6CE6C__INCLUDED_)
#define AFX_STDAFX_H__E30B2003_188B_4EB4_AB99_3F3734D6CE6C__INCLUDED_

#pragma once

#define _CRT_SECURE_NO_DEPRECATE

// Remove pointless warning messages
#ifdef _MSC_VER
#pragma warning (disable : 4511) // copy operator could not be generated
#pragma warning (disable : 4512) // assignment operator could not be generated
#pragma warning (disable : 4702) // unreachable code (bugs in Microsoft's STL)
#pragma warning (disable : 4786) // identifier was truncated
#pragma warning (disable : 4996) // function or variable may be unsafe (deprecated)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS // eliminate deprecation warnings for VS2005
#endif
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -w-8027		   // function not expanded inline
#endif

// Required for VS 2008 (fails on XP and Win2000 without this fix)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include "UIlib.h"

#include <olectl.h>

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define MAX max
#define MIN min
#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

/// 内存泄露测试
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#endif // !defined(AFX_STDAFX_H__E30B2003_188B_4EB4_AB99_3F3734D6CE6C__INCLUDED_)

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

//#ifndef WINVER				// 允许使用特定于 Windows XP 或更高版本的功能。
//#define WINVER 0x0501		// 将此值更改为相应的值，以适用于 Windows 的其他版本。
//#endif
//
//#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
//#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
//#endif						
//
//#ifndef _WIN32_WINDOWS		// 允许使用特定于 Windows 98 或更高版本的功能。
//#define _WIN32_WINDOWS 0x0510 // 将此值更改为适当的值，以指定将 Windows Me 或更高版本作为目标。
//#endif
//
//#ifndef _WIN32_IE			// 允许使用特定于 IE 6.0 或更高版本的功能。
//#define _WIN32_IE 0x0601	// 将此值更改为相应的值，以适用于 IE 的其他版本。
//#endif
//
//
//#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
//#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_
//
//#pragma once
//
//#include "UIlib.h"
//
//#include <olectl.h>
//
//#define lengthof(x) (sizeof(x)/sizeof(*x))
//#define MAX max
//#define MIN min
//#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))
//
//
////{{AFX_INSERT_LOCATION}}
//// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//
//#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

///// 内存泄露测试
//#ifdef _DEBUG
//#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
//#endif