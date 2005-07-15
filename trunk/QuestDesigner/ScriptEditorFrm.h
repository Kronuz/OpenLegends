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
/*! \file		ScriptEditorFrm.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the CScriptEditorFrame class.
	\date		April 15, 2003
*/

#pragma once

#include "ChildFrm.h"
#include "ScriptEditorView.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// This class manages the script editor frame
class CScriptEditorFrame :
	public CChildFrame
{
	typedef CChildFrame baseClass;

protected:
	// The window's view (the child control)
	CScriptEditorView *m_pScriptEditorView;

public:

	// Construction/Destruction:
	CScriptEditorFrame(CMainFrame *pMainFrame);

	BEGIN_MSG_MAP(CScriptEditorFrame)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		
		MESSAGE_HANDLER(WMQD_SELECT, OnSelectLine)

		//MESSAGE_HANDLER(UWM_MDICHILDSHOWTABCONTEXTMENU, OnShowTabContextMenu)

		CHAIN_MSG_MAP(baseClass)

	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL /*&bHandled*/);

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnSelectLine(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

	// Return the window's child control
	CScriptEditorView* GetView() { return m_pScriptEditorView; }
};
