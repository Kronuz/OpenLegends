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
/*! \file		WorldEditorFrm.cpp 
	\brief		Implementation of the CWorldEditorFrame class.
	\date		April 15, 2003
*/

#include "stdafx.h"
#include "WorldEditorFrm.h"
#include "MainFrm.h"

CWorldEditorFrame::CWorldEditorFrame(CMainFrame *pMainFrame) :
	CChildFrame(pMainFrame, tWorldEditor),
	m_pWorldEditorView(NULL)
{ 
}	

LRESULT CWorldEditorFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HICON hIcon = (HICON)::LoadImage(
				_Module.GetResourceInstance(),
				MAKEINTRESOURCE(IDI_DOC_WORLD),
				IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_SHARED);
	SetIcon(hIcon, ICON_SMALL);
	m_hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDIWORLDED));

	m_pChildView = m_pWorldEditorView = new CWorldEditorView(this);

	// create our view
	m_hWndClient = m_pWorldEditorView->Create(m_hWnd, 
										rcDefault,
										NULL, 
										WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 
										WS_EX_CLIENTEDGE);
	if ( NULL == m_hWndClient ) {
		ATLTRACE(_T("Error: failed to create child window\n"));
		return FALSE;
	}

	m_sChildName = _T("World Editor");
	SetTitle(m_sChildName);

	m_pChildView = m_pWorldEditorView;

	CChildFrame::Register(tWorldEditor);
	bHandled = FALSE;

	return TRUE;
}
LRESULT CWorldEditorFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CChildFrame::Unregister();
	bHandled = FALSE;

	return 0;
}
