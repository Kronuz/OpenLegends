/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003 Kronuz
   Copyright (C) 2001/2003 Open Zelda's Project
 
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
	\brief		Include file for standard system include files,
		or project specific include files that are used frequently, but
		are changed infrequently
	\date		April 15, 2003
*/

#pragma once

#pragma warning ( disable : 4530 )

// Change these values to use different versions
#define WINVER			0x0400
#define _WIN32_WINNT	0x0400
#define _WIN32_IE		0x0400
#define _RICHEDIT_VER	0x0300

// This is required for hosting browser in ATL7
#define _ATL_DLL

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

extern CAppModule _Module;

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

#include "misc.h"
#include "IdleHandlerPump.h"

#include "resource.h"

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

#define WMQD_ADDTREE		(WMQD_FIRST+7)
#define WMQD_DELTREE		(WMQD_FIRST+8)

#define ICO_FOLDER			0x0001
#define ICO_PICTURE			0x0202
#define ICO_DOC1			0x0303
#define ICO_DOC2			0x0404
#define ICO_PROJECT			0x0505
#define ICO_WAV				0x0606
#define ICO_MIDI			0x0707
#define ICO_PICFOLDER		0x0808
#define ICO_EMPTY			0x0909

#define SIMPLE_MESSAGE_HANDLER(msg, func) if(uMsg==msg) { bHandled=TRUE; lResult=func(wParam,lParam); if(bHandled) return TRUE; }
