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
/*! \file		WorldEditorView.cpp 
	\brief		Implementation of the CWorldEditorView class.
	\date		April 15, 2003
*/

#include "stdafx.h"

#include "MainFrm.h"
#include "WorldEditorView.h"
#include "WorldEditorFrm.h"

#define MAXMAPSX 256
#define MAXMAPSY 256

CWorldEditorView::CWorldEditorView(CWorldEditorFrame *pParentFrame) :
	CChildView(pParentFrame),
	m_bClean(true),
	m_szMap(0, 0),
	m_Zoom(4),
	m_MousePoint(0,0),
	m_pSelMapGroup(NULL)
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
	m_hPenMapGroupSelected = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));

	m_hFont12 =  CreateFont(12, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");
	m_hFont10 =  CreateFont(10, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");
	m_hFont8 =   CreateFont(8, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");
	m_hFont6 =   CreateFont(6, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");

	CProjectFactory::Interface()->GetMapSize(m_szMap);
	CSize szMap(((m_szMap.cx+m_Zoom-1)/m_Zoom), (m_szMap.cy+m_Zoom-1)/m_Zoom);
	m_WorldSize.cx = szMap.cx*MAXMAPSX;
	m_WorldSize.cy = szMap.cy*MAXMAPSY;
	SetScrollSize(m_WorldSize);

	return nResult;
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
	// First we need to get the current client window,
	// and relocate ourselves to use the scrolling position

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

	dcMem.SelectPen(m_hPenMapGroup);

	CBString strOut;
	CMapGroup *MapGroup;
	dcMem.SetTextColor(RGB(0, 0, 0));	// Text settings
	dcMem.SetBkMode(TRANSPARENT);
	for(x=rcClient.left/szMap.cx; x<=rcClient.right/szMap.cx; x++) {
		for(y=rcClient.top/szMap.cy; y<=rcClient.bottom/szMap.cy; y++) {
			if((MapGroup = DrawThumbnail(dcMem, x, y, szMap)) == NULL) {
				strOut.Format(_T("%d,%d"), x, y);
				dcMem.TextOut(x*szMap.cx+2,y*szMap.cy+2, strOut);
			} else {
				if(MapGroup->isMapGroupHead(x,y)) {
					dcMem.SetTextColor(RGB(255, 255, 225));
					strOut.Format(_T("%d,%d: %s"), x, y, MapGroup->GetMapGroupID());
					dcMem.TextOut(x*szMap.cx+5,y*szMap.cy+5, strOut);
					dcMem.SetTextColor(RGB(0, 0, 0));
				}
				CRect Rect;
				MapGroup->GetMapGroupRect(Rect);
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
	
	m_bClean = true;
	UpdateMouse(m_MousePoint);
}

CMapGroup* CWorldEditorView::DrawThumbnail(CDC &dc, int x, int y, CSize &szMap)
{
	CMapGroup *MapGroup = CProjectFactory::Interface()->FindMapGroup(x,y);
	if(!MapGroup) return NULL;

	CRect Rect;
	BITMAP *pBitmap = MapGroup->GetThumbnail(&Rect, x, y);
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
		MapGroup->GetMapGroupSize(szMapGroupSize);

		DWORD dwRop = (m_pSelMapGroup==MapGroup)?NOTSRCCOPY:SRCCOPY;

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
	return MapGroup;
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
	m_WorldSize.cx = szMap.cx*MAXMAPSX;
	m_WorldSize.cy = szMap.cy*MAXMAPSY;
	
	// Now we validate the new scrolling position:
	CRect rcClient;
	GetClientRect(&rcClient);
	CPoint ScrollPoint(WorldPoint.x/m_Zoom-MousePoint.x, WorldPoint.y/m_Zoom-MousePoint.y);
	SetScrollSize(m_WorldSize, FALSE);

	if(ScrollPoint.x < 0) ScrollPoint.x = 0;
	if(ScrollPoint.y < 0) ScrollPoint.y = 0;
	if(ScrollPoint.x > m_WorldSize.cx-rcClient.right) ScrollPoint.x = m_WorldSize.cx-rcClient.right;
	if(ScrollPoint.y > m_WorldSize.cy-rcClient.bottom) ScrollPoint.y = m_WorldSize.cy-rcClient.bottom;
	SetScrollOffset(ScrollPoint);

	Invalidate();   
	UpdateWindow();

	return 0;
}

LRESULT CWorldEditorView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint point(lParam);
	point.x += GetScrollPos(SB_HORZ);
	point.y += GetScrollPos(SB_VERT);

	point.x *= m_Zoom;
	point.y *= m_Zoom;

	UpdateMouse(point);

	return 0;
}

bool CWorldEditorView::ScrollTo(CPoint &point, CRect &rcClient, CSize &szMap)
{
	bool bScroll = false;
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	CPoint ScrollPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	int xPos = ((point.x/m_Zoom)/szMap.cx)*szMap.cx;
	int yPos = ((point.y/m_Zoom)/szMap.cy)*szMap.cy;
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
	if(ScrollPoint.x > m_WorldSize.cx-rcClient.right) ScrollPoint.x = m_WorldSize.cx-rcClient.right;
	if(ScrollPoint.y > m_WorldSize.cy-rcClient.bottom) ScrollPoint.y = m_WorldSize.cy-rcClient.bottom;
	SetScrollOffset(ScrollPoint);

	UpdateWindow();

	return true;
}
LRESULT CWorldEditorView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	Invalidate();
	UpdateWindow();
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
	} else if(wParam == VK_LEFT) {
		point.x -= m_szMap.cx;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_RIGHT) {
		point.x += m_szMap.cx;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_UP) {
		point.y -= m_szMap.cy;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_DOWN) {
		point.y += m_szMap.cy;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_NEXT) {
		point.y += (rcClient.bottom/szMap.cy)*szMap.cy*m_Zoom;
		ScrollTo(point, rcClient, szMap);
	} else if(wParam == VK_PRIOR) {
		point.y -= (rcClient.bottom/szMap.cy)*szMap.cy*m_Zoom;
		ScrollTo(point, rcClient, szMap);
	} else {
		bHandled = FALSE;
	}
	return 0;
}

void CWorldEditorView::UpdateMouse(CPoint point)
{
	if( point.x<0 || point.x>m_WorldSize.cx*m_Zoom ||
		point.y<0 || point.y>m_WorldSize.cy*m_Zoom ) return;

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CBString sText;
	sText.Format(_T("X: %3d, Y: %3d"), point.x, point.y);
	pStatusBar->SetPaneText(ID_POSITION_PANE, sText);

	////////////////////////
	m_MousePoint = point;

	point.x /= m_Zoom;
	point.y /= m_Zoom;
	CSize szMap(((m_szMap.cx+m_Zoom-1)/m_Zoom), (m_szMap.cy+m_Zoom-1)/m_Zoom);

	point.x = (point.x/szMap.cx)*(szMap.cx) + (szMap.cx)/2;
	point.y = (point.y/szMap.cy)*(szMap.cy) + (szMap.cy)/2;

	static CPoint sMap;

	CRect rcSelect;
	CDCHandle dc = GetDC();
	
	int iBox;
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

	if(point != sMap && !m_bClean) {
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

	sMap = point;
	m_bClean = false;


	rcSelect.top = point.y-iBox;
	rcSelect.bottom = point.y+iBox;
	rcSelect.left = point.x-iBox;
	rcSelect.right = point.x+iBox;

	rcSelect.OffsetRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));

	dc.FillRect(rcSelect, 1);
	ReleaseDC(dc);
}
LRESULT CWorldEditorView::OnLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);
	Point.Offset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	Point.x /= (m_szMap.cx/m_Zoom);
	Point.y /= (m_szMap.cy/m_Zoom);
	GetMainFrame()->MapFileOpen(Point);
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
	m_pSelMapGroup = CProjectFactory::Interface()->FindMapGroup(Point.x, Point.y);
	Invalidate();
	return 0;
}
LRESULT CWorldEditorView::OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CWorldEditorView::OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CWorldEditorView::OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CWorldEditorView::OnMButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return 0;
}


