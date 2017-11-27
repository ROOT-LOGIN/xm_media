#pragma once

#ifndef __INCLUDE_XM_LOG_H__
#define __INCLUDE_XM_LOG_H__

extern const char* const str_def_logfile;

void __stdcall XmPrepareLog(
	HANDLE* phFile = NULL,
	LPCRITICAL_SECTION lpCs = NULL
);

void __stdcall XmWriteLog(
	const char* Event,
	const DWORD cbEvent, 
	const char* Discription,
	const DWORD cbDiscription,
	const HANDLE* phFile = NULL,
	LPCRITICAL_SECTION lpCs = NULL
);

void __stdcall XmFinishLog(
	HANDLE* phFile = NULL,
	LPCRITICAL_SECTION lpCs = NULL
);

#endif
