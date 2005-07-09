// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <cassert>

#define ASSERT assert
#define interface struct

#include <IConsole.h>
extern IConsole *g_pConsole;

#ifdef _DEBUG
#	include <Dxerr8.h>
#	pragma comment(lib, "Dxerr8.lib")

#	define CONSOLE_DEBUG if(g_pConsole) g_pConsole->printf
#	define CONSOLE_DEBUG1 __noop
#	define CONSOLE_DEBUG2 __noop
#	define CONSOLE_DEBUG3 __noop
#	define CONSOLE_DEBUG4 __noop

#	define CONSOLE_PRINTF if(g_pConsole) g_pConsole->printf
#	define CONSOLE_LOG if(g_pConsole) g_pConsole->printf
#	define D3DVERIFY(expr) { HRESULT hr = (expr); if(FAILED(hr)) CONSOLE_DEBUG("DEBUG ERROR (D3D8, %s:%d): HRESULT = %s\n", __FILE__, __LINE__, DXGetErrorString8(hr)); }
#else
#	define CONSOLE_DEBUG __noop
#	define CONSOLE_DEBUG1 __noop
#	define CONSOLE_DEBUG2 __noop
#	define CONSOLE_DEBUG3 __noop
#	define CONSOLE_DEBUG4 __noop
#	define CONSOLE_PRINTF if(g_pConsole) g_pConsole->printf
#	define CONSOLE_LOG if(g_pConsole) g_pConsole->fprintf
#	define D3DVERIFY(expr) (expr)
#endif
