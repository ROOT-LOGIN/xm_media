#pragma once

#ifndef __INCLUDE_XM_DLLMAIN_H__
#define __INCLUDE_XM_DLLMAIN_H__

class CXpeulMediaModule
	: public CAtlDllModuleT<CXpeulMediaModule>
{
public:
	CXpeulMediaModule(void);
	~CXpeulMediaModule(void);

};

extern CXpeulMediaModule _AtlModule;


#endif