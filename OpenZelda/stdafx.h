// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <cassert>
#include <windowsx.h>

#define ASSERT assert
#define interface struct

#include "../Misc.h"
#include "../IConsole.h"
extern IConsole *g_pConsole;

#ifdef _DEBUG
#	define CONSOLE_DEBUG if(g_pConsole) g_pConsole->printf
#	define CONSOLE_PRINTF if(g_pConsole) g_pConsole->printf
#	define CONSOLE_LOG if(g_pConsole) g_pConsole->printf
#	define CONSOLE_PUTCHAR(c) (g_pConsole?g_pConsole->putch(c):0)
#	define CONSOLE_GETCHAR(c) (g_pConsole?g_pConsole->getch(c):0)
#	define CONSOLE_GETS(s,n) (g_pConsole?g_pConsole->gets(s,n):0)
#else
#	define CONSOLE_DEBUG __noop
#	define CONSOLE_PRINTF if(g_pConsole) g_pConsole->printf
#	define CONSOLE_LOG if(g_pConsole) g_pConsole->printf
#	define CONSOLE_PUTCHAR(c) (g_pConsole?g_pConsole->putch(c):0)
#	define CONSOLE_GETCHAR(c) (g_pConsole?g_pConsole->getch(c):0)
#	define CONSOLE_GETS(s,n) (g_pConsole?g_pConsole->gets(s,n):0)
#endif
