#include "stdafx.h"
#include "resource.h"

#include "dllmain.h"

#include "xm_back.h"
#include "xm_effect.h"
#include "xm_scrnsvr.h"

#include "_xm_helpers.h"
#include "xm_log.h"

#if defined(DEBUG)||defined(_DEBUG)
#define new XM_NEW
#endif

CXpeulMediaModule::CXpeulMediaModule(void)
{
	::CoInitialize(NULL);
}

CXpeulMediaModule::~CXpeulMediaModule(void)
{
	::CoUninitialize();
}

CXpeulMediaModule _AtlModule;

GlobalData g_data;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD call_reason, LPVOID lpReserved)
{
	switch(call_reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_data.hDllInst=hModule;
			::XmPrepareLog();
			::XmWriteLog("DllMain_Enter", sizeof("DllMain_Enter")-1, "DllMain_Enter", sizeof("DllMain_Enter")-1);
		}break;
	case DLL_PROCESS_DETACH:
		{
			::XmWriteLog("DllMain_Leave", sizeof("DllMain_Leave")-1, "DllMain_Leave", sizeof("DllMain_Leave")-1);
			::XmFinishLog();
#if defined( DEBUG)||defined( _DEBUG)
			::_CrtDumpMemoryLeaks();
#endif
		}break;
	}
	return _AtlModule.DllMain(call_reason, lpReserved);
}


STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


STDAPI DllRegisterServer(void)
{
	::XmWriteLog("DllRegisterServer", sizeof("DllRegisterServer")-1, "DllRegisterServer", sizeof("DllRegisterServer")-1);

	_AtlModule.DllRegisterServer(FALSE);

	OSVERSIONINFOW os;
	os.dwOSVersionInfoSize=sizeof(OSVERSIONINFOW);
	::GetVersionExW(&os);
	if(os.dwMajorVersion!=6||os.dwMinorVersion!=0){
		MessageBoxW(NULL, L"Warning: Current OS is NOT Windows Vista,\n   the XpeulMedia plug-in may run unexpectedly!",
				L"Warning: Windows Media Player", MB_OK|MB_ICONEXCLAMATION);
	}
	CXpeulMediaEffect::UpdateRegistry(TRUE);
	CXpeulMediaEffect::FillRegistry();
	CXpeulMediaBack::UpdateRegistry(TRUE);
	CXpeulMediaSSInvoker::UpdateRegistry(TRUE);
	CXpeulMediaSSInvoker::FillRegistry();
    WMPNotifyPluginAddRemove();
	
	return S_OK;
}


STDAPI DllUnregisterServer(void)
{
	::XmWriteLog("DllUnregisterServer", sizeof("DllUnregisterServer")-1, "DllUnregisterServer", sizeof("DllUnregisterServer")-1);

	CXpeulMediaEffect::UpdateRegistry(FALSE);
	CXpeulMediaBack::UpdateRegistry(FALSE);
	CXpeulMediaSSInvoker::UpdateRegistry(FALSE);
	CXpeulMediaSSInvoker::FillRegistry(false);
    WMPNotifyPluginAddRemove();
	HKEY hKey=NULL;
	RegOpenKey(HKEY_CURRENT_USER, _T("Software\\XpeulEnterprise"), &hKey);
	if(hKey){
		RegDeleteKey(hKey, _T("XpeulMedia"));
		RegCloseKey(hKey);
	}
	return _AtlModule.DllUnregisterServer(FALSE);
}
