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
/*! \file		SptShtEditorView.h 
	\brief		Interface of the CSptShtEditorView class.
	\date		September 30, 2003
*/

#pragma once

#include "GEditorView.h"

#include <vector>

#include "../IGame.h"
#include "../Core.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CSptShtEditorFrame;

class CSptShtEditorView : 
	public CGEditorView
{
	typedef CGEditorView baseClass;
private:
	ISoundManager *m_pSoundManager;
	CSize m_szSptSht;
	CImage m_Image;

	CVFile m_fnFile;
	float m_fScale;
	int m_nStep;

//////////////////////////////////
	CString m_sInitSprite;
	CSpriteSheet *m_pSpriteSheet;

	typedef std::vector<CSprite*> vectorSprite;
	vectorSprite m_Sprites;


	CSprite* LocateInitSprite();
	void SelectionBox(HDC hDC, const CRect &rectDest, COLORREF rgbColor);

//////////////////////////////////
public:

	// Construction/Destruction
	CSptShtEditorView(CSptShtEditorFrame *pParentFrame);

	// Called to translate window messages before they are dispatched 
	virtual BOOL PreTranslateMessage(MSG *pMsg) { return FALSE; }

	// Called to clean up after window is destroyed
	virtual void OnFinalMessage(HWND /*hWnd*/) { delete this; }

	DECLARE_WND_CLASS_EX(NULL, 0, -1)

	BEGIN_MSG_MAP(CSptShtEditorView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)

		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		
		MENU_COMMAND_HANDLER(ID_COPY,				Copy)
		MENU_COMMAND_HANDLER(ID_PASTE,				Paste)
		MENU_COMMAND_HANDLER(ID_CUT,				Cut)

		MENU_COMMAND_HANDLER(ID_MAPED_ARROW,		SingleSel)
		MENU_COMMAND_HANDLER(ID_MAPED_SELECT,		MultipleSel)
		MENU_COMMAND_HANDLER(ID_MAPED_SELECT_ALL,	SelectAll)
		MENU_COMMAND_HANDLER(ID_MAPED_SELECT_NONE,	SelectNone)
		MENU_COMMAND_HANDLER(ID_MAPED_NOZOOM,		NoZoom)
		MENU_COMMAND_HANDLER(ID_MAPED_ZOOMIN,		ZoomIn)
		MENU_COMMAND_HANDLER(ID_MAPED_ZOOMOUT,		ZoomOut)
		MENU_COMMAND_HANDLER(ID_MAPED_GRID,			ToggleGrid)
		MENU_COMMAND_HANDLER(ID_MAPED_GRIDSNAP,		ToggleSnap)
		MENU_COMMAND_HANDLER(ID_MAPED_SELHOLD,		ToggleHold)
		
		MENU_COMMAND_HANDLER(ID_APP_SOUND,			OnSound)
		MENU_COMMAND_HANDLER(ID_APP_ANIM,			OnAnim)
		MENU_COMMAND_HANDLER(ID_APP_PARALLAX,		OnParallax)

		CHAIN_MSG_MAP(baseClass);
	END_MSG_MAP()

	LRESULT OnStateChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	void UIUpdateMenuItems();
	void UIUpdateStatusBar();

	void OnAnim();
	void OnParallax();
	void OnSound();

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
	virtual void Render(WPARAM wParam);
	virtual void UpdateView();

	virtual void OnChangeSel(int type, IPropertyEnabled *pPropObj = NULL);

	virtual bool DoFileOpen(LPCTSTR lpszFilePath, LPCTSTR lpszTitle = _T("Untitled"), WPARAM wParam = NULL, LPARAM lParam = NULL);
	virtual bool DoFileClose();
	virtual bool DoFileSave(LPCTSTR lpszFilePath);
	virtual bool DoFileSaveAs();
	virtual bool DoFileReload();
};
