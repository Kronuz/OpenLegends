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
/*! \file		MapEditorFrm.cpp 
	\brief		Implementation of the CMapEditorFrame class.
	\date		April 27, 2003
*/

#include "stdafx.h"
#include "MapEditorFrm.h"
#include "MainFrm.h"

CMapEditorFrame::CMapEditorFrame(CMainFrame *pMainFrame) :
	CChildFrame(pMainFrame, tMapEditor),
	m_pMapEditorView(NULL)
{ 
}	
void CMapEditorFrame::OnFinalMessage(HWND /*hWnd*/)
{
	delete this;
}

/* Note that vertical toolbars need a WTL bugfix to work. Change in the WTL include file 'atlframe.h' :
	void UpdateBarsPosition(RECT& rect, BOOL bResizeBars = TRUE) {
		// resize toolbar
		DWORD dwStyles = (DWORD)::GetWindowLong(m_hWndToolBar, GWL_STYLE);
			if(m_hWndToolBar != NULL && (dwStyles & WS_VISIBLE)) {
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
LRESULT CMapEditorFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMapEditorView = new CMapEditorView(this);

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
	HWND hMapEdObjectToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_MAPED_OBJECT, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | CCS_VERT | TBSTYLE_WRAPABLE );
	// add the toolbar to the UI update map

	// create a rebat to hold both: the command bar and the toolbar
	if(!CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE | CCS_VERT)) {
		ATLTRACE("Failed to create applications rebar\n");
		return -1;      // fail to create
	}	
	AddSimpleReBarBand(hMapEdBasicToolBar, NULL, TRUE);
	AddSimpleReBarBand(hMapEdObjectToolBar, NULL, TRUE);

	CChildFrame::Register(tMapEditor);
	SetMsgHandled(FALSE);
	m_sChildName = _T("Map Editor");
	SetTitle(m_sChildName);
	return TRUE;
}
LRESULT CMapEditorFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CChildFrame::Unregister();
	return 0;
}
LRESULT CMapEditorFrame::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMSG pMsg = (LPMSG)lParam;
	// we need the base class to do its stuff
	if(baseClass::PreTranslateMessage(pMsg))
		return TRUE;

	// the messages need to be hended to the active view
	return m_pMapEditorView->PreTranslateMessage(pMsg);
}
LRESULT CMapEditorFrame::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// System settings or metrics have changed.  Propogate this message
	// to all the child windows so they can update themselves as appropriate.
	SendMessageToDescendants(uMsg, wParam, lParam, TRUE);

	return 0;
}
LRESULT CMapEditorFrame::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// We give the focus to the view
	m_pMapEditorView->SetFocus();

	return 0;
}
