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
// ScriptEditorView.cpp : implementation of the CScriptEditorView class
//

#include "stdafx.h"

#include "WorldEditorView.h"
#include "WorldEditorFrm.h"

CWorldEditorView::CWorldEditorView(CWorldEditorFrame *pParentFrame) :
m_pParentFrame(pParentFrame)
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
/*
	RECT rect;
	GetClientRect(&rect);
	int x = 0;
	int y = 0;

	x = 100;
	y = 100; 

	CDCHandle dc = (HDC)wParam;
	if(rect.right > m_sizeAll.cx)
	{
		RECT rectRight = rect;
		rectRight.left = x;
		rectRight.bottom = y;
		dc.FillRect(&rectRight, (HBRUSH)(COLOR_WINDOW + 1));
	}
	if(rect.bottom > m_sizeAll.cy)
	{
		RECT rectBottom = rect;
		rectBottom.top = y;
		dc.FillRect(&rectBottom, (HBRUSH)(COLOR_WINDOW + 1));
	}

	dc.MoveTo(0, 0);
	dc.LineTo(20, 20);
	dc.LineTo(0, 20);
*/
	return 0;
}

LRESULT CWorldEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	LONG lRet = DefWindowProc();

	m_szWorld.cx = 40960;
	m_szWorld.cy = 30720;

	m_hPenGrid = CreatePen(PS_SOLID, 1, RGB(127, 180, 127));
	m_hFont = CreateFont(12, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");

	SetScrollSize(m_szWorld);

	return lRet;
}

void CWorldEditorView::DoPaint(CDCHandle dc)
{
	// First we need to get the current client window,
	// and relocate ourselves to use the scrolling position
	CRect rcClient;
	GetClientRect(&rcClient);
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	// now we get a compatible device context to use as a
	// back buffer. Fill everything with white:
	//CDC dcMem;
	//dcMem.CreateCompatibleDC(dc);
	dc.FillRect(rcClient, RGB(255, 255, 255));

	CSize szMap(160*4, 120*4);	// Get the map dimensions
	szMap.cx /= 4; szMap.cy /= 4;

	// Draw Grid in the Background
	HPEN oldPen = dc.SelectPen(m_hPenGrid);
	for(int x = (rcClient.left/szMap.cx)*szMap.cx; x <= rcClient.right; x += szMap.cx) {	// Vertical
		dc.MoveTo(x, rcClient.top);
		dc.LineTo(x, rcClient.bottom);
	}
	for(int y = (rcClient.top/szMap.cy)*szMap.cy; y <= rcClient.bottom; y += szMap.cy) {	// Horizontal
		dc.MoveTo(rcClient.left,  y);
		dc.LineTo(rcClient.right, y);
	}

	// Draw the map position string (draws only onscreen text)
	CString strOut;
	HFONT oldFont = dc.SelectFont(m_hFont);
	dc.SetTextColor(RGB(0, 0, 0));	// Text settings
	dc.SetBkMode(TRANSPARENT);
	for(x=rcClient.left/szMap.cx; x<=rcClient.right/szMap.cx; x++) {
		for(y=rcClient.top/szMap.cy; y<=rcClient.bottom/szMap.cy; y++) {
			strOut.Format("%d,%d", x, y);
			dc.TextOut(x*szMap.cx+2,y*szMap.cy+2, strOut);
		}
	}

	// Reselect GDI objects
	dc.SelectPen(oldPen);
	dc.SelectFont(oldFont);
	
	//dc.BitBlt(0, 0, 100, 100, dcMem, 0, 0, SRCCOPY);

}
