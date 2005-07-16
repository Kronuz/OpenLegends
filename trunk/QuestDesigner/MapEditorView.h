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
/*! \file		MapEditorView.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the CMapEditorView class.
	\date		April 27, 2003
*/

#pragma once

#include "GEditorView.h"
#include "SuperCombo.h"

#include <IGraphics.h>
#include <IGame.h>
#include <Core.h>

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMapEditorFrame;

class CMapEditorView : 
	public CGEditorView
{
	typedef CGEditorView baseClass;
private:
	int m_nCheckPoint;
	int m_nCheckPointLimit;
	IGraphics *m_pGraphicsI;
	CMapGroup *m_pMapGroupI;
	ISoundManager *m_pSoundManager;
	CSpriteSelection *m_SelectionI;
	bool m_bShowMasks;
	bool m_bShowBoundaries;

	CSize m_szMap;

	bool ToTop();
	bool ObjectDown();
	bool ObjectUp();
	bool ToBottom();

	bool AlignTop();
	bool AlignBottom();
	bool AlignRight();
	bool AlignLeft();
	bool AlignCenter();
	bool AlignMiddle();

	bool Flip();
	bool Mirror();
	bool CWRotate();
	bool CCWRotate();

	bool InsertPlayer();
	bool ToggleMask();
	bool ToggleBounds();

	CString m_sPasting;

public:
	// Construction/Destruction
	CMapEditorView(CMapEditorFrame *pParentFrame);

	// Called to translate window messages before they are dispatched 
	virtual BOOL PreTranslateMessage(MSG *pMsg) { return FALSE; }

	// Called to clean up after window is destroyed
	virtual void OnFinalMessage(HWND /*hWnd*/) { delete this; }

	DECLARE_WND_CLASS_EX(NULL, 0, -1)

	BEGIN_MSG_MAP(CMapEditorView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)

		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		
		MENU_COMMAND_HANDLER(ID_MAPED_TOTOP,		ToTop)
		MENU_COMMAND_HANDLER(ID_MAPED_OBJDWN,		ObjectDown)
		MENU_COMMAND_HANDLER(ID_MAPED_OBJUP,		ObjectUp)
		MENU_COMMAND_HANDLER(ID_MAPED_TOBOTTOM,		ToBottom)

		MENU_COMMAND_HANDLER(ID_MAPED_ALTOP,		AlignTop)
		MENU_COMMAND_HANDLER(ID_MAPED_ALBOTTOM,		AlignBottom)
		MENU_COMMAND_HANDLER(ID_MAPED_ALRIGHT,		AlignRight)
		MENU_COMMAND_HANDLER(ID_MAPED_ALLEFT,		AlignLeft)
		MENU_COMMAND_HANDLER(ID_MAPED_ALCX,			AlignCenter)
		MENU_COMMAND_HANDLER(ID_MAPED_ALCY,			AlignMiddle)

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
		MENU_COMMAND_HANDLER(ID_MAPED_GRIDSNAP,		ToggleSnap)
		MENU_COMMAND_HANDLER(ID_MAPED_SELHOLD,		ToggleHold)

		MENU_COMMAND_HANDLER(ID_MAPED_MERGE,		OnMerge)

		MENU_COMMAND_HANDLER(ID_UNDO,				OnUndo)
		MENU_COMMAND_HANDLER(ID_REDO,				OnRedo)

		MENU_COMMAND_HANDLER(ID_APP_SOUND,			OnSound)
		MENU_COMMAND_HANDLER(ID_APP_ANIM,			OnAnim)
		MENU_COMMAND_HANDLER(ID_APP_PARALLAX,		OnParallax)

		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)

		COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnSelChange)
		COMMAND_CODE_HANDLER(CBN_STATECHANGE, OnStateChange)

		CHAIN_MSG_MAP(baseClass);
	END_MSG_MAP()

	LRESULT OnStateChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	void RunPopUpCmd(int nCmd);
	
	void UIUpdateMenuItems();
	void UIUpdateStatusBar();

	void OnAnim();
	void OnParallax();
	void OnSound();
	void OnMerge();
	void OnUndo();
	void OnRedo();

	void OnZoom();

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
	virtual void DoFrame();
	virtual void Render(WPARAM wParam);
	virtual void UpdateView();

	virtual void OnChangeSel(int type, IPropertyEnabled *pPropObj = NULL);

	virtual bool DoFileOpen(LPCTSTR lpszFilePath, LPCTSTR lpszTitle = _T("Untitled"), WPARAM wParam = NULL, LPARAM lParam = NULL);
	virtual bool DoFileClose();
	virtual bool DoFileSave(LPCTSTR lpszFilePath);
	virtual bool DoFileSaveAs();
	virtual bool DoFileReload();
};
