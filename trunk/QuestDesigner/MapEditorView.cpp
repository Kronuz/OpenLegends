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
/*! \file		MapEditorView.cpp 
	\brief		Implementation of the CMapEditorView class.
	\date		April 27, 2003
*/

#include "stdafx.h"

#include "MainFrm.h"
#include "MapEditorView.h"
#include "MapEditorFrm.h"

CMapEditorView::CMapEditorView(CMapEditorFrame *pParentFrame) :
	CChildView(pParentFrame)
{
}

// Called to translate window messages before they are dispatched 
BOOL CMapEditorView::PreTranslateMessage(MSG *pMsg)
{
	return FALSE;
}
// Called to clean up after window is destroyed (called when WM_NCDESTROY is sent)
void CMapEditorView::OnFinalMessage(HWND /*hWnd*/)
{
	delete this;
}

LRESULT CMapEditorView::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMapEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	LONG lRet = DefWindowProc();

	m_MapSize.cx = 320*2;
	m_MapSize.cy = 240*2;

	SetScrollSize(m_MapSize);

	return lRet;
}

void CMapEditorView::DoPaint(CDCHandle dc)
{
	RECT rect;
	GetClientRect(&rect);

	CBrush brush;
	brush.CreateSolidBrush(RGB(74,158,74));

	dc.FillRect(&rect, brush);
}

LRESULT CMapEditorView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint point(lParam);
	point.x += GetScrollPos(SB_HORZ);
	point.y += GetScrollPos(SB_VERT);

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("X: %3d, Y: %3d"), point.x/2, point.y/2);
	pStatusBar->SetPaneText(ID_POSITION_PANE, sText);

	return 0;
}
