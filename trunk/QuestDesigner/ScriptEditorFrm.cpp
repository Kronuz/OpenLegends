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
// ScriptEditorFrm.cpp : implementation of the CScriptEditorFrame class
//

#include "stdafx.h"
#include "ScriptEditorFrm.h"
#include "MainFrm.h"

CScriptEditorFrame::CScriptEditorFrame(CMainFrame *pMainFrame) :
	CChildFrame(pMainFrame, tScriptEditor),
	m_pScriptEditorView(NULL)
{ 
}	

void CScriptEditorFrame::OnFinalMessage(HWND /*hWnd*/)
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
BOOL CScriptEditorFrame::OnIdle()
{
	// update the main window's title bar
	//UIUpdateTitle();
		
	// check if we are we the active window...
	if(m_pMainFrame->MDIGetActive()==m_hWnd) {
		// fake idle processing for the view so it updates
		m_pScriptEditorView->OnIdle();
	}
	return FALSE;
}

LRESULT CScriptEditorFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HICON hIcon = (HICON)::LoadImage(
				_Module.GetResourceInstance(),
				MAKEINTRESOURCE(IDI_DOC_SCRIPT),
				IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_SHARED);
	SetIcon(hIcon, ICON_SMALL);
	SetMenu(::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDISCRIPTED)));

	m_pScriptEditorView = new CScriptEditorView(this);

	// create our view
	m_hWndClient = m_pScriptEditorView->Create(	m_hWnd, 
										rcDefault,
										NULL, 
										WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 
										WS_EX_CLIENTEDGE);
	if ( NULL == m_hWndClient ) {
		ATLTRACE ( _T ( "Error: failed to create child window\n" ) );
		return FALSE;
	}

	// register ourselves for idle updates
	CMessageLoop * pLoop = _Module.GetMessageLoop();
	ATLASSERT(NULL!=pLoop);
	pLoop->AddIdleHandler(this);		

	CChildFrame::Register(tScriptEditor);
	SetMsgHandled(FALSE);
	return TRUE;
}
LRESULT CScriptEditorFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CChildFrame::Unregister();
	return 0;
}

LRESULT CScriptEditorFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL /*&bHandled*/)
{
	if(m_pScriptEditorView->IsModified()) {
		CString sSave;
		sSave.Format("Save Changes to %s?", m_pScriptEditorView->GetTitle());
		int ret = MessageBox(sSave, _T("Quest Designer - Script Editor"), MB_YESNOCANCEL);
		BOOL bTmp;
		switch(ret) {
			case IDCANCEL: return 0;
			case IDYES: m_pScriptEditorView->OnFileSave(0,0,0,bTmp);
			case IDNO: return DefWindowProc();
		}
	} else {
		return DefWindowProc();
	}
	return 0;
}

LRESULT CScriptEditorFrame::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMSG pMsg = (LPMSG)lParam;
	// we need the base class to do its stuff
	if(baseClass::PreTranslateMessage(pMsg))
		return TRUE;

	// the messages need to be hended to the active view
	return m_pScriptEditorView->PreTranslateMessage(pMsg);
}
LRESULT CScriptEditorFrame::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// System settings or metrics have changed.  Propogate this message
	// to all the child windows so they can update themselves as appropriate.
	SendMessageToDescendants(uMsg, wParam, lParam, TRUE);

	return 0;
}
LRESULT CScriptEditorFrame::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// We give the focus to the view
	m_pScriptEditorView->SetFocus();

	// Pumping idle messages to update the main window
	PumpIdleMessages();
	return 0;
}

LRESULT CScriptEditorFrame::OnSelectLine(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_pScriptEditorView->SelectLine(lParam-1);
	return 0;
}