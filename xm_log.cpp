#include "stdafx.h"
#include "xm_log.h"
#include "_xm_helpers.h"

const char* const str_def_logfile = "C:\\Program Files\\Windows Media Player\\XpeulMedia.log.xml";
CRITICAL_SECTION g_cs_log;
HANDLE g_hfile_log=NULL;

#define IsValidFileHandle(h) (h!=NULL)&&(h!=INVALID_HANDLE_VALUE)?true:false

bool __stdcall __XmMakeValidLogHandle(HANDLE* &ph)
{
	bool b=IsValidFileHandle(g_hfile_log);
	if(ph)
	{
		if(!IsValidFileHandle(*ph))
		{
			if(b)
				ph=&g_hfile_log;
			else
				return false;
		} 
	}
	else
	{
		if(b)
			ph=&g_hfile_log;
		else
			return false;
	}
	return true;
}

#define NORMALIZE_TIME_STRING(data, _str_buf_, index) \
	if(data<10){ \
		_str_buf_[index+1]=_str_buf_[index]; \
		_str_buf_[index]='0'; \
	}		

void __stdcall XmPrepareLog(HANDLE* phFile, LPCRITICAL_SECTION lpCs)
{
	if(g_data.GetWriteLog()==false)
		return;

	if(::__XmMakeValidLogHandle(phFile))
		XmFinishLog(phFile, lpCs);

	if(phFile==NULL)
		phFile=&g_hfile_log;
	else
	{
		if(*phFile==INVALID_HANDLE_VALUE)
			phFile=&g_hfile_log;
		else
			CloseHandle(*phFile);
	}
	if(lpCs==NULL)
		lpCs=&g_cs_log;

	::InitializeCriticalSectionEx(lpCs, 60, 
#if defined(DEBUG)||defined(_DEBUG)
	0);
#else
	CRITICAL_SECTION_NO_DEBUG_INFO);
#endif

	HKEY hKey=NULL;
	RegOpenKeyA(HKEY_CURRENT_USER, "Software\\XpeulEnterprise\\XpeulMedia\\XpeulMediaBack", &hKey);
	char buffer[MAX_PATH];
	const char* logfile=NULL;
	DWORD dw=MAX_PATH;
	if(hKey)
	{
		memset(buffer, 0, MAX_PATH);
		::RegGetValueA(hKey, NULL, NULL, RRF_RT_REG_SZ, NULL, buffer, &dw);
		if(dw)
			logfile=buffer;
		RegCloseKey(hKey);
	}else
		logfile=str_def_logfile;

	if(!TryEnterCriticalSection(lpCs))
		return;

	bool bReCreated=false;
	*phFile=CreateFileA(logfile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if(*phFile==NULL||*phFile==INVALID_HANDLE_VALUE)
	{
label_CreateFile_Always:
		*phFile=CreateFileA(logfile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
		if(*phFile==NULL||*phFile==INVALID_HANDLE_VALUE)
			return;
		else{
			WriteFile(*phFile, "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n", 
				sizeof("<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n")-1, &dw, NULL);
			WriteFile(*phFile, "<!--this is log file for XpeulMedia Wmp Plug-in, Do Not Modify it please!-->",
				sizeof("<!--this is log file for XpeulMedia Wmp Plug-in, Do Not Modify it please!-->")-1, &dw, NULL);
			WriteFile(*phFile, "<XmLogRoot Application=\"XpeulMedia\" Version=\"1.0.0.0\">\n",
				sizeof("<XmLogRoot Application=\"XpeulMedia\" Version=\"1.0.0.0\">\n")-1, &dw, NULL);
		}
	}
	else
	{
		char buf[sizeof("</XmLogRoot>")];
		memset(buf, 0, sizeof("</XmLogRoot>"));
		SetFilePointer(*phFile, 0, NULL, FILE_END);
		do
		{
			SetFilePointer(*phFile, -1, NULL, FILE_CURRENT);
			ReadFile(*phFile, &buf, 1, &dw, NULL);
			SetFilePointer(*phFile, -1, NULL, FILE_CURRENT);		
		}while(*buf!='>');
		SetFilePointer(*phFile, 2-sizeof("</XmLogRoot>"), NULL, FILE_CURRENT);
		ReadFile(*phFile, buf, sizeof("</XmLogRoot>")-1, &dw, NULL);
		if(strcmp(buf, "</XmLogRoot>")==0)
		{
			SetFilePointer(*phFile, 1-sizeof("</XmLogRoot>"), NULL, FILE_CURRENT);
		}
		else
		{
			CloseHandle(*phFile);
			DeleteFileA(logfile);
			bReCreated=true;
			goto label_CreateFile_Always;
		}

	}
	SYSTEMTIME ltm={0};
	GetLocalTime(&ltm);
	
	char entry_buf[]={'<', 'X', 'm', 'L', 'o', 'g', 'E', 'n', 't', 'r', 'y', ' ',
		'Y', 'e', 'a', 'r', '=', '\"', '1', '2', '3', '4', '\"', ' ',
		'D', 'a', 't', 'e', '=', '\"', '1', '2', '-', '3', '4', '\"', ' ',
		'I', 'n', 'i', 't', 'T', 'i', 'm', 'e', '=', '\"', '1', '2', ':', '3', '4', ':', '5', '6', '\"',
		'>', '\n'
	};
	_itoa(ltm.wYear, &entry_buf[18], 10);
	entry_buf[22]='\"';
	_itoa(ltm.wMonth, &entry_buf[30], 10);
	NORMALIZE_TIME_STRING(ltm.wMonth, entry_buf, 30)
	entry_buf[32]='-';
	_itoa(ltm.wDay, &entry_buf[33], 10);
	NORMALIZE_TIME_STRING(ltm.wDay, entry_buf, 33)
	entry_buf[35]='\"';
	_itoa(ltm.wHour, &entry_buf[47], 10);
	NORMALIZE_TIME_STRING(ltm.wHour, entry_buf, 47)
	entry_buf[49]=':';
	_itoa(ltm.wMinute, &entry_buf[50], 10);
	NORMALIZE_TIME_STRING(ltm.wMinute, entry_buf, 50)
	entry_buf[52]=':';
	_itoa(ltm.wSecond, &entry_buf[53], 10);
	NORMALIZE_TIME_STRING(ltm.wSecond, entry_buf, 53)
	entry_buf[55]='\"';

	WriteFile(*phFile, entry_buf,
		sizeof("<XmLogEntry Year=\"\" Date=\"\" InitTime=\"\">\n")+16, &dw, NULL);
	::FlushFileBuffers(*phFile);

	::LeaveCriticalSection(lpCs);
	if(bReCreated){
		::XmWriteLog("Log Update", sizeof("Log Update")-1,
			"The previous log file has Been Deleted due to its incorrect format",
				sizeof("The previous log file has Been Deleted due to its incorrect format")-1, phFile, lpCs);
	}
}

char record_buf[]={'<', 'X', 'm', 'L', 'o', 'g', 'R', 'e', 'c', 'o', 'r', 'd', ' ',
		'T', 'i', 'm', 'e', '=', '\"', '1', '2', ':', '3', '4', ':', '5', '6', '\"', ' ',
		'E', 'v', 'e', 'n', 't', '=', '\"'
	};

static const char* const str_logrec_dis_begin="\">\n<XmLogEventDiscription>";
static const char* const str_logrec_dis_end="</XmLogEventDiscription>\n</XmLogRecord>\n";

void __stdcall XmWriteLog(const char* Event, const DWORD cbEvent, 
				const char* Discription, const DWORD cbDiscription, 
				const HANDLE* phFile, LPCRITICAL_SECTION lpCs)
{
	if(g_data.GetWriteLog()==false)
		return;

	if(!::__XmMakeValidLogHandle((HANDLE*&)phFile))
		return;
	if(lpCs==NULL)
		lpCs=&g_cs_log;

	if(::TryEnterCriticalSection(lpCs))
	{
		SYSTEMTIME ltm={0};
		GetLocalTime(&ltm);
		_itoa(ltm.wHour, &record_buf[19], 10);
		NORMALIZE_TIME_STRING(ltm.wHour, record_buf, 19)
		record_buf[21]=':';
		_itoa(ltm.wMinute, &record_buf[22], 10);
		NORMALIZE_TIME_STRING(ltm.wMinute, record_buf, 22)
		record_buf[24]=':';
		_itoa(ltm.wSecond, &record_buf[25], 10);
		NORMALIZE_TIME_STRING(ltm.wSecond, record_buf, 25)
		record_buf[27]='\"';
		DWORD dw=0;
		WriteFile(*phFile, record_buf, sizeof("<XmLogRecord Time=\"::\" Event=\"")+5, &dw, NULL);
		if(Event&&cbEvent)
			WriteFile(*phFile, Event, cbEvent, &dw, NULL);
		WriteFile(*phFile, str_logrec_dis_begin, sizeof("\">\n<XmLogEventDiscription>")-1, &dw, NULL);
		if(Event&&cbEvent)
			WriteFile(*phFile, Discription, cbDiscription, &dw, NULL);
		WriteFile(*phFile,  str_logrec_dis_end, sizeof("</XmLogEventDiscription>\n</XmLogRecord>\n")-1, &dw, NULL);
		FlushFileBuffers(*phFile);
		::LeaveCriticalSection(lpCs);
	}
}

static const char* const str_log_end="</XmLogEntry>\n</XmLogRoot>";

void __stdcall XmFinishLog(HANDLE* phFile, LPCRITICAL_SECTION lpCs)
{
	if(g_data.GetWriteLog()==false)
		return;

	if(lpCs==NULL)
		lpCs=&g_cs_log;
	DeleteCriticalSection(lpCs);

	if(!::__XmMakeValidLogHandle(phFile))
		return;

	DWORD dw=0;
	WriteFile(*phFile, str_log_end, sizeof("</XmLogEntry>\n</XmLogRoot>")-1, &dw, NULL);
	//WriteFile(*phFile, "</XmLogEntry>\n", sizeof("</XmLogEntry>\n")-1, &dw, NULL);
	//WriteFile(*phFile, "</XmLogRoot>", sizeof("</XmLogRoot>")-1, &dw, NULL);
	FlushFileBuffers(*phFile);
	CloseHandle(*phFile);
	*phFile=NULL;
}
