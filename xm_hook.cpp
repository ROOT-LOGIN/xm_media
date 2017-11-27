#include "stdafx.h"
#include "resource.h"

#include "xm_back.h"
#include "_xm_helpers.h"
#include "xm_log.h"
#include "shellapi.h" 

const wchar_t* const str_hotkey_playpause = L"hotkey_playpause";
const wchar_t* const str_hotkey_stop = L"hotkey_stop";
const wchar_t* const str_hotkey_next = L"hotkey_next";
const wchar_t* const str_hotkey_prev = L"hotkey_prev";
const wchar_t* const str_hotkey_volup = L"hotkey_volup";
const wchar_t* const str_hotkey_voldn = L"hotkey_voldn";
const wchar_t* const str_hotkey_trayicon = L"hotkey_trayicon";
const wchar_t* const str_hotkey_videohook = L"hotkey_videohook";
const wchar_t* const str_hotkey_replaybegin = L"hotkey_replaybegin";
const wchar_t* const str_hotkey_replayend = L"hotkey_replayend";
extern const wchar_t* const str_hotkey_replaycancle= L"hotkey_replaycancle";


extern CXpeulMediaBackHelper xmb_helper;

HWND XmCreateNotifyIconSink(void);

HRESULT _XmPlayPause(IWMPCore* pCore)
{
	if(pCore==NULL) return E_INVALIDARG;
	HRESULT rst=S_OK;

	IWMPControls* pCtrl=NULL;
	WMPPlayState wmpst_play=(WMPPlayState)-1;
	WMPOpenState wmpst_open=(WMPOpenState)-1;

	rst=pCore->get_controls(&pCtrl);
		FAILED_RET(rst);
	rst=pCore->get_openState(&wmpst_open);
		FAILED_RET(rst);

	if((wmpst_open==WMPOpenState::wmposMediaOpen)||
		(wmpst_open==WMPOpenState::wmposPlaylistOpenNoMedia))
	{
		rst=pCore->get_playState(&wmpst_play);
			FAILED_RET(rst);
		if(wmpst_play==WMPPlayState::wmppsPlaying)
			rst=pCtrl->pause();
		if((wmpst_play==WMPPlayState::wmppsPaused)||
			(wmpst_play==WMPPlayState::wmppsStopped)||
			(wmpst_play==WMPPlayState::wmppsUndefined))
			rst=pCtrl->play();
	}
	return rst;
}

HRESULT _XmStop(IWMPCore* pCore)
{
	if(pCore==NULL) return E_INVALIDARG;
	HRESULT rst=S_OK;

	IWMPControls* pCtrl=NULL;
	WMPPlayState wmpst_play=(WMPPlayState)-1;

	rst=pCore->get_controls(&pCtrl);
		FAILED_RET(rst);

	rst=pCore->get_playState(&wmpst_play);
		FAILED_RET(rst);

	if(wmpst_play!=WMPPlayState::wmppsStopped)
			rst=pCtrl->stop();
		return rst;
}

HRESULT _XmPrevNext(IWMPCore* pCore, bool toNext=true)
{
	if(pCore==NULL) return E_INVALIDARG;
	HRESULT rst=S_OK;

	WMPPlayState wmpst_play=(WMPPlayState)-1;

	rst=pCore->get_playState(&wmpst_play);
		FAILED_RET(rst);

	if((wmpst_play==WMPPlayState::wmppsPlaying)||
		(wmpst_play==WMPPlayState::wmppsPaused))
	{
		IWMPControls* pCtrl=NULL;
		rst=pCore->get_controls(&pCtrl);
			FAILED_RET(rst);
		if(toNext) rst=pCtrl->next();
		else rst=pCtrl->previous();
	}
	return rst;
}

HRESULT _XmMute(IWMPCore* pCore)
{
	if(pCore==NULL) return E_INVALIDARG;
	HRESULT rst=S_OK;

	IWMPControls* pCtrl=NULL;
	rst=pCore->get_controls(&pCtrl);
		FAILED_RET(rst);
	IWMPSettings* pSet=NULL;
	rst=pCore->get_settings(&pSet);
		FAILED_RET(rst);
	VARIANT_BOOL mute;
	pSet->get_mute(&mute);
	pSet->put_mute(!mute);
	return rst;
}

XM_HOOKPROC HookProc_GetMsg(int nCode, WPARAM wparam, LPARAM lparam)
{
	static DWORD last_message_time;
	MSG* pMsg=reinterpret_cast<MSG*>(lparam);

	if(pMsg->message!=WM_HOTKEY)
		return CallNextHookEx(NULL, nCode, wparam, lparam);

	IWMPCore* pCore=xmb_helper.GetCore();
	if(!pCore) 	return CallNextHookEx(NULL, nCode, wparam, lparam);

	if(pMsg->lParam == xmb_helper.GetHotkey_voldn())
	{
		IWMPControls* pCtrl=NULL;
		HRESULT rst=pCore->get_controls(&pCtrl);
			FAILED_RET(rst);
		IWMPSettings* pSet=NULL;
		rst=pCore->get_settings(&pSet);
			FAILED_RET(rst);
		long vol=0;
		pSet->get_volume(&vol);
		vol-=1;
		pSet->put_volume(vol);
		return 0;
	}
	if(pMsg->lParam == xmb_helper.GetHotkey_volup())
	{
		IWMPControls* pCtrl=NULL;
		HRESULT rst=pCore->get_controls(&pCtrl);
			FAILED_RET(rst);
		IWMPSettings* pSet=NULL;
		rst=pCore->get_settings(&pSet);
			FAILED_RET(rst);
		long vol=0;
		pSet->get_volume(&vol);
		vol+=1;
		pSet->put_volume(vol);
		return 0;
	}

	//To limit response time
	if(pMsg->time-last_message_time<500)
		return 0;
	last_message_time=pMsg->time;

	if(pMsg->lParam == xmb_helper.GetHotkey_playpause())
		return _XmPlayPause(pCore);
	if(pMsg->lParam == xmb_helper.GetHotkey_stop())
		return _XmStop(pCore);
	if(pMsg->lParam == xmb_helper.GetHotkey_next())
		return _XmPrevNext(pCore);
	if(pMsg->lParam == xmb_helper.GetHotkey_prev())
		return _XmPrevNext(pCore, false);
	if(pMsg->lParam == xmb_helper.GetHotkey_trayicon())
	{
		xmb_helper.SetTrayIconSinkWnd();
		xmb_helper.ShowWmp();
		return 0;
	}
	if(::toupper(pMsg->lParam) == xmb_helper.GetHotkey_videohook())
	{
		xmb_helper.SetVideoSinkWnd();
		return 0;
	}
}

LRESULT CALLBACK XmTrayIconProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch(uMsg)
	{
	case XM_WM_NOTIFYICON:{
		switch(LOWORD(lp)){
			case WM_LBUTTONDBLCLK:{
				xmb_helper.SetTrayIconSinkWnd();
				xmb_helper.ShowWmp();
								}return 0;
			case WM_CONTEXTMENU:{
				HMENU hMenuContext=LoadMenu(g_data.GetInstance(), MAKEINTRESOURCE(IDM_CONTEXTMENU));
				HMENU hMenuTrayIcon=GetSubMenu(hMenuContext, 2);
				POINT pt={ LOWORD(wp), HIWORD(wp) };
				SetForegroundWindow(hWnd);
				TrackPopupMenu(hMenuTrayIcon, TPM_CENTERALIGN|TPM_LEFTBUTTON|TPM_NOANIMATION,
					pt.x, pt.y, NULL, hWnd, NULL);
				DestroyMenu(hMenuContext);
								}return 0;
		}
	}
	case WM_COMMAND:{
		switch(LOWORD(wp)){
		case ID_TrayIcon_OPEN:
		{
			OPENFILENAME ofn;
			memset(&ofn, 0, sizeof(OPENFILENAME));
			ofn.lStructSize=sizeof(OPENFILENAME);
			ofn.hwndOwner=hWnd;
			ofn.hInstance=g_data.GetInstance();
			ofn.lpstrFilter=TEXT("All Files\0*.*\0\0");
			ofn.nFilterIndex=1;
			ofn.Flags=OFN_ALLOWMULTISELECT|OFN_DONTADDTORECENT|OFN_READONLY|OFN_LONGNAMES|OFN_EXPLORER;
			wchar_t file[4096]; memset(file, 0, 4096*sizeof(wchar_t));
			wchar_t title[]=L"打开文件 - Windows Media Player";
			ofn.lpstrFile=file;
			ofn.nMaxFile=4096;
			ofn.lpstrTitle=title;
			if(GetOpenFileName(&ofn)) xmb_helper.NewPlayList(xmb_helper.GetCore(), file);
		}break;
		case ID_TrayIcon_RESTORE:
			xmb_helper.SetTrayIconSinkWnd();
			xmb_helper.ShowWmp();
			break;
		case ID_TrayIcon_CLOSE:
			if(g_data.GetWmp()) PostMessage(g_data.GetWmp(), WM_CLOSE, NULL, NULL);
			break;
		case ID_TrayIcon_PLAYPAUSE:
			_XmPlayPause(xmb_helper.GetCore());
			break;
		case ID_TrayIcon_STOP:
			_XmStop(xmb_helper.GetCore());
			break;
		case ID_TrayIcon_PREV:
			_XmPrevNext(xmb_helper.GetCore(), false);
			break;
		case ID_TrayIcon_NEXT:
			_XmPrevNext(xmb_helper.GetCore());
			break;
		case ID_TrayIcon_MUTE:
			_XmMute(xmb_helper.GetCore());
			break;
		default:
			break;
		}
	return 0;
	}
	default:
		return DefWindowProc(hWnd, uMsg, wp, lp);		
	}
	return 0;
}

#pragma warning(default: 4482)

//===========================================

CXpeulMediaBack::HotkeyHook::HotkeyHook(CXpeulMediaBack* pback)
	: m_hotkey_PlayPause(str_hotkey_playpause), m_hotkey_Stop(str_hotkey_stop), m_hotkey_Next(str_hotkey_next),
		m_hotkey_Prev(str_hotkey_prev), m_hotkey_VolUp(str_hotkey_volup), m_hotkey_VolDn(str_hotkey_voldn),
		m_hotkey_TrayIcon(str_hotkey_trayicon), m_hotkey_VideoHook(str_hotkey_videohook)
{
	last_message_time = 0;
	m_hookhandle = NULL;
}

CXpeulMediaBack::HotkeyHook::~HotkeyHook(void)
{
}

void CXpeulMediaBack::HotkeyHook::LastCleanUp(void)
{
	m_trayicon.SetNotifyIcon(true);
	m_trayicon.LastCleanUp();
//	m_videoquay.AnchorVideoWnd();
	m_videoquay.LastCleanUp();
}

LRESULT CXpeulMediaBack::HotkeyHook::entry_function(int code, WPARAM wp, LPARAM lp)
{
	MSG* pMsg=reinterpret_cast<MSG*>(lp);

	if((xmback_data.mp_Core==NULL) || (pMsg->message!=WM_HOTKEY))
		return CallNextHookEx(NULL, code, wp, lp);
	
	if(pMsg->lParam == m_hotkey_VolDn.GetLong())
	{
		IWMPControls* pCtrl=NULL;
		HRESULT rst=xmback_data.mp_Core->get_controls(&pCtrl);
			FAILED_RET(rst);
		IWMPSettings* pSet=NULL;
		rst=xmback_data.mp_Core->get_settings(&pSet);
			FAILED_RET(rst);
		long vol=0;
		pSet->get_volume(&vol);
		vol-=1;
		pSet->put_volume(vol);
		return 0;
	}
	if(pMsg->lParam == m_hotkey_VolUp.GetLong())
	{
		IWMPControls* pCtrl=NULL;
		HRESULT rst=xmback_data.mp_Core->get_controls(&pCtrl);
			FAILED_RET(rst);
		IWMPSettings* pSet=NULL;
		rst=xmback_data.mp_Core->get_settings(&pSet);
			FAILED_RET(rst);
		long vol=0;
		pSet->get_volume(&vol);
		vol+=1;
		pSet->put_volume(vol);
		return 0;
	}

	//To limit reponse time
	if(pMsg->time-last_message_time<500)
		return 0;
	last_message_time=pMsg->time;

	if(pMsg->lParam == m_hotkey_PlayPause.GetLong())
		return _XmPlayPause(xmback_data.mp_Core);
	if(pMsg->lParam == m_hotkey_Stop.GetLong())
		return _XmStop(xmback_data.mp_Core);
	if(pMsg->lParam == m_hotkey_Next.GetLong())
		return _XmPrevNext(xmback_data.mp_Core);
	if(pMsg->lParam == m_hotkey_Prev.GetLong())
		return _XmPrevNext(xmback_data.mp_Core, false);
	if(pMsg->lParam == m_hotkey_TrayIcon.GetLong())
	{
		m_trayicon.SetNotifyIcon();
		ManageWmpState();
		return 0;
	}
	if(::toupper(pMsg->lParam) == m_hotkey_VideoHook.GetLong())
	{
		m_videoquay.AnchorVideoWnd();
		ManageWmpState();
		return 0;
	}
}

//===========================================

NotifyIconResponser::NotifyIconResponser(void)
	: m_notifyicon_visible(false), m_tip(this)
{
}

NotifyIconResponser::~NotifyIconResponser(void)
{
}

void NotifyIconResponser::LastCleanUp(void)
{
	DestroyWindow();
	m_tip.DestroyWindow();
}

void NotifyIconResponser::SetNotifyIcon(bool do_force_delete)
{
	if(do_force_delete)
	{
		::Shell_NotifyIcon(NIM_DELETE, &m_nid);
		m_notifyicon_visible = false;
	}
	else
	{
		if(m_notifyicon_visible)
		{
			::Shell_NotifyIcon(NIM_DELETE, &m_nid);
		}
		else
		{
			if(m_hWnd==NULL)
			{
				Create(HWND_MESSAGE);
				InitNotifyIconData();
				m_tip.Create(NULL);
			}
			::Shell_NotifyIcon(NIM_ADD, &m_nid);
			::Shell_NotifyIcon(NIM_SETVERSION, &m_nid);
		}
		m_notifyicon_visible = !m_notifyicon_visible;
	}
}

bool NotifyIconResponser::InitNotifyIconData(void)
{
	if(!m_hWnd) return false;
	memset(&m_nid, 0, sizeof(NOTIFYICONDATA));
	m_nid.cbSize=sizeof(NOTIFYICONDATA);
	m_nid.hWnd=m_hWnd;
	m_nid.uID=0x1234;
	m_nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_nid.hIcon=LoadIcon(g_data.GetInstance(), MAKEINTRESOURCE(IDI_TRAYICON));
	m_nid.uCallbackMessage=XM_WM_NOTIFYICON;
	m_nid.szTip[0] = L'x'; m_nid.szTip[1] = L'\0';
	m_nid.uVersion=NOTIFYICON_VERSION_4;
	return true;
}

static const int xm_notifyicon_tipwnd_width = 288;
static const int xm_notifyicon_tipwnd_height = 128;

impl_msg_hdlr(NotifyIconResponser, OnNotifyIcon)
{
	switch(LOWORD(lp))
	{
		case WM_LBUTTONDBLCLK:
		{
			SetNotifyIcon();
//			xmb_helper.ShowWmp();
			break;
		}
		case WM_CONTEXTMENU:
		{
			HMENU hMenuContext=LoadMenu(g_data.GetInstance(), MAKEINTRESOURCE(IDM_CONTEXTMENU));
			HMENU hMenuTrayIcon=GetSubMenu(hMenuContext, 2);
			POINT pt={ LOWORD(wp), HIWORD(wp) };
			SetForegroundWindow(m_hWnd);
			TrackPopupMenu(hMenuTrayIcon, TPM_CENTERALIGN|TPM_LEFTBUTTON|TPM_NOANIMATION,
				pt.x, pt.y, NULL, m_hWnd, NULL);
			DestroyMenu(hMenuContext);
			break;
		}
		case NIN_POPUPOPEN:
		{
			APPBARDATA abd;
			QueryTaskbarPos(abd, true);
			POINT pt;
			switch(abd.uEdge)
			{
			case ABE_LEFT:
				pt.x = abd.rc.left;
				pt.y = abd.rc.bottom - xm_notifyicon_tipwnd_height;
				break;
			case ABE_RIGHT:
				pt.x = abd.rc.right - xm_notifyicon_tipwnd_width;
				pt.y = abd.rc.bottom - xm_notifyicon_tipwnd_height;
				break;
			case ABE_TOP:
				pt.y = abd.rc.top;
				pt.x = abd.rc.right - xm_notifyicon_tipwnd_width;
				break;
			case ABE_BOTTOM:
				pt.y = abd.rc.bottom - xm_notifyicon_tipwnd_height;
				pt.x = abd.rc.right - xm_notifyicon_tipwnd_width;
				break;
			}
			m_tip.SetWindowPos(HWND_TOPMOST, pt.x, pt.y, 
				xm_notifyicon_tipwnd_width, xm_notifyicon_tipwnd_height, SWP_SHOWWINDOW);
			break;
		}
		case NIN_POPUPCLOSE:
		{
			m_tip.ShowWindow(SW_HIDE);
			break;
		}
	}
	return 0;
}

impl_msg_hdlr(NotifyIconResponser, OnCommand)
{
	switch(LOWORD(wp))
	{
		case ID_TrayIcon_OPEN:
		{
			OPENFILENAME ofn;
			memset(&ofn, 0, sizeof(OPENFILENAME));
			ofn.lStructSize=sizeof(OPENFILENAME);
			ofn.hwndOwner=m_hWnd;
			ofn.hInstance=g_data.GetInstance();
			ofn.lpstrFilter=TEXT("All Files\0*.*\0\0");
			ofn.nFilterIndex=1;
			ofn.Flags=OFN_ALLOWMULTISELECT|OFN_DONTADDTORECENT|OFN_READONLY|OFN_LONGNAMES|OFN_EXPLORER;
			wchar_t file[4096]; memset(file, 0, 4096*sizeof(wchar_t));
			wchar_t title[]=L"打开文件 - Windows Media Player";
			ofn.lpstrFile=file;
			ofn.nMaxFile=4096;
			ofn.lpstrTitle=title;
			if(GetOpenFileName(&ofn)) xmb_helper.NewPlayList(xmb_helper.GetCore(), file);
		}break;
		case ID_TrayIcon_RESTORE:
			SetNotifyIcon();
//			xmb_helper.ShowWmp();
			break;
		case ID_TrayIcon_CLOSE:
			if(g_data.GetWmp()) 
				::PostMessage(g_data.GetWmp(), WM_CLOSE, NULL, NULL);
			break;
		case ID_TrayIcon_PLAYPAUSE:
			_XmPlayPause(xmb_helper.GetCore());
			break;
		case ID_TrayIcon_STOP:
			_XmStop(xmb_helper.GetCore());
			break;
		case ID_TrayIcon_PREV:
			_XmPrevNext(xmb_helper.GetCore(), false);
			break;
		case ID_TrayIcon_NEXT:
			_XmPrevNext(xmb_helper.GetCore());
			break;
		case ID_TrayIcon_MUTE:
			_XmMute(xmb_helper.GetCore());
			break;
		default:
			return ::DefWindowProc(m_hWnd, msg, wp, lp);
		}
	return 0;
}

impl_msg_hdlr(NotifyIconResponser::TipRender, OnActivate)
{
	MARGINS mars = {-1,-1,-1,-1};
	DwmExtendFrameIntoClientArea(m_hWnd, &mars);
	return DefWindowProc();
}

impl_msg_hdlr(NotifyIconResponser::TipRender, OnPaint)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	SetBkMode(hdc, TRANSPARENT);
	IWMPMedia* pMedia;
	xmback_data.mp_Core->get_currentMedia(&pMedia);
	if(pMedia)
	{
		HTHEME hT = OpenThemeData(m_hWnd, L"CompositedWindow::Window");
		BSTR title;
		pMedia->getItemInfo(L"Title", &title);
		BSTR author;
		pMedia->getItemInfo(L"Author", &author);
		BSTR duration;
		pMedia->get_durationString(&duration);
		IWMPControls* pControl;
		xmback_data.mp_Core->get_controls(&pControl);
		BSTR position;
		pControl->get_currentPositionString(&position);
		BSTR pos_dura = ::SysAllocStringLen(NULL, SysStringLen(position)+SysStringLen(duration)+3);
		wsprintf(pos_dura, L"%s / %s", position, duration);
		CRect rc;
		GetClientRect(&rc);
		DTTOPTS dttopts;
		dttopts.dwSize = sizeof(DTTOPTS);
		dttopts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE | DTT_TEXTCOLOR;
		dttopts.crText = RGB(255, 0, 0);
		dttopts.iGlowSize = 8;

		HDC hmemdc = ::CreateCompatibleDC(hdc);
		BITMAPINFO bmpinfo;
		bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFO);
		bmpinfo.bmiHeader.biWidth = rc.Width();
		bmpinfo.bmiHeader.biHeight = 0-rc.Height();
		bmpinfo.bmiHeader.biPlanes = 1;
		bmpinfo.bmiHeader.biBitCount = 32;
		bmpinfo.bmiHeader.biCompression = BI_RGB;
		bmpinfo.bmiHeader.biSizeImage = 0;
		bmpinfo.bmiHeader.biXPelsPerMeter = bmpinfo.bmiHeader.biWidth;
		bmpinfo.bmiHeader.biYPelsPerMeter = bmpinfo.bmiHeader.biHeight;
		bmpinfo.bmiHeader.biClrUsed = 0;
		bmpinfo.bmiHeader.biClrImportant = 0;
		char* bits;
		HBITMAP hbmp = ::CreateDIBSection(hmemdc, &bmpinfo, DIB_RGB_COLORS, (void**)&bits, 0, 0);
		HGDIOBJ hbmpold = SelectObject(hmemdc, hbmp);

		rc.bottom = rc.Height()/3;
		DrawThemeTextEx(hT, hmemdc, WP_CAPTION, CS_ACTIVE, title, -1, 
			DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS, &rc, &dttopts);
		rc.OffsetRect(0, rc.Height());
		DrawThemeTextEx(hT, hmemdc, WP_CAPTION, CS_ACTIVE, author, -1, 
			DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS, &rc, &dttopts);
		rc.OffsetRect(0, rc.Height());
		DrawThemeTextEx(hT, hmemdc, WP_CAPTION, CS_ACTIVE, pos_dura, -1, 
			DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS, &rc, &dttopts);

		BitBlt(hdc, 0, 0, bmpinfo.bmiHeader.biWidth, 0-bmpinfo.bmiHeader.biHeight, hmemdc, 0, 0, SRCCOPY);

		SelectObject(hmemdc, hbmpold);
		DeleteObject(hbmp);
		DeleteDC(hmemdc);

		SysFreeString(title);
		SysFreeString(author);
		SysFreeString(position);
		SysFreeString(duration);
		SysFreeString(pos_dura);
		CloseThemeData(hT);

		this->StartWindowProc 
	}
	EndPaint(&ps);
	return 0;
}

//===========================================

#define SRCWND_STYLE (WS_POPUP|WS_SIZEBOX)
#define SRCWND_EXSTYLE (WS_EX_TOOLWINDOW)

VideoWndQuay::VideoWndQuay(void) 
	: m_videownd_anchorring(false), m_rc(32,32,512,352),
	m_sourcestub(this), m_wmpstub(this, &m_sourcestub)
{
	::AdjustWindowRectEx(m_rc, SRCWND_STYLE, FALSE, SRCWND_EXSTYLE);
}

VideoWndQuay::~VideoWndQuay(void)
{
}

void VideoWndQuay::LastCleanUp(void)
{
	this->m_sourcestub.LastCleanUp();
	this->m_sourcestub.DestroyWindow();
	this->m_sourcestub.Detach();

}

void VideoWndQuay::SaveOrLoadState(bool do_load)
{
	HKEY hKey=NULL;
	::RegOpenKey(HKEY_CURRENT_USER, _T("Software\\XpeulEnterprise\\XpeulMedia\\XpeulVedioSink"), &hKey);
	if(hKey)
	{
		if(do_load)
		{
			DWORD size=sizeof(DWORD);
			::RegGetValue(hKey, NULL, _T("left"), RRF_RT_REG_DWORD, NULL, &m_rc.left, &size);
			size=sizeof(DWORD);
			::RegGetValue(hKey, NULL, _T("top"), RRF_RT_REG_DWORD, NULL, &m_rc.top, &size);
			size=sizeof(DWORD);
			::RegGetValue(hKey, NULL, _T("right"), RRF_RT_REG_DWORD, NULL, &m_rc.right, &size);
			size=sizeof(DWORD);
			::RegGetValue(hKey, NULL, _T("bottom"), RRF_RT_REG_DWORD, NULL, &m_rc.bottom, &size);
		}
		else
		{
			::RegSetKeyValue(hKey, NULL, _T("left"), REG_DWORD, &m_rc.left, sizeof(DWORD));
			::RegSetKeyValue(hKey, NULL, _T("top"), REG_DWORD, &m_rc.top, sizeof(DWORD));
			::RegSetKeyValue(hKey, NULL, _T("right"), REG_DWORD, &m_rc.right, sizeof(DWORD));
			::RegSetKeyValue(hKey, NULL, _T("bottom"), REG_DWORD, &m_rc.bottom, sizeof(DWORD));
		}
		::RegCloseKey(hKey);
	}
}

bool VideoWndQuay::DetectContext(void)
{
	if(::IsWindow(m_hWndSource))
	{
		if(::IsWindow(m_hWndDestination))
		{
			return true;
		}
	}

	HWND hWmpAppHost=FindWindowEx(g_data.GetWmp(), NULL, L"WMPAppHost", L"WmpAppHost");
	if(hWmpAppHost)
	{
		HWND hWmpSkinHost=FindWindowEx(hWmpAppHost,  NULL, L"WMP Skin Host", L"");
		if(hWmpSkinHost)
		{
			HWND hWmpLibCntr=FindWindowEx(hWmpSkinHost, NULL, NULL, L"LibraryContainer");
			if(hWmpLibCntr)
			{
				HWND hWmpCtrlCntr = NULL;
				HWND hWmpPluginHost;
				do
				{
					hWmpCtrlCntr=FindWindowEx(hWmpLibCntr, hWmpCtrlCntr, L"CWmpControlCntr", L"");
					if(hWmpCtrlCntr)
					{
						hWmpPluginHost=FindWindowEx(hWmpCtrlCntr, NULL, L"WMP Plugin UI Host", L"");
						if(hWmpPluginHost)
						{
							this->m_hWndSource = hWmpCtrlCntr;
							this->m_hWndDestination = hWmpLibCntr;
							return true;
						}
					}
					else
					{
						break;
					}
				}while(true);
			}
		}
	}
	return false;
}

void VideoWndQuay::AnchorVideoWnd(void)
{
	if(DetectContext())
	{
		if(IsWindow(m_sourcestub.m_hWnd))
		{ // do restore
			//::ShowWindow(m_hWndSource, SW_HIDE);
			::GetWindowRect(m_hWndSource, m_rc);
			::SetWindowLong(m_hWndSource, GWL_STYLE, m_wstylesrc);
			::SetWindowLong(m_hWndSource, GWL_EXSTYLE, m_wexstylesrc);
			::SetParent(m_hWndSource, m_hWndDestination);
			::SetWindowPos(m_hWndSource, NULL, m_rcsrc.left, m_rcsrc.top, m_rcsrc.Width(), m_rcsrc.Height(), 
				/*SWP_FRAMECHANGED|*/SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOSENDCHANGING);
			m_sourcestub.EndInterceptionGame();
			m_wmpstub.EndInterceptionGame();
			::ShowWindow(g_data.GetWmp(), SW_SHOW);
			m_videownd_anchorring = false;
		}
		else
		{ // do anchore
			m_wstylesrc = ::GetWindowLong(m_hWndSource, GWL_STYLE);
			m_wexstylesrc = ::GetWindowLong(m_hWndSource, GWL_EXSTYLE);
			::GetClientRect(m_hWndSource, m_rcsrc);
			::ShowWindow(g_data.GetWmp(), SW_HIDE);
			::SetParent(m_hWndSource, NULL);
			::SetWindowLong(m_hWndSource, GWL_STYLE, SRCWND_STYLE);
			DWORD exstyle = SRCWND_EXSTYLE;
			if(m_sourcestub.IsTopmost())
				exstyle |= WS_EX_TOPMOST;
			::SetWindowLong(m_hWndSource, GWL_EXSTYLE, exstyle);
			m_sourcestub.BeginInterceptionGame(m_hWndSource);
			::SetWindowPos(m_hWndSource, HWND_TOPMOST, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), 
				/*SWP_NOSIZE|SWP_NOMOVE|*/SWP_SHOWWINDOW);
			m_videownd_anchorring = true;
			m_wmpstub.BeginInterceptionGame(g_data.GetWmp());
		}
	}
}
