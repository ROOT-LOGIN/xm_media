#pragma once

#ifndef __INCLUDE_XM_HOOK_H__
#define __INCLUDE_XM_HOOK_H__

#define XM_HOOKPROC LRESULT CALLBACK

XM_HOOKPROC HookProc_GetMsg(int nCode, WPARAM wparam, LPARAM lparam);

#define MOD_SA (MOD_SHIFT | MOD_ALT)
#define MOD_SC (MOT_SHIFT | MOD_CONTROL)
#define MOD_AC (MOD_ALT | MOD_CONTROL)
#define MOD_ASC (MOD_ALT | MOD_SHIFT | MOD_CONTROL)
#define MOD_WS (MOD_WIN | MOD_SHIFT)
#define MOD_WA (MOD_WIN | MOD_ALT)
#define MOD_WC (MOD_WIN | MOD_CONTROL)
#define MOD_WSA (MOD_WIN | MOD_SA)
#define MOD_WSC (MOD_WIN | MOD_SC)
#define MOD_WAC (MOD_WIN | MOD_AC)
#define MOD_WASC (MOD_WIN | MOD_ASC)
#define MOD_NONE NULL

extern const wchar_t* const str_hotkey_playpause;
extern const wchar_t* const str_hotkey_stop;
extern const wchar_t* const str_hotkey_next;
extern const wchar_t* const str_hotkey_prev;
extern const wchar_t* const str_hotkey_volup;
extern const wchar_t* const str_hotkey_voldn;
extern const wchar_t* const str_hotkey_trayicon;
extern const wchar_t* const str_hotkey_videohook;
extern const wchar_t* const str_hotkey_replaybegin;
extern const wchar_t* const str_hotkey_replayend;
extern const wchar_t* const str_hotkey_replaycancle;

struct HotkeyInfo
{
	const wchar_t* name;
	ATOM id;
	UINT mod;
	UINT key;

	inline void SetKey(UINT m=MOD_NONE, UINT k=NULL){ mod=m; key=k;}
	inline long GetLong(void) const{ return ((key<<16) | mod);}

	inline BOOL Register(HWND hwnd=NULL) const{ return ::RegisterHotKey(hwnd, id, mod, key);}
	inline BOOL Unregister(HWND hwnd=NULL) const{ return ::UnregisterHotKey(hwnd, id);}
		
	HotkeyInfo(const wchar_t* str) : name(str)
	{
		id=::GlobalAddAtom(str);
		mod=MOD_NONE; key=NULL;
	}
	~HotkeyInfo(void)
	{
		name=NULL;
		::GlobalDeleteAtom(id);
	}

private:
	HotkeyInfo& operator=(const HotkeyInfo&);
	HotkeyInfo(const HotkeyInfo&);
};

#endif
