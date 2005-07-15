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
/*! \file		SptShtEditorFrame.cpp 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the CSptShtEditorFrame class.
	\date		September 30, 2003
*/

#include "stdafx.h"
#include "SptShtEditorFrm.h"
#include "MainFrm.h"

CSptShtEditorFrame::CSptShtEditorFrame(CMainFrame *pMainFrame) :
	CGEditorFrame(pMainFrame, tSpriteEditor),
	m_pSptShtEditorView(NULL)
{ 
}	
LRESULT CSptShtEditorFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HICON hIcon = (HICON)::LoadImage(
				_Module.GetResourceInstance(),
				MAKEINTRESOURCE(IDI_DOC_SPTSHT),
				IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_SHARED);
	SetIcon(hIcon, ICON_SMALL);
	m_hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDISPTSHTED));

	m_pChildView = m_pSptShtEditorView = new CSptShtEditorView(this);

	// create our view
	m_hWndClient = m_pSptShtEditorView->Create(m_hWnd, 
										rcDefault,
										NULL, 
										WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 
										WS_EX_CLIENTEDGE);
	if ( NULL == m_hWndClient ) {
		ATLTRACE(_T("Error: failed to create child window\n"));
		return FALSE;
	}

	m_pGEditorView = m_pSptShtEditorView;

	CGEditorFrame::Register(tSpriteEditor);
	bHandled = FALSE;

	return TRUE;
}
LRESULT CSptShtEditorFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CGEditorFrame::Unregister();
	bHandled = FALSE;

	return 0;
}

LRESULT CSptShtEditorFrame::OnSelectSprite(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_pSptShtEditorView->SpriteStep((LPCSTR)lParam);
	return 0;
}