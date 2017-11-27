#pragma once

#ifndef __INCLUDE_XM_BACK_H__
#define __INCLUDE_XM_BACK_H__

#include "_xm_helpers.h"

#define WS_POPUP_SC (WS_POPUP|WS_SIZEBOX|WS_CLIPCHILDREN|WS_CLIPSIBLINGS)

class NotifyIconResponser : public CWindowImpl<NotifyIconResponser, CWindow, CNullTraits>
{
protected:
	decl_msg_hdlr(OnNotifyIcon);
	decl_msg_hdlr(OnCommand);

	BEGIN_MSG_MAP(NotifyIconResponser)
		MESSAGE_HANDLER(XM_WM_NOTIFYICON, OnNotifyIcon)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	END_MSG_MAP()

	class TipRender : public CWindowImpl<TipRender, CWindow, 
		CWinTraits<WS_POPUP_SC, WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE>>
	{
		friend class NotifyIconResponser;
	public:
		DECLARE_WND_CLASS_EX(L"XmAeroTipRender", 0, (HBRUSH)GetStockObject(BLACK_BRUSH)-1)

		explicit TipRender(NotifyIconResponser* ptrayicon)
		{
			mp_trayicon = ptrayicon;
		}

		~TipRender(void)
		{
			
		}
	protected:
		decl_msg_hdlr(OnPaint);
		decl_msg_hdlr(OnActivate);

		BEGIN_MSG_MAP(TipRender)
			MESSAGE_HANDLER(WM_ACTIVATE, OnActivate);
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
		END_MSG_MAP()

		NotifyIconResponser* mp_trayicon;
	};

public:
	DECLARE_WND_CLASS_EX(L"XmWmpNotifyIconResponser", 0, 0);

	void SetNotifyIcon(bool do_force_delete = false);
	bool InitNotifyIconData(void);
	inline bool IsNotifyIconVisible(void) const{ return m_notifyicon_visible;}

	void LastCleanUp(void);

	NotifyIconResponser(void);
	~NotifyIconResponser(void);

private:
	NOTIFYICONDATA m_nid;
	bool m_notifyicon_visible;
	TipRender m_tip;
};

#define decl_cmd_hdlr(_func) \
	LRESULT _func(int code, int id, LPARAM lp)
#define impl_cmd_hdlr(_cls, _func) \
	LRESULT _cls::_func(int code, int id, LPARAM lp)

class VideoWndQuay
{
protected:
	bool DetectContext(void);

	class SOURCE_STUB_WINDOW : public STUB_WINDOW
	{
		decl_msg_hdlr(OnInitMenu);
		decl_msg_hdlr(OnInitMenuPopup);
		decl_msg_hdlr(OnUninitMenuPopup);
		decl_msg_hdlr(OnParentNotify);
		decl_msg_hdlr(OnWndPosChanged);
		decl_msg_hdlr(OnEnterMenuLoop);
		decl_msg_hdlr(OnExitMenuLoop);
		decl_msg_hdlr(OnNcCalcSize);

		decl_msg_hdlr(OnMenuSelect);

		decl_msg_hdlr(OnSysCmd)
		{
			if(wp == SC_CLOSE)
			{
				this->SetInterceptionFlag(IF_Intercepte);
			}
			return 0;
		}

		decl_cmd_hdlr(OnPlayCtrlCmd);
		decl_cmd_hdlr(OnPlaylistCmd);
		decl_cmd_hdlr(OnOptionCmd);

		decl_msg_hdlr(OnSetCursor);

		BEGIN_MSG_MAP(SOURCE_STUB_WINDOW)
			MESSAGE_HANDLER(WM_INITMENU, OnInitMenu)
			MESSAGE_HANDLER(WM_ENTERMENULOOP, OnEnterMenuLoop)
			MESSAGE_HANDLER(WM_EXITMENULOOP, OnExitMenuLoop)
			MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
			MESSAGE_HANDLER(WM_UNINITMENUPOPUP, OnUninitMenuPopup)
			MESSAGE_HANDLER(WM_PARENTNOTIFY, OnParentNotify)
			//MESSAGE_HANDLER(WM_NCCALCSIZE, OnNcCalcSize)
			MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWndPosChanged)

			MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
			MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCmd)

			//MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		END_MSG_MAP()

		HMENU _m_hmainuimenu;
		VideoWndQuay* _mp_thequay;
		bool m_topmost;

		class SRC_CONSOLE : public CWindowImpl<SRC_CONSOLE>
		{
			BEGIN_MSG_MAP(SRC_CONSOLE)
			END_MSG_MAP()
		public:
			DECLARE_WND_CLASS_EX(L"SRC_CONSOLE_PANEL", 0, COLOR_WINDOW)
		};

		SRC_CONSOLE _m_consolepanel;
		struct MONITOR_DATA
		{
			HWND hparant;
			HWND hsource;
		} _m_monitordata;
		HANDLE _m_hmonitorthread;
	public:
		explicit SOURCE_STUB_WINDOW(VideoWndQuay* pvwq)
			: _m_hmainuimenu(NULL), _mp_thequay(pvwq), m_topmost(true), _m_hmonitorthread(NULL){}
		void LastCleanUp(void){ }
		inline bool IsTopmost(void) const{ return m_topmost;}
		bool DispatchReflectedCommand(LRESULT* plr, WPARAM wp, LPARAM lp, UINT msg = WM_COMMAND);
		virtual DWORD BeginInterceptionGame(HWND hwnd);
		virtual DWORD EndInterceptionGame(void);
		//~SOURCE_STUB_WINDOW(void)
		//	{ if(_m_hmainuimenu){DestroyMenu(_m_hmainuimenu); _m_hmainuimenu=NULL;}}
	};

	class WMP_STUB_WINDOW : public STUB_WINDOW
	{
		decl_msg_hdlr(OnCommand);
		decl_msg_hdlr(OnMenuCommand);

		BEGIN_MSG_MAP(WMP_STUB_WINDOW)
			MESSAGE_HANDLER(WM_COMMAND, OnCommand)
			MESSAGE_HANDLER(WM_MENUCOMMAND, OnMenuCommand)
		END_MSG_MAP()
		
		VideoWndQuay* _mp_thequay;
		SOURCE_STUB_WINDOW* _mp_thesource;
	public:
		WMP_STUB_WINDOW(VideoWndQuay* pvwq, SOURCE_STUB_WINDOW* pstw)
			: _mp_thequay(pvwq), _mp_thesource(pstw){}
	};

public:
//	DECLARE_WND_CLASS_EX(L"XmWmpVideoWndQuay", 0, 0);

	void SaveOrLoadState(bool do_load);
	void AnchorVideoWnd(void);
	inline bool IsVideoWndAnchorring(void) const{ return m_videownd_anchorring;}

	void LastCleanUp(void);

	VideoWndQuay();
	~VideoWndQuay();

private:
	HWND m_hWndSource; // the window will be anchorred to this window
	HWND m_hWndDestination; // the m_hWndSource's parent window before anchorred

	LONG m_wstylesrc;
	LONG m_wexstylesrc;
	CRect m_rcsrc; // the window rect before been anchorred
	CRect m_rc; // the window rect before been un-anchorred
	SOURCE_STUB_WINDOW m_sourcestub;
	WMP_STUB_WINDOW m_wmpstub;
	bool m_videownd_anchorring;
};

extern "C" const CLSID CLSID_CXpeulMediaBack;

typedef struct XpeulMediaBackData
{
	IWMPCore* mp_Core;

	HRESULT SetCore(IWMPCore* pCore)
	{
		if(mp_Core)
			mp_Core->Release();

		if(pCore)
		{
			return pCore->QueryInterface(__uuidof(IWMPCore), (void**)&mp_Core);
		}
		else
		{
			mp_Core = NULL;
			return S_OK;
		}
	}
	XpeulMediaBackData(void)
	{
		memset(this, 0, sizeof(XpeulMediaBackData));
	}

	~XpeulMediaBackData(void)
	{
	}
} XMBackData;

extern XMBackData xmback_data;

class __declspec(uuid("F3DE884F-C8BD-4665-8734-FE62648B7DDC")) //ATL_NO_VTABLE
CXpeulMediaBack
	:	public CComObjectRoot, //Ex<CComMultiThreadModel>,
		public CComCoClass<CXpeulMediaBack, &CLSID_CXpeulMediaBack>,
		public IWMPPluginUI,
		public IWMPEvents
{
public:
	DECLARE_REGISTRY_RESOURCEID(IDR_RGS_BACK)

	DECLARE_NOT_AGGREGATABLE(CXpeulMediaBack)

	BEGIN_COM_MAP(CXpeulMediaBack)
		COM_INTERFACE_ENTRY(IWMPPluginUI)
		COM_INTERFACE_ENTRY(IWMPEvents)
	END_COM_MAP()
	
public:
//IWMPPluginUI
	STDMETHODIMP SetCore(IWMPCore *pCore);
	STDMETHODIMP DisplayPropertyPage(HWND hwndParent);
	STDMETHODIMP SetProperty(const WCHAR* name, const VARIANT* pVariant);
	STDMETHODIMP GetProperty(const WCHAR* name, VARIANT* pVariant);
	STDMETHODIMP Create(HWND hwndparent, HWND* phWnd);
	STDMETHODIMP Destroy(void);
	STDMETHODIMP TranslateAccelerator(LPMSG lpmsg);

//IWMPEvents
	void STDMETHODCALLTYPE OpenStateChange( long NewState);
	void STDMETHODCALLTYPE PlayStateChange( long NewState);
	void STDMETHODCALLTYPE AudioLanguageChange( long LangID);
	void STDMETHODCALLTYPE StatusChange( void);
	void STDMETHODCALLTYPE ScriptCommand( BSTR scType,BSTR Param);
	void STDMETHODCALLTYPE NewStream( void);
	void STDMETHODCALLTYPE Disconnect( long Result);
	void STDMETHODCALLTYPE Buffering( VARIANT_BOOL Start);
	void STDMETHODCALLTYPE Error( void);
	void STDMETHODCALLTYPE Warning( long WarningType,long Param,BSTR Description);
	void STDMETHODCALLTYPE EndOfStream( long Result);
	void STDMETHODCALLTYPE PositionChange( double oldPosition,double newPosition);
	void STDMETHODCALLTYPE MarkerHit( long MarkerNum);
	void STDMETHODCALLTYPE DurationUnitChange( long NewDurationUnit);
	void STDMETHODCALLTYPE CdromMediaChange( long CdromNum);
	void STDMETHODCALLTYPE PlaylistChange( IDispatch *Playlist, WMPPlaylistChangeEventType change);
	void STDMETHODCALLTYPE CurrentPlaylistChange( WMPPlaylistChangeEventType change);
	void STDMETHODCALLTYPE CurrentPlaylistItemAvailable( BSTR bstrItemName);
	void STDMETHODCALLTYPE MediaChange( IDispatch *Item);
	void STDMETHODCALLTYPE CurrentMediaItemAvailable( BSTR bstrItemName);
	void STDMETHODCALLTYPE CurrentItemChange( IDispatch *pdispMedia);
	void STDMETHODCALLTYPE MediaCollectionChange( void);
	void STDMETHODCALLTYPE MediaCollectionAttributeStringAdded( BSTR bstrAttribName,BSTR bstrAttribVal);
	void STDMETHODCALLTYPE MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,BSTR bstrAttribVal);
	void STDMETHODCALLTYPE MediaCollectionAttributeStringChanged( BSTR bstrAttribName,BSTR bstrOldAttribVal,BSTR bstrNewAttribVal);
	void STDMETHODCALLTYPE PlaylistCollectionChange( void);
	void STDMETHODCALLTYPE PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName);
	void STDMETHODCALLTYPE PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName);
	void STDMETHODCALLTYPE PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName,VARIANT_BOOL varfIsDeleted);
	void STDMETHODCALLTYPE ModeChange( BSTR ModeName,VARIANT_BOOL NewValue);
	void STDMETHODCALLTYPE MediaError( IDispatch *pMediaObject);
	void STDMETHODCALLTYPE OpenPlaylistSwitch( IDispatch *pItem);
	void STDMETHODCALLTYPE DomainChange( BSTR strDomain);
	void STDMETHODCALLTYPE SwitchedToPlayerApplication( void);
	void STDMETHODCALLTYPE SwitchedToControl( void);
	void STDMETHODCALLTYPE PlayerDockedStateChange( void);
	void STDMETHODCALLTYPE PlayerReconnect( void);
	void STDMETHODCALLTYPE Click( short nButton,short nShiftState,long fX,long fY);
	void STDMETHODCALLTYPE DoubleClick( short nButton,short nShiftState,long fX,long fY);
	void STDMETHODCALLTYPE KeyDown( short nKeyCode,short nShiftState);
	void STDMETHODCALLTYPE KeyPress( short nKeyAscii);
	void STDMETHODCALLTYPE KeyUp( short nKeyCode,short nShiftState);
	void STDMETHODCALLTYPE MouseDown( short nButton,short nShiftState,long fX,long fY);
	void STDMETHODCALLTYPE MouseMove( short nButton,short nShiftState,long fX,long fY);
	void STDMETHODCALLTYPE MouseUp( short nButton,short nShiftState,long fX,long fY);

	CXpeulMediaBack(void);
	~CXpeulMediaBack(void);

protected:
	// bug: if multiple CXpeulMediaBacks are created, multiple hook will be set
	class HotkeyHook : public __ThisCallWinAPIRoutine<HOOKPROC, HotkeyHook>
	{
		friend __ThisCallWinAPIRoutine<HOOKPROC, HotkeyHook>;
		friend CXpeulMediaBack;

	public:
		explicit HotkeyHook(CXpeulMediaBack* pback);
		~HotkeyHook (void);

		void LastCleanUp(void);
	protected:
		typedef LRESULT (__thiscall HotkeyHook::*PFN_ENTRY)(int, WPARAM, LPARAM);

		LRESULT __thiscall entry_function(int code, WPARAM wp, LPARAM lp);

		typedef HotkeyInfo XMB_HOTKEY;

		XMB_HOTKEY m_hotkey_PlayPause;
		XMB_HOTKEY m_hotkey_Stop;
		XMB_HOTKEY m_hotkey_Next;
		XMB_HOTKEY m_hotkey_Prev;
		XMB_HOTKEY m_hotkey_VolUp;
		XMB_HOTKEY m_hotkey_VolDn;
		XMB_HOTKEY m_hotkey_TrayIcon;
		XMB_HOTKEY m_hotkey_VideoHook;
		
		NotifyIconResponser m_trayicon;

		VideoWndQuay m_videoquay;

		void ManageWmpState(void)
		{
		}

	private:
		HHOOK m_hookhandle;
		DWORD last_message_time;
	};

private:
	HotkeyHook m_hotkeyhook;

	BOOL SetHookAndRegHotkey(void);
	BOOL UnHookAndUnRegHotkey(void);

	CComPtr<IConnectionPoint> mp_ConnPnt;
	DWORD m_dwAdviseCookie;
};

#endif
