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
/*! \file		WorldEditorView.h 
	\brief		Interface of the CWorldEditorView class.
	\date		April 15, 2003
*/

#pragma once

#include "ChildView.h"
/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CWorldEditorFrame;
class CMapGroup;

class CWorldEditorView : 
	public CChildView,
	public CScrollWindowImpl<CWorldEditorView>
{
private:

	CSize m_WorldFullSize;
	HPEN m_hPenGrid;
	HPEN m_hPenMapGroupSelected;
	HPEN m_hPenMapGroupHover;
	HPEN m_hPenMapGroup;
	HFONT m_hFont12; 
	HFONT m_hFont10;
	HFONT m_hFont8;
	HFONT m_hFont6;

	bool m_bClean;
	CMapGroup *m_pSelMapGroup;
	CPoint m_MousePoint;
	CPoint m_MapPoint;
	CSize m_szMap;		// Map dimensions (default is 640x480)
	CSize m_szWorld;	// World dimensions (default is 256x256)
	int m_Zoom;			// Current zoom

protected:
	CMapGroup* DrawThumbnail(CDC &dc, int x, int y, CSize &szMap);
public:
	// Construction/Destruction
	CWorldEditorView(CWorldEditorFrame *pParentFrame);

	//DECLARE_WND_CLASS_EX(NULL, 0, -1)

	// Called to translate window messages before they are dispatched 
	BOOL PreTranslateMessage(MSG *pMsg);

	// Called to clean up after window is destroyed
	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CWorldEditView)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_MOUSEMOVE,		OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL,		OnMouseWheel)

		MESSAGE_HANDLER(WM_LBUTTONDBLCLK,	OnLButtonDblClk)
		MESSAGE_HANDLER(WM_LBUTTONDOWN,		OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP,		OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN,		OnRButtonDown)
		MESSAGE_HANDLER(WM_RBUTTONUP,		OnRButtonUp)
		MESSAGE_HANDLER(WM_MBUTTONDOWN,		OnMButtonDown)
		MESSAGE_HANDLER(WM_MBUTTONUP,		OnMButtonUp)

		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)

		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)

		CHAIN_MSG_MAP(CScrollWindowImpl<CWorldEditorView>);
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);

	LRESULT OnLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

	void DoPaint(CDCHandle dc);

	void UpdateSelections();
	void UpdateMouse(const CPoint &point);
	bool ScrollTo(CPoint &point, CRect &rcClient, CSize &szMap);
};