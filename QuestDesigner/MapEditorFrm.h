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
/*! \file		MapEditorFrm.h 
	\brief		Interface of the CMapEditorFrame class.
	\date		April 27, 2003
*/

#pragma once

#include "GEditorFrm.h"
#include "MapEditorView.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// This class manages the script editor frame
class CMapEditorFrame :
	public CGEditorFrame,
	public CUpdateUI<CMapEditorFrame>
{
protected:
	CTrueColorToolBarCtrl m_ctrlBasicToolbar;
	CTrueColorToolBarCtrl m_ctrlObjectToolbar;

	// The window's view (the child control)
	CMapEditorView *m_pMapEditorView;

public:

	// Construction/Destruction:
	CMapEditorFrame(CMainFrame *pMainFrame);

	BEGIN_UPDATE_UI_MAP(CMapEditorFrame)
		UPDATE_ELEMENT(ID_MAPED_PLAYER, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_ARROW, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_SELECT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_SELECT_ALL, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_SELECT_NONE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_SELHOLD, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_NOZOOM, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_ZOOMIN, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_ZOOMOUT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_MASK, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_BOUNDS, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_GRID, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_GRIDSNAP, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )

		UPDATE_ELEMENT(ID_MAPED_TOTOP, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_OBJUP, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_OBJDWN, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_TOBOTTOM, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_FLIP, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_MIRROR, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_C90, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_CC90, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_ALTOP, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_ALBOTTOM, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_ALLEFT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_ALRIGHT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_ALCY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT(ID_MAPED_ALCX, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMapEditorFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		//MESSAGE_HANDLER(UWM_MDICHILDSHOWTABCONTEXTMENU, OnShowTabContextMenu)

		CHAIN_MSG_MAP(CGEditorFrame)

		CHAIN_MSG_MAP(CUpdateUI<CMapEditorFrame>)

	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	// Return a the window's UI updater
	CUpdateUIBase* GetUpdateUI() {
		return static_cast<CUpdateUIBase *>(this);
	}

	// Return the window's child control
	CMapEditorView* GetView() { return m_pMapEditorView; }
};
