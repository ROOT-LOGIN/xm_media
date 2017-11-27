#include "StdAfx.h"
#include "resource.h"

#include "_xm_helpers.h"
#include "xm_scrnsvr.h"
#include "shellapi.h"
#include <iostream>

#if defined(DEBUG)||defined(_DEBUG)
#define new XM_NEW
#endif

// {F2061DDE-EDBA-4474-9240-0B233A6BDDA0}
extern "C" const CLSID CLSID_CXpeulMediaSSInvoker=
{ 0xf2061dde, 0xedba, 0x4474, { 0x92, 0x40, 0xb, 0x23, 0x3a, 0x6b, 0xdd, 0xa0 } };

CXpeulMediaSSInvoker::CXpeulMediaSSInvoker(void)
{
	m_hWndparent=NULL;
	mp_wmpCore=NULL;
	m_CurrentPreset=0;
	memset(&m_wndRect, 0, sizeof(RECT));
	memset(&m_ProcessInfo, 0,sizeof(PROCESS_INFORMATION));
}

CXpeulMediaSSInvoker::~CXpeulMediaSSInvoker(void)
{
	TerminateScreenSaver();
}

//IWMPEffects
STDMETHODIMP CXpeulMediaSSInvoker::Render(TimedLevel* pLevels, HDC hdc, RECT* prc)
{
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::MediaInfo(long lChannelCount, long lSampleRate, BSTR bstrTitle)
{
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::GetCapabilities(DWORD* pdwCapabilities)
{
	*pdwCapabilities=EFFECT_CANGOFULLSCREEN|EFFECT_HASPROPERTYPAGE;
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::GetTitle(BSTR* bstrTitle)
{
	*bstrTitle=SysAllocString(L"XpeulMediaSSInvoker");
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::GetPresetTitle(long nPreset, BSTR* bstrPresetTitle)
{
	if(bstrPresetTitle)
	{
		HKEY hKey;
		RegOpenKey(HKEY_CURRENT_USER, L"SoftWare\\XpeulEnterprise\\XpeulMedia\\XpeulScrnSvrInvoker", &hKey);
		WCHAR name[6]; memset(name, 0, sizeof(WCHAR)*6);
		_i64tow(nPreset, name, 10);
		WCHAR buffer[MAX_PATH];
		DWORD dwc=MAX_PATH;
		RegGetValue(hKey, NULL, name, RRF_RT_REG_SZ, NULL, buffer, &dwc);
		RegCloseKey(hKey);
		WCHAR* p=buffer;
		while(*p!=L'|') p++;
		*p=L'\0';
		*bstrPresetTitle=SysAllocString(buffer);
	}
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::GetPresetCount(long* pnPresetCount)
{
	HKEY hKey;
	RegOpenKey(HKEY_CURRENT_USER, L"SoftWare\\XpeulEnterprise\\XpeulMedia\\XpeulScrnSvrInvoker", &hKey);
	DWORD dwc=sizeof(long);
	RegGetValue(hKey, NULL, L"counts", RRF_RT_REG_DWORD, NULL, pnPresetCount, &dwc);
	RegCloseKey(hKey);
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::SetCurrentPreset(long nPreset)
{
	m_CurrentPreset=nPreset;
	return RunScreenSaver();
}

STDMETHODIMP CXpeulMediaSSInvoker::GetCurrentPreset(long* pnPreset)
{
	*pnPreset=m_CurrentPreset;

	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::DisplayPropertyPage(HWND hwndOwner)
{
	wchar_t info[MAX_PATH]={'\0'};
	LoadString(g_data.GetInstance(), IDS_SCRNSVR_PROPERTYPAGE_INFO, info, MAX_PATH);
	wchar_t title[MAX_PATH]={'\0'};
	LoadString(g_data.GetInstance(), IDS_SCRNSVR_NAME, title, MAX_PATH);

	MessageBoxW(hwndOwner, info, title, MB_ICONINFORMATION|MB_OK);
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::GoFullscreen(BOOL isfullscreen)
{
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::RenderFullScreen(TimedLevel* pLevels)
{
	return RenderWindowed(pLevels, TRUE);
}

//IWMPEffects2
STDMETHODIMP CXpeulMediaSSInvoker::Create(HWND hwndparent)
{
	m_hWndparent=hwndparent;
	RunScreenSaver();
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::Destroy(void)
{
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::SetCore(IWMPCore* pCore)
{	
	if(pCore==NULL)
		mp_wmpCore->Release();
	else{
		pCore->AddRef();
		mp_wmpCore=pCore;
	}
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::NotifyNewMedia(IWMPMedia* pMedia)
{
	return S_OK;
}

STDMETHODIMP CXpeulMediaSSInvoker::OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT* plResult)
{
	return S_FALSE;
}

STDMETHODIMP CXpeulMediaSSInvoker::RenderWindowed(TimedLevel* pLevels, BOOL bRequiredRender)
{
		if(m_ProcessInfo.hProcess)
		{
			RECT wrc;
			GetClientRect(m_hWndparent, &wrc);
			if(memcmp(&wrc, &m_wndRect, sizeof(RECT))!=0)
			{
				HWND hwndScrn=FindWindowEx(m_hWndparent, NULL, L"D3DSaverWndClass", NULL);
				SetWindowPos(hwndScrn, NULL, wrc.left, wrc.top, wrc.right-wrc.left, wrc.bottom-wrc.top, SWP_SHOWWINDOW);
				memcpy(&m_wndRect, &wrc, sizeof(RECT));
			}
		}
	return S_OK;
}

void CXpeulMediaSSInvoker::FillRegistry(bool bInstall)
{
	HKEY hKey=NULL;
	if(bInstall==false){
		RegOpenKey(HKEY_CURRENT_USER, L"SoftWare\\XpeulEnterprise\\XpeulMedia", &hKey);
		NULL_RET(hKey);
		RegDeleteKey(hKey, L"XpeulScrnSvrInvoker");
		goto label_CloseKey;
	}
		
	RegOpenKey(HKEY_CURRENT_USER, L"SoftWare\\XpeulEnterprise\\XpeulMedia\\XpeulScrnSvrInvoker", &hKey);
	if(!hKey){
		RegCreateKey(HKEY_CURRENT_USER, L"SoftWare\\XpeulEnterprise\\XpeulMedia\\XpeulScrnSvrInvoker", &hKey);
	}
	NULL_RET(hKey);
	WCHAR SysDir[32];
	memset(SysDir, 0, sizeof(WCHAR)*32);
	GetSystemDirectory(SysDir, 32);
	long nCount=0;
	WCHAR Scrn[32];
	memcpy(Scrn, SysDir, sizeof(WCHAR)*32);
	PathAppend(Scrn, L"*.scr");
	WIN32_FIND_DATA find_data;
	HANDLE handle=FindFirstFile(Scrn, &find_data);
	if(handle!=INVALID_HANDLE_VALUE)
	{
		do{
			WCHAR Scrn[MAX_PATH];
			wmemcpy(Scrn, SysDir, MAX_PATH);
			PathAppend(Scrn, find_data.cFileName);
			std::wstring val;
			WCHAR ser[6];
			_i64tow(nCount, ser, 10);
			HINSTANCE hi = LoadLibraryEx(Scrn, NULL, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
			WCHAR name[32];
			if(hi)
			{
				wmemset(name, 0, 32);
				LoadString(hi, 1, name, 32);
				FreeLibrary(hi);
			}
			val=name; val+=L'|'; val+=Scrn;
			RegSetValueEx(hKey, ser, NULL, REG_SZ, (const BYTE*)val.c_str(), (val.size()+1)*sizeof(WCHAR));
			nCount++;
		}while(FindNextFileW(handle, &find_data));
		FindClose(handle);
	}
	RegSetValueExW(hKey, L"counts", NULL, REG_DWORD, (const BYTE*)&nCount, sizeof(long));
	
label_CloseKey:
	RegCloseKey(hKey);
}

STDMETHODIMP CXpeulMediaSSInvoker::RunScreenSaver(void)
{
	if(m_hWndparent)
	{
		TerminateScreenSaver();
		HKEY hKey=NULL;
		RegOpenKey(HKEY_CURRENT_USER, L"SoftWare\\XpeulEnterprise\\XpeulMedia\\XpeulScrnSvrInvoker", &hKey);
		NULL_RET(hKey) E_FAIL;
		WCHAR name[6]; wmemset(name, 0, 6);
		_i64tow(m_CurrentPreset, name, 10);
		WCHAR buffer[MAX_PATH];
		DWORD dwc=MAX_PATH;
		RegGetValue(hKey, NULL, name, RRF_RT_REG_SZ, NULL, buffer, &dwc);
		RegCloseKey(hKey);

		WCHAR* p=buffer;
		while(*p!=L'|') p++; p++;
		WCHAR* path=p;
		while(*p) p++;
		while(*p!=L'\\') p--; p++;
		std::wstring cmd_line=p;
		cmd_line+=L" /p ";
		WCHAR nh[16];
		wmemset(nh, 0, 16);
		_i64tow((__int64)m_hWndparent, nh, 10);
		cmd_line+=nh;
		STARTUPINFO StartupInfo;
		memset(&StartupInfo, 0, sizeof(STARTUPINFO));
		BOOL b=CreateProcess(path, (LPWSTR)cmd_line.c_str(), NULL, NULL, FALSE, IDLE_PRIORITY_CLASS, NULL, NULL, &StartupInfo, &m_ProcessInfo);
		if(b)
			return S_OK;
	}
	return E_FAIL;
}

BOOL CXpeulMediaSSInvoker::TerminateScreenSaver(void)
{
	if(m_ProcessInfo.hProcess)
	{
		BOOL b=TerminateProcess(m_ProcessInfo.hProcess, 0);
		CloseHandle(m_ProcessInfo.hThread);
		CloseHandle(m_ProcessInfo.hProcess);
		m_ProcessInfo.hProcess=NULL;
		return b;
	}
	return TRUE;
}