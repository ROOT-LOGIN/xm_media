#pragma once

#ifndef __INCLUDE_XM_SCRNSVR_H__
#define __INCLUDE_XM_SCRNSVR_H__

extern "C" const CLSID CLSID_CXpeulMediaSSInvoker;

class __declspec(uuid("F2061DDE-EDBA-4474-9240-0B233A6BDDA0")) //ATL_NOVTABLE
CXpeulMediaSSInvoker
	:	public CComObjectRootEx<CComMultiThreadModel>,
		public CComCoClass<CXpeulMediaSSInvoker, &CLSID_CXpeulMediaSSInvoker>,
		public IWMPEffects2
{
public:
	DECLARE_REGISTRY_RESOURCEID(IDR_RGS_SCRNSVR)

	DECLARE_NOT_AGGREGATABLE(CXpeulMediaSSInvoker)

	BEGIN_COM_MAP(CXpeulMediaSSInvoker)
		COM_INTERFACE_ENTRY(IWMPEffects)
		COM_INTERFACE_ENTRY(IWMPEffects2)
	END_COM_MAP()

public:
	enum PresetID{ ID_LYRICS_H, ID_LYRICS_V};
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

	CXpeulMediaSSInvoker(void);
	~CXpeulMediaSSInvoker(void);

	static void FillRegistry(bool bInstall = true);
private:
	HWND m_hWndparent;
	IWMPCore* mp_wmpCore;

	long m_CurrentPreset;
	PROCESS_INFORMATION m_ProcessInfo;
	
	RECT m_wndRect;
private:
	STDMETHOD(RunScreenSaver)(void);
	BOOL TerminateScreenSaver(void);
};

OBJECT_ENTRY_AUTO(CLSID_CXpeulMediaSSInvoker, CXpeulMediaSSInvoker)

#endif