/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		GEditorView.h 
	\brief		Interface of the CGEditorView virtual base class.
	\date		April 27, 2003
				September 02, 2003
*/

#pragma once

#include "ChildView.h"

#include "../IGraphics.h"
#include "../IGame.h"
#include "../Core.h"

#include "WindowDropTarget.h"

#define OCS_UPDATE	-1
#define OCS_AUTO	0
#define OCS_RENEW	1	

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CGEditorFrame;

class CGEditorView : 
	public CChildView,
	public CScrollWindowImpl<CGEditorView>
{
	typedef CScrollWindowImpl<CGEditorView> baseClass;

protected:
	DWORD m_dwTick;
	HWND  m_hContainer;

	CRect m_rcScrollLimits;

	bool m_bModified;
	float m_Zoom;
	int m_nSnapSize;
	enum { tNone, tToDrag, tWaiting, tDragging } m_DragState;

	bool m_bAllowAutoScroll;
	bool m_bAllowSnapOverride;
	bool m_bAllowMulSelection;

	bool m_bScrolling;
	WPARAM m_wMouseState;
	LPARAM m_lMousePos;

	bool m_bSnapToGrid;
	bool m_bShowGrid;
	bool m_bMulSelection;
	bool m_bFloating; // floating selection (no need to press the button to start moving the selection)

	bool m_bPanning;
	bool m_bDuplicating;
	CPoint m_PanningPoint;

	CURSOR m_CursorStatus;
	CURSOR m_OldCursorStatus;

	CIDropTarget* m_pDropTarget;
	CIDropSource* m_pDropSource;

	LRESULT BeginDrag();
	CIDataObject* CreateOleObj(CIDropSource *pDropSource);

	bool Duplicate(const CPoint &Point);
	bool Paste(const CPoint &Point);

	bool Zoom(float zoom);
	void ScrollTo(CPoint ScrollPoint);
	void ScrollTo(int x, int y);
public:
	bool Cut();
	bool Copy();
	bool Paste();
	bool Duplicate();
	bool Delete();

	bool SingleSel();
	bool MultipleSel();
	bool SelectAll();
	bool SelectNone();
	bool NoZoom();
	bool ZoomIn();
	bool ZoomOut();
	bool ToggleGrid();
	bool ToggleSnap();
	bool ToggleHold();

	// Initialize drag and drop
	bool InitDragDrop();

	// Construction/Destruction
	CGEditorView(CGEditorFrame *pParentFrame);

	DECLARE_WND_CLASS_EX(NULL, 0, -1)

	BEGIN_MSG_MAP(CGEditorView)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_TIMER, OnTimer)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)

		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		
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

		MESSAGE_HANDLER(WMQD_DROPOBJ, OnDropObject)
		
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)

		MENU_COMMAND_HANDLER(ID_APP_ADJUST, OnAdjustLimits)

		MENU_COMMAND_HANDLER(ID_APP_OPEN, OnFileOpen)
		MENU_COMMAND_HANDLER(ID_APP_CLOSE, OnFileClose)
		MENU_COMMAND_HANDLER(ID_APP_RELOAD, OnFileReload)
		MENU_COMMAND_HANDLER(ID_APP_SAVE, OnFileSave)
		MENU_COMMAND_HANDLER(ID_APP_SAVE_AS, OnFileSaveAs)

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
	LRESULT OnMButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnDropObject(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void OnAdjustLimits();

	bool OnFileOpen();
	bool OnFileClose();

	bool OnFileReload();
	// Save handlers
	bool OnFileSave();
	bool OnFileSaveAs();

	void DoPaint(CDCHandle dc);
	CURSOR ToCursor(CURSOR _cursor);

	virtual void ViewToWorld(CPoint *_pPoint) = 0;

	virtual void HoldOperation() = 0;
	virtual void CancelOperation(bool bPropagate = true) = 0;

	virtual bool isResizing() = 0;
	virtual bool isMoving() = 0;
	virtual bool isFloating() = 0;
	virtual bool isSelecting() = 0;
	virtual bool isHeld() = 0;

	virtual void HoldSelection(bool bHold) = 0;
	virtual void StartSelBox(const CPoint &_Point, CURSOR *_pCursor) = 0;
	virtual void SizeSelBox(const CPoint &_Point, CURSOR *_pCursor) = 0;
	virtual IPropertyEnabled* EndSelBoxRemove(const CPoint &_Point, LPARAM lParam) = 0;
	virtual IPropertyEnabled* EndSelBoxAdd(const CPoint &_Point, LPARAM lParam) = 0;
	virtual void CancelSelBox() = 0;
	virtual IPropertyEnabled* SelectPoint(const CPoint &_Point, CURSOR *_pCursor) = 0;

	virtual void GetSelectionBounds(CRect *_pRect) = 0;

	virtual void StartMoving(const CPoint &_Point, CURSOR *_pCursor) = 0;
	virtual void MoveTo(const CPoint &_Point, CURSOR *_pCursor) = 0;
	virtual void EndMoving(const CPoint &_Point, LPARAM lParam) = 0;

	virtual void StartResizing(const CPoint &_Point, CURSOR *_pCursor) = 0;
	virtual void ResizeTo(const CPoint &_Point, CURSOR *_pCursor) = 0;
	virtual void EndResizing(const CPoint &_Point, LPARAM lParam) = 0;

	virtual bool SelectedAt(const CPoint &_Point) = 0;
	virtual int SelectedCount() = 0;

	virtual HGLOBAL CopySelection(BITMAP **ppBitmap, bool bDeleteBitmap) = 0;
	virtual void PasteSelection(LPVOID _pBuffer, const CPoint &_Point) = 0;
	virtual BITMAP* CaptureSelection(float _fZoom) = 0;
	virtual void CleanSelection() = 0;
	virtual int DeleteSelection() = 0;

	virtual bool GetMouseStateAt(const CPoint &_Point, CURSOR *_pCursor) = 0;
	virtual void CalculateLimits() = 0;
	virtual void UpdateSnapSize(int _SnapSize) = 0;
	virtual void DoFrame() {}
	virtual void Render(WPARAM wParam) = 0;
	virtual void UpdateView() = 0;

	virtual void OnChangeSel(int type, IPropertyEnabled *pPropObj = NULL) = 0;
	virtual void OnZoom() = 0; // Called after a zooming

	virtual BOOL OnIdle() = 0; // Called on idle
	virtual bool hasChanged() = 0;

	virtual bool DoFileOpen(LPCTSTR lpszFilePath, LPCTSTR lpszTitle = _T("Untitled"), WPARAM wParam = NULL, LPARAM lParam = NULL) = 0;
	virtual bool DoFileClose() = 0;
	virtual bool DoFileSave(LPCTSTR lpszFilePath = NULL) = 0;
	virtual bool DoFileSaveAs() = 0;
	virtual bool DoFileReload() = 0;
};
