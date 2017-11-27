#pragma once

#ifndef __XM_STDAFX_H__
#define __XM_STDAFX_H__


#include "targetver.h"

#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_NO_WARNINGS

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

//#include <windows.h>
//#include <windowsx.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS 

#include <atlbase.h>
#include <atlwin.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atltypes.h>

#include <uxtheme.h>
#include <vsstyle.h>
#pragma comment(lib, "uxtheme.lib")
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")

#include <Gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <wmp.h>
#include <wmpplug.h>
#include <effects.h>
#include <shlobj.h>

using namespace ATL;

// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

#pragma warning(disable:4100)

#endif