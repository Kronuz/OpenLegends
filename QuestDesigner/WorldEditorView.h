/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		WorldEditorView.h 
	\brief		Interface of the CWorldEditorView class.
	\date		April 15, 2003
*/

#pragma once

#include <Core.h>

#include "GEditorView.h"
/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CWorldEditorFrame;
class CMapGroup;

class CWorldEditorView : 
	public CGEditorView
{
	typedef CGEditorView baseClass;
private:

	CRect m_rcOldSelect;
	CSize m_WorldFullSize;
	HPEN m_hPenGrid;
	HPEN m_hPenMapGroupSelected;
	HPEN m_hPenMapGroupHover;
	HPEN m_hPenMapGroup;
	HFONT m_hFont12; 
	HFONT m_hFont10;
	HFONT m_hFont8;
	HFONT m_hFont6;

	bool m_bPanning;
	bool m_bWasModified;

	CPoint m_PanningPoint;
	CMapGroup *m_pSelMapGroup;
	CPoint m_MousePoint;
	CPoint m_MapPoint;
	CSize m_szMap;		// Map dimensions (default is 640x480)
	CSize m_szWorld;	// World dimensions (default is 256x256)

protected:
	CMapGroup* DrawThumbnail(CDC &dc, int x, int y, CSize &szMap);
public:
	// Construction/Destruction
	CWorldEditorView(CWorldEditorFrame *pParentFrame);

	// Called to translate window messages before they are dispatched 
	virtual BOOL PreTranslateMessage(MSG *pMsg) { return FALSE; }

	// Called to clean up after window is destroyed
	virtual void OnFinalMessage(HWND /*hWnd*/) { delete this; }

	DECLARE_WND_CLASS_EX(NULL, 0, -1)

	BEGIN_MSG_MAP(CWorldEditView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)

		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)

		MENU_COMMAND_HANDLER(ID_COPY,				Copy)
		MENU_COMMAND_HANDLER(ID_PASTE,				Paste)
		MENU_COMMAND_HANDLER(ID_CUT,				Cut)

/*
		MESSAGE_HANDLER(WM_LBUTTONDOWN,		OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP,		OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN,		OnRButtonDown)
		MESSAGE_HANDLER(WM_RBUTTONUP,		OnRButtonUp)
		MESSAGE_HANDLER(WM_MBUTTONDOWN,		OnMButtonDown)
		MESSAGE_HANDLER(WM_MBUTTONUP,		OnMButtonUp)
*/
		CHAIN_MSG_MAP(baseClass);
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
//	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
//	LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);

	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
//	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
//	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
//	LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
//	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
//	LRESULT OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
//	LRESULT OnMButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

	void UIUpdateMenuItems();
	void UIUpdateStatusBar();

	void UpdateSelections();
	void UpdateMouse(const CPoint &point);

	BOOL CanUndo();
	BOOL CanRedo();
	BOOL CanCut();
	BOOL CanCopy();
	BOOL CanPaste();
	BOOL IsSelection();
	BOOL IsReadOnly();

	// Called to do idle processing
	virtual BOOL OnIdle();
	// has the content of the control changed?
	virtual bool hasChanged();

	virtual HWND SetFocus() { return ::SetFocus(m_hWnd); }

	bool ScrollTo(CPoint &point, CRect &rcClient, CSize &szMap);
//////////////////////
	virtual void ViewToWorld(CPoint *_pPoint);

	virtual void HoldOperation();
	virtual void CancelOperation(bool bPropagate = true);

	virtual bool isResizing();
	virtual bool isMoving();
	virtual bool isFloating();
	virtual bool isSelecting();
	virtual bool isHeld();

	virtual void HoldSelection(bool bHold);
	virtual void StartSelBox(const CPoint &_Point, CURSOR *_pCursor);
	virtual void SizeSelBox(const CPoint &_Point, CURSOR *_pCursor);
	virtual IPropertyEnabled* EndSelBoxRemove(const CPoint &_Point, LPARAM lParam);
	virtual IPropertyEnabled* EndSelBoxAdd(const CPoint &_Point, LPARAM lParam);
	virtual void CancelSelBox();
	virtual IPropertyEnabled* SelectPoint(const CPoint &_Point, CURSOR *_pCursor);

	virtual void GetSelectionBounds(CRect *_pRect);

	virtual void StartMoving(const CPoint &_Point, CURSOR *_pCursor);
	virtual void MoveTo(const CPoint &_Point, CURSOR *_pCursor);
	virtual void EndMoving(const CPoint &_Point, LPARAM lParam);

	virtual void StartResizing(const CPoint &_Point, CURSOR *_pCursor);
	virtual void ResizeTo(const CPoint &_Point, CURSOR *_pCursor);
	virtual void EndResizing(const CPoint &_Point, LPARAM lParam);

	virtual bool SelectedAt(const CPoint &_Point);
	virtual int SelectedCount();

	virtual void PasteSelection(LPVOID _pBuffer, const CPoint &_Point);
	virtual HGLOBAL CopySelection(BITMAP **ppBitmap, bool bDeleteBitmap);
	virtual BITMAP* CaptureSelection(float _fZoom);
	virtual void CleanSelection();
	virtual int DeleteSelection();

	virtual bool GetMouseStateAt(const CPoint &_Point, CURSOR *_pCursor);
	virtual void CalculateLimits();
	virtual void UpdateSnapSize(int _SnapSize);
	virtual void Render(WPARAM wParam);
	virtual void UpdateView();

	virtual void OnChangeSel(int type, IPropertyEnabled *pPropObj = NULL);
	virtual void OnZoom();

	virtual bool DoFileOpen(LPCTSTR lpszFilePath, LPCTSTR lpszTitle = _T("Untitled"), WPARAM wParam = NULL, LPARAM lParam = NULL);
	virtual bool DoFileClose();
	virtual bool DoFileSave(LPCTSTR lpszFilePath);
	virtual bool DoFileSaveAs();
	virtual bool DoFileReload();
};
