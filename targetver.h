#pragma once

#ifndef WINVER                      
#define WINVER 0x0600          
#endif

#ifndef _WIN32_WINNT           
#define _WIN32_WINNT 0x0600
#endif

#ifndef _WIN32_WINDOWS     
#define _WIN32_WINDOWS 0x0600
#endif

#ifndef _WIN32_IE                   
#define _WIN32_IE 0x0700       
#endif

//Major, Minor, Function-change[add(2)del(2)], Bug-fix|Function-fixed
#define XM_VERSION			2,0,1204,2011 
#define XM_VERSION_STR	"2.0.1204.2011"

#pragma warning(disable:4208)
#pragma warning(disable:4244)
#pragma warning(disable:4245)
#pragma warning(disable:4482)
