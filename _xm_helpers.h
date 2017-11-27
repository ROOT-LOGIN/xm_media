#pragma once

#include "shellapi.h" 

#ifndef ___INCLUDE_XM_HELPERS_H__
#define ___INCLUDE_XM_HELPERS_H__

#define NULL_RET(v) if(v==NULL) return
#define FAILED_RET(rst) if(FAILED(rst)) return rst
#define DEL_AND_SET_NULL(p) if(p){delete p; p=NULL;}
#define REL_AND_SET_NULL(p) if(p){p->Release(); p=NULL;}

// call __thiscall as __stdcall
// Usage: your class derive from __ThisCallWinAPIRoutine and make it friend
// then typedef "PFN_ENTRY" as your entry and define the real-entry "entry_function"
template<typename _Ty_WinApiDefinedEntry, typename _Ty_Class>
class __ThisCallWinAPIRoutine
{
private:
#pragma pack(1)
	class __thiscall_thunk
	{
		//__asm mov ecx, this
		BYTE __asm_mov_ecx;
		DWORD _this_ptr;
		//__asm mov eax, _entry_func
		BYTE __asm_mov_eax;
		DWORD _entry_func_ptr;
		//__asm jmp eax
		WORD __asm_jmp_eax;
	public:
		BOOL initialize(_Ty_Class* entry)
		{
			__asm_mov_ecx = 0xb9;
			_this_ptr = (DWORD)entry;
			__asm_mov_eax = 0xb8;
			_entry_func_ptr = (DWORD)(entry->MakeEntry());
			__asm_jmp_eax = 0xe0ff;

			return FlushInstructionCache(GetCurrentProcess(), this, sizeof(__thiscall_thunk));
		}

		_Ty_WinApiDefinedEntry MakeEntry(void)
		{
			return (_Ty_WinApiDefinedEntry)(this);
		}
	};
#pragma pack()
	__thiscall_thunk* _p_loader;

	__ThisCallWinAPIRoutine(const __ThisCallWinAPIRoutine&);
	__ThisCallWinAPIRoutine& operator=(const __ThisCallWinAPIRoutine&);

protected:
	void prepare_entry_function(void*)
	{
		if(_p_loader == NULL)
		{
			_p_loader = (__thiscall_thunk*)VirtualAlloc(NULL, sizeof(__thiscall_thunk), 
				MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			_p_loader->initialize((_Ty_Class*)this);
			DWORD oldprot;
			VirtualProtect(_p_loader, sizeof(__thiscall_thunk), PAGE_EXECUTE_READ, &oldprot);
		}
	}

public:
	_Ty_WinApiDefinedEntry MakeEntry(void)
	{
		_Ty_Class::PFN_ENTRY ptr = &_Ty_Class::entry_function;
		return (_Ty_WinApiDefinedEntry)(void*)(int)*((int*)(&ptr));
	}

	_Ty_WinApiDefinedEntry make_entry(void)
	{
		if(_p_loader)
		{
			return _p_loader->MakeEntry();
		}
		else
			return (_Ty_WinApiDefinedEntry)NULL;
	}

	__ThisCallWinAPIRoutine(void) : _p_loader(NULL)
	{
	}

	~__ThisCallWinAPIRoutine(void)
	{
		if(_p_loader)
		{
			VirtualFree(_p_loader, sizeof(__thiscall_thunk), MEM_FREE);
		}
	}
};

/*
// *****************************************************************************
*/

// let m_oldwndproc process the Origion Message
#define IF_Pass							0x0F
// we Modified the m_pCurrentMsg, 
// let m_oldwndproc process the "Current" Message
#define	IF_Dispatch					0xF0
// if IF_INVALID_METHOD is SET, IF_Dispatch will be ignored
// and in debug, this will cause an assert.
#define IF_INVALID_METHOD		(IF_Pass | IF_Dispatch)
//we need the second chance to procss the message
#define	IF_Reflect						0xf00d0000UL
#define IF_PassAndReflect			(IF_Reflect | (WORD)IF_Pass)
#define IF_DispatchAndReflect		(IF_Reflect | (WORD)IF_Dispatch)
//we will do everything
#define IF_Intercepte					0xffffffffUL

class STUB_WINDOW : public CWindowImpl<STUB_WINDOW, CWindow, CNullTraits>
{
public:
	//returns GetLastError()
	virtual DWORD BeginInterceptionGame(HWND hwnd);
	virtual DWORD EndInterceptionGame(void);

protected:
	virtual WNDPROC GetWindowProc(){ return WindowProc;}
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	STUB_WINDOW(void) : m_oldwndproc(NULL){}
	virtual ~STUB_WINDOW(void){}

	typedef LONG InterceptionFlag;
	// default value is IF_Pass
	InterceptionFlag m_if_flag;
	inline void SetInterceptionFlag(InterceptionFlag flag){ m_if_flag = flag;}
	inline InterceptionFlag GetInterceptionFlag(void) const{ return m_if_flag;}
	//automatically set to 0 before any message processed
	LRESULT m_lresult;

private:
	WNDPROC m_oldwndproc;

	STUB_WINDOW(const STUB_WINDOW&);
	STUB_WINDOW& operator=(const STUB_WINDOW&);
	HWND Create(HWND,_U_RECT,LPCTSTR,DWORD,DWORD,_U_MENUorID,LPVOID);
};

/*
// *****************************************************************************
*/

#define decl_msg_hdlr(_func) \
	LRESULT _func(UINT msg, WPARAM wp, LPARAM lp, BOOL& bH)

#define impl_msg_hdlr(_class, _func) \
	LRESULT _class::_func(UINT msg, WPARAM wp, LPARAM lp, BOOL& bH)

struct GlobalData
{
	HINSTANCE GetInstance(void) const{ return hDllInst;}
	HWND GetWmp(void) const{ return hWmp;}
	bool GetWriteLog(void) const{ return dwLog>0?true:false;}

	GlobalData(void) : hDllInst(NULL), dwLog(0xFFFFFFFF)
	{
		hWmp=FindWindow(L"WMPlayerApp", L"Windows Media Player");
		HKEY hKey=NULL;
		RegOpenKey(HKEY_CURRENT_USER, L"Software\\XpeulEnterprise\\XpeulMedia\\XpeulMediaBack", &hKey);
		if(hKey){
			DWORD dw=sizeof(DWORD);
			RegGetValue(hKey, NULL, L"writelog", RRF_RT_REG_DWORD, NULL, &dwLog, &dw);
			RegCloseKey(hKey);
		}
	}
	~GlobalData(void){}

private:
	HINSTANCE hDllInst;
	HWND hWmp;
	DWORD dwLog;

	friend BOOL CALLBACK DllMain( HINSTANCE, DWORD, LPVOID);
};

extern GlobalData g_data;

#include "xm_hook.h"

class CXpeulMediaBackHelper
{
	CComPtr<IWMPCore> mp_wmpCore;

	typedef HotkeyInfo XMB_HOTKEY;
	XMB_HOTKEY m_hotkey_Play_Pause;
	XMB_HOTKEY m_hotkey_Stop;
	XMB_HOTKEY m_hotkey_Next;
	XMB_HOTKEY m_hotkey_Prev;
	XMB_HOTKEY m_hotkey_VolUp;
	XMB_HOTKEY m_hotkey_VolDn;
	XMB_HOTKEY m_hotkey_TrayIcon;
	XMB_HOTKEY m_hotkey_VideoHook;

public:
	CXpeulMediaBackHelper(void);
	~CXpeulMediaBackHelper(void);

	IWMPCore* GetCore(void) const{return mp_wmpCore;}

#define GET_HOTKEY_LONG(hotkey) ((hotkey.key<<16) | hotkey.mod)

	long GetHotkey_playpause(void) const{ return GET_HOTKEY_LONG(m_hotkey_Play_Pause);}
	long GetHotkey_stop(void) const{ return GET_HOTKEY_LONG(m_hotkey_Stop);}
	long GetHotkey_next(void) const{ return GET_HOTKEY_LONG(m_hotkey_Next);}
	long GetHotkey_prev(void) const{ return GET_HOTKEY_LONG(m_hotkey_Prev);}
	long GetHotkey_volup(void) const{ return GET_HOTKEY_LONG(m_hotkey_VolUp);}
	long GetHotkey_voldn(void) const{ return GET_HOTKEY_LONG(m_hotkey_VolDn);}
	long GetHotkey_trayicon(void) const{ return GET_HOTKEY_LONG(m_hotkey_TrayIcon);}
	long GetHotkey_videohook(void) const{ return GET_HOTKEY_LONG(m_hotkey_VideoHook);}

#undef GET_HOTKEY_LONG

	void ShowWmp(void) const;
	bool SetTrayIconSinkWnd(void);
	void SetVideoSinkWnd(void);

	LONG GetWndproc(void) const{return WPH_WndProc;}

	HRESULT NewPlayList(IWMPCore* pCore, const wchar_t* lpstrFile);

private:
	CXpeulMediaBackHelper(CXpeulMediaBackHelper&);
	CXpeulMediaBackHelper& operator=(CXpeulMediaBackHelper&);
	
private:
	bool TrayIconState;
	HWND hwndTrayIconSinkWnd;

	bool VideoSinkState;
	HWND hwndVideoSinkWnd;
	HWND hwndWmpCtrlCntr;
	HWND hwndWmpPluginHost;
	RECT rcWPH;
	LONG WPH_WndProc;
	
	friend class CXpeulMediaBack;
};


extern CXpeulMediaBackHelper xmb_helper;

#define XM_WM_NOTIFYICON (WM_USER+0x1111)
LRESULT CALLBACK XmTrayIconProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK XmVideoSinkProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK XmWmpWPHProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);

void QueryTaskbarPos(
	APPBARDATA& abd,
	bool force_to_workarea = false
);

class CXpeulMediaEffectHelper
{
	CComPtr<IWMPCore> mp_wmpCore;
	CComPtr<IWMPMedia> mp_wmpMedia;

	Gdiplus::Color* gdips_color_bg;

	int n_waveform_points;
	Gdiplus::Pen* gdips_pen_wave_l;
	Gdiplus::Pen* gdips_pen_wave_r;

	Gdiplus::SolidBrush* gdips_sbrush_info;

	Gdiplus::SolidBrush* gdips_sbrush_lrc_pre_h;
	Gdiplus::SolidBrush* gdips_sbrush_lrc_finish_h;

	Gdiplus::SolidBrush* gdips_sbrush_lrc_v;
	Gdiplus::SolidBrush* gdips_sbrush_lrc_pre_v;
	Gdiplus::SolidBrush* gdips_sbrush_lrc_finish_v;

	Gdiplus::Font* gdips_font_lrc;
	Gdiplus::Font* gdips_font_lrc_v;
	Gdiplus::Font* gdips_font_info;
	Gdiplus::FontFamily* gdips_fontfamily_lrc;
	Gdiplus::StringFormat* gdips_strfmt_lrc;

	Gdiplus::SolidBrush* gdips_sbrush_freq_l;
	Gdiplus::SolidBrush* gdips_sbrush_freq_r;
	Gdiplus::SolidBrush* gdips_sbrush_freq_hilite;
	BOOL bFreqDoExchange;

	ULONG_PTR token;
	Gdiplus::GdiplusStartupInput GdiInput;
	Gdiplus::GdiplusStartupOutput GdiOutput;

public:

	void InitGdiplusObject(void);
	void DeleteGdiplusObject(void);
	void ReCreateGdiplusObject(void);
public:
	IWMPCore* GetCore(void) const{ return mp_wmpCore.p;}
	IWMPMedia* GetMedia(void) const{ return mp_wmpMedia.p;}
	
	CXpeulMediaEffectHelper(void);
	~CXpeulMediaEffectHelper(void);
	friend class CXpeulMediaEffect;
};

extern CXpeulMediaEffectHelper xme_helper;

#endif
