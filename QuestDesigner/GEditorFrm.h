/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
   Copyright (C) 2001-2003. Open Zelda's Project
 
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
/*! \file		GEditorFrm.h 
	\brief		Interface of the CGEditorFrame virtual base class.
	\date		April 27, 2003
				September 02, 2003
*/

#pragma once

#include "ChildFrm.h"
#include "GEditorView.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// This class manages the script editor frame
class CGEditorFrame :
	public CChildFrame
{
	typedef CChildFrame baseClass;

protected:
	// The window's view (the child control)
	CGEditorView *m_pGEditorView;

public:

	// Construction/Destruction:
	CGEditorFrame(CMainFrame *pMainFrame, _child_type ChildType);

	BEGIN_MSG_MAP(CGEditorFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)

		CHAIN_MSG_MAP(baseClass)

	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	
	// Return the window's child control
	CGEditorView* GetView() { return m_pGEditorView; }
};
