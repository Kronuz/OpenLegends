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
#include "resource.h"

#include "MainFrm.h"
#include "MapEditorView.h"
#include "MapEditorFrm.h"
CMapGroup *CMapEditorView::m_MapGroup = NULL;

CMapEditorView::CMapEditorView(CMapEditorFrame *pParentFrame) :
	CChildView(pParentFrame),
	m_pGraphics(NULL),
	m_szMap(320, 240),
	m_nTimer(0),
	m_Zoom(2),
	m_CursorStatus(eIDC_ARROW),
	m_OldCursorStatus(eIDC_ARROW),
	m_Selection(reinterpret_cast<CDrawableContext**>(&m_MapGroup))
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
	if(m_pGraphics) m_pGraphics->Finalize();
	delete this;
}

LRESULT CMapEditorView::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMapEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
 /////////////////////////////
	m_pGraphics = &m_CGraphicsD3D8;
	m_pGraphics->Initialize(GetMainFrame()->m_hWnd);
/////////////////////

	LONG lRet = DefWindowProc();

	m_Zoom = 1.0f;

	if(!m_MapGroup) m_MapGroup = new CMapGroup;
	//m_MapGroup->GetSize(m_szMap);

	m_szMap.SetSize(320*4, 240*2);

	SetScrollSize(m_szMap.cx*m_Zoom, m_szMap.cy*m_Zoom);

	UpdateView();

	return lRet;
}
LRESULT CMapEditorView::OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) return 0;
	if(m_nTimer) { KillTimer(m_nTimer); m_nTimer = 0; }
	return 0;
}
LRESULT CMapEditorView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(m_nTimer == 0) m_nTimer = SetTimer(1, 1000/30);
	return 0;
}
void CMapEditorView::UpdateView()
{
	// We need to recalculate the window's size and position:
	CRect rcClient, rcClip;
	GetClientRect(&rcClient);
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	rcClip.SetRect(0, 0, m_szMap.cx, m_szMap.cy);
	m_pGraphics->SetWindowView(m_hWnd, rcClient, rcClip, m_Zoom);

}
void CMapEditorView::Render()
{
	if(m_pGraphics->BeginPaint()) {
		m_MapGroup->Draw(m_pGraphics);
		m_Selection.Draw(m_pGraphics);
		m_pGraphics->EndPaint();
	}
}

LRESULT CMapEditorView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(GetParentFrame()->m_hWnd != GetMainFrame()->m_tabbedClient.GetTopWindow()) {
		KillTimer(m_nTimer); 
		m_nTimer = 0;
	}
	Render();
	return 0;
}
void CMapEditorView::DoPaint(CDCHandle dc)
{
	Render();
}

LRESULT CMapEditorView::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CScrollWindowImpl<CMapEditorView>::OnVScroll(uMsg, wParam, lParam, bHandled);
	SetFocus();
	UpdateView();
	return 1;
}
LRESULT CMapEditorView::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CScrollWindowImpl<CMapEditorView>::OnHScroll(uMsg, wParam, lParam, bHandled);
	SetFocus();
	UpdateView();
	return 1;
}
LRESULT CMapEditorView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CScrollWindowImpl<CMapEditorView>::OnSize(uMsg, wParam, lParam, bHandled);
	UpdateView();
	return 1;
}

LRESULT CMapEditorView::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	float oldZoom = m_Zoom;
	bool flip = false;

	if(zDelta>0) m_Zoom *= 1.5f;
	else m_Zoom *= 0.5f;

	if(m_Zoom < 1) {
		m_Zoom = 1/m_Zoom;
		flip=true;
	}

	m_Zoom = (float)(int)(m_Zoom+0.5);
	if(flip) m_Zoom = 1/m_Zoom;

	if(m_Zoom>20.0f) m_Zoom = 20.0f;
	if(m_Zoom<0.10f) m_Zoom = 0.10f;

	if(oldZoom == m_Zoom) return 0;

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("%4d%%"), (int)(100.0f * m_Zoom));
	pStatusBar->SetPaneText(ID_OVERTYPE_PANE, sText);

	// We need to get the mouse position in the world and in the client area
	CPoint MousePoint(lParam);
	ScreenToClient(&MousePoint);
	CPoint WorldPoint(MousePoint);

	WorldPoint.x += GetScrollPos(SB_HORZ);
	WorldPoint.y += GetScrollPos(SB_VERT);
	WorldPoint.x /= oldZoom;
	WorldPoint.y /= oldZoom;

	CRect rcClient;
	GetClientRect(&rcClient);
	CPoint ScrollPoint(WorldPoint.x*m_Zoom-MousePoint.x, WorldPoint.y*m_Zoom-MousePoint.y);

	// We need to recalculate the new map size (in pixeles)
	SetScrollSize(m_szMap.cx*m_Zoom , m_szMap.cy*m_Zoom , FALSE);

	// Now we validate the new scrolling position:
	if(ScrollPoint.x < 0) ScrollPoint.x = 0;
	if(ScrollPoint.y < 0) ScrollPoint.y = 0;
	if(ScrollPoint.x > m_szMap.cx*m_Zoom-rcClient.right) ScrollPoint.x = m_szMap.cx*m_Zoom-rcClient.right;
	if(ScrollPoint.y > m_szMap.cy*m_Zoom-rcClient.bottom) ScrollPoint.y = m_szMap.cy*m_Zoom-rcClient.bottom;
	SetScrollOffset(ScrollPoint);

	UpdateView();
	CPoint Point(lParam);
	ScreenToClient(&Point);
	ToCursor(m_Selection.GetMouseStateAt(m_pGraphics, Point));

	return 0;
}

void CMapEditorView::ToCursor(CURSOR cursor_)
{
	HCURSOR hCursor;
	
	switch(cursor_) {
		case eIDC_ARROW:	hCursor = LoadCursor(NULL, IDC_ARROW); break;
		case eIDC_CROSS:	hCursor = LoadCursor(NULL, IDC_CROSS); break;
		case eIDC_NO:		hCursor = LoadCursor(NULL, IDC_NO); break;
		case eIDC_SIZEALL:	hCursor = LoadCursor(NULL, IDC_SIZEALL); break;
		case eIDC_SIZENESW: hCursor = LoadCursor(NULL, IDC_SIZENESW); break;
		case eIDC_SIZENS:	hCursor = LoadCursor(NULL, IDC_SIZENS); break;
		case eIDC_SIZENWSE: hCursor = LoadCursor(NULL, IDC_SIZENWSE); break;
		case eIDC_SIZEWE:	hCursor = LoadCursor(NULL, IDC_SIZEWE); break;
		case eIDC_ARROWADD: hCursor = LoadCursor(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDC_ARROWADD)); break;
		case eIDC_ARROWDEL: hCursor = LoadCursor(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDC_ARROWDEL)); break;
	}
	SetCursor(hCursor);
}

LRESULT CMapEditorView::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	m_CursorStatus = m_Selection.GetMouseStateAt(m_pGraphics, Point);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	m_pGraphics->GetWorldPosition(Point);
	if(m_CursorStatus==eIDC_ARROW || m_CursorStatus==eIDC_ARROWADD || m_CursorStatus==eIDC_ARROWDEL) 
		m_Selection.StartSelection(Point);
	
	SetCapture();

	return 0;
}
LRESULT CMapEditorView::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	m_CursorStatus = m_Selection.GetMouseStateAt(m_pGraphics, Point);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	m_pGraphics->GetWorldPosition(Point);
	if((wParam&MK_SHIFT)==MK_SHIFT) m_Selection.EndSelectionRemove(Point);
	else {
		if((wParam&MK_CONTROL)==0) m_Selection.CleanSelection();
		m_Selection.EndSelectionAdd(Point);
	}

	ReleaseCapture();

	return 0;
}
LRESULT CMapEditorView::OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	m_CursorStatus = m_Selection.GetMouseStateAt(m_pGraphics, Point);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	if(m_CursorStatus==eIDC_ARROW || m_CursorStatus==eIDC_ARROWADD || m_CursorStatus==eIDC_ARROWDEL) {
		m_Selection.CancelSelection();
		m_Selection.CleanSelection();
	}

	return 0;
}
LRESULT CMapEditorView::OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	m_CursorStatus = m_Selection.GetMouseStateAt(m_pGraphics, Point);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	return 0;
}
LRESULT CMapEditorView::OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	m_CursorStatus = m_Selection.GetMouseStateAt(m_pGraphics, Point);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	return 0;
}
LRESULT CMapEditorView::OnMButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	m_CursorStatus = m_Selection.GetMouseStateAt(m_pGraphics, Point);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	return 0;
}

LRESULT CMapEditorView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	m_CursorStatus = m_Selection.GetMouseStateAt(m_pGraphics, Point);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	m_pGraphics->GetWorldPosition(Point);

	if((wParam&MK_LBUTTON)==MK_LBUTTON) {
		m_Selection.DragSelection(Point);
		Invalidate();
	}

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("X: %3d, Y: %3d"), Point.x, Point.y);
	pStatusBar->SetPaneText(ID_POSITION_PANE, sText);

	if(GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) SetFocus();

	UpdateWindow();
	return 0;
}
LRESULT CMapEditorView::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(wParam == VK_CONTROL) {
		ToCursor(eIDC_ARROWADD);
	}
	if(wParam == VK_SHIFT) {
		ToCursor(eIDC_ARROWDEL);
	}
	return 0;
}
LRESULT CMapEditorView::OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ToCursor(m_OldCursorStatus);

	return 0;
}
