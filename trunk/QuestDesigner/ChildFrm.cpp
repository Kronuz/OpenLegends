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
// ChildFrm.cpp : implementation of the CChildFrame base class
//

#include "stdafx.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ChildView.h"

CChildFrame::CChildFrame(CMainFrame *pMainFrame, _child_type ChildType) :
	m_pMainFrame(pMainFrame),
	m_ChildType(ChildType),
	m_pChildView(NULL),
	m_pCmdBar(NULL)
{
}

// Called to clean up after window is destroyed (called when WM_NCDESTROY is sent)
void CChildFrame::OnFinalMessage(HWND /*hWnd*/)
{
	// remove ourseves from the idle message pump
	CMessageLoop *pLoop = _Module.GetMessageLoop();
	ATLASSERT(NULL!=pLoop);
	pLoop->RemoveIdleHandler(this);

	// the main toolbar buttons seem to stay active for a long time
	// after we have closed _all_ the MDI child window so were going 
	// to force idle processing to update the toolbar.
	PumpIdleMessages();	

	delete this;
}

LRESULT CChildFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLASSERT(m_pChildView); // this should have been set by the user class

	// register ourselves for idle updates
	CMessageLoop * pLoop = _Module.GetMessageLoop();
	ATLASSERT(NULL!=pLoop);
	pLoop->AddIdleHandler(this);		

	SetMsgHandled(FALSE);
	return TRUE;
}

LRESULT CChildFrame::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMSG pMsg = (LPMSG)lParam;
	// we need the base class to do its stuff
	if(baseClass::PreTranslateMessage(pMsg))
		return TRUE;

	if(!m_pChildView) return 0;

	// the messages need to be hended to the active view
	return m_pChildView->PreTranslateMessage(pMsg);
}

LRESULT CChildFrame::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// System settings or metrics have changed.  Propogate this message
	// to all the child windows so they can update themselves as appropriate.
	SendMessageToDescendants(uMsg, wParam, lParam, TRUE);

	return 0;
}

LRESULT CChildFrame::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(!m_pChildView) return false;

	// We give the focus to the view
	m_pChildView->SetFocus();

	// Pumping idle messages to update the main window
	PumpIdleMessages();
	return 0;
}

BOOL CChildFrame::OnIdle()
{
	// check if we are we the active window...
	if(m_pMainFrame->MDIGetActive()==m_hWnd) {
		// fake idle processing for the view so it updates
		if(m_pChildView) return m_pChildView->OnIdle();
	}
	return FALSE;
}

bool CChildFrame::hasChanged() 
{ 
	if(!m_pChildView) return false;
	return m_pChildView->hasChanged(); 
}

void CChildFrame::SetCommandBarCtrlForContextMenu(CTabbedMDICommandBarCtrl* pCmdBar)
{
	m_pCmdBar = pCmdBar;
}

LRESULT CChildFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
	bHandled = FALSE;
	if(wParam == SIZE_MINIMIZED) ShowWindow(FALSE);
	return 0;
}

LRESULT CChildFrame::Register(_child_type ChildType)
{
	ATLASSERT(m_pMainFrame);

	m_pMainFrame->m_ChildList.Add(this);
	m_ChildType = ChildType;

	return TRUE;
}

LRESULT CChildFrame::Unregister()
{
	ATLASSERT(m_pMainFrame);
	m_pMainFrame->m_ChildList.Remove(this);
	return 0;
}
