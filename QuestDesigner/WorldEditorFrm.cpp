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
// WorldEditorFrm.cpp : implementation of the CWorldEditorFrame class
//

#include "stdafx.h"
#include "WorldEditorFrm.h"
#include "MainFrm.h"

CWorldEditorFrame::CWorldEditorFrame(CMainFrame *pMainFrame) :
	m_pWorldEditorView(NULL)
{ 
	m_pMainFrame = pMainFrame;
}	
void CWorldEditorFrame::OnFinalMessage(HWND /*hWnd*/)
{
	delete this;
}

LRESULT CWorldEditorFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pWorldEditorView = new CWorldEditorView(this);

	// create our view
	m_hWndClient = m_pWorldEditorView->Create(	m_hWnd, 
										rcDefault,
										NULL, 
										WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 
										WS_EX_CLIENTEDGE);
	if ( NULL == m_hWndClient ) {
		ATLTRACE ( _T ( "Error: failed to create child window\n" ) );
		return FALSE;
	}

	SetMsgHandled(FALSE);
	return TRUE;
}
LRESULT CWorldEditorFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CChildFrame::Unregister();
	return 0;
}
LRESULT CWorldEditorFrame::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMSG pMsg = (LPMSG)lParam;
	// we need the base class to do its stuff
	if(baseClass::PreTranslateMessage(pMsg))
		return TRUE;

	// the messages need to be hended to the active view
	return m_pWorldEditorView->PreTranslateMessage(pMsg);
}
LRESULT CWorldEditorFrame::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// System settings or metrics have changed.  Propogate this message
	// to all the child windows so they can update themselves as appropriate.
	SendMessageToDescendants(uMsg, wParam, lParam, TRUE);

	return 0;
}
LRESULT CWorldEditorFrame::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// We give the focus to the view
	m_pWorldEditorView->SetFocus();

	return 0;
}
