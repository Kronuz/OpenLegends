/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germán Méndez Bravo)
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
/*! \file		MapEditorFrm.cpp 
	\brief		Implementation of the CMapEditorFrame class.
	\date		April 27, 2003
*/

#include "stdafx.h"
#include "MapEditorFrm.h"
#include "MainFrm.h"

CMapEditorFrame::CMapEditorFrame(CMainFrame *pMainFrame) :
	CGEditorFrame(pMainFrame, tMapEditor),
	m_pMapEditorView(NULL)
{ 
}	
LRESULT CMapEditorFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HICON hIcon = (HICON)::LoadImage(
				_Module.GetResourceInstance(),
				MAKEINTRESOURCE(IDI_DOC_MAP),
				IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_SHARED);
	SetIcon(hIcon, ICON_SMALL);
	SetMenu(::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDIMAPED)));

	m_pChildView = m_pMapEditorView = new CMapEditorView(this);

	// create our view
	m_hWndClient = m_pMapEditorView->Create(m_hWnd, 
										rcDefault,
										NULL, 
										WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 
										WS_EX_CLIENTEDGE);
	if ( NULL == m_hWndClient ) {
		ATLTRACE(_T("Error: failed to create child window\n"));
		return FALSE;
	}

	// create a toolbar
	HWND hMapEdBasicToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_MAPED_BASIC, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | CCS_VERT | TBSTYLE_WRAPABLE );
	// add the toolbar to the UI update map
	UIAddToolBar(hMapEdBasicToolBar);

	// create a toolbar
	HWND hMapEdObjectToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_MAPED_OBJECT, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | CCS_VERT | TBSTYLE_WRAPABLE );
	// add the toolbar to the UI update map
	UIAddToolBar(hMapEdObjectToolBar);

	m_ctrlBasicToolbar.SubclassWindow( hMapEdBasicToolBar );
	m_ctrlBasicToolbar.LoadTrueColorToolBar(18, IDR_TB1_MAPED_BASIC);

	m_ctrlObjectToolbar.SubclassWindow( hMapEdObjectToolBar );
	m_ctrlObjectToolbar.LoadTrueColorToolBar(18, IDR_TB1_MAPED_OBJECT);

	// create a rebat to hold both: the command bar and the toolbar
	if(!CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE | CCS_VERT)) {
		ATLTRACE("Failed to create applications rebar\n");
		return -1;      // fail to create
	}	
	AddSimpleReBarBand(hMapEdBasicToolBar, NULL, TRUE);
	AddSimpleReBarBand(hMapEdObjectToolBar, NULL, TRUE);

	UIEnable(ID_MAPED_TOTOP, FALSE);
	UIEnable(ID_MAPED_OBJUP, FALSE);
	UIEnable(ID_MAPED_OBJDWN, FALSE);
	UIEnable(ID_MAPED_TOBOTTOM, FALSE);
	UIEnable(ID_MAPED_FLIP, FALSE);
	UIEnable(ID_MAPED_MIRROR, FALSE);
	UIEnable(ID_MAPED_C90, FALSE);
	UIEnable(ID_MAPED_CC90, FALSE);
	UIEnable(ID_MAPED_ALTOP, FALSE);
	UIEnable(ID_MAPED_ALBOTTOM, FALSE);
	UIEnable(ID_MAPED_ALLEFT, FALSE);
	UIEnable(ID_MAPED_ALRIGHT, FALSE);
	UIEnable(ID_MAPED_ALCY, FALSE);
	UIEnable(ID_MAPED_ALCX, FALSE);

	// Update all the menu items

	m_pGEditorView = m_pMapEditorView;

	CGEditorFrame::Register(tMapEditor);
	bHandled = FALSE;

	return TRUE;
}
LRESULT CMapEditorFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CGEditorFrame::Unregister();
	bHandled = FALSE;

	return 0;
}
