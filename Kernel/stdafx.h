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

#include "../Misc.h"
#include "../IConsole.h"
extern IConsole *g_pConsole;

#ifdef _DEBUG
#define CONSOLE_DEBUG if(g_pConsole) g_pConsole->printf
#define CONSOLE_OUTPUT if(g_pConsole) g_pConsole->printf
#else
#define CONSOLE_DEBUG __noop
#define CONSOLE_OUTPUT if(g_pConsole) g_pConsole->printf
#endif
