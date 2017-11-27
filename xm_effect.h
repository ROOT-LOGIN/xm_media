#pragma once

#ifndef __INCLUDE_XM_EFFECT_H__
#define __INCLUDE_XM_EFFECT_H__

#include "resource.h"
#include "xm_lyrics_parse.h"

extern "C" const CLSID CLSID_CXpeulMediaEffect;

extern const wchar_t* const str_def_lrycspath;

class __declspec(uuid("F0C629D0-9E85-4672-87E6-54BA08C9487A")) //ATL_NO_VTABLE
CXpeulMediaEffect
	:	public CComObjectRoot,//Ex<CComMultiThreadModel>,
		public CComCoClass<CXpeulMediaEffect, &CLSID_CXpeulMediaEffect>,
		public IWMPEffects2
{
public:
	DECLARE_REGISTRY_RESOURCEID(IDR_RGS_EFFECT)

	DECLARE_NOT_AGGREGATABLE(CXpeulMediaEffect)

	BEGIN_COM_MAP(CXpeulMediaEffect)
		COM_INTERFACE_ENTRY(IWMPEffects)
		COM_INTERFACE_ENTRY(IWMPEffects2)
	END_COM_MAP()

public:
	enum PresetID{ ID_LYRICS_H, ID_LYRICS_V, ID_LYRICS_B};
//IWMPEffects
	STDMETHODIMP Render(TimedLevel* pLevels, HDC hdc, RECT* prc);
	STDMETHODIMP MediaInfo(long lChannelCount, long lSampleRate, BSTR bstrTitle);
	STDMETHODIMP GetCapabilities(DWORD* pdwCapabilities);
	STDMETHODIMP GetTitle(BSTR* bstrTitle);
	STDMETHODIMP GetPresetTitle(long nPreset, BSTR* bstrPresetTitle);
	STDMETHODIMP GetPresetCount(long* pnPresetCount);
	STDMETHODIMP SetCurrentPreset(long nPreset);
	STDMETHODIMP GetCurrentPreset(long* pnPreset);
	STDMETHODIMP DisplayPropertyPage(HWND hwndOwner);
	STDMETHODIMP GoFullscreen(BOOL isfullscreen);
	STDMETHODIMP RenderFullScreen(TimedLevel* pLevels);

//IWMPEffects2
	STDMETHODIMP Create(HWND hwndparent);
	STDMETHODIMP Destroy(void);
	STDMETHODIMP SetCore(IWMPCore* pCore);
	STDMETHODIMP NotifyNewMedia(IWMPMedia* pMedia);
	STDMETHODIMP OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT* plResult);
	STDMETHODIMP RenderWindowed(TimedLevel* pLevels, BOOL bRequiredRender);

	CXpeulMediaEffect(void);
	~CXpeulMediaEffect(void);

	static void FillRegistry(void);
private:
	STDMETHODIMP RenderH(TimedLevel* pLevels, HDC hdc, RECT* prc);
	STDMETHODIMP RenderV(TimedLevel* pLevels, HDC hdc, RECT* prc);
	STDMETHODIMP RenderB(TimedLevel* pLevels, HDC hdc, RECT* prc);
	STDMETHODIMP RenderNoLyrics(TimedLevel* pLevels, HDC hdc, RECT* prc);
	STDMETHODIMP DrawWaveform(TimedLevel* pLevels, Gdiplus::Graphics* gdips_gps, Gdiplus::Rect* gdips_rect);
	STDMETHODIMP InternalError(HDC hdc, RECT* prc);

	HWND m_hWndparent;
	long m_CurrentPreset;
	static const int m_XmEffectPresetCount;

	Lyrics* mp_LyricsObject;
	__int64 m_nDuration;
	MAP_N2WSTR::iterator m_itLyricsValptr;
	
friend class CXpeulMediaDeskBand;
};

DWORD __stdcall _XM_StartParseLyricsThread(
	void* pv
);

#endif
