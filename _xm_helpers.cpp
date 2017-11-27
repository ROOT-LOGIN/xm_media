#include "stdafx.h"
#include "resource.h"

#include "_xm_helpers.h"
#include "xm_log.h"
#include "shellapi.h" 

void QueryTaskbarPos(APPBARDATA& abd, bool force_to_workarea)
{
	abd.cbSize = sizeof(APPBARDATA);
	::SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
	if(force_to_workarea)
	{
		::SystemParametersInfo(SPI_GETWORKAREA, NULL, &abd.rc, 0);
	}
}

/*
// *****************************************************************************
*/

DWORD STUB_WINDOW::BeginInterceptionGame(HWND hwnd)
{
	_ASSERT(::IsWindow(hwnd));
	// make sure NO window be attached
	_ASSERT(m_hWnd == NULL);
	m_hWnd = hwnd;
	AtlTrace(L"BeginInterception: hwnd: 0x%08X.\n", m_hWnd);
	// since we won't create window, let's Init thunk first
	this->m_thunk.Init(GetWindowProc(), this);
	// now, change WndProc to this->m_thunk
	SetLastError(0);
	m_oldwndproc = (WNDPROC)SetWindowLong(GWL_WNDPROC, (LONG)this->m_thunk.GetWNDPROC());
#ifdef DEBUG
	AtlTrace(L"-->GetLastError() returns %d.\n", GetLastError());
#endif
	return GetLastError();
}

DWORD STUB_WINDOW::EndInterceptionGame(void)
{
	_ASSERT(m_hWnd); _ASSERT(m_oldwndproc);
	AtlTrace(L"EndInterception: hwnd: 0x%08X.\n", m_hWnd);
	SetLastError(0);
	if(IsWindow()) //our window is still existing
	{
		//since we havn't create window, the only necessary is restore WndProc
		SetWindowLong(GWL_WNDPROC, (LONG)this->m_oldwndproc);
	}
	//else //our window has been destroyed
	m_hWnd = NULL;
	m_oldwndproc = NULL;
	return GetLastError();
}

LRESULT CALLBACK STUB_WINDOW::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ATLASSERT(hWnd);
	ATLASSERT(::IsWindow(hWnd) == FALSE);
	static HWND hwndParent;
#ifdef DEBUG
	STUB_WINDOW* pThis = dynamic_cast<STUB_WINDOW*>((STUB_WINDOW*)hWnd);
	ATLASSERT(pThis);
#else
	STUB_WINDOW* pThis = (STUB_WINDOW*)hWnd;
#endif
	if(hwndParent != pThis->GetParent())
	{
		hwndParent = pThis->GetParent();
	}
	ATLASSERT(pThis->m_hWnd);
	ATLASSERT(pThis->m_oldwndproc);
// set m_pCurrentMsg point to this message
	_ATL_MSG msg(pThis->m_hWnd, uMsg, wParam, lParam);
	pThis->m_pCurrentMsg = &msg;

	pThis->m_if_flag = 0;
// since we are intercepting, let pass this msg to our handler first
	pThis->m_lresult = 0;
	pThis->ProcessWindowMessage(pThis->m_hWnd, msg.message, msg.wParam, msg.lParam, pThis->m_lresult, 0);
	if((pThis->m_if_flag == IF_Intercepte)||(pThis->m_oldwndproc == NULL))
	{
		return pThis->m_lresult;
	}
// now, let origion wndproc to do something
	if(pThis->m_if_flag == 0)
		pThis->m_if_flag = IF_Pass;
#ifdef DEBUG
	else if(LOWORD(pThis->m_if_flag) == IF_INVALID_METHOD)
	{
		ATLASSERT(false); //IF_Pass and IF_Dispatch cann't combine
		pThis->m_if_flag &= ~IF_Dispatch;
	}
#endif
	if(pThis->m_if_flag & IF_Pass)
	{
		pThis->m_lresult = ::CallWindowProcW(pThis->m_oldwndproc, pThis->m_hWnd, uMsg, wParam, lParam);
	}
	else if(pThis->m_if_flag & IF_Dispatch)
	{
		pThis->m_lresult = ::CallWindowProcW(pThis->m_oldwndproc, pThis->m_hWnd, msg.message, msg.wParam, msg.lParam);
	}
// finally, let we process this msg again
	if(pThis->m_if_flag == IF_PassAndReflect)
	{
		pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, pThis->m_lresult, 0);
	}
	else if(pThis->m_if_flag == IF_DispatchAndReflect)
	{
		pThis->ProcessWindowMessage(pThis->m_hWnd, msg.message, msg.wParam, msg.lParam, pThis->m_lresult, 0);
	}
	return pThis->m_lresult;
}

/*
// *****************************************************************************
*/


/*
===========================================
*/

CXpeulMediaBackHelper::CXpeulMediaBackHelper(void)
	: m_hotkey_Play_Pause(str_hotkey_playpause), m_hotkey_Stop(str_hotkey_stop), m_hotkey_Next(str_hotkey_next),
		m_hotkey_Prev(str_hotkey_prev), m_hotkey_VolUp(str_hotkey_volup), m_hotkey_VolDn(str_hotkey_voldn),
		m_hotkey_TrayIcon(str_hotkey_trayicon), m_hotkey_VideoHook(str_hotkey_videohook)
{
	mp_wmpCore=NULL;
	TrayIconState=false; hwndTrayIconSinkWnd=NULL;
	VideoSinkState=false; hwndVideoSinkWnd=NULL;
	hwndWmpCtrlCntr=NULL; hwndWmpPluginHost=NULL;
	WPH_WndProc=NULL; memset(&rcWPH, 0, sizeof(RECT));
}
	
CXpeulMediaBackHelper::~CXpeulMediaBackHelper(void)
{
	mp_wmpCore.Release();
	SetTrayIconSinkWnd();

	if(hwndVideoSinkWnd){
		CloseWindow(hwndVideoSinkWnd);
		DestroyWindow(hwndVideoSinkWnd);
		hwndVideoSinkWnd=NULL;
	}
}

void CXpeulMediaBackHelper::ShowWmp(void) const
{
	if((TrayIconState==false)&&(VideoSinkState==false)){
		ShowWindow(g_data.GetWmp(), SW_SHOW);
		SetForegroundWindow(g_data.GetWmp());
	}
	else
		ShowWindow(g_data.GetWmp(), SW_HIDE);
}

bool CXpeulMediaBackHelper::SetTrayIconSinkWnd(void)
{
	DWORD dwMsg=NIM_ADD;
	if(hwndTrayIconSinkWnd&&TrayIconState){
		dwMsg=NIM_DELETE;
		goto label_SET_TRAY_ICON;
	}

	WNDCLASSEX wcsex;
	memset(&wcsex, 0, sizeof(WNDCLASSEX));
	wcsex.cbSize=sizeof(WNDCLASSEX);
	wcsex.lpfnWndProc=XmTrayIconProc;
	wcsex.hInstance=g_data.GetInstance();
	wcsex.lpszClassName=L"XmWmpTrayIconSinkWnd";
	wcsex.style=CS_DBLCLKS;
	RegisterClassEx(&wcsex);

	hwndTrayIconSinkWnd=CreateWindowEx(WS_EX_LEFT, L"XmWmpTrayIconSinkWnd", L"XmWmpTrayIcon Sink Window",
		WS_CHILD, 0, 0, 16, 16, g_data.GetWmp(), NULL, g_data.GetInstance(), NULL);

	if(hwndTrayIconSinkWnd==NULL){ TrayIconState=false; return false;	}

label_SET_TRAY_ICON:
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(NOTIFYICONDATA));
	nid.cbSize=sizeof(NOTIFYICONDATA);
	nid.hWnd=hwndTrayIconSinkWnd;
	nid.hIcon=LoadIcon(g_data.GetInstance(), MAKEINTRESOURCE(IDI_TRAYICON));
	if(dwMsg==NIM_ADD){
		nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
		nid.uCallbackMessage=XM_WM_NOTIFYICON;
		memcpy(nid.szTip, _T("Windows Media Player NotifyIcon\nDeveloped by XpeulEnterprise Co.Ltd."), 
			sizeof("Windows Media Player NotifyIcon\nDeveloped by XpeulEnterprise Co.Ltd.")*sizeof(wchar_t));
	}
	if(Shell_NotifyIcon(dwMsg, &nid)){
		TrayIconState=!TrayIconState;
		if(dwMsg==NIM_DELETE){
			DestroyWindow(hwndTrayIconSinkWnd);
			hwndTrayIconSinkWnd=NULL;
		}else{
			nid.uVersion=NOTIFYICON_VERSION_4;
			Shell_NotifyIcon(NIM_SETVERSION, &nid);
		}
		return true;
	}else
		return false;
}

void CXpeulMediaBackHelper::SetVideoSinkWnd(void)
{
	if(hwndVideoSinkWnd) goto label_DETTACH;
//find plug-in host
	HWND hWmpAppHost=FindWindowEx(g_data.GetWmp(), NULL, L"WMPAppHost", L"WmpAppHost");
		NULL_RET(hWmpAppHost);
	HWND hWmpSkinHost=FindWindowEx(hWmpAppHost,  NULL, L"WMP Skin Host", L"");
		NULL_RET(hWmpSkinHost);
	HWND hWmpLibCntr=FindWindowEx(hWmpSkinHost, NULL, NULL, L"LibraryContainer");
		NULL_RET(hWmpLibCntr);
label_RE_SEARCH:
	hwndWmpCtrlCntr=FindWindowEx(hWmpLibCntr, NULL, L"CWmpControlCntr", L"");
		NULL_RET(hwndWmpCtrlCntr);
	hwndWmpPluginHost=FindWindowEx(hwndWmpCtrlCntr, NULL, L"WMP Plugin UI Host", L"");
	if(!hwndWmpPluginHost) goto label_RE_SEARCH;

//create sink window
	WNDCLASSEX wcsex;
	memset(&wcsex, 0, sizeof(WNDCLASSEX));
	wcsex.cbSize=sizeof(WNDCLASSEX);
	wcsex.lpfnWndProc=XmVideoSinkProc;
	wcsex.hInstance=g_data.GetInstance();
	wcsex.hIcon=LoadIcon(g_data.GetInstance(), MAKEINTRESOURCE(IDI_TRAYICON));
	wcsex.lpszClassName=L"XmWmpVideoSinkWnd";
	wcsex.style=CS_DBLCLKS;
	wcsex.hbrBackground=CreateSolidBrush(RGB(0,0,0));
	RegisterClassEx(&wcsex);

	HKEY hKey=NULL;
	::RegOpenKey(HKEY_CURRENT_USER, _T("Software\\XpeulEnterprise\\XpeulMedia\\XpeulVedioSink"), &hKey);
	int l=0, t=0, r=480, b=320;
	if(hKey){
		DWORD size=sizeof(DWORD);
		::RegGetValue(hKey, NULL, _T("left"), RRF_RT_REG_DWORD, NULL, &l, &size);
		::RegGetValue(hKey, NULL, _T("top"), RRF_RT_REG_DWORD, NULL, &t, &size);
		::RegGetValue(hKey, NULL, _T("right"), RRF_RT_REG_DWORD, NULL, &r, &size);
		::RegGetValue(hKey, NULL, _T("bottom"), RRF_RT_REG_DWORD, NULL, &b, &size);
		::RegCloseKey(hKey);
	}	
	hwndVideoSinkWnd=CreateWindowEx(WS_EX_LEFT|WS_EX_TOOLWINDOW|WS_EX_TOPMOST, 
		L"XmWmpVideoSinkWnd", L"XmWmpVideo Sink Window", WS_POPUPWINDOW|WS_THICKFRAME,
		l, t, r-l, b-t, NULL, NULL, g_data.GetInstance(), NULL);
	NULL_RET(hwndVideoSinkWnd);

//attach to sink window
	GetWindowRect(hwndWmpPluginHost, &rcWPH);
	SetParent(hwndWmpPluginHost, hwndVideoSinkWnd);
	RECT crc;
	GetClientRect(hwndVideoSinkWnd, &crc);
	SetWindowPos(hwndWmpPluginHost, NULL, 0, 0, crc.right-crc.left, crc.bottom-crc.top,
			SWP_NOZORDER|SWP_NOMOVE|SWP_SHOWWINDOW);
	ShowWindow(hwndVideoSinkWnd, SW_SHOW);
	WPH_WndProc=GetWindowLong(hwndWmpPluginHost, GWL_WNDPROC);
	SetWindowLong(hwndWmpPluginHost, GWL_WNDPROC, (LONG)XmWmpWPHProc);
	VideoSinkState=true; ShowWmp();
	return;
	
//dettach from sink window
label_DETTACH:
	if(hwndWmpPluginHost&&hwndWmpCtrlCntr){
		VideoSinkState=false;
		RECT wrc;
		::GetWindowRect(hwndVideoSinkWnd, &wrc);
		SetParent(hwndWmpPluginHost, hwndWmpCtrlCntr);
		ShowWindow(hwndVideoSinkWnd, SW_HIDE);
		SetWindowPos(hwndWmpPluginHost, NULL, rcWPH.top, rcWPH.top, rcWPH.right-rcWPH.left, 
			rcWPH.bottom-rcWPH.top, SWP_NOZORDER|SWP_NOMOVE|SWP_SHOWWINDOW);
		ShowWmp();
		SetWindowLong(hwndWmpPluginHost, GWL_WNDPROC, WPH_WndProc);
		hwndWmpCtrlCntr=NULL;
		hwndWmpPluginHost=NULL;
		CloseWindow(hwndVideoSinkWnd);
		DestroyWindow(hwndVideoSinkWnd);
		hwndVideoSinkWnd=NULL;
		HKEY hKey=NULL;
		::RegOpenKey(HKEY_CURRENT_USER, _T("Software\\XpeulEnterprise\\XpeulMedia\\XpeulVedioSink"), &hKey);
		if(hKey){
			::RegSetKeyValue(hKey, NULL, _T("left"), REG_DWORD, &wrc.left, sizeof(DWORD));
			::RegSetKeyValue(hKey, NULL, _T("top"), REG_DWORD, &wrc.top, sizeof(DWORD));
			::RegSetKeyValue(hKey, NULL, _T("right"), REG_DWORD, &wrc.right, sizeof(DWORD));
			::RegSetKeyValue(hKey, NULL, _T("bottom"), REG_DWORD, &wrc.bottom, sizeof(DWORD));
			::RegCloseKey(hKey);
		}	
	}
	return;
}

//don't support folder selection currently!
HRESULT CXpeulMediaBackHelper::NewPlayList(IWMPCore *pCore, const wchar_t *lpstrFile)
{
	if((pCore==NULL)||(lpstrFile==NULL)) return E_INVALIDARG;

	HRESULT rst=S_OK;
//test file path and create the first item
	IWMPMediaCollection* pMC=NULL;
	rst=pCore->get_mediaCollection(&pMC);
		FAILED_RET(rst);
	IWMPMedia* pMedia=NULL;
	BSTR media_url=SysAllocString(lpstrFile);
	rst=pMC->add(media_url, &pMedia);
	if(SUCCEEDED(rst))
		rst=pCore->put_currentMedia(pMedia);
	else{
		const wchar_t* seekptr=lpstrFile;
		wchar_t file_name[MAX_PATH]; memset(file_name, 0, MAX_PATH*sizeof(wchar_t));
		wchar_t* fnseekptr=file_name;
		wchar_t* editptr=file_name;
//create the first file path
		while(*seekptr!=L'\0') 	*fnseekptr++=*seekptr++;
		*fnseekptr++=L'\\'; editptr=fnseekptr;
		if(*++seekptr==L'\0') return E_INVALIDARG;
		while(*seekptr!=L'\0') *editptr++=*seekptr++;
		media_url=SysAllocString(file_name);
		rst=pMC->add(media_url, &pMedia);
			FAILED_RET(rst);
		rst=pCore->put_currentMedia(pMedia);
//if has any, append to current playlist
		if(*++seekptr==L'\0') return rst;
		IWMPPlaylist* pPL=NULL;
		rst=pCore->get_currentPlaylist(&pPL);
			FAILED_RET(rst);

label_ADDNEXTITEM:
		editptr=fnseekptr;
		while(*seekptr!=L'\0') *editptr++=*seekptr++;
		media_url=SysAllocString(file_name);
		rst=pMC->add(media_url, &pMedia);
			FAILED_RET(rst);
		rst=pPL->appendItem(pMedia);
		if(*++seekptr!=L'\0') goto label_ADDNEXTITEM;
	}
	return rst;
}

//======================================
CXpeulMediaEffectHelper::CXpeulMediaEffectHelper(void)
{
	InitGdiplusObject();
}

CXpeulMediaEffectHelper::~CXpeulMediaEffectHelper(void)
{
	DeleteGdiplusObject();
}

#define SAFE_DEL_OBJ(p) if(p){ delete p; p=NULL;}
void CXpeulMediaEffectHelper::InitGdiplusObject(void)
{
	ATL::AtlTrace("CXpeulMediaEffectHelper::InitGdiplusObject().\n");
	token=NULL;
	//GdiInput.GdiplusVersion=1;
	//GdiInput.DebugEventCallback=NULL;
	//GdiInput.SuppressExternalCodecs=FALSE;
	//GdiInput.SuppressBackgroundThread=FALSE;
	Gdiplus::GdiplusStartup(&token, &GdiInput, &GdiOutput);

	xme_helper.n_waveform_points=0;

	HKEY hKey=NULL;
	RegOpenKeyW(HKEY_CURRENT_USER, L"Software\\XpeulEnterprise\\XpeulMedia\\XpeulLyricsShow", &hKey);
	NULL_RET(hKey);

	char buffer[32];
	DWORD size=8;

	LSTATUS s=RegGetValueW(hKey, NULL, L"WaveformAlphaColor", RRF_RT_REG_BINARY, NULL, buffer, &size);
	if(s==ERROR_SUCCESS){
		gdips_pen_wave_l=new Gdiplus::Pen(Gdiplus::Color(buffer[0], buffer[1], buffer[2], buffer[3]), 2.0);
		gdips_pen_wave_r=new Gdiplus::Pen(Gdiplus::Color(buffer[4], buffer[5], buffer[6], buffer[7]), 2.0);
	}

	size=4;
	s=RegGetValueW(hKey, NULL, L"WaveformPoints", RRF_RT_REG_DWORD, NULL, &n_waveform_points, &size);
	if(n_waveform_points>1024) n_waveform_points=1024;

	size=8;
	s=RegGetValueW(hKey, NULL, L"LyricsAlphaColorH", RRF_RT_REG_BINARY, NULL, buffer, &size);
	if(s==ERROR_SUCCESS){
		gdips_sbrush_lrc_pre_h=new Gdiplus::SolidBrush(Gdiplus::Color(buffer[0], buffer[1], buffer[2], buffer[3]));
		gdips_sbrush_lrc_finish_h=new Gdiplus::SolidBrush(Gdiplus::Color(buffer[4], buffer[5], buffer[6], buffer[7]));
	}

	size=12;
	s=RegGetValueW(hKey, NULL, L"LyricsAlphaColorV", RRF_RT_REG_BINARY, NULL, buffer, &size);
	if(s==ERROR_SUCCESS){
		gdips_sbrush_lrc_v=new Gdiplus::SolidBrush(Gdiplus::Color(buffer[0], buffer[1], buffer[2], buffer[3]));
		gdips_sbrush_lrc_pre_v=new Gdiplus::SolidBrush(Gdiplus::Color(buffer[4], buffer[5], buffer[6], buffer[7]));
		gdips_sbrush_lrc_finish_v=new Gdiplus::SolidBrush(Gdiplus::Color(buffer[8], buffer[9], buffer[10], buffer[11]));
	}

	size=16;
	s=RegGetValueW(hKey, NULL, L"BackGroundColor", RRF_RT_REG_BINARY, NULL, buffer, &size);
	if(s==ERROR_SUCCESS)
		gdips_color_bg=new Gdiplus::Color(buffer[0], buffer[1], buffer[2], buffer[3]);
	
	size=32;
	s=RegGetValueW(hKey, NULL, L"LyricsFont", RRF_RT_REG_SZ, NULL, buffer, &size);
	if(s==ERROR_SUCCESS){
		gdips_fontfamily_lrc=new Gdiplus::FontFamily((wchar_t*)buffer);
		gdips_font_lrc=new Gdiplus::Font((wchar_t*)buffer, 45, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		gdips_font_lrc_v=new Gdiplus::Font((wchar_t*)buffer, 30, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	}

	gdips_sbrush_info=new Gdiplus::SolidBrush(Gdiplus::Color(255,255,255,0));
	gdips_font_info=new Gdiplus::Font (L"Î¢ÈíÑÅºÚ", 20, Gdiplus::FontStyleRegular, Gdiplus::UnitWorld);
	gdips_strfmt_lrc=new Gdiplus::StringFormat;

	size=12;
	s=RegGetValueW(hKey, NULL, L"FrequencyAlphaColor", RRF_RT_REG_BINARY, NULL, buffer, &size);
	if(s==ERROR_SUCCESS){
		gdips_sbrush_freq_l=new Gdiplus::SolidBrush(Gdiplus::Color(buffer[0], buffer[1], buffer[2], buffer[3]));
		gdips_sbrush_freq_r=new Gdiplus::SolidBrush(Gdiplus::Color(buffer[4], buffer[5], buffer[6], buffer[7]));
		gdips_sbrush_freq_hilite=new Gdiplus::SolidBrush(Gdiplus::Color(buffer[8], buffer[9], buffer[10], buffer[11]));
	}
	size=4;
	s=RegGetValueW(hKey, NULL, L"FrequencyDoExchange", RRF_RT_REG_DWORD, NULL, &bFreqDoExchange, &size);

	RegCloseKey(hKey);
}

void CXpeulMediaEffectHelper::DeleteGdiplusObject(void)
{
	SAFE_DEL_OBJ(gdips_color_bg)
	SAFE_DEL_OBJ(gdips_pen_wave_l)
	SAFE_DEL_OBJ(gdips_pen_wave_r)
	SAFE_DEL_OBJ(gdips_sbrush_info)
	SAFE_DEL_OBJ(gdips_sbrush_lrc_pre_h)
	SAFE_DEL_OBJ(gdips_sbrush_lrc_finish_h)
	SAFE_DEL_OBJ(gdips_sbrush_lrc_v)
	SAFE_DEL_OBJ(gdips_sbrush_lrc_pre_v)
	SAFE_DEL_OBJ(gdips_sbrush_lrc_finish_v)
	SAFE_DEL_OBJ(gdips_font_lrc)
	SAFE_DEL_OBJ(gdips_font_lrc_v)
	SAFE_DEL_OBJ(gdips_font_info)
	SAFE_DEL_OBJ(gdips_fontfamily_lrc)
	SAFE_DEL_OBJ(gdips_strfmt_lrc)
	SAFE_DEL_OBJ(gdips_sbrush_freq_l)
	SAFE_DEL_OBJ(gdips_sbrush_freq_r)
	SAFE_DEL_OBJ(gdips_sbrush_freq_hilite)

	ATL::AtlTrace("Enter CXpeulMediaEffectHelper::DeleteGdiplusObject().\n");

//	Gdiplus::GdiplusShutdown(token);

	ATL::AtlTrace("Exit CXpeulMediaEffectHelper::DeleteGdiplusObject().\n");
}
#undef SAFE_DEL_OBJ
