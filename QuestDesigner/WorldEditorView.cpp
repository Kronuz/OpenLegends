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
/*! \file		WorldEditorView.cpp 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the CWorldEditorView class.
	\date		April 15, 2003
*/

#include "stdafx.h"

#include "MainFrm.h"
#include "WorldEditorView.h"
#include "WorldEditorFrm.h"

CWorldEditorView::CWorldEditorView(CWorldEditorFrame *pParentFrame) :
	CGEditorView(pParentFrame),
	m_MousePoint(0,0),
	m_MapPoint(0,0),
	m_bPanning(false),

	m_bWasModified(false),
	m_szMap(0, 0),
	m_pSelMapGroup(NULL),
	m_pOldMapGroup(NULL),
	m_hPenGrid(NULL),
	m_hPenMapGroupSelected(NULL),
	m_hPenMapGroupHover(NULL),
	m_hPenMapGroup(NULL),
	m_hFont12(NULL),
	m_hFont10(NULL),
	m_hFont8(NULL),
	m_hFont6(NULL)
{
}

LRESULT CWorldEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	LRESULT nResult = DefWindowProc();

	m_sTitle = "World Editor";

	m_hPenGrid = CreatePen(PS_SOLID, 1, RGB(127, 180, 127));
	m_hPenMapGroup = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
	m_hPenMapGroupHover = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));
	m_hPenMapGroupSelected = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));

	m_hFont12 =  CreateFont(12, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");
	m_hFont10 =  CreateFont(10, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");
	m_hFont8 =   CreateFont(8, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");
	m_hFont6 =   CreateFont(6, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");

	CProjectFactory::Interface()->GetWorldSize(m_szWorld);
	CProjectFactory::Interface()->GetMapSize(m_szMap);

	m_WorldFullSize.cx = m_szMap.cx*m_szWorld.cx;
	m_WorldFullSize.cy = m_szMap.cy*m_szWorld.cy;

	m_Zoom = 0.25f;
	m_MaxZoom = 1.0f;
	m_MinZoom = 0.05f;
	m_bAllowAnimate = false;
	m_bAllowMulSelection = false;
	m_bMulSelection = false;

	CalculateLimits();
	SetScrollSize(m_rcScrollLimits.Size());

	SetMsgHandled(FALSE);

	return nResult;
}
LRESULT CWorldEditorView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	if(m_hPenGrid) DeleteObject(m_hPenGrid);
	if(m_hPenMapGroupSelected) DeleteObject(m_hPenMapGroupSelected);
	if(m_hPenMapGroupHover) DeleteObject(m_hPenMapGroupHover);
	if(m_hPenMapGroup) DeleteObject(m_hPenMapGroup);
	if(m_hFont12) DeleteObject(m_hFont12); 
	if(m_hFont10) DeleteObject(m_hFont10);
	if(m_hFont8) DeleteObject(m_hFont8);
	if(m_hFont6) DeleteObject(m_hFont6);

	return 0;
}
LRESULT CWorldEditorView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	// If the last Registerd Window to have the focus was this very same window, do nothing:
	if(GetMainFrame()->GetOldFocus(tAny) == m_hWnd) return 0;
	GetMainFrame()->SetOldFocus(tWorldEditor, m_hWnd); // leave a trace of our existence

	// Update the properties window:
	HWND hWnd = GetMainFrame()->m_hWnd;
	::SendMessage(hWnd, WMP_CLEAR, 0, 0);

	Invalidate();

	OnZoom(); // called to update the zoom information (perhaps in the status bar)

	return 0;
}

LRESULT CWorldEditorView::OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	// check if we are still the focused window or other application's window is to be focused, 
	// if so, we just do nothing:
	if(wParam == NULL) return 0;
	if(GetParentFrame()->m_hWnd == (HWND)wParam) return 0;

	// we don't do anything anyway :P ... 
	return 0;
}

LRESULT CWorldEditorView::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return 0;
}

void CWorldEditorView::UIUpdateStatusBar()
{
}
void CWorldEditorView::UIUpdateMenuItems()
{
	// Get the main window's UI updater
	CMainFrame *pMainFrm = GetMainFrame();
	CUpdateUIBase *pMainUpdateUI = pMainFrm->GetUpdateUI();

	if(pMainFrm->m_bLayers == FALSE) {
		pMainFrm->m_bLayers = TRUE;
		pMainFrm->m_ctrlLayers.EnableWindow(TRUE);
	}
	if(pMainFrm->m_PaneWindows.size() >= (ID_VIEW_PANELAST-ID_VIEW_PANEFIRST)) {
		CTabbedDockingWindow *pThumbnails = pMainFrm->m_PaneWindows[ID_PANE_THUMBNAILS-ID_VIEW_PANEFIRST];
	}

	pMainUpdateUI->UIEnable(ID_APP_SAVE, hasChanged());	

/* Undo features and stuff will be left pending for the next major release */

	pMainFrm->UIEnable(ID_UNDO, CanUndo());
	pMainFrm->UIEnable(ID_REDO, CanRedo());

	pMainFrm->UIEnable(ID_CUT, CanCut());
	pMainFrm->UIEnable(ID_COPY, CanCopy());
	pMainFrm->UIEnable(ID_PASTE, CanPaste());
	pMainFrm->UIEnable(ID_ERASE, IsSelection());
}

BOOL CWorldEditorView::OnIdle()
{
	// Update all the menu items
	UIUpdateMenuItems();
	
	// Update position display in the status bar
	UIUpdateStatusBar();

	CMainFrame *pMainFrm = GetMainFrame();
	CUpdateUIBase *pMainUpdateUI = pMainFrm->GetUpdateUI();

	// Update all the toolbar items
	pMainUpdateUI->UIUpdateToolBar();

	bool bModified = hasChanged();
	if(bModified != m_bModified) {
		m_bModified = bModified;
		if(m_bModified) {
			m_pParentFrame->SetTabText(m_sTitle+"*");
		} else {
			m_pParentFrame->SetTabText(m_sTitle);
		}
	}
	return FALSE;
}

bool CWorldEditorView::hasChanged()
{
	return m_bWasModified;
}

void CWorldEditorView::ViewToWorld(CPoint *_pPoint)
{
	_pPoint->x += GetScrollPos(SB_HORZ);
	_pPoint->y += GetScrollPos(SB_VERT);

	_pPoint->x = (int)((float)_pPoint->x / m_Zoom + 0.5f);
	_pPoint->y = (int)((float)_pPoint->y / m_Zoom + 0.5f);
}

void CWorldEditorView::HoldOperation()
{
}
void CWorldEditorView::CancelOperation(bool bPropagate)
{
}

bool CWorldEditorView::isResizing()
{
	return false;
}
bool CWorldEditorView::isMoving()
{
	return false;
}
bool CWorldEditorView::isFloating()
{
	return false;
}
bool CWorldEditorView::isSelecting()
{
	return false;
}
bool CWorldEditorView::isHeld()
{
	return false;
}

void CWorldEditorView::HoldSelection(bool bHold)
{
}
void CWorldEditorView::StartSelBox(const CPoint &_Point, CURSOR *_pCursor)
{
}
void CWorldEditorView::SizeSelBox(const CPoint &_Point, CURSOR *_pCursor)
{
}
IPropertyEnabled* CWorldEditorView::EndSelBoxRemove(const CPoint &_Point, LPARAM lParam)
{
	return NULL;
}
IPropertyEnabled* CWorldEditorView::EndSelBoxAdd(const CPoint &_Point, LPARAM lParam)
{
	return NULL;
}
void CWorldEditorView::CancelSelBox()
{
}
IPropertyEnabled* CWorldEditorView::SelectPoint(const CPoint &_Point, CURSOR *_pCursor)
{
	UpdateMouse(_Point);
	CMapGroup *pMapGroup = CProjectFactory::Interface()->FindMapGroup(m_MapPoint.x, m_MapPoint.y);
	if(m_pSelMapGroup == pMapGroup) {
		m_pSelMapGroup = NULL;
	} else m_pSelMapGroup = pMapGroup;
	Invalidate();
	return NULL;
}

void CWorldEditorView::GetSelectionBounds(CRect *_pRect)
{
}

void CWorldEditorView::StartMoving(const CPoint &_Point, CURSOR *_pCursor)
{
}
void CWorldEditorView::MoveTo(const CPoint &_Point, CURSOR *_pCursor)
{
}
void CWorldEditorView::EndMoving(const CPoint &_Point, LPARAM lParam)
{
}

void CWorldEditorView::StartResizing(const CPoint &_Point, CURSOR *_pCursor)
{
}
void CWorldEditorView::ResizeTo(const CPoint &_Point, CURSOR *_pCursor)
{
}
void CWorldEditorView::EndResizing(const CPoint &_Point, LPARAM lParam)
{
}

bool CWorldEditorView::SelectedAt(const CPoint &_Point)
{
	return false;
}
int CWorldEditorView::SelectedCount()
{
	return 0;
}

void CWorldEditorView::PasteSelection(LPVOID _pBuffer, const CPoint &_Point)
{
}
HGLOBAL CWorldEditorView::CopySelection(BITMAP **ppBitmap, bool bDeleteBitmap)
{
	return 0;
}
BITMAP* CWorldEditorView::CaptureSelection(float _fZoom)
{
	return NULL;
}

void CWorldEditorView::CleanSelection()
{
}
int CWorldEditorView::DeleteSelection()
{
	return 0;
}

bool CWorldEditorView::GetMouseStateAt(const CPoint &_Point, CURSOR *_pCursor)
{
	return false;
}
void CWorldEditorView::CalculateLimits()
{
	CSize szMap((int)((float)m_szMap.cx*m_Zoom+0.5f), (int)((float)m_szMap.cy*m_Zoom+0.5f));
	m_rcScrollLimits.SetRect(
		0,
		0,
		szMap.cx*m_szWorld.cx+1,
		szMap.cy*m_szWorld.cy+1
	);
}
void CWorldEditorView::UpdateSnapSize(int _SnapSize)
{
}

void CWorldEditorView::Render(WPARAM wParam)
{
	ASSERT(wParam);
	{ // scope for dcMem, needed for the UpdateMouse() below to really update
		// create memory DC
		CMemDC dcMem(wParam ? (HDC)wParam : GetDC(), NULL);

		CRect rcSrc (0, 0, m_WorldFullSize.cx, m_WorldFullSize.cy);
//		CRect rcDst(m_rcScrollLimits);
		CRect rcDst;
		GetClientRect(&rcDst);
		rcDst.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

		// Background:
		dcMem.FillSolidRect(&dcMem.m_rc, ::GetSysColor(COLOR_APPWORKSPACE));

		// White world background:
		dcMem.FillRect(rcDst, -1);

		CSize szMap((int)((float)m_szMap.cx*m_Zoom+0.5f), (int)((float)m_szMap.cy*m_Zoom+0.5f));

		// Draw Grid in the Background
		HPEN oldPen = dcMem.SelectPen(m_hPenGrid);
		for(int x = (rcDst.left / szMap.cx) * szMap.cx; x <= rcDst.right; x += szMap.cx) {	// Vertical
			dcMem.MoveTo(x, rcDst.top);
			dcMem.LineTo(x, rcDst.bottom);
		}
		for(int y = (rcDst.top/szMap.cy)*szMap.cy; y <= rcDst.bottom; y += szMap.cy) {	// Horizontal
			dcMem.MoveTo(rcDst.left,  y);
			dcMem.LineTo(rcDst.right, y);
		}

		HFONT oldFont;
		if(m_Zoom < 0.10f) {
			oldFont = dcMem.SelectFont(m_hFont6);
		} else if(m_Zoom < 0.22f) {
			oldFont = dcMem.SelectFont(m_hFont8);
		} else if(m_Zoom < 0.25f ) {
			oldFont = dcMem.SelectFont(m_hFont10);
		} else {
			oldFont = dcMem.SelectFont(m_hFont12);
		}

		CString strOut;
		CMapGroup *pMapGroup;
		dcMem.SetBkMode(TRANSPARENT);
		HBRUSH hOldBrush = dcMem.SelectStockBrush(NULL_BRUSH);
		for(x=rcDst.left/szMap.cx; x<=rcDst.right/szMap.cx; x++) {
			for(y=rcDst.top/szMap.cy; y<=rcDst.bottom/szMap.cy; y++) {
				if((pMapGroup = DrawThumbnail(dcMem, x, y, szMap)) == NULL) {
					if(m_Zoom > 0.08f) {
						strOut.Format(_T("%d,%d"), x, y);
						dcMem.SetTextColor(RGB(25, 25, 22));	// Text settings
						dcMem.TextOut(x*szMap.cx+2,y*szMap.cy+2, strOut);
					}

				} else {
					if(pMapGroup->isMapGroupHead(x,y)) {
						if(m_Zoom >= 0.08f) {
							strOut.Format(_T("%d,%d: %s"), x, y, pMapGroup->GetMapGroupID());
							dcMem.SetTextColor(RGB(25, 25, 22));
							dcMem.TextOut(x*szMap.cx+4, y*szMap.cy+4, strOut);
							dcMem.TextOut(x*szMap.cx+5, y*szMap.cy+4, strOut);
							dcMem.TextOut(x*szMap.cx+6, y*szMap.cy+4, strOut);

							dcMem.TextOut(x*szMap.cx+4, y*szMap.cy+5, strOut);
							dcMem.TextOut(x*szMap.cx+6, y*szMap.cy+5, strOut);

							dcMem.TextOut(x*szMap.cx+4, y*szMap.cy+6, strOut);
							dcMem.TextOut(x*szMap.cx+5, y*szMap.cy+6, strOut);
							dcMem.TextOut(x*szMap.cx+6, y*szMap.cy+6, strOut);

							dcMem.SetTextColor(RGB(255, 255, 225));
							dcMem.TextOut(x*szMap.cx+5, y*szMap.cy+5, strOut);
						}
					}

					if(pMapGroup == m_pSelMapGroup) dcMem.SelectPen(m_hPenMapGroupSelected);
					else dcMem.SelectPen(m_hPenMapGroup);

					CRect Rect;
					pMapGroup->GetMapGroupRect(Rect);
					Rect.top *= szMap.cy;
					Rect.left *= szMap.cx;
					Rect.bottom *= szMap.cy;
					Rect.right *= szMap.cx;
					Rect.DeflateRect(4,4);
					dcMem.Rectangle(Rect);
				}
			}
		}
		// Reselect GDI objects
		dcMem.SelectBrush(hOldBrush);
		dcMem.SelectFont(oldFont);

		dcMem.SelectPen(oldPen);
	}

	UpdateSelections(); // updates the selections (after bliting the memDC)

}

void CWorldEditorView::UpdateView()
{
	// We need to recalculate the window's size and position:
	CRect rcClient, rcClip;
	GetClientRect(&rcClient);
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	rcClip.SetRect(0, 0, m_WorldFullSize.cx, m_WorldFullSize.cy);
	// and we do nothing :P
}

void CWorldEditorView::OnChangeSel(int type, IPropertyEnabled *pPropObj)
{
	HWND hWnd = GetMainFrame()->m_hWnd;
	::SendMessage(hWnd, WMP_CLEAR, 0, (LPARAM)m_hWnd);
}
// Called after a zooming:
void CWorldEditorView::OnZoom()
{
	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("%4d%%"), (int)(100.0f * m_Zoom));
	pStatusBar->SetPaneText(ID_OVERTYPE_PANE, sText);
}

bool CWorldEditorView::DoFileOpen(LPCTSTR lpszFilePath, LPCTSTR lpszTitle, WPARAM wParam, LPARAM lParam)
{
	m_pSelMapGroup = NULL;
	m_pOldMapGroup = NULL;
	return false;
}
bool CWorldEditorView::DoFileClose()
{
	static bool bClosing = false;
	if(bClosing) {
		if(hasChanged()) {
			CString sSave;
			sSave.Format("Save Changes to %s?", GetTitle());
			int ret = MessageBox(sSave, QD_MSG_TITLE, MB_YESNOCANCEL|MB_ICONWARNING);
			switch(ret) {
				case IDCANCEL: 
					return false;
				case IDYES: 
					if(!OnFileSave()) { 
						MessageBox("Couldn't save!", QD_MSG_TITLE, MB_OK|MB_ICONERROR); 
						return false; 
					}
				case IDNO: 
					return true;
			}
		}
		bClosing = false;
	} else {
		bClosing = true;
		CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
		pMainFrm->SendMessage(WM_COMMAND, ID_QUEST_CLOSE);
	}

	// the quest gets closed in the mainframe by OnQuestClose() -> CloseWorld()
	return true;
}
bool CWorldEditorView::DoFileSave(LPCTSTR lpszFilePath)
{
	if(CProjectFactory::Interface()->SaveWorld()) {
		m_bWasModified = false;
		return true;
	}
	return false;
}
bool CWorldEditorView::DoFileSaveAs()
{
	return false;
}
bool CWorldEditorView::DoFileReload()
{
	return false;
}

LRESULT CWorldEditorView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CGEditorView::OnMouseMove(uMsg, wParam, lParam, bHandled);

	CPoint Point(lParam);
	ViewToWorld(&Point);

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("X: %3d, Y: %3d"), Point.x, Point.y);
	pStatusBar->SetPaneText(ID_POSITION_PANE, sText);

	if(::GetFocus() != m_hWnd && ::GetFocus()) ::SetFocus(m_hWnd);

	UpdateMouse(Point);

	return 0;
}
LRESULT CWorldEditorView::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPoint Point(lParam);
	ViewToWorld(&Point);
	UpdateMouse(Point);
	GetMainFrame()->MapFileOpen(m_MapPoint);
	m_pSelMapGroup = NULL;
	Invalidate();
	return 0;
}
LRESULT CWorldEditorView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	CSize szMap((int)((float)m_szMap.cx*m_Zoom+0.5f), (int)((float)m_szMap.cy*m_Zoom+0.5f));

	bool bEnd = false;
	static bool s_bEnd = false;
	bool bCtrl = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0x8000);
	bool bShift = ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0x8000);

	CPoint point(m_MousePoint);
	if(wParam == VK_RETURN) {
		if(m_pSelMapGroup == NULL) {
			GetMainFrame()->MapFileOpen(m_MapPoint);
		}
		m_pSelMapGroup = NULL;
		Invalidate();
	} else if(wParam == VK_HOME) {
		if(bCtrl) {
			point.x = m_szMap.cx/2;
			point.y = m_szMap.cy/2;
			ScrollTo(point, rcClient, szMap);
		}
	} else if(wParam == VK_END) {
		bEnd = true;
	} else if(wParam == VK_ESCAPE) {
		m_pSelMapGroup = NULL;
		Invalidate();
	} else if(wParam == VK_LEFT) {
		if(m_pSelMapGroup) {
			CRect Rect;
			m_pSelMapGroup->GetMapGroupRect(Rect);
			point.x = Rect.left * m_szMap.cx + m_szMap.cx/2;
			point.y = Rect.top * m_szMap.cy + m_szMap.cy/2;
			m_bWasModified = true;

			m_pSelMapGroup->OffsetMapGroup(-1,0);
			Invalidate();
		}
		if(s_bEnd || bCtrl) point.x = m_szMap.cx/2;
		else point.x -= m_szMap.cx;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_RIGHT) {
		if(m_pSelMapGroup) {
			CRect Rect;
			m_pSelMapGroup->GetMapGroupRect(Rect);
			point.x = Rect.left * m_szMap.cx + m_szMap.cx/2;
			point.y = Rect.top * m_szMap.cy + m_szMap.cy/2;
			m_bWasModified = true;

			m_pSelMapGroup->OffsetMapGroup(1,0);
			Invalidate();
		}
		if(s_bEnd || bCtrl) point.x = m_WorldFullSize.cx - m_szMap.cx/2;
		else point.x += m_szMap.cx;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_UP) {
		if(m_pSelMapGroup) {
			CRect Rect;
			m_pSelMapGroup->GetMapGroupRect(Rect);
			point.x = Rect.left * m_szMap.cx + m_szMap.cx/2;
			point.y = Rect.top * m_szMap.cy + m_szMap.cy/2;
			m_bWasModified = true;

			m_pSelMapGroup->OffsetMapGroup(0,-1);
			Invalidate();
		}
		if(s_bEnd || bCtrl) point.y = m_szMap.cy/2;
		else point.y -= m_szMap.cy;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_DOWN) {
		if(m_pSelMapGroup) {
			CRect Rect;
			m_pSelMapGroup->GetMapGroupRect(Rect);
			point.x = Rect.left * m_szMap.cx + m_szMap.cx/2;
			point.y = Rect.top * m_szMap.cy + m_szMap.cy/2;
			m_bWasModified = true;

			m_pSelMapGroup->OffsetMapGroup(0,1);
			Invalidate();
		}
		if(s_bEnd || bCtrl) point.y = m_WorldFullSize.cy - m_szMap.cy/2;
		else point.y += m_szMap.cy;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_NEXT) {
		point.y += (rcClient.bottom/szMap.cy)*m_szMap.cy;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_PRIOR) {
		point.y -= (rcClient.bottom/szMap.cy)*m_szMap.cy;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_SPACE) {
		CMapGroup *pMapGroup = CProjectFactory::Interface()->FindMapGroup(m_MapPoint.x, m_MapPoint.y);
		if(m_pSelMapGroup == pMapGroup) {
			m_pSelMapGroup = NULL;
		} else m_pSelMapGroup = pMapGroup;
		Invalidate();
	} else {
		bHandled = FALSE;
	}

	s_bEnd = bEnd;
	return 0;
}

// HELPER FUNCTIONS:
CMapGroup* CWorldEditorView::DrawThumbnail(CDC &dc, int x, int y, CSize &szMap)
{
	CMapGroup *pMapGroup = CProjectFactory::Interface()->FindMapGroup(x,y);
	if(!pMapGroup) return NULL;

	CRect Rect;
	BITMAP *pBitmap = pMapGroup->GetThumbnail(&Rect, x, y);
	if(pBitmap) {
		// fill BITMAPINFO struct
		BITMAPINFO bmpInfo;
		BITMAPINFOHEADER* pInfo = &bmpInfo.bmiHeader;
		pInfo->biSize = sizeof(BITMAPINFOHEADER);
		pInfo->biWidth  = pBitmap->bmWidth;
		pInfo->biHeight = pBitmap->bmHeight;
		pInfo->biPlanes = 1;
		pInfo->biBitCount = pBitmap->bmBitsPixel;
		pInfo->biCompression = BI_RGB;
		pInfo->biSizeImage = 0;
		pInfo->biXPelsPerMeter = 0;
		pInfo->biYPelsPerMeter = 0;
		pInfo->biClrUsed = 0;
		pInfo->biClrImportant = 0;

		CSize szMapGroupSize;
		pMapGroup->GetMapGroupSize(szMapGroupSize);

		DWORD dwRop = (m_pSelMapGroup==pMapGroup)?NOTSRCCOPY:SRCCOPY;

		dc.StretchDIBits(
			x * szMap.cx,			// x-coordinate of upper-left corner of dest. rectangle
			y * szMap.cy,			// y-coordinate of upper-left corner of dest. rectangle
			szMap.cx,				// width of destination rectangle
			szMap.cy,				// height of destination rectangle
			Rect.left,				// x-coordinate of upper-left corner of source rectangle
			Rect.top,				// y-coordinate of upper-left corner of source rectangle
			Rect.Width(),			// width of source rectangle
			Rect.Height(),			// height of source rectangle
			pBitmap->bmBits,		// address of bitmap bits
			&bmpInfo,				// address of bitmap data
			DIB_RGB_COLORS,			// usage flags
			dwRop					// raster operation code
		);
	} else {
	}
	return pMapGroup;
}
// point contains the x,y coordinates of the map, in world map coordinates. 
// i.e. 1,0 is the second map to the right.
void CWorldEditorView::UpdateSelections()
{
	if(m_bPanning) return;
	////////////////////////
	CSize szMap((int)((float)m_szMap.cx*m_Zoom+0.5f), (int)((float)m_szMap.cy*m_Zoom+0.5f));
	CPoint Point( (m_MapPoint.x)*(szMap.cx) + (szMap.cx)/2, (m_MapPoint.y)*(szMap.cy) + (szMap.cy)/2 );

	CRect rcSelect;
	CDC dc(GetDC());
	
	// find out the size of the little selection box:
	int iBox = (int)(10.0f * m_Zoom + 0.5f);

	rcSelect.top = Point.y - iBox;
	rcSelect.bottom = Point.y + iBox;
	rcSelect.left = Point.x - iBox;
	rcSelect.right = Point.x + iBox;

	if(m_rcOldSelect != rcSelect) {
		m_rcOldSelect.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));
		InvalidateRect(m_rcOldSelect);

		m_rcOldSelect = rcSelect;
		rcSelect.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));
		dc.InvertRect(rcSelect);
	}

/////////////////////

	CMapGroup *pMapGroup = NULL;
	if(m_pSelMapGroup) {
		if(m_pSelMapGroup->isMapGroupAt(m_MapPoint.x, m_MapPoint.y)) {
			pMapGroup = m_pSelMapGroup;
		} 
	}
	if(pMapGroup == NULL) pMapGroup = CProjectFactory::Interface()->FindMapGroup(m_MapPoint.x, m_MapPoint.y);

	CRect Rect;
	HPEN oldPen = dc.SelectPen(m_hPenMapGroup);
	HBRUSH hOldBrush = dc.SelectStockBrush(NULL_BRUSH);

	if(m_pOldMapGroup != pMapGroup && m_pOldMapGroup) {
		if(m_pOldMapGroup == m_pSelMapGroup) dc.SelectPen(m_hPenMapGroupSelected);
		m_pOldMapGroup->GetMapGroupRect(Rect);
		Rect.top *= szMap.cy;
		Rect.left *= szMap.cx;
		Rect.bottom *= szMap.cy;
		Rect.right *= szMap.cx;
		Rect.DeflateRect(4,4);
		Rect.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));
		dc.Rectangle(Rect);
	}
	if(pMapGroup) {
		if(pMapGroup == m_pSelMapGroup) dc.SelectPen(m_hPenMapGroupSelected);
		else dc.SelectPen(m_hPenMapGroupHover);
		pMapGroup->GetMapGroupRect(Rect);
		Rect.top *= szMap.cy;
		Rect.left *= szMap.cx;
		Rect.bottom *= szMap.cy;
		Rect.right *= szMap.cx;
		Rect.DeflateRect(4,4);
		Rect.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));
		dc.Rectangle(Rect);
	}

	m_pOldMapGroup = pMapGroup;

	dc.SelectBrush(hOldBrush);
	dc.SelectPen(oldPen);

	ReleaseDC(dc);
}

void CWorldEditorView::UpdateMouse(const CPoint &point)
{
	if( point.x<0 || point.x>m_WorldFullSize.cx ||
		point.y<0 || point.y>m_WorldFullSize.cy ) return;

	m_MousePoint = point;
	m_MapPoint.x = point.x/m_szMap.cx;
	m_MapPoint.y = point.y/m_szMap.cy;

	UpdateSelections();
}

bool CWorldEditorView::ScrollTo(CPoint &point, CRect &rcClient, CSize &szMap)
{
	bool bScroll = false;
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	CPoint ScrollPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	int xPos = (int)(point.x*m_Zoom+0.5f);
	int yPos = (int)(point.y*m_Zoom+0.5f);
	if(xPos-szMap.cx/2 < rcClient.left) {
		int delta = ((rcClient.left - xPos + szMap.cx - 1)/szMap.cx)*szMap.cx;
		ScrollPoint.x -= delta;
		bScroll = true;
	} 
	if(xPos+szMap.cx/2 > rcClient.right) {
		int delta = ((xPos - rcClient.right + 2*szMap.cx - 1)/szMap.cx)*szMap.cx;
		ScrollPoint.x += delta;
		bScroll = true;
	} 
	if(yPos-szMap.cy/2 < rcClient.top) {
		int delta = ((rcClient.top - yPos + szMap.cy - 1)/szMap.cy)*szMap.cy;
		ScrollPoint.y -= delta;
		bScroll = true;
	} 
	if(yPos+szMap.cy/2 > rcClient.bottom) {
		int delta = ((yPos - rcClient.bottom + szMap.cy - 1)/szMap.cy)*szMap.cy;
		ScrollPoint.y += delta;
		bScroll = true;
	} 

	if(bScroll == false) {
		UpdateMouse(point);
		return false;
	}

	rcClient.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));

	if(ScrollPoint.x < 0) ScrollPoint.x = 0;
	if(ScrollPoint.y < 0) ScrollPoint.y = 0;
	if(ScrollPoint.x > m_WorldFullSize.cx-rcClient.right) ScrollPoint.x = m_WorldFullSize.cx-rcClient.right;
	if(ScrollPoint.y > m_WorldFullSize.cy-rcClient.bottom) ScrollPoint.y = m_WorldFullSize.cy-rcClient.bottom;
	SetScrollOffset(ScrollPoint);

	UpdateMouse(point);
	m_rcOldSelect.SetRectEmpty();
	
	return true;
}

BOOL CWorldEditorView::CanUndo()
{
	return FALSE;
}
BOOL CWorldEditorView::CanRedo()
{
	return FALSE;
}
BOOL CWorldEditorView::CanCut()
{
	return FALSE;
}
BOOL CWorldEditorView::CanCopy()
{
	return FALSE;
}
BOOL CWorldEditorView::CanPaste()
{
	return FALSE;
}

BOOL CWorldEditorView::IsSelection()
{
	return FALSE;
}
BOOL CWorldEditorView::IsReadOnly()
{
	return FALSE;
}
