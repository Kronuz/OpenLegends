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

LRESULT CScriptEditorFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HICON hIcon = (HICON)::LoadImage(
				_Module.GetResourceInstance(),
				MAKEINTRESOURCE(IDI_DOC_SCRIPT),
				IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_SHARED);
	SetIcon(hIcon, ICON_SMALL);
	m_hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDISCRIPTED));

	m_pChildView = m_pScriptEditorView = new CScriptEditorView(this);

	// create our view
	m_hWndClient = m_pScriptEditorView->Create(	m_hWnd, 
										rcDefault,
										NULL, 
										WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 
										WS_EX_CLIENTEDGE);
	if ( NULL == m_hWndClient ) {
		ATLTRACE(_T("Error: failed to create child window\n"));
		return FALSE;
	}

	m_pChildView = m_pScriptEditorView;

	CChildFrame::Register(tScriptEditor);
	bHandled = FALSE;

	return TRUE;
}
LRESULT CScriptEditorFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CChildFrame::Unregister();
	bHandled = FALSE;

	return 0;
}

LRESULT CScriptEditorFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL /*&bHandled*/)
{
	if(m_pScriptEditorView->DoFileClose()) {
		DefWindowProc();
	}
	return 0;
}

LRESULT CScriptEditorFrame::OnSelectLine(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if((int)lParam < 0) m_pScriptEditorView->LineStep(-(int)lParam - 1);
	else m_pScriptEditorView->SelectLine((int)lParam - 1);
	return 0;
}