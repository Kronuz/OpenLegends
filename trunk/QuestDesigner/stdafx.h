/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germán Méndez Bravo (Kronuz)
   Copyright (C) 2001-2003. Open Legends's Project
 
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/////////////////////////////////////////////////////////////////////////////
/*! \file		stdafx.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Include file for standard system include files,
				or project specific include files that are used frequently, but
				are changed infrequently
	\date		April 15, 2003
				July 20, 2003
*/

#include "../Version.h"

#define APPNAME _T(QD_NAME)
#define COMPANY _T(OL_NAME)

#pragma once

#pragma warning ( disable : 4530 )
#pragma warning ( disable : 4244 )

// Change these values to use different versions
#define WINVER			0x0500
#define _WIN32_WINNT	0x0400
#define _WIN32_IE		0x0500
#define _RICHEDIT_VER	0x0300

// This is required for hosting browser in ATL7
//#define _ATL_DLL

#include <atlbase.h>
#if _ATL_VER >= 0x0700
	#include <atlcoll.h>
	#include <atlstr.h>
	#include <atltypes.h>
	#define _WTL_NO_CSTRING
	#define _WTL_NO_WTYPES
	//extern "C" const int _fltused = 0;
#else
	#define _WTL_USE_CSTRING
#endif

#include <atlapp.h>

#include <HtmlHelp.h>
#pragma comment ( lib, "htmlhelp.lib" )

extern CAppModule _Module;

#include <atlimage.h>

#include <atlmisc.h>
#include <atlcom.h>
#include <atlhost.h>
#include <atlwin.h>
#include <atlctl.h>
#include <atlscrl.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>

#include <atlctrlx.h>
#include <atlgdix.h>

#include <CustomTabCtrl.h>
#include <DotNetTabCtrl.h>
#include <TabbedFrame.h>

#define _TABBEDMDI_MESSAGES_EXTERN_REGISTER
#define _TABBEDMDI_MESSAGES_NO_WARN_ATL_MIN_CRT
#include <TabbedMDI.h>

#define ASSERT ATLASSERT

//#define _NO_MISCSTRING
#define _NO_MISCTYPES

#include <Misc.h>
#include "Console.h"

#include "IdleHandlerPump.h"

#include "resource.h"

#include "ToolBarBox.h"
#include <WtlFileTreeCtrl.h>

#define WMQD_FIRST			(WM_USER+4000)

/*! Message sent to the window when a building step is about to begin. */
#define WMQD_STEPBEGIN		(WMQD_FIRST+1)
/*! Message sent to the window when a message is to be shown. */
#define WMQD_MESSAGE		(WMQD_FIRST+2)
/*! Message sent to the window when a building step hast just ended. */
#define WMQD_STEPEND		(WMQD_FIRST+3)
/*! Message sent to the window when the project building is about to begin. */
#define WMQD_BUILDBEGIN		(WMQD_FIRST+4)
/*! Message sent to the window when the project building has ended. */
#define WMQD_BUILDEND		(WMQD_FIRST+5)
/*! Message sent to the window when a file needs to be opened and line or object shown. */
#define WMQD_SELECT			(WMQD_FIRST+6)

#define WMGP_ADDTREE		(WMQD_FIRST+8)
#define WMGP_DELTREE		(WMQD_FIRST+9)

#define WMQ_ADDTREE			(WMQD_FIRST+10)
#define WMQ_DELTREE			(WMQD_FIRST+11)

#define WMQD_DRAGLEAVE		(WMQD_FIRST+12)
#define WMQD_DRAGOVER		(WMQD_FIRST+13)
#define WMQD_DRAGENTER		(WMQD_FIRST+14)

#define WMP_CLEAR			(WMQD_FIRST+15)
#define WMP_UPDATE			(WMQD_FIRST+16)
#define WMP_ADDINFO			(WMQD_FIRST+17)
#define WMP_SETPROP			(WMQD_FIRST+18)

/*! Message sent to a window view when an object has been dropped to it. */
#define WMQD_DROPOBJ		(WMQD_FIRST+20)

#define MakeIco_(normal, selected) (((normal)<<8) | (selected))
#define MakeIco(both) MakeIco_(both, both)

#define II_DOC_UNKNOWN		IDI_DOC_UNKNOWN - IDI_BEGIN
#define II_DOC_SCRIPT		IDI_DOC_SCRIPT - IDI_BEGIN
#define II_DOC_MAP			IDI_DOC_MAP - IDI_BEGIN
#define II_DOC_WORLD		IDI_DOC_WORLD - IDI_BEGIN
#define II_DOC_SPRITE		IDI_DOC_SPRITE - IDI_BEGIN
#define II_DOC_SPTSHT		IDI_DOC_SPTSHT - IDI_BEGIN
#define II_DOC_SPTSET		IDI_DOC_SPTSET - IDI_BEGIN
#define II_DOC_WAV			IDI_DOC_WAV - IDI_BEGIN
#define II_DOC_MIDI			IDI_DOC_MIDI - IDI_BEGIN
#define II_FOLDER_PROJECT	IDI_FOLDER_PROJECT - IDI_BEGIN
#define II_FOLDER_CLOSED	IDI_FOLDER_CLOSED - IDI_BEGIN
#define II_FOLDER_OPEN		IDI_FOLDER_OPEN - IDI_BEGIN
#define II_FOLDER_LIB		IDI_FOLDER_LIB - IDI_BEGIN
#define II_END				((IDI_END - IDI_BEGIN) + 1)

#define ICO_UNKNOWN			MakeIco(II_DOC_UNKNOWN)
#define ICO_SCRIPT			MakeIco(II_DOC_SCRIPT)
#define ICO_MAP				MakeIco(II_DOC_MAP)
#define ICO_WORLD			MakeIco(II_DOC_WORLD)
#define ICO_SPRITE			MakeIco(II_DOC_SPRITE)
#define ICO_SPTSHT			MakeIco(II_DOC_SPTSHT)
#define ICO_SPTSET			MakeIco(II_DOC_SPTSET)
#define ICO_WAV				MakeIco(II_DOC_WAV)
#define ICO_MIDI			MakeIco(II_DOC_MIDI)
#define ICO_PROJECT			MakeIco(II_FOLDER_PROJECT)
#define ICO_FOLDER			MakeIco_(II_FOLDER_CLOSED, II_FOLDER_OPEN)
#define ICO_LIBRARY			MakeIco(II_FOLDER_LIB)

#define SIMPLE_MESSAGE_HANDLER(msg, func) if(uMsg==msg) { bHandled=TRUE; lResult=func(wParam,lParam); if(bHandled) return TRUE; }
#define MENU_COMMAND_HANDLER(id, func) \
	if(uMsg == WM_COMMAND && id == LOWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = 0; \
		func(); \
		return TRUE; \
	}

void ShowHelp(HWND hWnd, LPCSTR szTopic = NULL);

// Load an image from a file in memory:
inline
bool LoadImage( CImage *pImage, LPCVOID pImageData, size_t ImageSize ) 
{
	bool ret = false;
	if(::IsBadReadPtr(pImageData, ImageSize) != 0) return false;

	HGLOBAL hBuffer  = ::GlobalAlloc(GMEM_MOVEABLE, ImageSize);
	if(!hBuffer) return false;

	void* pBuffer = ::GlobalLock(hBuffer);
	if (!pBuffer) {
		::GlobalFree(hBuffer);
		return false;
	}

	CopyMemory(pBuffer, pImageData, ImageSize);

	IStream* pStream = NULL;
	if (::CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) == S_OK) {
		ret = SUCCEEDED(pImage->Load(pStream));
		pStream->Release();
	}

	::GlobalUnlock(hBuffer);
	::GlobalFree(hBuffer);
	return ret;
}

// Load an image from a resource:
inline
bool LoadImage( CImage *pImage, HINSTANCE hInstance, UINT nIDResource ) 
{
	HRSRC hResource = ::FindResource(hInstance, MAKEINTRESOURCE(nIDResource), RT_BITMAP);
	// What we have here is a regular bitmap:
	if(hResource) { 
		pImage->LoadFromResource(hInstance, nIDResource);
		return true;
	}
	
	hResource = ::FindResource(hInstance, MAKEINTRESOURCE(nIDResource), _T("PNG"));
	if(!hResource) return false;

	DWORD imageSize = ::SizeofResource(hInstance, hResource);
	if(!imageSize) return false;

	const void* pResourceData = ::LockResource(::LoadResource(hInstance, hResource));
	if (!pResourceData) return false;

	return LoadImage(pImage, pResourceData, imageSize);
}


#ifdef _DEBUG
#	define CONSOLE_DEBUG CConsole::Instance()->printf
#	define CONSOLE_PRINTF CConsole::Instance()->printf
#	define CONSOLE_LOG CConsole::Instance()->printf
#	define CONSOLE_PUTCHAR(c) CConsole::Instance()->putch(c)
#	define CONSOLE_GETCHAR() CConsole::Instance()->getch()
#	define CONSOLE_GETS(s,n) CConsole::Instance()->gets(s,n)
#else
#	define CONSOLE_DEBUG __noop
#	define CONSOLE_PRINTF CConsole::Instance()->printf
#	define CONSOLE_LOG CConsole::Instance()->fprintf
#	define CONSOLE_PUTCHAR(c) CConsole::Instance()->putch(c)
#	define CONSOLE_GETCHAR() CConsole::Instance()->getch()
#	define CONSOLE_GETS(s,n) CConsole::Instance()->gets(s,n)
#endif

/////////////////////////////////////////////////////////////////////////////
// Bug fixes and updates needed:
#if (_WTL_VER == 0x0700)
#ifndef __MAXIMIZE_FIXUPDATE
	#error WTL BUGFIX: MDI windows need a maximization related bugfix to work (see here)
/*
	// This are the fixes by Daniel Bowen to the maximization and focus issues:
	// change in CMDIChildWindowImpl class (WTL7.0, atlframe.h)
	#define __MAXIMIZE_FIXUPDATE												// <<- Changed/Added
	HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			UINT nMenuID = 0, LPVOID lpCreateParam = NULL)
	{
		ATOM atom = T::GetWndClassInfo().Register(&m_pfnSuperWindowProc);

		if(nMenuID != 0)
			m_hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(nMenuID));

		dwStyle = T::GetWndStyle(dwStyle);
		dwExStyle = T::GetWndExStyle(dwExStyle);

		dwExStyle |= WS_EX_MDICHILD;	// force this one
		m_pfnSuperWindowProc = ::DefMDIChildProc;
		m_hWndMDIClient = hWndParent;
		ATLASSERT(::IsWindow(m_hWndMDIClient));

		if(rect.m_lpRect == NULL)
			rect.m_lpRect = &TBase::rcDefault;

		BOOL bMaximized = FALSE;												// <<- Changed/Added
		HWND hWndOld = (HWND)::SendMessage(m_hWndMDIClient,						// <<- Changed/Added
			WM_MDIGETACTIVE, 0, (LPARAM)&bMaximized);							// <<- Changed/Added
	    if(bMaximized == TRUE) ::SendMessage(m_hWndMDIClient,					// <<- Changed/Added
			WM_SETREDRAW, FALSE, 0);											// <<- Changed/Added

		HWND hWnd = CFrameWindowImplBase<TBase, TWinTraits >::Create(hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, (UINT)0U, atom, lpCreateParam);

		if(hWnd != NULL && ::IsWindowVisible(m_hWnd) && !::IsChild(hWnd, ::GetFocus()))
			::SetFocus(hWnd);

		if(bMaximized == TRUE) {												// <<- Changed/Added
			::ShowWindow(hWnd, SW_SHOWMAXIMIZED);								// <<- Changed/Added
			::SendMessage(m_hWndMDIClient, WM_SETREDRAW, TRUE, 0);				// <<- Changed/Added
			::RedrawWindow(m_hWndMDIClient, NULL, NULL,							// <<- Changed/Added
				RDW_INVALIDATE | RDW_ALLCHILDREN);								// <<- Changed/Added
		}																		// <<- Changed/Added

		return hWnd;
	}
*/
#endif
#endif

#if (_WTL_VER <= 0x0750)
#ifndef __FOCUS_FIXUPDATE
	#error WTL BUGFIX: MDI windows need a focus related bugfix to work (see here)
/*
	// This are the fixes by Daniel Bowen to the maximization and focus issues:
	// change in CFrameWindowImplBase class (WTL7.0, atlframe.h)
	#define __FOCUS_FIXUPDATE													// <<- Changed/Added
	LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		if(m_hWndClient != NULL && ::IsWindow(m_hWndClient))					// <<- Changed/Added
			::SetFocus(m_hWndClient);

		bHandled = FALSE;
		return 1;
	}
*/
#endif
#ifndef __TOOLBARS_FIXUPDATE
	#error WTL BUGFIX: Vertical toolbars need a WTL bugfix to work (see here)
/*	
	// change in CFrameWindowImplBase class (WTL7.0, atlframe.h)
	#define __TOOLBARS_FIXUPDATE
	void UpdateBarsPosition(RECT& rect, BOOL bResizeBars = TRUE) {
		// resize toolbar
		DWORD dwStyles = (DWORD)::GetWindowLong(m_hWndToolBar, GWL_STYLE);		// <<- Changed/Added
		if(m_hWndToolBar != NULL && (dwStyles & WS_VISIBLE)) {					// <<- Changed/Added
			if(bResizeBars)
				::SendMessage(m_hWndToolBar, WM_SIZE, 0, 0);
			RECT rectTB;
			::GetWindowRect(m_hWndToolBar, &rectTB);

			if(dwStyles & CCS_VERT) rect.left += rectTB.right - rectTB.left;	// <<- Changed/Added
			else rect.top += rectTB.bottom - rectTB.top;						// <<- Changed/Added
		}

		// resize status bar
		if(m_hWndStatusBar != NULL && ((DWORD)::GetWindowLong(m_hWndStatusBar, GWL_STYLE) & WS_VISIBLE)) {
			if(bResizeBars)
				::SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
			RECT rectSB;
			::GetWindowRect(m_hWndStatusBar, &rectSB);
			rect.bottom -= rectSB.bottom - rectSB.top;
			if(dwStyles & CCS_VERT) {											// <<- Changed/Added
				::SetWindowPos(m_hWndStatusBar , HWND_TOP, 0, 0, 0, 0,			// <<- Changed/Added
				SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE );						// <<- Changed/Added
			}																	// <<- Changed/Added
		}
	}
*/
#endif
#endif
