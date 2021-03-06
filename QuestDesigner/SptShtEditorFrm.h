/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germ�n M�ndez Bravo (Kronuz)
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
#pragma once

#include "GEditorFrm.h"
#include "SptShtEditorView.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// This class manages the script editor frame
class CSptShtEditorFrame :
	public CGEditorFrame,
	public CUpdateUI<CSptShtEditorFrame>
{
protected:
	CTrueColorToolBarCtrl m_ctrlBasicToolbar;
	CTrueColorToolBarCtrl m_ctrlObjectToolbar;

	// The window's view (the child control)
	CSptShtEditorView *m_pSptShtEditorView;

public:

	// Construction/Destruction:
	CSptShtEditorFrame(CMainFrame *pMainFrame);

	BEGIN_UPDATE_UI_MAP(CSptShtEditorFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CSptShtEditorFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WMQD_SELECT, OnSelectSprite)

		//MESSAGE_HANDLER(UWM_MDICHILDSHOWTABCONTEXTMENU, OnShowTabContextMenu)

		CHAIN_MSG_MAP(CGEditorFrame)

		CHAIN_MSG_MAP(CUpdateUI<CSptShtEditorFrame>)

	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	// Return a the window's UI updater
	CUpdateUIBase* GetUpdateUI() {
		return static_cast<CUpdateUIBase *>(this);
	}

	LRESULT OnSelectSprite(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

	// Return the window's child control
	CSptShtEditorView* GetView() { return m_pSptShtEditorView; }
};
