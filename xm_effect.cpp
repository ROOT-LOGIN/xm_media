#include "stdafx.h"
#include "resource.h"

#include "xm_effect.h"
#include "_xm_helpers.h"
#include "xm_log.h"

// {F0C629D0-9E85-4672-87E6-54BA08C9487A}
extern "C" const CLSID CLSID_CXpeulMediaEffect= 
{ 0xf0c629d0, 0x9e85, 0x4672, { 0x87, 0xe6, 0x54, 0xba, 0x8, 0xc9, 0x48, 0x7a } };

OBJECT_ENTRY_AUTO(CLSID_CXpeulMediaEffect, CXpeulMediaEffect)

const wchar_t* const str_def_lrycspath = L"C:\\Program Files\\Windows Media Player\\Lyrics\\"; 

const int CXpeulMediaEffect::m_XmEffectPresetCount = 3;

CXpeulMediaEffectHelper xme_helper;

CXpeulMediaEffect::CXpeulMediaEffect(void)
{
	m_hWndparent=NULL;
	m_CurrentPreset=0;
	mp_LyricsObject=NULL;
}

CXpeulMediaEffect::~CXpeulMediaEffect(void)
{
	if(mp_LyricsObject){ delete mp_LyricsObject; mp_LyricsObject=NULL;}
}

//IWMPEffects
STDMETHODIMP CXpeulMediaEffect::Render(TimedLevel* pLevels, HDC hdc, RECT* prc)
{
	switch(m_CurrentPreset)
	{
	case ID_LYRICS_H: 
		return RenderH(pLevels, hdc, prc);
	case ID_LYRICS_V:
		return RenderV(pLevels, hdc, prc);
	case ID_LYRICS_B:
		return RenderB(pLevels, hdc, prc);
	default:
		return E_FAIL;
	}
}

STDMETHODIMP CXpeulMediaEffect::MediaInfo(long lChannelCount, long lSampleRate, BSTR bstrTitle)
{
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::GetCapabilities(DWORD* pdwCapabilities)
{
	*pdwCapabilities=EFFECT_CANGOFULLSCREEN | EFFECT_HASPROPERTYPAGE |
		EFFECT_VARIABLEFREQSTEP;
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::GetTitle(BSTR* bstrTitle)
{
	*bstrTitle=SysAllocString(L"XpeulMediaEffect");
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::GetPresetTitle(long nPreset, BSTR* bstrPresetTitle)
{
	if(bstrPresetTitle==NULL)
		return E_INVALIDARG;
	
	wchar_t buffer[MAX_PATH]={'\0'};
	switch(nPreset)
	{
	case ID_LYRICS_H:
		LoadString(g_data.GetInstance(), IDS_EFFECT_PRESET_H, buffer, MAX_PATH);
		break;
	case ID_LYRICS_V:
		LoadString(g_data.GetInstance(), IDS_EFFECT_PRESET_V, buffer, MAX_PATH);
		break;
	case ID_LYRICS_B:
		LoadString(g_data.GetInstance(), IDS_EFFECT_PRESET_B, buffer, MAX_PATH);
		break;
	default:
		return E_INVALIDARG;
	}
	*bstrPresetTitle=SysAllocString(buffer);
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::GetPresetCount(long* pnPresetCount)
{
	*pnPresetCount= m_XmEffectPresetCount;
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::SetCurrentPreset(long nPreset)
{
	m_CurrentPreset=nPreset;
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::GetCurrentPreset(long* pnPreset)
{
	*pnPreset=m_CurrentPreset;
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::DisplayPropertyPage(HWND hwndOwner)
{
	wchar_t info[MAX_PATH]={'\0'};
	LoadString(g_data.GetInstance(), IDS_EFFECT_PROPERTYPAGE_INFO, info, MAX_PATH);
	wchar_t title[MAX_PATH]={'\0'};
	LoadString(g_data.GetInstance(), IDS_EFFECT_NAME, title, MAX_PATH);

	MessageBoxW(hwndOwner, info, title, MB_ICONINFORMATION|MB_OK);

	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::GoFullscreen(BOOL isfullscreen)
{
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::RenderFullScreen(TimedLevel* pLevels)
{
	return S_OK;
}

//IWMPEffects2
STDMETHODIMP CXpeulMediaEffect::Create(HWND hwndparent)
{
	m_hWndparent=hwndparent;
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::Destroy(void)
{
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::SetCore(IWMPCore* pCore)
{
	if(pCore==NULL)
		xme_helper.mp_wmpCore.Release();
	else{
		pCore->AddRef();
		xme_helper.mp_wmpCore=pCore;
	}
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::NotifyNewMedia(IWMPMedia* pMedia)
{
	if(mp_LyricsObject){ delete mp_LyricsObject; mp_LyricsObject=NULL;}
	mp_LyricsObject=new Lyrics;

	if(xme_helper.GetMedia()!=NULL)
		xme_helper.mp_wmpMedia.Release();

	if(pMedia){
		pMedia->AddRef();
		xme_helper.mp_wmpMedia=pMedia;

		LPVOID pv=CoTaskMemAlloc(sizeof(D4GL));
		((D4GL*)pv)->pLyrics=this->mp_LyricsObject;
		pMedia->get_sourceURL(&(((D4GL*)pv)->path));
		::CreateThread(NULL, 0, _XM_StartParseLyricsThread, pv, 0, NULL);
	}
	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT* plResult)
{
	return S_FALSE;
}

STDMETHODIMP CXpeulMediaEffect::RenderWindowed(TimedLevel* pLevels, BOOL bRequiredRender)
{	
	if(m_hWndparent==NULL)
		return S_FALSE;
	HDC hdc=::GetDC(m_hWndparent);
	RECT rc;
	::GetClientRect(m_hWndparent, &rc);

	if(pLevels->state==::pause_state)
	{
		::ReleaseDC(m_hWndparent, hdc);
		return S_OK;
	}
	if(pLevels->state==::stop_state)
	{
		HBRUSH hbr=::CreateSolidBrush(RGB(0,0,0));
		FillRect(hdc, &rc, hbr);
		DeleteObject(hbr);
		::ReleaseDC(m_hWndparent, hdc);
		return S_OK;
	}

	Render(pLevels, hdc, &rc);
	::ReleaseDC(m_hWndparent, hdc);

	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::RenderNoLyrics(TimedLevel* pLevels, HDC hdc, RECT* prc)
{
	Gdiplus::Graphics gdips_gps_scr(hdc);
	Gdiplus::Rect gdips_rect_client(prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top);
	gdips_gps_scr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	Gdiplus::Bitmap gdips_bmp_mem(gdips_rect_client.Width, gdips_rect_client.Height, &gdips_gps_scr);
	
	Gdiplus::Graphics gdips_gps_mem(&gdips_bmp_mem);
	gdips_gps_mem.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	
	xme_helper.gdips_strfmt_lrc->SetAlignment(Gdiplus::StringAlignmentFar);
	
	if(xme_helper.gdips_color_bg)
		gdips_gps_mem.Clear(*xme_helper.gdips_color_bg);
	else
		gdips_gps_mem.Clear(Gdiplus::Color(255, 0, 0, 0));

	WSTR tips;
	if(mp_LyricsObject->GetParsingState()==Lyrics::LyricsPS_ParsedNoLyrics)
	{
		tips.clear();
		wchar_t buffer[MAX_PATH]={'\0'};
		LoadString(g_data.GetInstance(), IDS_EFFECT_PARSED_NOLYRICS, buffer, MAX_PATH);
		tips= buffer;
		goto label_SHOW_WAVEFORM;
	}
	if(mp_LyricsObject->GetParsingState()==Lyrics::LyricsPS_ParsingFromEmbed)
	{
		tips.clear();
		wchar_t buffer[MAX_PATH]={'\0'};
		LoadString(g_data.GetInstance(), IDS_EFFECT_PARSING_EMBED, buffer, MAX_PATH);
		tips= buffer;
		goto label_SHOW_TIP;
	}
	if(mp_LyricsObject->GetParsingState()==Lyrics::LyricsPS_ParsingFromLRCFfile)
	{
		tips.clear();
		wchar_t buffer[MAX_PATH]={'\0'};
		LoadString(g_data.GetInstance(), IDS_EFFECT_PARSING_FILE, buffer, MAX_PATH);
		tips= buffer;
		goto label_SHOW_TIP;
	}

label_SHOW_WAVEFORM:
	DrawWaveform(pLevels, &gdips_gps_mem, &gdips_rect_client);

label_SHOW_TIP:
	gdips_gps_scr.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
	gdips_gps_mem.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
	gdips_gps_mem.DrawString(tips.c_str(), -1, xme_helper.gdips_font_info, Gdiplus::RectF(prc->left,prc->top, prc->right, 50), 
		xme_helper.gdips_strfmt_lrc, xme_helper.gdips_sbrush_info);

	Gdiplus::CachedBitmap gdips_cachedbmp(&gdips_bmp_mem, &gdips_gps_scr);
	gdips_gps_scr.DrawCachedBitmap(&gdips_cachedbmp, prc->left, prc->top);

	return S_OK;
}

DWORD __stdcall _XM_StartParseLyricsThread(void* pv)
{
	DWORD dw_ret=_GetLyricsEmbed(pv);

	if(dw_ret==0){
		CoTaskMemFree(pv);
		return dw_ret;
	}
	
	BSTR title=NULL;
	IWMPMedia* pMedia=xme_helper.GetMedia();
	if(pMedia==NULL){
		CoTaskMemFree(pv);
		return -1;
	}
	HRESULT rst=-1;
	rst=pMedia->getItemInfo(L"Title", &title);
	if(FAILED(rst))
		rst=pMedia->get_name(&title);
	if(FAILED(rst)){
		CoTaskMemFree(pv);
		return -1;
	}
	HKEY hKey=NULL;
	RegOpenKey(HKEY_CURRENT_USER, _T("Software\\XpeulEnterprise\\XpeulMedia\\XpeulLyricsShow"), &hKey);
	WSTR _path;
	if(hKey){
		TCHAR* buffer=new TCHAR[MAX_PATH];
		DWORD size=MAX_PATH;
		RegGetValue(hKey, NULL, NULL, RRF_RT_REG_SZ, NULL, buffer, &size);
		RegCloseKey(hKey);
		_path=buffer;
		delete buffer;
	}
	else{
		_path=str_def_lrycspath;
		_path+=L'*';
	}
	_path+=title;
	_path+=L"*.lrc";
	((D4GL*)pv)->path=::SysAllocString(_path.c_str());

	dw_ret=_GetLyricsFromLRCFile(pv);
	CoTaskMemFree(pv);
	return dw_ret;
}

STDMETHODIMP CXpeulMediaEffect::RenderH(TimedLevel* pLevels, HDC hdc, RECT* prc)
{	
	if(mp_LyricsObject==NULL)
		return InternalError(hdc, prc);

	if(mp_LyricsObject->GetParsingState()!=Lyrics::LyricsPS_ParsedGetLyrics)
		return RenderNoLyrics(pLevels, hdc, prc);

	if(mp_LyricsObject->GetParsingState()==Lyrics::LyricsPS_ParsedGetLyrics)
	{
		Gdiplus::Rect gdips_rect_client(prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top);
		Gdiplus::Rect gdips_rect_lrc(prc->left, prc->bottom-50, prc->right-prc->left, 50);

		m_itLyricsValptr=mp_LyricsObject->GetCurrentLyrics(pLevels->timeStamp);

		xme_helper.gdips_strfmt_lrc->SetAlignment(Gdiplus::StringAlignmentCenter);
		Gdiplus::GraphicsPath gdips_path_lrc;
		gdips_path_lrc.AddString(m_itLyricsValptr->second.c_str(), -1, xme_helper.gdips_fontfamily_lrc, 
						Gdiplus::FontStyleRegular, 50, gdips_rect_lrc, xme_helper.gdips_strfmt_lrc);
		
		Gdiplus::RectF gdips_rect_lrcbound;
		gdips_path_lrc.GetBounds(&gdips_rect_lrcbound);
		static double nX=0;
		static double a, b, v;
		if(m_itLyricsValptr==(--(mp_LyricsObject->map_N2Lyrics->end()))){
			a=(double)(pLevels->timeStamp-m_itLyricsValptr->first);
			b=(double)(m_nDuration-m_itLyricsValptr->first);
			v=a/b;
			nX=gdips_rect_lrcbound.Width*v;
			//nX = gdips_rect_lrcbound.Width* (double)(pLevels->timeStamp-m_itLyricsValptr->first)/(double)(m_nDuration-m_itLyricsValptr->first);
		}else{
			//For Release ver., temp-variable will be optmized by Compiler, so, we must do this else.
			a=(double)(pLevels->timeStamp-m_itLyricsValptr->first);
			static __int64 s1, s2;
			s1=(++m_itLyricsValptr)->first;
			s2=(--m_itLyricsValptr)->first;
			b=(double)(s1-s2);
			v=a/b;
			nX=gdips_rect_lrcbound.Width*v;
			//nX = gdips_rect_lrcbound.Width* (double)(pLevels->timeStamp-m_itLyricsValptr->first)/(double)((++m_itLyricsValptr)->first-(--m_itLyricsValptr)->first);
		}

		Gdiplus::Graphics gdips_gps_scr(hdc);
		gdips_gps_scr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

		Gdiplus::Bitmap gdips_bmp_mem(gdips_rect_client.Width, gdips_rect_client.Height, &gdips_gps_scr);
		Gdiplus::Graphics gdips_gps_mem(&gdips_bmp_mem);
		gdips_gps_mem.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

		if(xme_helper.gdips_color_bg)
			gdips_gps_mem.Clear(*xme_helper.gdips_color_bg);
		else
			gdips_gps_mem.Clear(Gdiplus::Color(255, 0, 0, 0));

		Gdiplus::GraphicsContainer gdips_gpscntr=gdips_gps_mem.BeginContainer();

		gdips_gps_mem.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

		static Gdiplus::PointF points_path[4];
		points_path[0].X=gdips_rect_lrc.X; points_path[0].Y=gdips_rect_lrc.Y;
		points_path[1].X=gdips_rect_lrc.X; points_path[1].Y=prc->bottom;
		points_path[2].X=(float)gdips_rect_lrcbound.X+nX; points_path[2].Y=prc->bottom;
		points_path[3].X=(float)gdips_rect_lrcbound.X+nX; points_path[3].Y=gdips_rect_lrc.Y;

		Gdiplus::GraphicsPath gdips_path_lrc_finish;
		gdips_path_lrc_finish.AddPolygon(points_path, 4);
		Gdiplus::Region gdips_rgn_lrc_finish(&gdips_path_lrc_finish);

		points_path[0].X=gdips_rect_lrcbound.GetRight();
		points_path[1].X=points_path[0].X;

		Gdiplus::GraphicsPath gdips_path_lrc_pre;
		gdips_path_lrc_pre.AddPolygon(points_path, 4);
		Gdiplus::Region gdips_rgn_lrc_pre(&gdips_path_lrc_pre);

		gdips_gps_mem.SetClip(&gdips_rgn_lrc_finish);
		gdips_gps_mem.FillPath(xme_helper.gdips_sbrush_lrc_finish_h, &gdips_path_lrc);
		gdips_gps_mem.SetClip(&gdips_rgn_lrc_pre);
		gdips_gps_mem.FillPath(xme_helper.gdips_sbrush_lrc_pre_h, &gdips_path_lrc);

		gdips_gps_mem.EndContainer(gdips_gpscntr);

		DrawWaveform(pLevels, &gdips_gps_mem, &gdips_rect_client);
	
		Gdiplus::CachedBitmap gdips_cachedbmp(&gdips_bmp_mem, &gdips_gps_scr);
		gdips_gps_scr.DrawCachedBitmap(&gdips_cachedbmp, prc->left, prc->top);
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CXpeulMediaEffect::RenderV(TimedLevel* pLevels, HDC hdc, RECT* prc)
{
	if(mp_LyricsObject==NULL)
		return InternalError(hdc, prc);

	if(mp_LyricsObject->GetParsingState()!=Lyrics::LyricsPS_ParsedGetLyrics)
		return RenderNoLyrics(pLevels, hdc, prc);
	
	if(mp_LyricsObject->GetParsingState()==Lyrics::LyricsPS_ParsedGetLyrics)
	{
		Gdiplus::Rect gdips_rect_client(prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top);
		
		__int64 t_end=0, t_start=0;
		MAP_N2WSTR::iterator drawptr=mp_LyricsObject->GetCurrentLyrics(pLevels->timeStamp);//map_N2Lyrics->begin();
		m_itLyricsValptr=drawptr;
		if(drawptr==(--(mp_LyricsObject->map_N2Lyrics->end()))){
			t_start=drawptr->first;
			t_end=m_nDuration;
		}else{
			t_end=(++drawptr)->first;
			t_start=m_itLyricsValptr->first;
		}
		xme_helper.gdips_strfmt_lrc->SetAlignment(Gdiplus::StringAlignmentCenter);

		Gdiplus::Graphics gdips_gps_scr(hdc);
		gdips_gps_scr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		gdips_gps_scr.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

		Gdiplus::Bitmap gdips_bmp_mem(gdips_rect_client.Width, gdips_rect_client.Height, &gdips_gps_scr);
		Gdiplus::Graphics gdips_gps_mem(&gdips_bmp_mem);
		gdips_gps_mem.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		gdips_gps_mem.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

		MAP_N2WSTR::size_type n_lrc=mp_LyricsObject->map_N2Lyrics->size();

		Gdiplus::Bitmap* pbmp_lrc=new Gdiplus::Bitmap(gdips_rect_client.Width, n_lrc*50, &gdips_gps_scr);

		Gdiplus::Graphics gdips_gps_lrc(pbmp_lrc);
		gdips_gps_lrc.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		gdips_gps_lrc.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		int curlrc_pos=0;
		drawptr=mp_LyricsObject->map_N2Lyrics->begin();
		Gdiplus::RectF gdips_rectf_perlrc(0, 0, gdips_rect_client.Width, 50);
		double a=pLevels->timeStamp-t_start;
		double b=t_end-t_start;
		double v=b/a;
		for(unsigned int i=0; i<n_lrc; i++)
		{
			if(drawptr->first==m_itLyricsValptr->first)
			{
				Gdiplus::Bitmap gdips_bmp_curlrc(gdips_rect_client.Width, 50, &gdips_gps_scr);
				Gdiplus::Graphics gdips_gps_curlrc(&gdips_bmp_curlrc);
				gdips_gps_curlrc.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

				Gdiplus::GraphicsPath gdips_path_curlrc;
				gdips_path_curlrc.AddString(drawptr->second.c_str(), -1, xme_helper.gdips_fontfamily_lrc, 
					Gdiplus::FontStyleBold, 45,
					Gdiplus::RectF(0, 0, gdips_rect_client.Width, 50), xme_helper.gdips_strfmt_lrc);
				
				Gdiplus::RectF gdips_rect_lrcbound;
				gdips_path_curlrc.GetBounds(&gdips_rect_lrcbound);
				float nX=gdips_rect_lrcbound.Width/v;
				Gdiplus::Region gdips_rgn_lrc_finish(Gdiplus::RectF(0, 0, gdips_rect_lrcbound.X+nX, 50));
				Gdiplus::Region gdips_rgn_lrc_pre(Gdiplus::RectF(gdips_rect_lrcbound.X+nX, 0, gdips_rect_client.Width-gdips_rect_lrcbound.X-nX, 50));
		
				gdips_gps_curlrc.SetClip(&gdips_rgn_lrc_finish);
				gdips_gps_curlrc.FillPath(xme_helper.gdips_sbrush_lrc_finish_v, &gdips_path_curlrc);	
				gdips_gps_curlrc.SetClip(&gdips_rgn_lrc_pre);
				gdips_gps_curlrc.FillPath(xme_helper.gdips_sbrush_lrc_pre_v, &gdips_path_curlrc);	
				
				gdips_gps_lrc.DrawImage(&gdips_bmp_curlrc, 0, i*50);
				drawptr++;
				curlrc_pos=i;
			}
			else
			{
				gdips_rectf_perlrc.Y=i*50;
				gdips_gps_lrc.DrawString(drawptr->second.c_str(), -1, xme_helper.gdips_font_lrc_v, gdips_rectf_perlrc,
					xme_helper.gdips_strfmt_lrc, xme_helper.gdips_sbrush_lrc_v);
				drawptr++;
			}
		}
		
		if(xme_helper.gdips_color_bg)
			gdips_gps_mem.Clear(*xme_helper.gdips_color_bg);
		else
			gdips_gps_mem.Clear(Gdiplus::Color(255, 0, 0, 0));

		int l=curlrc_pos*50;
		float y=gdips_rect_client.Height/2-l-50/v;
		gdips_gps_mem.DrawImage(pbmp_lrc, 0.0f, y);
		delete pbmp_lrc;
		Gdiplus::CachedBitmap gdips_cachedbmp(&gdips_bmp_mem, &gdips_gps_scr);
		gdips_gps_scr.DrawCachedBitmap(&gdips_cachedbmp, prc->left, prc->top);

		return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP CXpeulMediaEffect::DrawWaveform(TimedLevel* pLevels, Gdiplus::Graphics* gdips_gps, Gdiplus::Rect* gdips_rect)
{
	if(gdips_rect->Width<xme_helper.n_waveform_points)
		xme_helper.n_waveform_points=gdips_rect->Width*3/4;

	double s=gdips_rect->Width;
	double n=xme_helper.n_waveform_points;
	Gdiplus::REAL per_width=s/n;

	Gdiplus::REAL start_left = (gdips_rect->Height-50.0f)/4.0f+128.0f;
	Gdiplus::REAL start_right = 3*(gdips_rect->Height-50.0f)/4.0f+128.0f;
	
	Gdiplus::PointF* pPntfAry_left=new Gdiplus::PointF[xme_helper.n_waveform_points];
	Gdiplus::PointF* pPntfAry_right=new Gdiplus::PointF[xme_helper.n_waveform_points];
	for(int i=0; i<xme_helper.n_waveform_points; i++)
	{
		pPntfAry_left[i].X=per_width*i;
		pPntfAry_left[i].Y=(float)start_left-pLevels->waveform[0][i];

		pPntfAry_right[i].X=pPntfAry_left[i].X;
		pPntfAry_right[i].Y=(float)start_right-pLevels->waveform[1][i];
	}
	
	pPntfAry_left[xme_helper.n_waveform_points-1].X=gdips_rect->GetRight();
	pPntfAry_right[xme_helper.n_waveform_points-1].X=pPntfAry_left[xme_helper.n_waveform_points-1].X;
		
	
	gdips_gps->DrawPolygon(xme_helper.gdips_pen_wave_l, pPntfAry_left, xme_helper.n_waveform_points);
	gdips_gps->DrawPolygon(xme_helper.gdips_pen_wave_r, pPntfAry_right, xme_helper.n_waveform_points);

	delete[xme_helper.n_waveform_points] pPntfAry_left;
	delete[xme_helper.n_waveform_points] pPntfAry_right;

	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::RenderB(TimedLevel *pLevels, HDC hdc, RECT *prc)
{
	static unsigned __int64 last_timestamp;
	if(pLevels->timeStamp - last_timestamp < 1800000)
		return S_OK;
	last_timestamp = pLevels->timeStamp;

	Gdiplus::Rect gdips_rect_client(prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top);

	Gdiplus::Graphics gdips_gps_scr(hdc);
	gdips_gps_scr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	gdips_gps_scr.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

	Gdiplus::Bitmap gdips_bmp_mem(gdips_rect_client.Width, gdips_rect_client.Height, &gdips_gps_scr);
	Gdiplus::Graphics gdips_gps_mem(&gdips_bmp_mem);
	gdips_gps_mem.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	gdips_gps_mem.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

	int per_width = 18;
	int margin_h = 2;
	int count = gdips_rect_client.Width / (per_width+margin_h);
	int margin_v = 2;
	int per_height = 4;
	int per_max_count = (gdips_rect_client.Height>>1) / (per_height+margin_v);
	int all_counts_l = 0;
	int all_counts_r = 0;
	int all_counted_l = 0;
	int all_counted_r = 0;
	int* pper_count = new int[count];
	for(int i=0; i<count; i++)
	{
		if( i & 0x1)
		{
			pper_count[i] = pLevels->frequency[1][i+8] * per_max_count / 255;
			all_counts_r+=pper_count[i];
		}
		else
		{
			pper_count[i] = pLevels->frequency[0][i+8] * per_max_count / 255;
			all_counts_l+=pper_count[i];
		}
	}
	all_counted_r = all_counts_r;
	all_counted_l = all_counts_l;

	Gdiplus::Rect* pRects_l = new Gdiplus::Rect[all_counts_l];
	Gdiplus::Rect* pRects_r = new Gdiplus::Rect[all_counts_r];
	Gdiplus::Rect* pRects_hilite = new Gdiplus::Rect[count];
	for(int i=0; i<count; i++)
	{
		if((all_counted_l || all_counted_r) && false)
			break;

		if(i & 0x1)
		{
			for(int j=0; j<pper_count[i]; j++)
			{
				--all_counted_r;
				if(last_timestamp & 0x1)
					pRects_r[all_counted_r].X = (i-1)*(per_width+margin_h);
				else
					pRects_r[all_counted_r].X = i*(per_width+margin_h);
				pRects_r[all_counted_r].Y = j*(per_height+margin_v)+(gdips_rect_client.Height>>1);
				pRects_r[all_counted_r].Width = per_width;
				pRects_r[all_counted_r].Height = per_height;
				if(all_counted_r == 0)
					break;
			}
			if(last_timestamp & 0x1)
				pRects_hilite[i].X = (i-1)*(per_width+margin_h);
			else
				pRects_hilite[i].X = i*(per_width+margin_h);
			pRects_hilite[i].Y = (pper_count[i]-1)*(per_height+margin_v) + (gdips_rect_client.Height>>1);
			pRects_hilite[i].Width = per_width;
			pRects_hilite[i].Height = per_height;
		}
		else
		{
			for(int j=0; j<pper_count[i]; j++)
			{
				--all_counted_l;
				if(last_timestamp & 0x1)
					pRects_l[all_counted_l].X = (i+1)*(per_width+margin_h);
				else
					pRects_l[all_counted_l].X = i*(per_width+margin_h);
				pRects_l[all_counted_l].Y = (gdips_rect_client.Height>>1) - (per_height*(j+1) + margin_v*j);//j*(per_height+margin_v);
				pRects_l[all_counted_l].Width = per_width;
				pRects_l[all_counted_l].Height = per_height;
				if(all_counted_l == 0)
					break;
			};
			if(last_timestamp & 0x1)
				pRects_hilite[i].X = (i+1)*(per_width+margin_h);
			else
				pRects_hilite[i].X = i*(per_width+margin_h);
			pRects_hilite[i].Y = (gdips_rect_client.Height>>1) - ((pper_count[i]-1)*(per_height+margin_v)+per_height);
			pRects_hilite[i].Width = per_width;
			pRects_hilite[i].Height = per_height;
		}
	}

	gdips_gps_mem.Clear(*xme_helper.gdips_color_bg);

	if(xme_helper.bFreqDoExchange == FALSE)
		goto label_DoNotExchange;
	if( pLevels->timeStamp & 0x1)
	{
		gdips_gps_mem.FillRectangles(xme_helper.gdips_sbrush_freq_r, pRects_l, all_counts_l);
		gdips_gps_mem.FillRectangles(xme_helper.gdips_sbrush_freq_l, pRects_r, all_counts_r);
	}
	else
label_DoNotExchange:
	{
		gdips_gps_mem.FillRectangles(xme_helper.gdips_sbrush_freq_l, pRects_l, all_counts_l);
		gdips_gps_mem.FillRectangles(xme_helper.gdips_sbrush_freq_r, pRects_r, all_counts_r);
	}
	gdips_gps_mem.FillRectangles(xme_helper.gdips_sbrush_freq_hilite, pRects_hilite, count);

	delete[all_counts_l] pRects_l;
	delete[all_counts_r] pRects_r;
	delete[count] pRects_hilite;
	delete[count] pper_count;

	Gdiplus::CachedBitmap gdips_bmp_cached(&gdips_bmp_mem, &gdips_gps_scr);
	gdips_gps_scr.DrawCachedBitmap(&gdips_bmp_cached, prc->left, prc->top);

	return S_OK;
}

STDMETHODIMP CXpeulMediaEffect::InternalError(HDC hdc, RECT* prc)
{
	Gdiplus::Graphics gdips_gps_scr(hdc);
	gdips_gps_scr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	gdips_gps_scr.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
	
	Gdiplus::Font gdips_font(L"Î¢ÈíÑÅºÚ", 20, Gdiplus::FontStyleRegular, Gdiplus::UnitWorld);
	Gdiplus::StringFormat gdips_strfmt;
	gdips_strfmt.SetAlignment(Gdiplus::StringAlignmentFar);

	if(xme_helper.gdips_color_bg)
		gdips_gps_scr.Clear(*xme_helper.gdips_color_bg);
	else
		gdips_gps_scr.Clear(Gdiplus::Color(255, 0, 0, 0));

	Gdiplus::SolidBrush gdips_brush_red(Gdiplus::Color(255,255,0,0));
	wchar_t buffer[MAX_PATH];
	LoadString(g_data.GetInstance(), IDS_EFFECT_INTERNAL_ERROR, buffer, MAX_PATH);
	gdips_gps_scr.DrawString(buffer, -1, &gdips_font, Gdiplus::RectF(prc->left,prc->top, prc->right, 60), &gdips_strfmt, &gdips_brush_red);
	
	return S_OK;
}


void CXpeulMediaEffect::FillRegistry(void)
{
	BROWSEINFOW info;
	memset(&info, 0, sizeof(BROWSEINFOW));
	wchar_t buffer[MAX_PATH];
	LoadString(g_data.GetInstance(), IDS_REGSVR_LRCFLDR_INFO, buffer, MAX_PATH);
	info.lpszTitle=buffer;
	wchar_t path[MAX_PATH];
	info.ulFlags=BIF_NEWDIALOGSTYLE|BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST p=::SHBrowseForFolderW(&info);
	wmemset(path, 0, MAX_PATH);
	::SHGetPathFromIDListW(p, path);
	CoTaskMemFree(p);

	if(path[0]==0){
		wchar_t info[MAX_PATH];
		LoadString(g_data.GetInstance(), IDS_REGSVR_LRCFLDR_DEFAULT, info, MAX_PATH);
		wchar_t title[MAX_PATH];
		LoadString(g_data.GetInstance(), IDS_EFFECT_NAME, title, MAX_PATH);

		MessageBoxW(NULL, info, title, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	int l=wcslen(path);
	if(path[l-1]!=L'\\'){
		path[l]=L'\\';
		path[++l]=L'*';
	}else{
		path[l]=L'*';
	}

	HKEY hKey=NULL;
	RegOpenKeyW(HKEY_CURRENT_USER, L"Software\\XpeulEnterprise\\XpeulMedia\\XpeulLyricsShow", &hKey);
	if(hKey){
		RegSetValueW(hKey, NULL, REG_SZ, path, l);
		RegCloseKey(hKey);
	}
}
