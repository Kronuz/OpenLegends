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
/*! \file		WorldEditorView.cpp 
	\brief		Implementation of the CWorldEditorView class.
	\date		April 15, 2003
*/

#include "stdafx.h"

#include "MainFrm.h"
#include "WorldEditorView.h"
#include "WorldEditorFrm.h"

CWorldEditorView::CWorldEditorView(CWorldEditorFrame *pParentFrame) :
	CChildView(pParentFrame),
	m_bClean(true),
	m_bPanning(false),
	m_CursorStatus(eIDC_ARROW),
	m_szMap(0, 0),
	m_Zoom(4),
	m_MousePoint(0,0),
	m_MapPoint(0,0),
	m_pSelMapGroup(NULL),
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

// Called to translate window messages before they are dispatched 
BOOL CWorldEditorView::PreTranslateMessage(MSG *pMsg)
{
	return FALSE;
}
// Called to clean up after window is destroyed (called when WM_NCDESTROY is sent)
void CWorldEditorView::OnFinalMessage(HWND /*hWnd*/)
{
	delete this;
}

LRESULT CWorldEditorView::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CWorldEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	LRESULT nResult = DefWindowProc();

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
	CSize szMap(((m_szMap.cx+m_Zoom-1)/m_Zoom), (m_szMap.cy+m_Zoom-1)/m_Zoom);
	m_WorldFullSize.cx = szMap.cx*m_szWorld.cx;
	m_WorldFullSize.cy = szMap.cy*m_szWorld.cy;
	SetScrollSize(m_WorldFullSize);

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

//-----------------------------------------------------------------------------
// Name: DrawRect()
// Desc: 
//-----------------------------------------------------------------------------
void DrawRect(CDC &dc, CRect rcRect)
{
	POINT points[] = {
		{ rcRect.left,	rcRect.top },
		{ rcRect.right,	rcRect.top }, 
		{ rcRect.right,	rcRect.bottom },
		{ rcRect.left,	rcRect.bottom },
		{ rcRect.left,	rcRect.top }
	};
	dc.Polyline(points, 5);
}

void CWorldEditorView::DoPaint(CDCHandle dc)
{
	{ // scope for dcMem, needed for the UpdateMouse() below to really update
		// create memory DC
		CMemDC dcMem(dc, NULL);

		CRect rcClient;
		GetClientRect(&rcClient);
		rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

		dcMem.FillRect(rcClient, -1);

		CSize szMap(((m_szMap.cx+m_Zoom-1)/m_Zoom), (m_szMap.cy+m_Zoom-1)/m_Zoom);

		// Draw Grid in the Background
		HPEN oldPen = dcMem.SelectPen(m_hPenGrid);
		
		for(int x = (rcClient.left / szMap.cx) * szMap.cx; x <= rcClient.right; x += szMap.cx) {	// Vertical
			dcMem.MoveTo(x, rcClient.top);
			dcMem.LineTo(x, rcClient.bottom);
		}
		
		for(int y = (rcClient.top/szMap.cy)*szMap.cy; y <= rcClient.bottom; y += szMap.cy) {	// Horizontal
			dcMem.MoveTo(rcClient.left,  y);
			dcMem.LineTo(rcClient.right, y);
		}

		HFONT oldFont;
		if(m_Zoom>8) {
			oldFont = dcMem.SelectFont(m_hFont6);
		} else if(m_Zoom>6) {
			oldFont = dcMem.SelectFont(m_hFont8);
		} else if(m_Zoom>4) {
			oldFont = dcMem.SelectFont(m_hFont10);
		} else {
			oldFont = dcMem.SelectFont(m_hFont12);
		}

		CString strOut;
		CMapGroup *pMapGroup;
		dcMem.SetTextColor(RGB(0, 0, 0));	// Text settings
		dcMem.SetBkMode(TRANSPARENT);
		for(x=rcClient.left/szMap.cx; x<=rcClient.right/szMap.cx; x++) {
			for(y=rcClient.top/szMap.cy; y<=rcClient.bottom/szMap.cy; y++) {
				if((pMapGroup = DrawThumbnail(dcMem, x, y, szMap)) == NULL) {
					strOut.Format(_T("%d,%d"), x, y);
					dcMem.TextOut(x*szMap.cx+2,y*szMap.cy+2, strOut);
				} else {
					if(pMapGroup->isMapGroupHead(x,y)) {
						dcMem.SetTextColor(RGB(255, 255, 225));
						strOut.Format(_T("%d,%d: %s"), x, y, pMapGroup->GetMapGroupID());
						dcMem.TextOut(x*szMap.cx+5,y*szMap.cy+5, strOut);
						dcMem.SetTextColor(RGB(0, 0, 0));
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
					DrawRect(dcMem, Rect);
				}
			}
		}

		// Reselect GDI objects
		dcMem.SelectPen(oldPen);
		dcMem.SelectFont(oldFont);
	}

	m_bClean = true;
	UpdateSelections(); // updates the selections (after bliting the memDC)
}

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

LRESULT CWorldEditorView::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	int oldZoom = m_Zoom;

	if(zDelta<0) {
		if(++m_Zoom > 20) m_Zoom=20;
	} else {
		if(--m_Zoom < 2) m_Zoom=2;
	}
	if(oldZoom == m_Zoom) return 0;

	CPoint MousePoint(lParam);
	ScreenToClient(&MousePoint);
	CPoint WorldPoint(MousePoint);

	WorldPoint.x += GetScrollPos(SB_HORZ);
	WorldPoint.y += GetScrollPos(SB_VERT);
	WorldPoint.x *= oldZoom;
	WorldPoint.y *= oldZoom;

	CSize szMap(((m_szMap.cx+m_Zoom-1)/m_Zoom), (m_szMap.cy+m_Zoom-1)/m_Zoom);
	m_WorldFullSize.cx = szMap.cx*m_szWorld.cx;
	m_WorldFullSize.cy = szMap.cy*m_szWorld.cy;
	
	// Now we validate the new scrolling position:
	CPoint ScrollPoint;

	CRect rcClient;
	GetClientRect(&rcClient);
	if((wParam&MK_CONTROL)==MK_CONTROL || m_bPanning) {
		ScrollPoint.SetPoint(WorldPoint.x/m_Zoom-MousePoint.x, WorldPoint.y/m_Zoom-MousePoint.y);
	} else {
		ScrollPoint.SetPoint(WorldPoint.x/m_Zoom-rcClient.CenterPoint().x, WorldPoint.y/m_Zoom-rcClient.CenterPoint().y);
	}

	SetScrollSize(m_WorldFullSize, FALSE);

	if(ScrollPoint.x < 0) ScrollPoint.x = 0;
	if(ScrollPoint.y < 0) ScrollPoint.y = 0;
	if(ScrollPoint.x > m_WorldFullSize.cx-rcClient.right) ScrollPoint.x = m_WorldFullSize.cx-rcClient.right;
	if(ScrollPoint.y > m_WorldFullSize.cy-rcClient.bottom) ScrollPoint.y = m_WorldFullSize.cy-rcClient.bottom;
	SetScrollOffset(ScrollPoint);

	return 0;
}

LRESULT CWorldEditorView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	if(m_bPanning) {
		if((wParam&MK_MBUTTON)!=MK_MBUTTON) {
			m_bPanning = false;
			ReleaseCapture();
		} else {
			CPoint ScrollPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
			ScrollPoint += (m_PanningPoint - Point);

			CRect rcClient;
			GetClientRect(&rcClient);
			if(ScrollPoint.x < 0) ScrollPoint.x = 0;
			if(ScrollPoint.y < 0) ScrollPoint.y = 0;
			if(ScrollPoint.x > m_WorldFullSize.cx-rcClient.right) ScrollPoint.x = m_WorldFullSize.cx-rcClient.right;
			if(ScrollPoint.y > m_WorldFullSize.cy-rcClient.bottom) ScrollPoint.y = m_WorldFullSize.cy-rcClient.bottom;

			SetScrollOffset(ScrollPoint);
			m_PanningPoint = Point;
			Invalidate();
			return 0;
		}
	}

	Point.Offset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	Point.x *= m_Zoom;
	Point.y *= m_Zoom;

	UpdateMouse(Point);

	if(GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) baseClass::SetFocus();

	return 0;
}

bool CWorldEditorView::ScrollTo(CPoint &point, CRect &rcClient, CSize &szMap)
{
	bool bScroll = false;
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	CPoint ScrollPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	int xPos = point.x/m_Zoom;
	int yPos = point.y/m_Zoom;
	if(xPos < rcClient.left) {
		int delta = ((rcClient.left - xPos + szMap.cx - 1)/szMap.cx)*szMap.cx;
		ScrollPoint.x -= delta;
		bScroll = true;
	} 
	if(xPos+szMap.cx > rcClient.right) {
		int delta = ((xPos - rcClient.right + 2*szMap.cx - 1)/szMap.cx)*szMap.cx;
		ScrollPoint.x += delta;
		bScroll = true;
	} 
	if(yPos < rcClient.top) {
		int delta = ((rcClient.top - yPos + szMap.cy - 1)/szMap.cy)*szMap.cy;
		ScrollPoint.y -= delta;
		bScroll = true;
	} 
	if(yPos+szMap.cy > rcClient.bottom) {
		int delta = ((yPos - rcClient.bottom + 2*szMap.cy - 1)/szMap.cy)*szMap.cy;
		ScrollPoint.y += delta;
		bScroll = true;
	} 

	UpdateMouse(point);
	if(bScroll == false) return false;

	rcClient.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));

	if(ScrollPoint.x < 0) ScrollPoint.x = 0;
	if(ScrollPoint.y < 0) ScrollPoint.y = 0;
	if(ScrollPoint.x > m_WorldFullSize.cx-rcClient.right) ScrollPoint.x = m_WorldFullSize.cx-rcClient.right;
	if(ScrollPoint.y > m_WorldFullSize.cy-rcClient.bottom) ScrollPoint.y = m_WorldFullSize.cy-rcClient.bottom;
	SetScrollOffset(ScrollPoint);

	return true;
}
LRESULT CWorldEditorView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(GetMainFrame()->GetOldFocus(tAny) == m_hWnd) return 0;
	GetMainFrame()->SetOldFocus(tWorldEditor, m_hWnd);

	HWND hWnd = GetMainFrame()->m_hWnd;
	::SendMessage(hWnd, WMP_CLEAR, 0, 0);

	Invalidate();

	return 0;
}
LRESULT CWorldEditorView::OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	return 0;
}
LRESULT CWorldEditorView::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	CSize szMap(((m_szMap.cx+m_Zoom-1)/m_Zoom), (m_szMap.cy+m_Zoom-1)/m_Zoom);

	CPoint point(m_MousePoint);
	if(wParam == VK_RETURN) {
		if(m_pSelMapGroup == NULL) {
			GetMainFrame()->MapFileOpen(m_MapPoint);
		}
		m_pSelMapGroup = NULL;
		Invalidate();
	} else if(wParam == VK_ESCAPE) {
		m_pSelMapGroup = NULL;
		Invalidate();
	} else if(wParam == VK_LEFT) {
		if(m_pSelMapGroup) {
			CRect Rect;
			m_pSelMapGroup->GetMapGroupRect(Rect);
			point.x = Rect.left * m_szMap.cx + m_szMap.cx/2;
			point.y = Rect.top * m_szMap.cy + m_szMap.cy/2;

			m_pSelMapGroup->OffsetMapGroup(-1,0);
			Invalidate();
		}
		point.x -= m_szMap.cx;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_RIGHT) {
		if(m_pSelMapGroup) {
			CRect Rect;
			m_pSelMapGroup->GetMapGroupRect(Rect);
			point.x = Rect.left * m_szMap.cx + m_szMap.cx/2;
			point.y = Rect.top * m_szMap.cy + m_szMap.cy/2;

			m_pSelMapGroup->OffsetMapGroup(1,0);
			Invalidate();
		}
		point.x += m_szMap.cx;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_UP) {
		if(m_pSelMapGroup) {
			CRect Rect;
			m_pSelMapGroup->GetMapGroupRect(Rect);
			point.x = Rect.left * m_szMap.cx + m_szMap.cx/2;
			point.y = Rect.top * m_szMap.cy + m_szMap.cy/2;

			m_pSelMapGroup->OffsetMapGroup(0,-1);
			Invalidate();
		}
		point.y -= m_szMap.cy;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_DOWN) {
		if(m_pSelMapGroup) {
			CRect Rect;
			m_pSelMapGroup->GetMapGroupRect(Rect);
			point.x = Rect.left * m_szMap.cx + m_szMap.cx/2;
			point.y = Rect.top * m_szMap.cy + m_szMap.cy/2;

			m_pSelMapGroup->OffsetMapGroup(0,1);
			Invalidate();
		}
		point.y += m_szMap.cy;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_NEXT) {
		point.y += (rcClient.bottom/szMap.cy)*szMap.cy*m_Zoom;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_PRIOR) {
		point.y -= (rcClient.bottom/szMap.cy)*szMap.cy*m_Zoom;
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
	return 0;
}

// point contains the x,y coordinates of the map, in world map coordinates. 
// i.e. 1,0 is the second map to the right.
void CWorldEditorView::UpdateSelections()
{
	if(m_bPanning) return;
	////////////////////////
	CSize szMap(((m_szMap.cx+m_Zoom-1)/m_Zoom), (m_szMap.cy+m_Zoom-1)/m_Zoom);
	CPoint Point( (m_MapPoint.x)*(szMap.cx) + (szMap.cx)/2, (m_MapPoint.y)*(szMap.cy) + (szMap.cy)/2 );

	static CPoint sMap;

	CRect rcSelect;
	CDCHandle dc = GetDC();
	
	int iBox;
	// find out the size of the little selection box:
	if(m_Zoom == 1) {
		iBox = 5;
	} else if(m_Zoom == 2) {
		iBox = 4;
	} else if(m_Zoom == 3) {
		iBox = 3;
	} else if(m_Zoom > 8) {
		iBox = 1;
	} else {
		iBox = 16/m_Zoom;
	}

	if(Point != sMap && !m_bClean) {
		rcSelect.top = sMap.y-iBox;
		rcSelect.bottom = sMap.y+iBox;
		rcSelect.left = sMap.x-iBox;
		rcSelect.right = sMap.x+iBox;
		rcSelect.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));

		CRect rcClient;
		GetClientRect(&rcClient);
		rcSelect &= rcClient;

		if(!rcSelect.IsRectEmpty()) {
			InvalidateRect(rcSelect);
		}
	}

	sMap = Point;
	m_bClean = false;

	rcSelect.top = Point.y-iBox;
	rcSelect.bottom = Point.y+iBox;
	rcSelect.left = Point.x-iBox;
	rcSelect.right = Point.x+iBox;

	rcSelect.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));

	dc.FillRect(rcSelect, 1);

/////////////////////

	static CMapGroup *pOldMapGroup = NULL;

	CMapGroup *pMapGroup = NULL;
	if(m_pSelMapGroup) {
		if(m_pSelMapGroup->isMapGroupAt(m_MapPoint.x, m_MapPoint.y)) {
			pMapGroup = m_pSelMapGroup;
		} 
	}
	if(pMapGroup == NULL) pMapGroup = CProjectFactory::Interface()->FindMapGroup(m_MapPoint.x, m_MapPoint.y);

	CRect Rect;
	HPEN oldPen = dc.SelectPen(m_hPenMapGroup);

	if(pOldMapGroup != pMapGroup && pOldMapGroup) {
		if(pOldMapGroup == m_pSelMapGroup) dc.SelectPen(m_hPenMapGroupSelected);
		pOldMapGroup->GetMapGroupRect(Rect);
		Rect.top *= szMap.cy;
		Rect.left *= szMap.cx;
		Rect.bottom *= szMap.cy;
		Rect.right *= szMap.cx;
		Rect.DeflateRect(4,4);
		Rect.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));
		DrawRect((CDC)dc, Rect);
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
		DrawRect((CDC)dc, Rect);
	}
	pOldMapGroup = pMapGroup;

	dc.SelectPen(oldPen);

	ReleaseDC(dc);
}

CURSOR CWorldEditorView::ToCursor(CURSOR cursor_)
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

void CWorldEditorView::UpdateMouse(const CPoint &point)
{
	if( point.x<0 || point.x>m_WorldFullSize.cx*m_Zoom ||
		point.y<0 || point.y>m_WorldFullSize.cy*m_Zoom ) return;

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("X: %3d, Y: %3d"), point.x, point.y);
	pStatusBar->SetPaneText(ID_POSITION_PANE, sText);

	////////////////////////
	m_MousePoint = point;

	CSize szMap(((m_szMap.cx+m_Zoom-1)/m_Zoom), (m_szMap.cy+m_Zoom-1)/m_Zoom);

	m_MapPoint.x = point.x/(m_Zoom*szMap.cx);
	m_MapPoint.y = point.y/(m_Zoom*szMap.cy);

	UpdateSelections();
}
LRESULT CWorldEditorView::OnLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	Point.Offset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	Point.x /= (m_szMap.cx/m_Zoom);
	Point.y /= (m_szMap.cy/m_Zoom);
	GetMainFrame()->MapFileOpen(Point);
	m_pSelMapGroup = NULL;
	Invalidate();
	return 0;
}
LRESULT CWorldEditorView::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CWorldEditorView::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	Point.Offset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	Point.x /= (m_szMap.cx/m_Zoom);
	Point.y /= (m_szMap.cy/m_Zoom);
	CMapGroup *pMapGroup = CProjectFactory::Interface()->FindMapGroup(Point.x, Point.y);
	if(m_pSelMapGroup == pMapGroup) {
		m_pSelMapGroup = NULL;
	} else m_pSelMapGroup = pMapGroup;
	Invalidate();
	return 0;
}
LRESULT CWorldEditorView::OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CWorldEditorView::OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	Point.Offset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	Point.x /= (m_szMap.cx/m_Zoom);
	Point.y /= (m_szMap.cy/m_Zoom);
	CMapGroup *pMapGroup = CProjectFactory::Interface()->FindMapGroup(Point.x, Point.y);
	if(m_pSelMapGroup != pMapGroup) {
		m_pSelMapGroup = NULL;
		Invalidate();
	}
	return 0;
}

LRESULT CWorldEditorView::OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_PanningPoint.SetPoint(LOWORD(lParam), HIWORD(lParam));

	m_bPanning = true;
	ToCursor(m_CursorStatus);
	SetCapture();

	return 0;
}
LRESULT CWorldEditorView::OnMButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_bPanning = false;
	ToCursor(m_CursorStatus);
	ReleaseCapture();
	Invalidate();
	return 0;
}

bool CWorldEditorView::hasChanged()
{
	return false;
}
