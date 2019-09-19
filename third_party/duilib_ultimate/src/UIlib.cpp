//
//
// DirectUI - UI Library
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2006-2007 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. These
// source files may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//
//


#include "stdafx.h"
#include "UIlib.h"
#include <shlobj.h>

inline void EnableMemLeakCheck()
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}

//#ifdef _DEBUG
//#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
//#endif

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID /*lpReserved*/)
{
	switch( dwReason ) {
   case DLL_PROCESS_ATTACH:
	   {
// #ifndef _DEBUG
			//CString strCurPath;
		   TCHAR szCurPath[MAX_PATH] = {0};
		   SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_DEFAULT,
				szCurPath);
// #endif
			//EnableMemLeakCheck();
			break;
	   }
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
       ::DisableThreadLibraryCalls((HMODULE)hModule);
       break;
    }
    return TRUE;
}

