#include "stdafx.h"
#include "xm_lyrics_parse.h"

#include <vector>
#include "_xm_helpers.h"

tagLyrics::tagLyrics(void)
{
	InitializeCriticalSectionAndSpinCount(&m_cs, 1);
	m_ParsingState=LyricsPS_ParsedNoLyrics;
	str_tag_title=new WSTR();
	str_tag_artist=new WSTR();
	str_tag_album=new WSTR();
	str_tag_by=new WSTR();
	n_tag_offset=0;
	map_N2Lyrics=new MAP_N2WSTR();
}

tagLyrics::~tagLyrics(void)
{
	if(str_tag_title!=NULL){ delete str_tag_title; str_tag_title=NULL;}
	if(str_tag_artist!=NULL){ delete str_tag_artist; str_tag_artist=NULL;}
	if(str_tag_album!=NULL){ delete str_tag_album;	str_tag_album=NULL;}
	if(str_tag_by!=NULL){ delete str_tag_by; str_tag_by=NULL;}
	if(map_N2Lyrics!=NULL){ delete map_N2Lyrics; map_N2Lyrics=NULL;}
	::DeleteCriticalSection(&m_cs);
}

MAP_N2WSTR::iterator tagLyrics::GetCurrentLyrics(const __int64 timestamp) const
{
	MAP_N2WSTR::iterator _ret;
	if(map_N2Lyrics){
		_ret=map_N2Lyrics->upper_bound(timestamp);
		--_ret;
	}
	return _ret;
}

void tagLyrics::SetPS_ParsedGetLyrics(void)
{
	EnterCriticalSection(&m_cs);
	m_ParsingState = Lyrics::LyricsPS_ParsedGetLyrics;
	LeaveCriticalSection(&m_cs);
}

void tagLyrics::SetPS_ParsedNoLyrics(void)
{
	EnterCriticalSection(&m_cs);
	m_ParsingState = Lyrics::LyricsPS_ParsedNoLyrics;
	LeaveCriticalSection(&m_cs);
}

void tagLyrics::SetPS_ParsingFromEmbed(void)
{
	EnterCriticalSection(&m_cs);
	m_ParsingState = Lyrics::LyricsPS_ParsingFromEmbed;
	LeaveCriticalSection(&m_cs);
}

void tagLyrics::SetPS_ParsingFromLRCFfile(void){
	EnterCriticalSection(&m_cs);
	m_ParsingState = Lyrics::LyricsPS_ParsingFromLRCFfile;
	LeaveCriticalSection(&m_cs);
}

const tagLyrics::LyricsParsingState& tagLyrics::GetParsingState(void) const
{
	__try{ 
		EnterCriticalSection((LPCRITICAL_SECTION)&m_cs);
		return m_ParsingState;
	}
	__finally
	{
		LeaveCriticalSection((LPCRITICAL_SECTION)&m_cs);
	}
}

#include <wmsdk.h>

#pragma comment(lib, "wmvcore.lib")

DWORD __stdcall ParseLyrics(Lyrics* pLyricsDst, BSTR LyricsSrc)
{
//find first L"["
	std::wstring* pwstrLyrics = new std::wstring(LyricsSrc);
	unsigned int ntag_begin_pos=pwstrLyrics->find_first_of(L"[");
	if(ntag_begin_pos==std::wstring::npos){
		pLyricsDst->SetPS_ParsedNoLyrics();
		return -1;
	}
//start parsing
	std::wstring::iterator seekptr=pwstrLyrics->begin();
	std::wstring::iterator endptr=pwstrLyrics->end();
	for(seekptr+=(ntag_begin_pos+1); seekptr!=endptr; seekptr++)
	{
		//All text tags mustn't contain "[" and/or "]" now,
		//This restriction will be hacked in future.
		if(::towlower(*seekptr)==L'a')
		{
			++seekptr;
			if(::towlower(*seekptr)==L'l')
			{
				++seekptr;
				if(::towlower(*seekptr)==L':')
				{
					++seekptr;
					while(*seekptr!=L']')
					{
						(pLyricsDst->str_tag_album)->operator+=(*seekptr++);
					}
					++seekptr;
				}
			}
			if(::towlower(*seekptr)==L'r')
			{
				++seekptr;
				if(::towlower(*seekptr)==L':')
				{
					++seekptr;
					while(*seekptr!=L']')
					{
						(pLyricsDst->str_tag_artist)->operator+=(*seekptr++);
					}
					++seekptr;
				}
			}
		}
		if(::towlower(*seekptr)==L'b')
		{
			++seekptr;
			if(::towlower(*seekptr)==L'y')
			{
				++seekptr;
				if(::towlower(*seekptr)==L':')
				{
					++seekptr;
					while(*seekptr!=L']')
					{
						(pLyricsDst->str_tag_by)->operator+=(*seekptr++);
					}
					++seekptr;
				}
			}
		}
		if(::towlower(*seekptr)==L't')
		{	
			++seekptr;
			if(::towlower(*seekptr)==L'i')
			{
				++seekptr;
				if(::towlower(*seekptr)==L':')
				{
					++seekptr;
					while(*seekptr!=L']')
					{
						(pLyricsDst->str_tag_title)->operator+=(*seekptr++);
					}
					++seekptr;
				}
			}
		}
		if(::towlower(*seekptr)==L'o')
		{
			++seekptr;
			if(::towlower(*seekptr)==L'f')
			{
				++seekptr;
				if(::towlower(*seekptr)==L'f')
				{
					++seekptr;
					if(::towlower(*seekptr)==L's')
					{
						++seekptr;
						if(::towlower(*seekptr)==L'e')
						{
							++seekptr;
							if(::towlower(*seekptr)==L't')
							{
								++seekptr;
								if(::towlower(*seekptr)==L':')
								{
									++seekptr;
									WSTR str_tag_offset;
									while(*seekptr!=L']')
									{
										str_tag_offset+=*seekptr++;
									}
									++seekptr;
									double s=::_wtof(str_tag_offset.c_str());
									pLyricsDst->n_tag_offset=(__int64)(s*10000);
								}
							}
						}
					}
				}
			}
		}
		if(::iswdigit(*seekptr))
		{
			typedef std::vector<__int64> Vec64;
			Vec64* pvectimes=new Vec64();
			WSTR str_lyrics;
			do{
				if(*seekptr==L'[') seekptr++;
				WSTR str_tag_time_mm;
				while(*seekptr!=L':')
				{
					str_tag_time_mm+=*seekptr++;
				}
				++seekptr;
				WSTR str_tag_time_ss;
				while(*seekptr!=L']')
				{
					str_tag_time_ss+=*seekptr++;
				}
				__int64 nTime=::_wtoi64(str_tag_time_mm.c_str())*60*10000000
					+(__int64)(::_wtof(str_tag_time_ss.c_str())*10000000)+pLyricsDst->n_tag_offset;
				int n=pvectimes->size();
				pvectimes->resize(n+1);
				pvectimes->operator[](n)=nTime;
				if((++seekptr)==endptr){
					--seekptr;
					str_lyrics=L"\0";
					goto label_MAKE_LYRICS;
				}
			}while(*seekptr==L'[');
			while(*seekptr!=L'[')
			{
				str_lyrics+=*seekptr++;
				if(seekptr==endptr){
					--seekptr;
					goto label_MAKE_LYRICS;
				}
			}
label_MAKE_LYRICS:
			for(Vec64::iterator vecptr=pvectimes->begin(); vecptr!=pvectimes->end(); vecptr++)
			{
				MAP_N2WSTR_VAL* plyricsval=new MAP_N2WSTR_VAL(*vecptr, str_lyrics);
				pLyricsDst->map_N2Lyrics->insert(*plyricsval);
				pLyricsDst->map_N2Lyrics->insert(MAP_N2WSTR_VAL(0, WSTR(L"XpeulMedia Lyrics Show")));
				delete plyricsval;
			}
			delete pvectimes;
		}
	}
	delete pwstrLyrics;
	pwstrLyrics = NULL;

	if(pLyricsDst->map_N2Lyrics->size()==0)
	{
		pLyricsDst->SetPS_ParsedNoLyrics();
		return -1;
	}	
	return 0;
}

DWORD __stdcall __GetLyricsEmbed_by_XXX(void* pv)
{
	D4GL* pd4gl=reinterpret_cast<D4GL*>(pv);
	pd4gl->pLyrics->SetPS_ParsingFromEmbed();
	HANDLE hSong=::CreateFile(pd4gl->path, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hSong==INVALID_HANDLE_VALUE){
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -3;
	}
	DWORD size=0;
	size=GetFileSize(hSong, NULL);
	DWORD theptr=NULL;
	theptr=SetFilePointer(hSong, size-10240, NULL, FILE_BEGIN);
	DWORD lp_end=0;
	DWORD lp_start=0;
#pragma region "Find Lyrics Pos"
//LYRICSBEGININD00003110LYR01076
	char c='0';
	char d[14]={'\0'};
label_FIND_START_L:
	while((toupper(c)!='L')&&(theptr<size))
	{
		ReadFile(hSong, (void*)&c, 1, &lp_start, NULL);
		++theptr;
	}
	ReadFile(hSong, (void*)&d, 13, &lp_start, NULL);
	theptr+=13;
	if(theptr>=size){
		CloseHandle(hSong);
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -1;
	}
	for(int i=0;i<14;i++)
		d[i]=toupper(d[i]);
	if(strcmp(d, "YRICSBEGININD")!=0){
		ReadFile(hSong, (void*)&c, 1, &lp_start, NULL);
		++theptr;
		goto label_FIND_START_L;
	}
	do{
		ReadFile(hSong, (void*)&c, 1, &lp_start, NULL);
		++theptr;
	}while((toupper(c)!='[')&&(theptr<size));
	lp_start=theptr-1;
	if(theptr>=size){
		CloseHandle(hSong);
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -1;
	}
//001106LYRICS200TAG
label_FIND_END_L:
	char e[12]={'\0'};
	while((toupper(c)!='L')&&(theptr<size))
	{
		ReadFile(hSong, (void*)&c, 1, &lp_end, NULL);
		++theptr;
	}
	ReadFile(hSong, (void*)&e, 11, &lp_end, NULL);
	theptr+=11;
	if(theptr>=size){
		CloseHandle(hSong);
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -1;
	}
	for(int i=0;i<12;i++)
		e[i]=toupper(e[i]);
	if(strcmp(e, "YRICS200TAG")!=0){
		ReadFile(hSong, (void*)&c, 1, &lp_end, NULL);
		++theptr;
		goto label_FIND_END_L;
	}
	lp_end=theptr-18;
#pragma endregion
	int datasize = lp_end-lp_start;
	char* pdata_raw = new char[datasize+2];
	memset(pdata_raw, 0,datasize+2);
	theptr=SetFilePointer(hSong, lp_start, NULL, FILE_BEGIN);
	DWORD stub=0;
	ReadFile(hSong, (void*)pdata_raw, datasize, &stub, NULL);
	CloseHandle(hSong);
	INT mask=IS_TEXT_UNICODE_NOT_ASCII_MASK;
	BOOL not_unicode=IsTextUnicode(pdata_raw, size+2, &mask);
	DWORD dw_ret=0;
	if(not_unicode==0){
		BSTR pLyrics_raw=A2WBSTR((LPCSTR)pdata_raw);
		dw_ret=ParseLyrics(pd4gl->pLyrics, (BSTR)pLyrics_raw);
	}else
		dw_ret=ParseLyrics(pd4gl->pLyrics, (BSTR)pdata_raw);
	pd4gl->pLyrics->SetPS_ParsedGetLyrics();
	delete[datasize+1] pdata_raw;
	return dw_ret;
}

DWORD __stdcall __GetLyricsEmbed_by_WMP(void* pv)
{
	D4GL* pd4gl=reinterpret_cast<D4GL*>(pv);
	pd4gl->pLyrics->SetPS_ParsingFromEmbed();

	IWMMetadataEditor* pMDE=NULL;
	HRESULT rst=WMCreateEditor(&pMDE);
	if(FAILED(rst)){
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -3;
	}
	pMDE->Open(pd4gl->path);
	if(FAILED(rst)){
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -1;
	}
	IWMHeaderInfo* pHI=NULL;
	rst=pMDE->QueryInterface(__uuidof(IWMHeaderInfo), (void**)&pHI);
	if(FAILED(rst)){
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -3;
	}
	WORD nStream=0;
	WMT_ATTR_DATATYPE data_type;
	WORD nLength=0;
	rst=pHI->GetAttributeByName(&nStream, L"WM/Lyrics", &data_type, NULL, &nLength);
	if(FAILED(rst)){
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -2;
	}
	BYTE* pdata_raw=new BYTE[nLength+2];
	memset(pdata_raw, 0, nLength+2);
	rst=pHI->GetAttributeByName(&nStream, L"WM/Lyrics", &data_type, pdata_raw, &nLength);
	if(FAILED(rst)){
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -2;
	}
	pMDE->Close();
	INT mask=IS_TEXT_UNICODE_NOT_ASCII_MASK;
	BOOL not_unicode=IsTextUnicode(pdata_raw, nLength+2, &mask);
	DWORD dw_ret=0;
	if(not_unicode==0){
		BSTR pLyrics_raw=A2WBSTR((LPCSTR)pdata_raw);
		dw_ret=ParseLyrics(pd4gl->pLyrics, pLyrics_raw);
	}else
		dw_ret=ParseLyrics(pd4gl->pLyrics, (BSTR)pdata_raw);
	pd4gl->pLyrics->SetPS_ParsedGetLyrics();
	delete[nLength+2] pdata_raw;
	return dw_ret;
}

DWORD __stdcall _GetLyricsEmbed(void* pv)
{
	DWORD dw_ret=__GetLyricsEmbed_by_WMP(pv);

	if(dw_ret==0) return dw_ret;
	
	return __GetLyricsEmbed_by_XXX(pv);
}

DWORD __stdcall _GetLyricsFromLRCFile(void* pv)
{
	D4GL* pd4gl=reinterpret_cast<D4GL*>(pv);
	pd4gl->pLyrics->SetPS_ParsingFromLRCFfile();

	WIN32_FIND_DATAW find_data={0};
	HANDLE hNext=FindFirstFile(pd4gl->path, &find_data);
	if(hNext==INVALID_HANDLE_VALUE){
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		FindClose(hNext);
		::SysFreeString(pd4gl->path);
		return -3;
	}
	FindClose(hNext);
	UINT L=::SysStringLen(pd4gl->path);
	UINT l=wcslen(find_data.cFileName);
	wchar_t* file = new wchar_t[l+L+1];
	wmemset(file, 0, l+L+1);
	wmemcpy(file, pd4gl->path, L);
	::SysFreeString(pd4gl->path);
	::PathRemoveFileSpecW(file);
	::PathAppendW(file, find_data.cFileName);

	HANDLE hFile=::CreateFileW(file, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	delete file;
	if(hFile==INVALID_HANDLE_VALUE){
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -3;
	}
	DWORD size=GetFileSize(hFile, NULL);
	if(size==0){
		pd4gl->pLyrics->SetPS_ParsedNoLyrics();
		return -4;
	}
	char* pdata_raw=new char[size+2];
	memset(pdata_raw, 0, size+2);
	DWORD pdw=0;
	ReadFile(hFile, pdata_raw, size, &pdw, NULL);
	CloseHandle(hFile);
	INT mask=IS_TEXT_UNICODE_NOT_ASCII_MASK;
	BOOL not_unicode=IsTextUnicode(pdata_raw, size+2, &mask);
	DWORD dw_ret=0;
	if(not_unicode==0){
		BSTR Lyrics_raw=A2WBSTR(pdata_raw);
		dw_ret=ParseLyrics(pd4gl->pLyrics, Lyrics_raw);
	}else
		dw_ret=	dw_ret=ParseLyrics(pd4gl->pLyrics, (BSTR)pdata_raw);
	pd4gl->pLyrics->SetPS_ParsedGetLyrics();
	delete[size+2] pdata_raw;
	return dw_ret;
}
