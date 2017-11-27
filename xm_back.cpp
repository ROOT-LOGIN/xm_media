#include "stdafx.h"
#include "resource.h"

#include "_xm_helpers.h"
#include "xm_back.h"
#include "xm_log.h"

// {F3DE884F-C8BD-4665-8734-FE62648B7DDC}
extern "C" const CLSID CLSID_CXpeulMediaBack = 
{ 0xf3de884f, 0xc8bd, 0x4665, { 0x87, 0x34, 0xfe, 0x62, 0x64, 0x8b, 0x7d, 0xdc } };

OBJECT_ENTRY_AUTO(CLSID_CXpeulMediaBack, CXpeulMediaBack)

CXpeulMediaBackHelper xmb_helper;

XMBackData xmback_data;

CXpeulMediaBack::CXpeulMediaBack(void)
	: m_hotkeyhook(this)
{
	m_dwAdviseCookie=0;
	m_hotkeyhook.prepare_entry_function(this);
}

//IWMPPluginUI
STDMETHODIMP CXpeulMediaBack::SetCore(IWMPCore *pCore)
{
	HRESULT rst = xmback_data.SetCore(pCore);
	if(pCore)
	{
		IConnectionPointContainer* pConnPntCont=NULL;
		rst=pCore->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnPntCont);
		if(SUCCEEDED(rst))
		{
			rst=pConnPntCont->FindConnectionPoint(__uuidof(IWMPEvents), &mp_ConnPnt);
			IUnknown* pUnk;
			if(SUCCEEDED(rst))
			{
				this->QueryInterface(IID_IUnknown, (void**)&pUnk);
				rst=mp_ConnPnt->Advise(pUnk, &m_dwAdviseCookie);
			}
			pConnPntCont->Release();
		}

		SetHookAndRegHotkey();
	}
	else
	{
		if(m_dwAdviseCookie)
			rst=mp_ConnPnt->Unadvise(m_dwAdviseCookie);
		mp_ConnPnt.Release();
		this->Release();
		
		this->m_hotkeyhook.LastCleanUp();
		UnHookAndUnRegHotkey();
	}
	return rst;
}

STDMETHODIMP CXpeulMediaBack::DisplayPropertyPage(HWND hwndParent)
{
	wchar_t info[MAX_PATH]={'\0'};
	LoadString(g_data.GetInstance(), IDS_BACK_PROPERTYPAGE_INFO, info, MAX_PATH);
	wchar_t title[MAX_PATH]={'\0'};
	LoadString(g_data.GetInstance(), IDS_BACK_NAME, title, MAX_PATH);

	MessageBoxW(hwndParent, info, title, MB_ICONINFORMATION|MB_OK);
	return S_OK;
}

STDMETHODIMP CXpeulMediaBack::SetProperty(const WCHAR* name, const VARIANT* pVariant)
{
	return E_NOTIMPL;
}

STDMETHODIMP CXpeulMediaBack::GetProperty(const WCHAR* name, VARIANT* pVariant)
{
	if(wcscmp(name, PLUGIN_MISC_QUERYDESTROY)==0)
	{
		pVariant->vt=VT_BSTR;
		pVariant->bstrVal=::SysAllocString(L"");
		if(!pVariant->bstrVal)
			return E_OUTOFMEMORY;
		return S_OK;
	}
	return E_INVALIDARG;
}

STDMETHODIMP CXpeulMediaBack::Create(HWND hwndparent, HWND* phWnd)
{return E_NOTIMPL;}

STDMETHODIMP CXpeulMediaBack::Destroy(void)
{return E_NOTIMPL;}

STDMETHODIMP CXpeulMediaBack::TranslateAccelerator(LPMSG lpmsg)
{ return S_FALSE;}

CXpeulMediaBack::~CXpeulMediaBack(void)
{
}

#define CASE_TO_STRING(_int_case_, _string_) \
	case _int_case_: \
		_string_=#_int_case_; \
	break

static const char* const str_default="default";
#define DEFAULT_TO_STRING(_string_) default: _string_=str_default

void STDMETHODCALLTYPE CXpeulMediaBack::OpenStateChange( long NewState)
{
	if(!g_data.GetWriteLog()) return;

	static const char* const event="OpenStateChange"; 
	const char* discription=NULL;
	switch(NewState)
	{
		CASE_TO_STRING(wmposUndefined, discription);
		CASE_TO_STRING(wmposPlaylistChanging, discription);
		CASE_TO_STRING(wmposPlaylistLocating, discription);
		CASE_TO_STRING(wmposPlaylistConnecting, discription);
		CASE_TO_STRING(wmposPlaylistLoading, discription);
		CASE_TO_STRING(wmposPlaylistOpening, discription);
		CASE_TO_STRING(wmposPlaylistOpenNoMedia, discription);
		CASE_TO_STRING(wmposPlaylistChanged, discription);
		CASE_TO_STRING(wmposMediaChanging, discription);
		CASE_TO_STRING(wmposMediaLocating, discription);
		CASE_TO_STRING(wmposMediaConnecting, discription);
		CASE_TO_STRING(wmposMediaLoading, discription);
		CASE_TO_STRING(wmposMediaOpening, discription);
		CASE_TO_STRING(wmposMediaOpen, discription);
		CASE_TO_STRING(wmposBeginCodecAcquisition, discription);
		CASE_TO_STRING(wmposEndCodecAcquisition, discription);
		CASE_TO_STRING(wmposBeginLicenseAcquisition, discription);
		CASE_TO_STRING(wmposEndLicenseAcquisition, discription);
		CASE_TO_STRING(wmposBeginIndividualization, discription);
		CASE_TO_STRING(wmposEndIndividualization, discription);
		CASE_TO_STRING(wmposMediaWaiting, discription);
		CASE_TO_STRING(wmposOpeningUnknownURL, discription);
		DEFAULT_TO_STRING(discription);
	}
	::XmWriteLog(event, strlen(event), discription, strlen(discription));
}

void STDMETHODCALLTYPE CXpeulMediaBack::PlayStateChange( long NewState)
{
	if(!g_data.GetWriteLog()) return;

	static const char* const event="PlayStateChange"; 
	const char* discription=NULL;
	switch(NewState)
	{
		CASE_TO_STRING(wmppsUndefined, discription);
		CASE_TO_STRING(wmppsStopped, discription);
		CASE_TO_STRING(wmppsPaused, discription);
		CASE_TO_STRING(wmppsPlaying, discription);
		CASE_TO_STRING(wmppsScanForward, discription);
		CASE_TO_STRING(wmppsScanReverse, discription);
		CASE_TO_STRING(wmppsBuffering, discription);
		CASE_TO_STRING(wmppsWaiting, discription);
		CASE_TO_STRING(wmppsMediaEnded, discription);
		CASE_TO_STRING(wmppsTransitioning, discription);
		CASE_TO_STRING(wmppsReady, discription);
		CASE_TO_STRING(wmppsReconnecting, discription);
		CASE_TO_STRING(wmppsLast, discription);
		DEFAULT_TO_STRING(discription);
	}
	::XmWriteLog(event, strlen(event), discription, strlen(discription));
}

void STDMETHODCALLTYPE CXpeulMediaBack::AudioLanguageChange( long LangID)
{
	if(!g_data.GetWriteLog()) return;
	static const char* const event="AudioLanguageChange";
	::XmWriteLog(event, sizeof("AudioLanguageChange")-1, event, sizeof("AudioLanguageChange")-1);
}

void STDMETHODCALLTYPE CXpeulMediaBack::StatusChange(void){}
void STDMETHODCALLTYPE CXpeulMediaBack::ScriptCommand( BSTR scType,BSTR Param){}
void STDMETHODCALLTYPE CXpeulMediaBack::NewStream( void){}
void STDMETHODCALLTYPE CXpeulMediaBack::Disconnect( long Result){}
void STDMETHODCALLTYPE CXpeulMediaBack::Buffering( VARIANT_BOOL Start){}
void STDMETHODCALLTYPE CXpeulMediaBack::Error( void){}
void STDMETHODCALLTYPE CXpeulMediaBack::Warning( long WarningType,long Param,BSTR Description){}
void STDMETHODCALLTYPE CXpeulMediaBack::EndOfStream( long Result){}
void STDMETHODCALLTYPE CXpeulMediaBack::PositionChange( double oldPosition,double newPosition)
{
	if(!g_data.GetWriteLog()) return;
	static const char* const event="PositionChange";
	char dscp[256]={'\0'};
	int i = sprintf(dscp, "old_pos: %f, new_pos: %f", oldPosition, newPosition);
	::XmWriteLog(event, sizeof("PositionChange")-1, dscp, i);
}
void STDMETHODCALLTYPE CXpeulMediaBack::MarkerHit( long MarkerNum){}
void STDMETHODCALLTYPE CXpeulMediaBack::DurationUnitChange( long NewDurationUnit){}
void STDMETHODCALLTYPE CXpeulMediaBack::CdromMediaChange( long CdromNum){}
void STDMETHODCALLTYPE CXpeulMediaBack::PlaylistChange( IDispatch *Playlist, WMPPlaylistChangeEventType change){}
void STDMETHODCALLTYPE CXpeulMediaBack::CurrentPlaylistChange( WMPPlaylistChangeEventType change){}
void STDMETHODCALLTYPE CXpeulMediaBack::CurrentPlaylistItemAvailable( BSTR bstrItemName){}
void STDMETHODCALLTYPE CXpeulMediaBack::MediaChange( IDispatch *Item){}
void STDMETHODCALLTYPE CXpeulMediaBack::CurrentMediaItemAvailable( BSTR bstrItemName){}
void STDMETHODCALLTYPE CXpeulMediaBack::CurrentItemChange( IDispatch *pdispMedia){}
void STDMETHODCALLTYPE CXpeulMediaBack::MediaCollectionChange(void){}
void STDMETHODCALLTYPE CXpeulMediaBack::MediaCollectionAttributeStringAdded( BSTR bstrAttribName,BSTR bstrAttribVal){}
void STDMETHODCALLTYPE CXpeulMediaBack::MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,BSTR bstrAttribVal){}
void STDMETHODCALLTYPE CXpeulMediaBack::MediaCollectionAttributeStringChanged( BSTR bstrAttribName,BSTR bstrOldAttribVal,BSTR bstrNewAttribVal){}
void STDMETHODCALLTYPE CXpeulMediaBack::PlaylistCollectionChange( void){}
void STDMETHODCALLTYPE CXpeulMediaBack::PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName){}
void STDMETHODCALLTYPE CXpeulMediaBack::PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName){}
void STDMETHODCALLTYPE CXpeulMediaBack::PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName,VARIANT_BOOL varfIsDeleted){}
void STDMETHODCALLTYPE CXpeulMediaBack::ModeChange( BSTR ModeName,VARIANT_BOOL NewValue){}
void STDMETHODCALLTYPE CXpeulMediaBack::MediaError( IDispatch *pMediaObject){}
void STDMETHODCALLTYPE CXpeulMediaBack::OpenPlaylistSwitch( IDispatch *pItem){}
void STDMETHODCALLTYPE CXpeulMediaBack::DomainChange( BSTR strDomain){}
void STDMETHODCALLTYPE CXpeulMediaBack::SwitchedToPlayerApplication( void){}
void STDMETHODCALLTYPE CXpeulMediaBack::SwitchedToControl( void){}
void STDMETHODCALLTYPE CXpeulMediaBack::PlayerDockedStateChange( void){}
void STDMETHODCALLTYPE CXpeulMediaBack::PlayerReconnect( void){}
void STDMETHODCALLTYPE CXpeulMediaBack::Click( short nButton,short nShiftState,long fX,long fY){}
void STDMETHODCALLTYPE CXpeulMediaBack::DoubleClick( short nButton,short nShiftState,long fX,long fY){}
void STDMETHODCALLTYPE CXpeulMediaBack::KeyDown( short nKeyCode,short nShiftState){}
void STDMETHODCALLTYPE CXpeulMediaBack::KeyPress( short nKeyAscii){}
void STDMETHODCALLTYPE CXpeulMediaBack::KeyUp( short nKeyCode,short nShiftState){}
void STDMETHODCALLTYPE CXpeulMediaBack::MouseDown( short nButton,short nShiftState,long fX,long fY){}
void STDMETHODCALLTYPE CXpeulMediaBack::MouseMove( short nButton,short nShiftState,long fX,long fY){}
void STDMETHODCALLTYPE CXpeulMediaBack::MouseUp( short nButton,short nShiftState,long fX,long fY){}

BOOL CXpeulMediaBack::SetHookAndRegHotkey(void)
{
	m_hotkeyhook.m_hotkey_PlayPause.SetKey(MOD_WIN, VK_SPACE);
	m_hotkeyhook.m_hotkey_PlayPause.Register();
	
	m_hotkeyhook.m_hotkey_Stop.SetKey(MOD_WC, VK_SPACE);
	m_hotkeyhook.m_hotkey_Stop.Register();

	m_hotkeyhook.m_hotkey_Prev.SetKey(MOD_WIN, VK_UP);
	m_hotkeyhook.m_hotkey_Prev.Register();

	m_hotkeyhook.m_hotkey_Next.SetKey(MOD_WIN, VK_DOWN);
	m_hotkeyhook.m_hotkey_Next.Register();

	m_hotkeyhook.m_hotkey_VolDn.SetKey(MOD_WIN, VK_LEFT);
	m_hotkeyhook.m_hotkey_VolDn.Register();

	m_hotkeyhook.m_hotkey_VolUp.SetKey(MOD_WIN, VK_RIGHT);
	m_hotkeyhook.m_hotkey_VolUp.Register();
	
	m_hotkeyhook.m_hotkey_TrayIcon.SetKey(MOD_WIN, VK_END);
	m_hotkeyhook.m_hotkey_TrayIcon.Register();

	m_hotkeyhook.m_hotkey_VideoHook.SetKey(MOD_WIN, 'V');
	m_hotkeyhook.m_hotkey_VideoHook.Register();

	m_hotkeyhook.m_hookhandle =SetWindowsHookEx(WH_GETMESSAGE, 
		m_hotkeyhook.make_entry(), g_data.GetInstance(), GetCurrentThreadId());
	return m_hotkeyhook.m_hookhandle?TRUE:FALSE;
}

BOOL CXpeulMediaBack::UnHookAndUnRegHotkey(void)
{
	BOOL b=UnhookWindowsHookEx(m_hotkeyhook.m_hookhandle);

	m_hotkeyhook.m_hotkey_PlayPause.Unregister();
	m_hotkeyhook.m_hotkey_Stop.Unregister();
	m_hotkeyhook.m_hotkey_Prev.Unregister();
	m_hotkeyhook.m_hotkey_Next.Unregister();
	m_hotkeyhook.m_hotkey_VolDn.Unregister();
	m_hotkeyhook.m_hotkey_VolUp.Unregister();
	m_hotkeyhook.m_hotkey_TrayIcon.Unregister();
	m_hotkeyhook.m_hotkey_VideoHook.Unregister();

	return b;
}

LRESULT CALLBACK XmWmpWPHProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	return CallWindowProc((WNDPROC)xmb_helper.GetWndproc(), hWnd, uMsg, wp, lp);
}

LPARAM lpt=0;
LRESULT CALLBACK XmVideoSinkProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	RECT wrc;
	GetWindowRect(hWnd, &wrc);
	switch(uMsg)
	{
	case WM_SIZING:{
		RECT crc;
		GetClientRect(hWnd, &crc);
		HWND hwndWmpPluginHost=FindWindowEx(hWnd, NULL, L"WMP Plugin UI Host", L"");
		DefWindowProc(hWnd, uMsg, wp, lp);
		SetWindowPos(hwndWmpPluginHost, NULL, 0, 0, crc.right-crc.left, crc.bottom-crc.top,
			SWP_NOZORDER|SWP_NOMOVE|SWP_SHOWWINDOW);
		return TRUE;
	}
	case WM_LBUTTONDOWN:
		MessageBox(NULL, L"BSASD", L"BSDASDA", MB_OK);
		return 0;
	case WM_PARENTNOTIFY:{
		if(LOWORD(wp)==WM_LBUTTONDOWN)
			PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lp);
		else if((LOWORD(wp)==WM_RBUTTONDOWN)&&(lpt!=lp)){
			lpt=lp;
			HMENU hMenuContext=LoadMenu(g_data.GetInstance(), MAKEINTRESOURCE(IDM_CONTEXTMENU));
			HMENU hMenuMainUI=GetSubMenu(hMenuContext, 0);
			POINT pt;
			pt.x=GET_X_LPARAM(lp);
			pt.y=GET_Y_LPARAM(lp);
			ClientToScreen(hWnd, &pt);
			TrackPopupMenu(hMenuMainUI, TPM_CENTERALIGN|TPM_LEFTBUTTON|TPM_NOANIMATION,
				pt.x, pt.y, NULL, hWnd, NULL);
			DestroyMenu(hMenuContext);
		}
		return 0;
	}
	case WM_COMMAND:{
		switch(LOWORD(wp))
		{
		case ID_MainUI_OPEN:{
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
			if(GetOpenFileName(&ofn))
				xmb_helper.NewPlayList(xmb_helper.GetCore(), file);
		}	break;
		case ID_MainUI_RESTORE:
			xmb_helper.SetVideoSinkWnd();
			break;
		case ID_MainUI_CLOSE:{
			if(g_data.GetWmp()) PostMessage(g_data.GetWmp(), WM_CLOSE, NULL, NULL);
			 }break;
		}
		return 0;
	}
	default:
		return ::DefWindowProc(hWnd, uMsg, wp, lp);
	}
}

/*
===========================================
*/

DWORD __stdcall MonitorThread(void* pv)
{
	struct THE_DATA
	{
		HWND hparant;
		HWND hsource;
	};
	THE_DATA* ptd = (THE_DATA*)pv;
	POINT pt;
	CRect rc;
	while(IsWindow(ptd->hsource))
	{
		GetCursorPos(&pt);
		GetWindowRect(ptd->hsource, rc);
		if(rc.PtInRect(pt))
		{
			if(!IsWindowVisible(ptd->hsource))
				ShowWindow(ptd->hsource, SW_SHOW);
		}
		else
		{
			if(IsWindowVisible(ptd->hsource))
				ShowWindow(ptd->hsource, SW_HIDE);
		}
		Sleep(300);
	}
	return 0;
}


DWORD VideoWndQuay::SOURCE_STUB_WINDOW::BeginInterceptionGame(HWND hwnd)
{
	DWORD err = __super::BeginInterceptionGame(hwnd);
	if(err == ERROR_SUCCESS)
	{
		TerminateThread(_m_hmonitorthread, 0);
		if(_m_consolepanel.IsWindow())
			_m_consolepanel.DestroyWindow();
		_m_consolepanel.Create(m_hWnd, _U_RECT(CRect(4,4,32,32)), L"", WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);//, WS_EX_TOOLWINDOW);
		err = GetLastError();
		if(err == ERROR_SUCCESS)
		{
			_m_consolepanel.SetWindowPos(HWND_TOP, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
			_m_monitordata.hparant = m_hWnd;
			_m_monitordata.hsource = _m_consolepanel.m_hWnd;
			_m_hmonitorthread = CreateThread(NULL, 0, MonitorThread, &_m_monitordata, 0, 0);
			err = GetLastError();
		}
	}
	return err;
}

DWORD VideoWndQuay::SOURCE_STUB_WINDOW::EndInterceptionGame(void)
{
	_m_consolepanel.DestroyWindow();
	TerminateThread(_m_hmonitorthread, 0);
	CloseHandle(_m_hmonitorthread);
	return __super::EndInterceptionGame();
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnInitMenu)
{
	if(GetInterceptionFlag() & IF_Reflect)
	{
		InsertMenu((HMENU)wp, 0, MF_SEPARATOR|MF_BYPOSITION, NULL, NULL);
		WCHAR buf[64];
		LoadString(g_data.GetInstance(), IDS_BACK_VIDEOMENU_FULLMODE, buf, 64);
		InsertMenu((HMENU)wp, 0, MF_STRING|MF_BYPOSITION, ID_MainUI_FULLMODE, buf);
		LoadString(g_data.GetInstance(), IDS_BACK_VIDEOMENU_TOPMOST, buf, 64);
		UINT flg = MF_STRING|MF_BYPOSITION;
		if(IsTopmost())
			flg |= MF_CHECKED;
		InsertMenu((HMENU)wp, 0, flg, ID_MainUI_TOPMOST, buf);
		if(_m_hmainuimenu)
		{
			AppendMenu((HMENU)wp, MF_SEPARATOR, NULL, NULL);
			LoadString(g_data.GetInstance(), IDS_BACK_VIDEOMENU_PLAYCTRL, buf, 64);
			AppendMenu((HMENU)wp, MF_POPUP, (UINT_PTR)GetSubMenu(_m_hmainuimenu, 3), buf);
			LoadString(g_data.GetInstance(), IDS_BACK_VIDEOMENU_PLAYLIST, buf, 64);
			AppendMenu((HMENU)wp, MF_POPUP, (UINT_PTR)GetSubMenu(_m_hmainuimenu, 4), buf);
			AppendMenu((HMENU)wp, MF_SEPARATOR, NULL, NULL);
			LoadString(g_data.GetInstance(), IDS_BACK_VIDEOMENU_OPTIONS, buf, 64);
			AppendMenu((HMENU)wp, MF_POPUP, (UINT_PTR)GetSubMenu(_m_hmainuimenu, 5), buf);
		}
	}
	else
		SetInterceptionFlag(IF_PassAndReflect);
	return 0;
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnInitMenuPopup)
{
	AtlTrace(L"InitMenuPopup, HMENU: %p, index: %d, sysmenu: %d\n", (HMENU)wp, LOWORD(lp), HIWORD(lp));
	return 0;
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnUninitMenuPopup)
{
	AtlTrace(L"UninitMenuPopup, HMENU: %p, index: %d, sysmenu: %d\n", (HMENU)wp, LOWORD(lp), HIWORD(lp));
	return 0;
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnParentNotify)
{
	if(LOWORD(wp)==WM_LBUTTONDOWN)
	{
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, lp);
		this->SetInterceptionFlag(IF_Intercepte);
	}
	return 0;
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnWndPosChanged)
{
	this->SetInterceptionFlag(IF_Intercepte);
	WINDOWPOS* pwp = (WINDOWPOS*)lp;
	CRect crc;
	GetClientRect(crc);
	HWND hWnd = ::ChildWindowFromPoint(m_hWnd, crc.TopLeft());
	if(hWnd)
	{
		::MoveWindow(hWnd, 0, 0, crc.Width(), crc.Height(), FALSE);
	}
	//ClientToScreen(crc);
	_m_consolepanel.MoveWindow(crc.left, crc.bottom-64, crc.Width(), 64, FALSE);
	return 0;
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnSetCursor)
{
	CRect crc;
	_m_consolepanel.GetWindowRect(crc);
//	ClientToScreen(crc);
	POINT pt;
	::GetCursorPos(&pt);
	if(crc.PtInRect(pt))
	{
		if(!_m_consolepanel.IsWindowVisible())
		{
			//_m_consolepanel.SetWindowPos(HWND_TOP, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
			//_m_consolepanel.ShowWindow(SW_SHOW);
		}
	}
	else
	{
		if(_m_consolepanel.IsWindowVisible())
		{
			_m_consolepanel.SetWindowPos(HWND_BOTTOM, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE|SWP_HIDEWINDOW);
			_m_consolepanel.ShowWindow(SW_HIDE);
		}
	}
	return 0;
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnEnterMenuLoop)
{
	_m_hmainuimenu = LoadMenu(g_data.GetInstance(), MAKEINTRESOURCEW(IDM_CONTEXTMENU));
	return 0;
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnExitMenuLoop)
{
	DestroyMenu(_m_hmainuimenu);
	_m_hmainuimenu = NULL;
	return 0;
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnNcCalcSize)
{
	this->SetInterceptionFlag(IF_Intercepte);
	return ::DefWindowProc(m_hWnd, WM_NCCALCSIZE, wp, lp);
}

impl_msg_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnMenuSelect)
{
	this->SetInterceptionFlag(IF_Intercepte);
	return 0;
}

#define notless_notmore(id, idbottombound, idupbound) \
	(id>=idbottombound)&&(id<=idupbound)

bool VideoWndQuay::SOURCE_STUB_WINDOW::DispatchReflectedCommand(
	LRESULT* plr, WPARAM wp, LPARAM lp, UINT msg)
{
	if(msg!=WM_COMMAND)
		return false;
	this->SetInterceptionFlag(IF_Intercepte);
	int id = LOWORD(wp); int code = HIWORD(wp);
	if(id==ID_MainUI_TOPMOST)
	{
		m_topmost = !m_topmost;
		long oldlong = GetWindowLong(GWL_EXSTYLE);
		if(m_topmost)
		{
			oldlong |= WS_EX_TOPMOST;
		}
		else
		{
			oldlong &= (~WS_EX_TOPMOST);
		}
		SetWindowLong(GWL_EXSTYLE, oldlong);
		if(m_topmost)
		{
			SetWindowPos(HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
		}
		else
		{
			SetWindowPos(HWND_NOTOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
		}
		return 0;
	}
	else if(id==ID_MainUI_FULLMODE)
	{
 		return 0;
	}
	else if(notless_notmore(id, __DONTUSE_ID_MainUI_PLAYCTRL_FIRST, __DONTUSE_ID_MainUI_PLAYCTRL_LAST))
		*plr = OnPlayCtrlCmd(code, id, lp);
	else if(notless_notmore(id, __DONTUSE_ID_MainUI_PLAYLIST_FIRST, __DONTUSE_ID_MainUI_PLAYLIST_LAST))
		*plr = OnPlaylistCmd(code, id, lp);
	else if(notless_notmore(id, __DONTUSE_ID_MainUI_OPTION_FIRST, __DONTUSE_ID_MainUI_OPTION_LAST))
		*plr = OnOptionCmd(code, id, lp);
	else
		return false;
	return true;
}

impl_cmd_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnPlayCtrlCmd)
{
	return 0;
}

impl_cmd_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnPlaylistCmd)
{
	return 0;
}

impl_cmd_hdlr(VideoWndQuay::SOURCE_STUB_WINDOW, OnOptionCmd)
{
	return 0;
}

/*
---------------------------------------------------------------------------------
*/


impl_msg_hdlr(VideoWndQuay::WMP_STUB_WINDOW, OnCommand)
{
	if(_mp_thesource->DispatchReflectedCommand(&m_lresult, wp, lp))
	{
		this->SetInterceptionFlag(IF_Intercepte);
	}
	return 0;
}

impl_msg_hdlr(VideoWndQuay::WMP_STUB_WINDOW, OnMenuCommand)
{
	//this->SetInterceptionFlag(IF_Intercepte);
	return 0;
}

