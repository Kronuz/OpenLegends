/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germ�n M�ndez Bravo)
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
/*! \file		GEditorView.cpp 
	\brief		Implementation of the CGEditorView virtual base class.
	\date		April 27, 2003
				September 02, 2003
*/

#include "stdafx.h"
#include "resource.h"

#include "MainFrm.h"
#include "GEditorView.h"
#include "GEditorFrm.h"

#include "GraphicsFactory.h"

CGEditorView::CGEditorView(CGEditorFrame *pParentFrame) :
	CChildView(pParentFrame),
	m_Zoom(1.0f),
	m_nSnapSize(16),
	m_DragState(tNone),
	m_CursorStatus(eIDC_ARROW),
	m_OldCursorStatus(eIDC_ARROW),
	m_pDropTarget(NULL),
	m_pDropSource(NULL),

	m_bAllowAutoScroll(true),
	m_bAllowSnapOverride(true),
	m_bAllowMulSelection(true),
	m_bMulSelection(true),
	m_bSnapToGrid(true),
	m_bShowGrid(false),
	m_bAnimated(true),
	m_bPanning(false),
	m_bModified(false),
	m_bFloating(false),
	m_bDuplicating(false),
	m_bScrolling(false),

	m_rcScrollLimits(0,0,0,0)
{
}

// Called to do idle processing
BOOL CGEditorView::OnIdle()
{
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

bool CGEditorView::InitDragDrop()
{
	m_pDropSource = new CIDropSource;
	if(m_pDropSource == NULL) 
		return false;
	m_pDropSource->AddRef();
	m_pDropTarget = new CWindowDropTarget(m_hWnd);
	if(m_pDropTarget == NULL)
		return false;
	m_pDropTarget->AddRef();
	if(FAILED(RegisterDragDrop(m_hWnd, m_pDropTarget))) { //calls addref
		m_pDropTarget = NULL;
		return false;
	} else {
		m_pDropTarget->Release(); //i decided to AddRef explicitly after new
	}

	FORMATETC ftetc={0};
	ftetc.cfFormat = CF_TEXT;
	ftetc.dwAspect = DVASPECT_CONTENT;
	ftetc.lindex = -1;

	ftetc.tymed = TYMED_ISTREAM;
	m_pDropTarget->AddSuportedFormat(ftetc);

	ftetc.tymed = TYMED_HGLOBAL;
	m_pDropTarget->AddSuportedFormat(ftetc);

	ftetc.cfFormat=CF_HDROP;
	m_pDropTarget->AddSuportedFormat(ftetc);

	ftetc.cfFormat = CF_BITMAP;
	ftetc.tymed = TYMED_GDI;
	m_pDropTarget->AddSuportedFormat(ftetc);

	return true;
}

// no background on these views (DirectX or OpenGM managed)
LRESULT CGEditorView::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CGEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	InitDragDrop();
	SetMsgHandled(FALSE);
	return 0;
}

LRESULT CGEditorView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	CleanSelection();
	OnChangeSel(OCS_AUTO);

	RevokeDragDrop(m_hWnd); //calls release
	m_pDropTarget=NULL;
	return 0;
}

LRESULT CGEditorView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	SetTimer(1, 1000/30);
	return 0;
}
LRESULT CGEditorView::OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	// stop the animations
	KillTimer(1);
	return 0;
}

LRESULT CGEditorView::OnDragLeave(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	m_dwTick = GetTickCount();
	return 0;
}
LRESULT CGEditorView::OnDragOver(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(GetTickCount() - m_dwTick > 500) {
		m_pDropSource->CancelDrag();
	}
	return 0;
}

LRESULT CGEditorView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(wParam == 1) { // Render stuf:
		if(GetParentFrame()->m_hWnd != GetMainFrame()->m_tabbedClient.GetTopWindow()) {
			KillTimer(1); 
		}
		if(m_bAnimated) Render(); // Animation
	} else if(wParam == 2) { // Drag and drop stuff:
		// cancel this timer
		KillTimer(2);
		if(m_DragState == tWaiting) {
			// now start draging
			BeginDrag();
		}
	} else if(wParam == 3) { // Scrolling stuff:
		// Simulate a mouse movement to update the object's position:
		PostMessage(WM_MOUSEMOVE, m_wMouseState, m_lMousePos);
	}
	return 0;
}
void CGEditorView::DoPaint(CDCHandle dc)
{
	Render();
}

LRESULT CGEditorView::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	LRESULT nResult = baseClass::OnVScroll(uMsg, wParam, lParam, bHandled);
	baseClass::SetFocus();
	UpdateView();
	return nResult;
}
LRESULT CGEditorView::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	LRESULT nResult = baseClass::OnHScroll(uMsg, wParam, lParam, bHandled);
	baseClass::SetFocus();
	UpdateView();
	return nResult;
}
LRESULT CGEditorView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	LRESULT nResult = baseClass::OnSize(uMsg, wParam, lParam, bHandled);
	UpdateView();
	return nResult;
}

LRESULT CGEditorView::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

	// We need to get the mouse position in the world and in the client area
	CPoint MousePoint(lParam);
	ScreenToClient(&MousePoint);
	CPoint WorldPoint(MousePoint);

	WorldPoint.x += GetScrollPos(SB_HORZ);
	WorldPoint.y += GetScrollPos(SB_VERT);
	WorldPoint.x /= m_Zoom;
	WorldPoint.y /= m_Zoom;

	if(zDelta>0) Zoom(m_Zoom * 1.5f);
	else Zoom(m_Zoom * 0.5f);

	CRect rcClient;
	GetClientRect(&rcClient);
	if((wParam&MK_CONTROL)==MK_CONTROL) {
		ScrollTo((int)((float)WorldPoint.x*m_Zoom)-MousePoint.x, (int)((float)WorldPoint.y*m_Zoom)-MousePoint.y);
	} else {
		ScrollTo((int)((float)WorldPoint.x*m_Zoom)-rcClient.CenterPoint().x, (int)((float)WorldPoint.y*m_Zoom)-rcClient.CenterPoint().y);
	}

	UpdateView();

	CURSOR Cursor;
	CPoint Point(lParam);
	ScreenToClient(&Point);
	GetMouseStateAt(Point, &Cursor);
	ToCursor(Cursor);

	return 0;
}
void CGEditorView::ScrollTo(int x, int y) 
{
	CRect rcClient;
	GetClientRect(&rcClient);
	CPoint ScrollPoint(x, y);

	// Now we validate the new scrolling position:
	if(ScrollPoint.x < m_rcScrollLimits.left) ScrollPoint.x = m_rcScrollLimits.left;
	if(ScrollPoint.y < m_rcScrollLimits.top) ScrollPoint.y = m_rcScrollLimits.top;
	if(ScrollPoint.x > m_rcScrollLimits.right) ScrollPoint.x = m_rcScrollLimits.right;
	if(ScrollPoint.y > m_rcScrollLimits.bottom) ScrollPoint.y = m_rcScrollLimits.bottom;
	SetScrollOffset(ScrollPoint);
}

CURSOR CGEditorView::ToCursor(CURSOR cursor_)
{
	HCURSOR hCursor;

	CURSOR OldCursor = m_CursorStatus;
	m_CursorStatus = cursor_;
	
	if(m_bPanning) {
		cursor_ = eIDC_HAND;
	}
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
		case eIDC_HAND:		hCursor = LoadCursor(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDC_MHAND)); break;
	}

	SetCursor(hCursor);
	return OldCursor;
}

LRESULT CGEditorView::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if(m_bFloating) return 0;

	CPoint Point(lParam);
	GetMouseStateAt(Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;

	if(m_bAllowMulSelection) {
		if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
		if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	}
	ToCursor(m_CursorStatus);
	
	GetWorldPosition(&Point);

	// call the update snap size
	UpdateSnapSize(m_bSnapToGrid?m_nSnapSize:1);

	// Is the cursor status ready to select objects?
	if((m_CursorStatus & ceToSelect) != 0) {
		if(m_bMulSelection || m_CursorStatus!=eIDC_ARROW || isHeld()) {
			StartSelBox(Point, &m_CursorStatus);
		} else  {
			SelectPoint(Point, &m_CursorStatus);
		}
	}

	if((m_CursorStatus & ceToMove) != 0) {
		StartMoving(Point, &m_CursorStatus);
		m_DragState = tToDrag;
	} else if(m_CursorStatus & ceToResize) {
		StartResizing(Point, &m_CursorStatus);
	}

	SetCapture();
	return 0;
}
LRESULT CGEditorView::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	GetMouseStateAt(Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;

	if(m_bAllowMulSelection) {
		if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
		if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	}
	ToCursor(m_CursorStatus);

	GetWorldPosition(&Point);

	// call the update snap size
	if((wParam&MK_CONTROL)==MK_CONTROL && m_bAllowSnapOverride) UpdateSnapSize(1);
	else UpdateSnapSize(m_bSnapToGrid?m_nSnapSize:1);

	if(isSelecting()) {
		if((wParam&MK_SHIFT)==MK_SHIFT || (wParam&MK_CONTROL)==0) {
			if((wParam&MK_SHIFT)==0) CleanSelection();
			OnChangeSel(OCS_AUTO, EndSelBoxAdd(Point, (LPARAM)((wParam&MK_CONTROL)==MK_CONTROL)?1:0));
		} else {
			OnChangeSel(OCS_AUTO, EndSelBoxRemove(Point, (LPARAM)((wParam&MK_CONTROL)==MK_CONTROL)?1:0));
		}
	} else if(isMoving() || isFloating()) {
		EndMoving(Point, NULL);
		if(m_bDuplicating) {
			Duplicate(Point);
		}
		OnChangeSel(OCS_UPDATE);
	} else if(isResizing()) {
		EndResizing(Point, NULL);
		OnChangeSel(OCS_UPDATE);
	}

	if(!isMoving() && !isResizing() && !isSelecting()) {
		m_DragState = tNone;
		ReleaseCapture();
	}

	Invalidate();
	return 0;
}
LRESULT CGEditorView::OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	GetMouseStateAt(Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;

	if(m_bAllowMulSelection) {
		if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
		if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	}
	ToCursor(m_CursorStatus);

	if(isSelecting()) {
		CancelSelBox();
	} else {
		GetWorldPosition(&Point);
		if(!SelectedAt(Point)) {
			CancelSelBox();
		}
	}

	SetCapture();

	return 0;
}
LRESULT CGEditorView::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CPoint Point(lParam);
	bool bInSelection = GetMouseStateAt(Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;

	if(m_bAllowMulSelection) {
		if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
		if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	}
	ToCursor(m_CursorStatus);

	if((wParam&MK_SHIFT)==MK_SHIFT && (wParam&MK_CONTROL)==MK_CONTROL) {
		GetWorldPosition(&Point);
		StartSelBox(Point, &m_CursorStatus);
		OnChangeSel(OCS_AUTO, EndSelBoxAdd(Point, -1));
	} else if((wParam&MK_SHIFT)==0 && (wParam&MK_CONTROL)==0) {
		if(!bInSelection && m_bMulSelection && m_bAllowMulSelection) {
			if(SelectedCount() == 0) {
				CPoint ScreenPoint(lParam);
				ClientToScreen(&ScreenPoint);
				PostMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(ScreenPoint.x, ScreenPoint.y));
			} else {
				if(!isFloating()) {
					CleanSelection();
					OnChangeSel(OCS_AUTO);
				} else return OnMouseMove(uMsg, wParam, lParam, bHandled);
			}
		} else {
			if(!bInSelection && (SelectedCount()==0 || !m_bMulSelection || !m_bAllowMulSelection)) {
				CleanSelection();
				GetWorldPosition(&Point);
				StartSelBox(Point, &m_CursorStatus);
				OnChangeSel(OCS_AUTO, EndSelBoxAdd(Point, 0));
			} 
			CPoint ScreenPoint(lParam);
			ClientToScreen(&ScreenPoint);
			PostMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(ScreenPoint.x, ScreenPoint.y));
		}
	}
	Invalidate();

	if(!isMoving() && !isResizing() && !isSelecting()) {
		m_DragState = tNone;
		ReleaseCapture();
	}

	return 0;
}
LRESULT CGEditorView::OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_PanningPoint.SetPoint(LOWORD(lParam), HIWORD(lParam));
	m_bPanning = true;
	ToCursor(m_CursorStatus);
	SetCapture();
	return 0;
}
LRESULT CGEditorView::OnMButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	m_bPanning = false;
	ToCursor(m_CursorStatus);
	if(!isMoving() && !isResizing() && !isSelecting()) {
		ReleaseCapture();
	} else {
		return OnMouseMove(uMsg, wParam, lParam, bHandled);
	}
	return 0;
}

CIDataObject* CGEditorView::CreateOleObj(CIDropSource *pDropSource)
{
	CIDataObject* pDataObject = new CIDataObject(pDropSource);
	if(pDataObject == NULL) return NULL;
	pDataObject->AddRef();

	BITMAP *pBitmap = CaptureSelection(0.0f);

	// Copy the current selection using pBitmap as the 
	// thumbnail. The memory gets freed the next time 
	// CaptureSelection is called.
	HGLOBAL hGlobal = CopySelection(&pBitmap, false);
	if(!hGlobal) return 0;

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

	HDC hDC = GetDC();
	HDC hDCMem = ::CreateCompatibleDC(hDC); 
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, pInfo->biWidth, pInfo->biHeight);
	HGDIOBJ hOldBmp = ::SelectObject(hDCMem, hBitmap);

	CRect Rect(0, 0, pBitmap->bmWidth, pBitmap->bmHeight);
	StretchDIBits(
		hDCMem,
		0,						// x-coordinate of upper-left corner of dest. rectangle
		0,						// y-coordinate of upper-left corner of dest. rectangle
		pBitmap->bmWidth,		// width of destination rectangle
		pBitmap->bmHeight,		// height of destination rectangle
		0,						// x-coordinate of upper-left corner of source rectangle
		0,						// y-coordinate of upper-left corner of source rectangle
		pBitmap->bmWidth,		// width of source rectangle
		pBitmap->bmHeight,		// height of source rectangle
		pBitmap->bmBits,		// address of bitmap bits
		&bmpInfo,				// address of bitmap data
		DIB_RGB_COLORS,			// usage flags
		SRCCOPY					// raster operation code
	);

	FORMATETC fmtetc = {0};
	STGMEDIUM medium = {0};
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	//////////////////////////////////////
	fmtetc.cfFormat = CF_BITMAP;
	fmtetc.tymed = TYMED_GDI;			
	medium.tymed = TYMED_GDI;
	HBITMAP hBitmapOle = (HBITMAP)OleDuplicateData(hBitmap, fmtetc.cfFormat, NULL);
	medium.hBitmap = hBitmapOle;
	pDataObject->SetData(&fmtetc, &medium, FALSE);
	//////////////////////////////////////
	fmtetc.cfFormat = CF_TEXT;
	fmtetc.tymed = TYMED_HGLOBAL;
	medium.tymed = TYMED_HGLOBAL;
	medium.hGlobal = hGlobal;
	pDataObject->SetData(&fmtetc, &medium, TRUE);
	//////////////////////////////////////
	fmtetc.cfFormat = CF_ENHMETAFILE;
	fmtetc.tymed = TYMED_ENHMF;
	medium.tymed = TYMED_ENHMF;
	HDC hMetaDC = CreateEnhMetaFile(hDC, NULL, NULL, NULL);
	::BitBlt(hMetaDC, 0, 0, Rect.Width(), Rect.Height(), hDCMem, 0, 0, SRCCOPY);
	medium.hEnhMetaFile = CloseEnhMetaFile(hMetaDC);
	pDataObject->SetData(&fmtetc, &medium, TRUE);
	//////////////////////////////////////
	::SelectObject(hDCMem, hOldBmp);
	::DeleteObject(hBitmap);
	::DeleteDC(hDCMem);
	ReleaseDC(hDC);

	if(pDropSource) {
		CDragSourceHelper dragSrcHelper;
		dragSrcHelper.InitializeFromBitmap(hBitmapOle, Rect.CenterPoint(), Rect, pDataObject, RGB(255,255,255)); //will own the bmp
	}

	return pDataObject;
}
LRESULT CGEditorView::BeginDrag()
{
	ReleaseCapture();

	m_pDropSource->InitDrag();

	CIDataObject *pDataObject = CreateOleObj(m_pDropSource);
	if(!pDataObject) return 0;

	m_DragState = tDragging;
	m_dwTick = GetTickCount();

	HoldOperation();
	Invalidate();
	UpdateWindow();

	DWORD dwEffect;
	HRESULT hr = ::DoDragDrop(pDataObject, m_pDropSource, DROPEFFECT_COPY, &dwEffect);
	pDataObject->Release();
	
	if(hr == DRAGDROP_S_DROP)  {
		m_DragState = tNone;
		CancelOperation();
	} else {
		m_DragState = tToDrag;
		SetCapture();
	}

	return 0;
}

#define SCROLL_SENSE 8 		// Scrolling sensitivity and speed at the frontiers of the client window
LRESULT CGEditorView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);

	m_wMouseState = wParam;
	m_lMousePos = lParam;

	if(m_bPanning) { // We are panning around, so just do that:
		if((wParam&MK_MBUTTON)!=MK_MBUTTON) {
			m_bPanning = false;
			ReleaseCapture();
		} else {
			CPoint ScrollPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
			ScrollPoint += (m_PanningPoint - Point);
			ScrollTo(ScrollPoint.x, ScrollPoint.y);
			m_PanningPoint = Point;
			UpdateView();
			return 0;
		}
	}

	CRect rcClient;
	CRect rcInflatedClient;
	bool bScroll = false;
	if(m_DragState != tNone || isFloating()) {
		GetClientRect(&rcClient);
		rcInflatedClient = rcClient;
		if(m_bAllowAutoScroll) {
			rcClient.DeflateRect(SCROLL_SENSE, SCROLL_SENSE);
			rcInflatedClient.InflateRect(SCROLL_SENSE, SCROLL_SENSE);
			if(!rcClient.PtInRect(Point)) bScroll = true;
		}
	}

	if(m_DragState != tNone && !isFloating()) {
		if(rcInflatedClient.PtInRect(Point)) {
			if(m_DragState == tWaiting) {
				m_DragState = tToDrag;
				KillTimer(2); // the mouse is back in the client area before timeout.
			}
		} else { // the mouse is out of the inflated client area
			bScroll = false;
			if(m_DragState == tToDrag) {
				m_DragState = tWaiting;
				SetTimer(2, 200); // set up a timer...
			}
		}
	}

	if(bScroll) {
		int nToScrollX = 0; // x ammount to scroll
		int nToScrollY = 0;	// y ammount to scroll

		if(Point.x > rcClient.right) {
			nToScrollX = Point.x - rcClient.right;
		} else if(Point.x < rcClient.left) {
			nToScrollX = Point.x - rcClient.left;
		}

		if(Point.y > rcClient.bottom) {
			nToScrollY = Point.y - rcClient.bottom;
		} else if(Point.y < rcClient.top) {
			nToScrollY = Point.y - rcClient.top;
		}

		if(nToScrollX || nToScrollY) {
			if(m_bScrolling) {
				if(nToScrollX > SCROLL_SENSE) nToScrollX = SCROLL_SENSE;
				else if(nToScrollX < -SCROLL_SENSE) nToScrollX = -SCROLL_SENSE;
				if(nToScrollY > SCROLL_SENSE) nToScrollY = SCROLL_SENSE;
				else if(nToScrollY < -SCROLL_SENSE) nToScrollY = -SCROLL_SENSE;

				nToScrollX += GetScrollPos(SB_HORZ);
				nToScrollY += GetScrollPos(SB_VERT);
				ScrollTo(nToScrollX, nToScrollY);
				UpdateView();
			}
			SetTimer(3, 100); // set up a timer...
			m_bScrolling = true;
		} else bScroll = false;
	}
	if(!bScroll && m_bScrolling) {
		m_bScrolling = false;
		KillTimer(3);
	}

	GetMouseStateAt(Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_LBUTTON)==0) {
		if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
		if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	}
	ToCursor(m_CursorStatus);

	GetWorldPosition(&Point);

	if((wParam&MK_LBUTTON)==MK_LBUTTON) {
		if((wParam&MK_CONTROL)==MK_CONTROL && m_bAllowSnapOverride) UpdateSnapSize(1);
		else UpdateSnapSize(m_bSnapToGrid?m_nSnapSize:1);

		if(isSelecting()) {
			SizeSelBox(Point, &m_CursorStatus);
		} else if(isMoving()) {
			MoveTo(Point, &m_CursorStatus);
		} else if(isResizing()) {
			ResizeTo(Point, &m_CursorStatus);
		}
		Invalidate();
	}

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("X: %3d, Y: %3d"), Point.x, Point.y);
	pStatusBar->SetPaneText(ID_POSITION_PANE, sText);

	if(GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) baseClass::SetFocus();

	// If no buttons are pressed, cancel the current operations and release the mouse capture:
	if((wParam&MK_LBUTTON)!=MK_LBUTTON && (wParam&MK_MBUTTON)!=MK_MBUTTON && (wParam&MK_RBUTTON)!=MK_RBUTTON) {
		if(isFloating()) {
			MoveTo(Point, &m_CursorStatus);
			Invalidate();
		} else {
			if(isMoving() || isResizing() || isSelecting()) {
				CancelOperation();
			}
			m_DragState = tNone;
			ReleaseCapture();
		}
	}

	return 0;
}
LRESULT CGEditorView::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	if(!isMoving() && !isResizing() && !isSelecting()) {
		if(wParam == VK_CONTROL) {
			if(m_CursorStatus != eIDC_ARROWADD) ToCursor(eIDC_ARROWDEL);
		} else if(wParam == VK_SHIFT) {
			ToCursor(eIDC_ARROWADD);
		} else if(wParam == VK_UP || wParam == VK_DOWN || wParam == VK_LEFT || wParam == VK_RIGHT) {
			CRect Rect;
			GetSelectionBounds(&Rect);
			UpdateSnapSize(1);
			StartMoving(Rect.TopLeft(), NULL);
			if(wParam == VK_UP) {
				Rect.OffsetRect(1,0);
			} else if(wParam == VK_DOWN) {
				Rect.OffsetRect(1,2);
			} else if(wParam == VK_LEFT) {
				Rect.OffsetRect(0,1);
			} else if(wParam == VK_RIGHT) {
				Rect.OffsetRect(2,1);
			}
			EndMoving(Rect.TopLeft(), NULL);
			OnChangeSel(OCS_UPDATE);
		}
	}

	return 0;
}
LRESULT CGEditorView::OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	if(wParam == VK_CONTROL) {
		if(m_CursorStatus == eIDC_ARROWDEL) ToCursor(m_OldCursorStatus);
	} else if(wParam == VK_SHIFT) {
		ToCursor(m_OldCursorStatus);
	} else if(wParam == VK_ESCAPE && !m_bPanning) {
		if(isSelecting()) CancelSelBox();
		else {
			bool bCancelOp = (isMoving() || isResizing()) && !isFloating();
			CancelOperation();
			m_DragState = tNone;
			m_bDuplicating = false;
			ReleaseCapture();
			if(!bCancelOp) OnChangeSel(OCS_AUTO);
		}
	} else if(wParam == VK_ADD || wParam == VK_OEM_PLUS) {
		ZoomIn();
	} else if(wParam == VK_SUBTRACT || wParam == VK_OEM_MINUS) {
		ZoomOut();
	} else if(wParam == VK_DELETE) {
		if(!isMoving() && !isResizing()) {
			if(DeleteSelection() <= 1) HoldSelection(false);
			OnChangeSel(OCS_RENEW);
		}
	}

	Invalidate();

	return 0;
}

LRESULT CGEditorView::OnDropObject(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(isHeld()) {
		CONSOLE_PRINTF("Warning: Can not paste objects while there is a held selection, release the selection first.\n");
		return 0;
	}

	::SetForegroundWindow(GetMainFrame()->m_hWnd);

	GetMainFrame()->BringWindowToTop();
	GetParentFrame()->BringWindowToTop();
	GetParentFrame()->SetFocus();

	PasteSelection((LPVOID)wParam, CPoint(lParam));
    SetCapture();

	OnChangeSel(OCS_AUTO);
	return 0;
}

bool CGEditorView::Delete()
{
	if(DeleteSelection() <= 1) HoldSelection(false);
	OnChangeSel(OCS_RENEW);
	return true;
}
bool CGEditorView::Duplicate(const CPoint &Point)
{
	ASSERT(!isHeld());
	m_bDuplicating = true;
	Copy();
	Paste(Point);
	return true;
}
bool CGEditorView::Duplicate()
{
	ASSERT(!isHeld());
	m_bDuplicating = true;
	CPoint Point;
	GetCursorPos(&Point);
	ScreenToClient(&Point);
	GetWorldPosition(&Point);
	Duplicate(Point);
	return true;
}
bool CGEditorView::Copy()
{
	// Using Ole clipboard:
	CIDataObject *pDataObject = CreateOleObj(NULL);
	if(!pDataObject) return false;

	OleSetClipboard(pDataObject);
	OleFlushClipboard(); //render the data
	pDataObject->Release();

	Invalidate();
	return true;
}
bool CGEditorView::Cut()
{
	ASSERT(!isHeld());
	if(!Copy()) return false;
	DeleteSelection();
	Invalidate();
	OnChangeSel(OCS_AUTO);
	return true;
}
bool CGEditorView::Paste()
{
	ASSERT(!isHeld());
	CPoint Point;
	GetCursorPos(&Point);
	ScreenToClient(&Point);
	GetWorldPosition(&Point);
	Paste(Point);
	return true;
}
bool CGEditorView::SingleSel()
{
	m_bMulSelection = false;
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CGEditorView::MultipleSel()
{
	m_bMulSelection = true;
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CGEditorView::SelectAll()
{
	Invalidate();
	OnChangeSel(OCS_AUTO);
	return true;
}
bool CGEditorView::SelectNone()
{
	CleanSelection();
	Invalidate();
	OnChangeSel(OCS_AUTO);
	return true;
}
bool CGEditorView::Zoom(float zoom)
{
	float oldZoom = m_Zoom;
	bool flip = false;

	m_Zoom = zoom;

	if(m_Zoom < 1.0f) {
		m_Zoom = 1.0f/m_Zoom;
		flip=true;
	}

	m_Zoom = (float)(int)(m_Zoom+0.5);
	if(flip) m_Zoom = 1.0f/m_Zoom;

	if(m_Zoom>10.0f) m_Zoom = 10.0f;
	if(m_Zoom<0.10f) m_Zoom = 0.10f;

	if(oldZoom == m_Zoom) return 0;

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("%4d%%"), (int)(100.0f * m_Zoom));
	pStatusBar->SetPaneText(ID_OVERTYPE_PANE, sText);

	// We need to recalculate the new map size (in pixeles)
	CalculateLimits();
	SetScrollSize(m_rcScrollLimits.Size());
	return true;
}
bool CGEditorView::NoZoom()
{
	CPoint Point(
		(int)((float)GetScrollPos(SB_HORZ)/m_Zoom), 
		(int)((float)GetScrollPos(SB_VERT)/m_Zoom)
	);

	Zoom(1.0f);

	ScrollTo(
		(int)((float)Point.x*m_Zoom), 
		(int)((float)Point.y*m_Zoom)
	);
	UpdateView();
	return true;
}
bool CGEditorView::ZoomIn()
{
	CPoint Point(
		(int)((float)GetScrollPos(SB_HORZ)/m_Zoom), 
		(int)((float)GetScrollPos(SB_VERT)/m_Zoom)
	);

	Zoom(m_Zoom * 1.5f);

	ScrollTo(
		(int)((float)Point.x*m_Zoom), 
		(int)((float)Point.y*m_Zoom)
	);
	UpdateView();
	return true;
}
bool CGEditorView::ZoomOut()
{
	CPoint Point(
		(int)((float)GetScrollPos(SB_HORZ)/m_Zoom), 
		(int)((float)GetScrollPos(SB_VERT)/m_Zoom)
	);

	Zoom(m_Zoom * 0.5f);

	ScrollTo(
		(int)((float)Point.x*m_Zoom), 
		(int)((float)Point.y*m_Zoom)
	);
	UpdateView();
	return true;
}
bool CGEditorView::ToggleGrid()
{
	m_bShowGrid = !m_bShowGrid;
	UpdateSnapSize(m_nSnapSize);
	Invalidate();
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CGEditorView::ToggleSnap()
{
	m_bSnapToGrid = !m_bSnapToGrid;
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CGEditorView::ToggleHold()
{
	if(SelectedCount() <= 1) return false;

	if(isHeld()) HoldSelection(false);
	else HoldSelection(true);
	return true;
}

inline bool CGEditorView::Paste(const CPoint &Point)
{
	ASSERT(!isHeld());
	// Using Ole clipboard:
	FORMATETC fmtetc = {0};
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	fmtetc.cfFormat = CF_TEXT;
	fmtetc.tymed = TYMED_HGLOBAL;

	CIDataObject *pGetDataObject = NULL;
	OleGetClipboard((LPDATAOBJECT*) &pGetDataObject);
	if(pGetDataObject) {
		if(pGetDataObject->QueryGetData(&fmtetc) == S_OK) {
			STGMEDIUM medium;
			pGetDataObject->GetData(&fmtetc, &medium);
			m_pDropTarget->DropAt(&fmtetc, medium, MAKELPARAM(Point.x,Point.y));
		}
		pGetDataObject->Release();
	}

	Invalidate();
	return true;
}
