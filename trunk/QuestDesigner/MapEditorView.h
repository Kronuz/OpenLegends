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
/*! \file		MapEditorView.h 
	\brief		Interface of the CMapEditorView class.
	\date		April 27, 2003
*/

#pragma once

#include "ChildView.h"

#include "../IGraphics.h"
#include "../IGame.h"
#include "../Core.h"

#include "WindowDropTarget.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMapEditorFrame;

class CMapEditorView : 
	public CChildView,
	public CScrollWindowImpl<CMapEditorView>
{
	typedef CScrollWindowImpl<CMapEditorView> baseClass;
private:
	bool m_bModified;

	CMapGroup *m_pMapGroupI;
	CSpriteSelection *m_SelectionI;

	CSize m_szMap;
	IGraphics *m_pGraphicsI;

	DWORD m_dwTick;
	float m_Zoom;
	int m_nSnapSize;
	bool m_bFloating;
	enum { tNone, tToDrag, tWaiting, tDragging } m_DragState;
	bool m_bIgnoreClick;

	bool m_bMulSelection;
	bool m_bSnapToGrid;
	bool m_bShowMasks;
	bool m_bShowBoundaries;
	bool m_bShowGrid;

	bool m_bAnimated;

	bool m_bPanning;
	CPoint m_PanningPoint;

	CURSOR m_CursorStatus;
	CURSOR m_OldCursorStatus;

	CIDropTarget* m_pDropTarget;
	CIDropSource* m_pDropSource;

	LRESULT BeginDrag();
	CIDataObject* CreateOleObj(CIDropSource *pDropSource);

	bool Paste(CPoint &Point);

	bool Cut();
	bool Copy();
	bool Paste();
	bool Delete();
	bool Flip();
	bool Mirror();
	bool CWRotate();
	bool CCWRotate();

	bool InsertPlayer();
	bool SingleSel();
	bool MultipleSel();
	bool SelectAll();
	bool SelectNone();
	bool NoZoom();
	bool ZoomIn();
	bool ZoomOut();
	bool ToggleMask();
	bool ToggleBounds();
	bool ToggleGrid();
	bool TogleSnap();

	bool Zoom(float zoom);
	void ScrollTo(int x, int y);
	void OnChange();
	void OnChangeSel(IPropertyEnabled *pPropObj = NULL);
public:

	// Initialize drag and drop
	bool InitDragDrop();

	// Construction/Destruction
	CMapEditorView(CMapEditorFrame *pParentFrame);

	DECLARE_WND_CLASS_EX(NULL, 0, -1)

	// Called to translate window messages before they are dispatched 
	BOOL PreTranslateMessage(MSG *pMsg);

	// Called to do idle processing
	virtual BOOL OnIdle();
	// Called to clean up after window is destroyed
	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CWorldEditView)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_TIMER, OnTimer)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)

		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WMQD_DRAGLEAVE, OnDragLeave)
		MESSAGE_HANDLER(WMQD_DRAGOVER, OnDragOver)

		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)

		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP,	OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_RBUTTONUP,	OnRButtonUp)
		MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMButtonDown)
		MESSAGE_HANDLER(WM_MBUTTONUP,	OnMButtonUp)

		MESSAGE_HANDLER(WMQD_DROPOBJ,	OnDropObject)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)

		MENU_COMMAND_HANDLER(ID_MAPED_FLIP,			Flip)
		MENU_COMMAND_HANDLER(ID_MAPED_MIRROR,		Mirror)
		MENU_COMMAND_HANDLER(ID_MAPED_C90,			CWRotate)
		MENU_COMMAND_HANDLER(ID_MAPED_CC90,			CCWRotate)
		MENU_COMMAND_HANDLER(ID_COPY,				Copy)
		MENU_COMMAND_HANDLER(ID_PASTE,				Paste)
		MENU_COMMAND_HANDLER(ID_CUT,				Cut)

		MENU_COMMAND_HANDLER(ID_MAPED_PLAYER,		InsertPlayer)
		MENU_COMMAND_HANDLER(ID_MAPED_ARROW,		SingleSel)
		MENU_COMMAND_HANDLER(ID_MAPED_SELECT,		MultipleSel)
		MENU_COMMAND_HANDLER(ID_MAPED_SELECT_ALL,	SelectAll)
		MENU_COMMAND_HANDLER(ID_MAPED_SELECT_NONE,	SelectNone)
		MENU_COMMAND_HANDLER(ID_MAPED_NOZOOM,		NoZoom)
		MENU_COMMAND_HANDLER(ID_MAPED_ZOOMIN,		ZoomIn)
		MENU_COMMAND_HANDLER(ID_MAPED_ZOOMOUT,		ZoomOut)
		MENU_COMMAND_HANDLER(ID_MAPED_MASK,			ToggleMask)
		MENU_COMMAND_HANDLER(ID_MAPED_BOUNDS,		ToggleBounds)
		MENU_COMMAND_HANDLER(ID_MAPED_GRID,			ToggleGrid)
		MENU_COMMAND_HANDLER(ID_MAPED_GRIDSNAP,		TogleSnap)

		CHAIN_MSG_MAP(baseClass);
	END_MSG_MAP()


	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);
	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);

	LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);
	LRESULT OnDragLeave(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);
	LRESULT OnDragOver(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);
	
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnDropObject(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void DoPaint(CDCHandle dc);
	bool DoMapOpen(CMapGroup *pMapGroupI, LPCTSTR lpszTitle = _T("Untitled"));

	void Render();
	void UpdateView();
	void ToCursor(CURSOR cursor_);

	void UIUpdateMenuItems();
	void UIUpdateStatusBar();

	bool hasChanged();
};