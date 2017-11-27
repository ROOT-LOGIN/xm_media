#pragma once

#ifndef __INCLUDE_XM_LYRICS_PARSE_H__
#define __INCLUDE_XM_LYRICS_PARSE_H__

#include <map>

typedef std::wstring WSTR;
typedef std::map<__int64, WSTR> MAP_N2WSTR;
typedef MAP_N2WSTR::value_type MAP_N2WSTR_VAL;

typedef struct tagLyrics{
	WSTR* str_tag_title;
	WSTR* str_tag_artist;
	WSTR* str_tag_album;
	WSTR* str_tag_by;
	__int64 n_tag_offset;
	MAP_N2WSTR* map_N2Lyrics;

	MAP_N2WSTR::iterator GetCurrentLyrics(const __int64 timestamp) const;

	enum LyricsParsingState{
		LyricsPS_ParsedGetLyrics=0,
		LyricsPS_ParsedNoLyrics,
		LyricsPS_ParsingFromEmbed,
		LyricsPS_ParsingFromLRCFfile //LRCF means .LRC Format
	};
	
	void SetPS_ParsedGetLyrics(void);
	void SetPS_ParsedNoLyrics(void);
	void SetPS_ParsingFromEmbed(void);
	void SetPS_ParsingFromLRCFfile(void);

	const LyricsParsingState& GetParsingState(void) const;

	tagLyrics(void);
	~tagLyrics(void);	

private:
	CRITICAL_SECTION m_cs;
	LyricsParsingState m_ParsingState;
} Lyrics;

DWORD __stdcall ParseLyrics(
	Lyrics* pLyricsDst,
	BSTR LyricsSrc
);

typedef struct DataForGetLyrics
{
	Lyrics* pLyrics;
	BSTR path;
} D4GL;

DWORD __stdcall _GetLyricsEmbed(
	void* pv
);

DWORD __stdcall _GetLyricsFromLRCFile(
	void* pv
);

#endif