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
/*! \file		MapEditorView.cpp 
	\brief		Implementation of the CMapEditorView class.
	\date		April 27, 2003
*/

#include "stdafx.h"
#include "resource.h"

#include "MainFrm.h"
#include "MapEditorView.h"
#include "MapEditorFrm.h"

#include "GraphicsFactory.h"

bool CMapEditorView::InitDragDrop()
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

CMapEditorView::CMapEditorView(CMapEditorFrame *pParentFrame) :
	CChildView(pParentFrame),
	m_pMapGroupI(NULL),
	m_szMap(0, 0),
	m_Zoom(1.0f),
	m_nSnapSize(16),
	m_bFloating(false),
	m_DragState(tNone),
	m_bIgnoreClick(false),
	m_pGraphicsI(NULL),
	m_CursorStatus(eIDC_ARROW),
	m_OldCursorStatus(eIDC_ARROW),
	m_SelectionI(NULL),
	m_pDropTarget(NULL),
	m_pDropSource(NULL),

	m_bMulSelection(true), // multi selection mode by default
	m_bSnapToGrid(true),
	m_bShowMasks(false),
	m_bShowBoundaries(false),
	m_bShowGrid(false),
	m_bAnimated(true),
	m_bPanning(false),
	m_bModified(false)
{
}

// Called to translate window messages before they are dispatched 
BOOL CMapEditorView::PreTranslateMessage(MSG *pMsg)
{
	return FALSE;
}
// Called to do idle processing
BOOL CMapEditorView::OnIdle()
{
	// Update all the menu items
	UIUpdateMenuItems();
	
	// Update position display in the status bar
	UIUpdateStatusBar();

	CMapEditorFrame *pMapEditorFrm = static_cast<CMapEditorFrame*>(GetParentFrame());
	CMainFrame *pMainFrm = GetMainFrame();

	CUpdateUIBase *pMapUpdateUI = pMapEditorFrm->GetUpdateUI();
	CUpdateUIBase *pMainUpdateUI = pMainFrm->GetUpdateUI();

	// Update all the toolbar items
	pMapUpdateUI->UIUpdateToolBar();
	pMainUpdateUI->UIUpdateToolBar();

	if(!m_SelectionI) return FALSE;

	bool bModified = (m_SelectionI->IsModified()?true:false);
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
// Called to clean up after window is destroyed (called when WM_NCDESTROY is sent)
void CMapEditorView::OnFinalMessage(HWND /*hWnd*/)
{
	CProjectFactory::Delete(&m_SelectionI);
	CGraphicsFactory::Delete(&m_pGraphicsI);
	delete this;
}

LRESULT CMapEditorView::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMapEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	LRESULT nResult = DefWindowProc();

	if(FAILED(CGraphicsFactory::New(&m_pGraphicsI, "GraphicsD3D8.dll"))) {
		MessageBox("Couldn't load graphics plugin, check plugin version.", "Quest Designer");
	}

	m_pGraphicsI->Initialize(GetMainFrame()->m_hWnd);

	InitDragDrop();

	return nResult;
}
bool CMapEditorView::DoMapOpen(CMapGroup *pMapGroupI, LPCTSTR lpszTitle) 
{
	m_pMapGroupI = pMapGroupI;
	if(FAILED(CProjectFactory::New(&m_SelectionI, reinterpret_cast<CDrawableContext**>(&m_pMapGroupI))))
		MessageBox("Couldn't load kernel, check kernel version.", "Quest Designer");

	m_pMapGroupI->GetSize(m_szMap);

	// Save file name for later
	m_sFilePath = m_pMapGroupI->GetMapGroupID();

	ISoundManager *pSoundManager = CProjectFactory::Interface()->GetSoundManager();
	pSoundManager->SwitchMusic(m_pMapGroupI->GetMusic(), 0);

	// Save the title for later
	m_sTitle = lpszTitle;

	m_pParentFrame->m_sChildName = m_sFilePath;
	m_pParentFrame->SetTitle(m_sFilePath);
	m_pParentFrame->SetTabText(m_sTitle);

	SetScrollSize((int)((float)m_szMap.cx*m_Zoom), (int)((float)m_szMap.cy*m_Zoom));

	m_SelectionI->SetSnapSize(m_nSnapSize, m_bShowGrid);

	BITMAP *pBitmap = m_SelectionI->Capture(m_pGraphicsI, 0.25f);
	m_pMapGroupI->SetThumbnail(pBitmap);

	UpdateView();
	return true;
}
LRESULT CMapEditorView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	if(m_pMapGroupI) {
		ISoundManager *pSoundManager = CProjectFactory::Interface()->GetSoundManager();
		pSoundManager->SwitchMusic(NULL, 0, false);
	}

	RevokeDragDrop(m_hWnd); //calls release
	m_pDropTarget=NULL;
	return 0;
}
LRESULT CMapEditorView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(GetMainFrame()->GetOldFocus(tMapEditor) == m_hWnd) return 0;
	GetMainFrame()->SetOldFocus(tMapEditor, m_hWnd);

	if(m_pMapGroupI) {
		ISoundManager *pSoundManager = CProjectFactory::Interface()->GetSoundManager();
		pSoundManager->SwitchMusic(m_pMapGroupI->GetMusic(), 0);
		OnChangeSel();
	}
/**/
	SetTimer(1, 1000/30);
	return 0;
}
LRESULT CMapEditorView::OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	// stop the animations
	if(GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) return 0;
	KillTimer(1);

	// Capture the MapGroup (to update the world editor)
	if(m_pMapGroupI && m_SelectionI) {
		if(m_SelectionI->HasChanged()) {
			BITMAP *pBitmap = m_SelectionI->Capture(m_pGraphicsI, 0.25f);
			m_pMapGroupI->SetThumbnail(pBitmap);
		}
	}
	return 0;
}
LRESULT CMapEditorView::OnMouseLeave(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(m_DragState == tToDrag) {
		m_DragState = tWaiting;

		// start tracking the mouse, to see if it enters into 
		// the client area again within 200 milliseconds:
		SetTimer(2, 200);
	}

	return 0;
}

LRESULT CMapEditorView::OnDragLeave(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	m_DragState = tToDrag;
	m_dwTick = GetTickCount();
	return 0;
}
LRESULT CMapEditorView::OnDragOver(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(GetTickCount() - m_dwTick > 500) m_pDropSource->CancelDrag();
	return 0;
}

void CMapEditorView::UpdateView()
{
	// We need to recalculate the window's size and position:
	CRect rcClient, rcClip;
	GetClientRect(&rcClient);
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	rcClip.SetRect(0, 0, m_szMap.cx, m_szMap.cy);
	m_pGraphicsI->SetWindowView(m_hWnd, rcClient, rcClip, m_Zoom);

}
void CMapEditorView::Render()
{
	ASSERT(m_pMapGroupI);

	// Update timings and stuff for the animations
	CProjectFactory::Interface()->UpdateFPS();

	ISoundManager *pSoundManager = CProjectFactory::Interface()->GetSoundManager();
	pSoundManager->DoMusic();

	// show entities, and if set, also sprite boundaries and masks
	WORD wFlags = SPRITE_ENTITIES | (m_bShowBoundaries?SPRITE_BOUNDS:0) | (m_bShowMasks?SPRITE_MASKS:0);
	m_SelectionI->Paint(m_pGraphicsI, wFlags);
}

LRESULT CMapEditorView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if(wParam == 1) {
		if(GetParentFrame()->m_hWnd != GetMainFrame()->m_tabbedClient.GetTopWindow()) {
			KillTimer(1); 
		}
		if(m_bAnimated) Render(); // Animation
	} else if(wParam == 2) {
		// cancel this timer
		KillTimer(2);
		if(m_DragState == tWaiting) {
			// now start draging
			BeginDrag();
		}
	}
	return 0;
}
void CMapEditorView::DoPaint(CDCHandle dc)
{
	Render();
}

LRESULT CMapEditorView::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	LRESULT nResult = baseClass::OnVScroll(uMsg, wParam, lParam, bHandled);
	SetFocus();
	UpdateView();
	return nResult;
}
LRESULT CMapEditorView::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	LRESULT nResult = baseClass::OnHScroll(uMsg, wParam, lParam, bHandled);
	SetFocus();
	UpdateView();
	return nResult;
}
LRESULT CMapEditorView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	LRESULT nResult = baseClass::OnSize(uMsg, wParam, lParam, bHandled);
	UpdateView();
	return nResult;
}

LRESULT CMapEditorView::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
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


	ScrollTo((int)((float)WorldPoint.x*m_Zoom)-MousePoint.x, (int)((float)WorldPoint.y*m_Zoom)-MousePoint.y);

	UpdateView();

	CPoint Point(lParam);
	ScreenToClient(&Point);
	CURSOR Cursor;
	m_SelectionI->GetMouseStateAt(m_pGraphicsI, Point, &Cursor);
	ToCursor(Cursor);

	return 0;
}
void CMapEditorView::ScrollTo(int x, int y) 
{
	CRect rcClient;
	GetClientRect(&rcClient);
	CPoint ScrollPoint(x, y);

	// Now we validate the new scrolling position:
	if(ScrollPoint.x < 0) ScrollPoint.x = 0;
	if(ScrollPoint.y < 0) ScrollPoint.y = 0;
	if(ScrollPoint.x > (int)((float)m_szMap.cx*m_Zoom)-rcClient.Width()) ScrollPoint.x = (int)((float)m_szMap.cx*m_Zoom)-rcClient.Width();
	if(ScrollPoint.y > (int)((float)m_szMap.cy*m_Zoom)-rcClient.Height()) ScrollPoint.y = (int)((float)m_szMap.cy*m_Zoom)-rcClient.Height();
	SetScrollOffset(ScrollPoint);
}
void CMapEditorView::ToCursor(CURSOR cursor_)
{
	HCURSOR hCursor;

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
}
void CMapEditorView::OnChange()
{
	HWND hWnd = GetMainFrame()->m_hWnd;
	::SendMessage(hWnd, WMP_UPDATE, 0, 0);
}
void CMapEditorView::OnChangeSel(IPropertyEnabled *pPropObj)
{
	HWND hWnd = GetMainFrame()->m_hWnd;
	::SendMessage(hWnd, WMP_CLEAR, 0, 0);

	SInfo Information;
	SPropertyList Properties;
	SObjProp *pOP = m_SelectionI->GetFirstSelection();
	while(pOP) {
		memset(&Information, 0, sizeof(SInfo));
		pOP->GetInfo(&Information);
		::SendMessage(hWnd, WMP_ADDINFO, (WPARAM)pPropObj, (LPARAM)&Information);
		if(pPropObj == Information.pPropObject) {
			memset(&Properties, 0, sizeof(SPropertyList));
			pOP->GetProperties(&Properties);
			::SendMessage(hWnd, WMP_SETPROP, 0, (LPARAM)&Properties);
		}
		pOP = m_SelectionI->GetNextSelection();
	}
	if(pPropObj == NULL) {
		pOP = m_SelectionI->GetFirstSelection();
		if(pOP) {
			memset(&Properties, 0, sizeof(SPropertyList));
			pOP->GetProperties(&Properties);
			::SendMessage(hWnd, WMP_SETPROP, 0, (LPARAM)&Properties);
		}
	}
}
LRESULT CMapEditorView::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if(m_bFloating) return 0;

	CPoint Point(lParam);
	m_SelectionI->GetMouseStateAt(m_pGraphicsI, Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	m_pGraphicsI->GetWorldPosition(&Point);

	m_SelectionI->SetSnapSize(m_bSnapToGrid?m_nSnapSize:1, m_bShowGrid);
	if(m_CursorStatus==eIDC_ARROW || m_CursorStatus==eIDC_ARROWADD || m_CursorStatus==eIDC_ARROWDEL) {
		if(m_bMulSelection || m_CursorStatus!=eIDC_ARROW) m_SelectionI->StartSelBox(Point);
		else {
			m_SelectionI->CleanSelection();
			m_SelectionI->StartSelBox(Point);
			OnChangeSel(m_SelectionI->EndSelBoxAdd(Point, 0));
			if(m_SelectionI->Count()) GetMainFrame()->m_Layers.SetCurSel(m_SelectionI->GetLayer());
			m_CursorStatus=eIDC_SIZEALL;
		}
	}
	if(m_CursorStatus==eIDC_SIZEALL) {
//		ShowCursor(FALSE);
		m_SelectionI->StartMoving(Point);
		m_DragState = tToDrag;
	} else if(m_CursorStatus==eIDC_SIZENESW || m_CursorStatus==eIDC_SIZENS || m_CursorStatus==eIDC_SIZENWSE ||m_CursorStatus==eIDC_SIZEWE) {
		m_SelectionI->StartResizing(Point);
	}

	if(m_DragState == tNone) SetCapture();
	return 0;
}
LRESULT CMapEditorView::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	ReleaseCapture();

	m_DragState = tNone;
	CPoint Point(lParam);
	m_SelectionI->GetMouseStateAt(m_pGraphicsI, Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	m_pGraphicsI->GetWorldPosition(&Point);
	
	if((wParam&MK_CONTROL)==MK_CONTROL) m_SelectionI->SetSnapSize(1, m_bShowGrid);
	else m_SelectionI->SetSnapSize(m_bSnapToGrid?m_nSnapSize:1, m_bShowGrid);

	if(m_SelectionI->isSelecting()) {
		if((wParam&MK_SHIFT)==MK_SHIFT || (wParam&MK_CONTROL)==0) {
			if((wParam&MK_SHIFT)==0) m_SelectionI->CleanSelection();
			OnChangeSel(m_SelectionI->EndSelBoxAdd(Point, ((wParam&MK_CONTROL)==MK_CONTROL)?1:0));
			if(m_SelectionI->Count()) GetMainFrame()->m_Layers.SetCurSel(m_SelectionI->GetLayer());
		} else {
			m_SelectionI->EndSelBoxRemove(Point);
			OnChangeSel();
		}
	} else if(m_SelectionI->isMoving() || m_SelectionI->isFloating()) {
		m_SelectionI->EndMoving(Point);
//		ShowCursor(TRUE);
		OnChange();
	} else if(m_SelectionI->isResizing()) {
		m_SelectionI->EndResizing(Point);
		OnChange();
	}

	Invalidate();
	return 0;
}
LRESULT CMapEditorView::OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
//	if(m_bIgnoreClick) { m_bIgnoreClick = false; return 0; }
	CPoint Point(lParam);
	m_SelectionI->GetMouseStateAt(m_pGraphicsI, Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	if(m_SelectionI->isSelecting()) {
		m_SelectionI->CancelSelBox();
	} else {
		m_pGraphicsI->GetWorldPosition(&Point);
		if(!m_SelectionI->SelectedAt(Point)) {
			m_SelectionI->CancelSelBox();
		}
	}

	return 0;
}
LRESULT CMapEditorView::OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
//	if(m_bIgnoreClick) { m_bIgnoreClick = false; return 0; }
	CPoint Point(lParam);
	bool bInSelection = m_SelectionI->GetMouseStateAt(m_pGraphicsI, Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
	if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	ToCursor(m_CursorStatus);

	if((wParam&MK_SHIFT)==MK_SHIFT && (wParam&MK_CONTROL)==MK_CONTROL) {
		m_pGraphicsI->GetWorldPosition(&Point);
		m_SelectionI->StartSelBox(Point);
		OnChangeSel(m_SelectionI->EndSelBoxAdd(Point, -1));
		if(m_SelectionI->Count()) GetMainFrame()->m_Layers.SetCurSel(m_SelectionI->GetLayer());
	} else if((wParam&MK_SHIFT)==0 && (wParam&MK_CONTROL)==0) {
		if(!bInSelection && m_bMulSelection==true) {
			if(m_SelectionI->Count()==0) {
				CPoint ScreenPoint(lParam);
				ClientToScreen(&ScreenPoint);
				PostMessage(WM_CONTEXTMENU, wParam, MAKELPARAM(ScreenPoint.x, ScreenPoint.y));
			} else {
				m_SelectionI->CleanSelection();
				OnChangeSel();
			}
		} else {
			if(!bInSelection && (m_SelectionI->Count()==0 || m_bMulSelection==false)) {
				m_SelectionI->CleanSelection();
				m_pGraphicsI->GetWorldPosition(&Point);
				m_SelectionI->StartSelBox(Point);
				OnChangeSel(m_SelectionI->EndSelBoxAdd(Point, 0));
				if(m_SelectionI->Count()) GetMainFrame()->m_Layers.SetCurSel(m_SelectionI->GetLayer());
			} 
			CPoint ScreenPoint(lParam);
			ClientToScreen(&ScreenPoint);
			PostMessage(WM_CONTEXTMENU, wParam, MAKELPARAM(ScreenPoint.x, ScreenPoint.y));
		}
	}
/**/
	Invalidate();

	return 0;
}
LRESULT CMapEditorView::OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_PanningPoint.SetPoint(LOWORD(lParam), HIWORD(lParam));

	m_bPanning = true;
	ToCursor(m_CursorStatus);
	SetCapture();

	return 0;
}
LRESULT CMapEditorView::OnMButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_bPanning = false;
	ToCursor(m_CursorStatus);
	ReleaseCapture();
	return 0;
}
LRESULT CMapEditorView::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CMenu menu;
	if(!menu.CreatePopupMenu())
		return 0;
	menu.AppendMenu(MF_STRING, 1, "Mirror");
	menu.AppendMenu(MF_STRING, 2, "Flip");
	menu.AppendMenu(MF_STRING, 3, "Rotate CW 90º");
	menu.AppendMenu(MF_STRING, 4, "Rotate CCW 90º");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, 5, "Move Top");
	menu.AppendMenu(MF_STRING, 6, "Move Bottom");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, 7, "Duplicate");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, 8, "Copy");
	menu.AppendMenu(MF_STRING, 9, "Paste");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, 10, "Delete");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, 11, "Select None");

	menu.EnableMenuItem(9, MF_GRAYED);
	if(m_SelectionI->Count() == 0) {
		menu.EnableMenuItem(1, MF_GRAYED);
		menu.EnableMenuItem(2, MF_GRAYED);
		menu.EnableMenuItem(3, MF_GRAYED);
		menu.EnableMenuItem(4, MF_GRAYED);
		menu.EnableMenuItem(5, MF_GRAYED);
		menu.EnableMenuItem(6, MF_GRAYED);
		menu.EnableMenuItem(7, MF_GRAYED);
		menu.EnableMenuItem(8, MF_GRAYED);
		menu.EnableMenuItem(10, MF_GRAYED);
		menu.EnableMenuItem(11, MF_GRAYED);
	}

	FORMATETC fmtetc = {0};
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	fmtetc.cfFormat = CF_TEXT;
	fmtetc.tymed = TYMED_HGLOBAL;
	
	CIDataObject *pGetDataObject = NULL;
	OleGetClipboard((LPDATAOBJECT*) &pGetDataObject);
	if(pGetDataObject) {
		if(pGetDataObject->QueryGetData(&fmtetc) == S_OK) {
			menu.EnableMenuItem(9, MF_ENABLED);
		}
		pGetDataObject->Release();
	}
	
	m_bIgnoreClick = false;

	int nCmd = menu.TrackPopupMenu(TPM_RETURNCMD, LOWORD(lParam), HIWORD(lParam), m_hWnd);
	switch(nCmd) {
		case 1:		Mirror();				break;
		case 2:		Flip();					break;
		case 3:		CWRotate();				break;
		case 4:		CCWRotate();			break;
		case 5:								break;
		case 6:								break;
		case 7:								break;
		case 8:		Copy();					break;
		case 9:		Paste(CPoint(lParam));	break;
		case 10:	Delete();				break;
		case 11:	SelectNone();			break;
		default: m_bIgnoreClick = true;
	}

	return 0;
}
CIDataObject* CMapEditorView::CreateOleObj(CIDropSource *pDropSource)
{
	HGLOBAL hGlobal = m_SelectionI->Copy();
	if(!hGlobal) return 0;

	CIDataObject* pDataObject = new CIDataObject(pDropSource);
	if(pDataObject == NULL) {
		::GlobalFree(hGlobal);
		return 0;
	}
	pDataObject->AddRef();

	BITMAP *pBitmap = m_SelectionI->CaptureSelection(m_pGraphicsI, 0);
	m_SelectionI->SetThumbnail(pBitmap);

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

	CRect Rect(0, 0, pBitmap->bmWidth, pBitmap->bmHeight);

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
	pDataObject->SetData(&fmtetc,&medium,FALSE);
	//////////////////////////////////////
	fmtetc.cfFormat = CF_TEXT;
	fmtetc.tymed = TYMED_HGLOBAL;
	medium.tymed = TYMED_HGLOBAL;
	medium.hGlobal = hGlobal;
	pDataObject->SetData(&fmtetc,&medium,TRUE);
	//////////////////////////////////////
	fmtetc.cfFormat = CF_ENHMETAFILE;
	fmtetc.tymed = TYMED_ENHMF;
	medium.tymed = TYMED_ENHMF;
	HDC hMetaDC = CreateEnhMetaFile(hDC, NULL, NULL, NULL);
	::BitBlt(hMetaDC, 0, 0, pBitmap->bmWidth, pBitmap->bmHeight, hDCMem, 0, 0, SRCCOPY);
	medium.hEnhMetaFile = CloseEnhMetaFile(hMetaDC);
	pDataObject->SetData(&fmtetc,&medium,TRUE);
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
LRESULT CMapEditorView::BeginDrag()
{
	m_pDropSource->InitDrag();

	CIDataObject *pDataObject = CreateOleObj(m_pDropSource);
	if(!pDataObject) return 0;

	m_DragState = tDragging;
	m_dwTick = GetTickCount();
	m_SelectionI->HoldOperation();
	Invalidate();
	UpdateWindow();

//	ShowCursor(TRUE);

	DWORD dwEffect;
	HRESULT hr = ::DoDragDrop(pDataObject, m_pDropSource, DROPEFFECT_COPY, &dwEffect);
	pDataObject->Release();
	
	if(hr == DRAGDROP_S_DROP)  {
		m_SelectionI->Cancel();
		OnChangeSel();
	}

	m_DragState = tToDrag;

	return 0;
}
LRESULT CMapEditorView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint Point(lParam);

	if(m_bPanning) {
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

	if(m_DragState==tWaiting) {
		// the mouse entered the client area again before timeout:
		KillTimer(2);
		m_DragState=tToDrag;
	}

	if(m_DragState==tToDrag && (wParam&MK_LBUTTON) == MK_LBUTTON) {
		TRACKMOUSEEVENT Track;
		Track.cbSize = sizeof(TRACKMOUSEEVENT);
		Track.dwFlags = TME_LEAVE;
		Track.hwndTrack = m_hWnd;
		Track.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&Track);
	}

	m_SelectionI->GetMouseStateAt(m_pGraphicsI, Point, &m_CursorStatus);
	m_OldCursorStatus = m_CursorStatus;
	if((wParam&MK_LBUTTON)==0) {
		if((wParam&MK_CONTROL)==MK_CONTROL) m_CursorStatus = eIDC_ARROWDEL;
		if((wParam&MK_SHIFT)==MK_SHIFT) m_CursorStatus = eIDC_ARROWADD;
	}
	ToCursor(m_CursorStatus);

	m_pGraphicsI->GetWorldPosition(&Point);

	if(m_bFloating != m_SelectionI->isFloating()) {
		m_bFloating = m_SelectionI->isFloating();
		if(m_bFloating) {
//			ShowCursor(FALSE);
//			SetCapture();
		}
	}
	if(m_bFloating) {
		m_SelectionI->MoveTo(Point);
		Invalidate();
	}

	if((wParam&MK_LBUTTON)==MK_LBUTTON) {

		if((wParam&MK_CONTROL)==MK_CONTROL) m_SelectionI->SetSnapSize(1, m_bShowGrid);
		else m_SelectionI->SetSnapSize(m_bSnapToGrid?m_nSnapSize:1, m_bShowGrid);

		if(m_SelectionI->isSelecting()) {
			m_SelectionI->SizeSelBox(Point);
		} else if(m_SelectionI->isMoving()) {
			m_SelectionI->MoveTo(Point);
		} else if(m_SelectionI->isResizing()) {
			m_SelectionI->ResizeTo(Point);
		}
		Invalidate();
	}

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("X: %3d, Y: %3d"), Point.x, Point.y);
	pStatusBar->SetPaneText(ID_POSITION_PANE, sText);

	if(GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) SetFocus();

	return 0;
}
LRESULT CMapEditorView::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(m_bIgnoreClick) { return 0; }
	if(!m_SelectionI->isMoving() && !m_SelectionI->isResizing() && !m_SelectionI->isSelecting()) {
		if(wParam == VK_CONTROL) {
			if(m_CursorStatus != eIDC_ARROWADD) ToCursor(eIDC_ARROWDEL);
		} else if(wParam == VK_SHIFT) {
			ToCursor(eIDC_ARROWADD);
		} else bHandled=FALSE;
	}
	return 0;
}
LRESULT CMapEditorView::OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(m_bIgnoreClick) { m_bIgnoreClick = false; return 0; }
	if(wParam == VK_CONTROL) {
		if(m_CursorStatus == eIDC_ARROWDEL) ToCursor(m_OldCursorStatus);
	} else if(wParam == VK_SHIFT) {
		ToCursor(m_OldCursorStatus);
	} else if(wParam == VK_ESCAPE) {
//		if(m_SelectionI->isMoving() || m_SelectionI->isFloating()) ShowCursor(TRUE);
		m_SelectionI->Cancel();
		m_DragState = tNone;
		ReleaseCapture();
		OnChangeSel();
	} else if(wParam == VK_ADD || wParam == VK_OEM_PLUS) {
		ZoomIn();
	} else if(wParam == VK_SUBTRACT || wParam == VK_OEM_MINUS) {
		ZoomOut();
	} else if(wParam == VK_DELETE) {
		if(!m_SelectionI->isMoving() && !m_SelectionI->isResizing()) Delete();
	}

	Invalidate();

	return 0;
}

LRESULT CMapEditorView::OnDropObject(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::SetForegroundWindow(GetMainFrame()->m_hWnd);

	GetMainFrame()->BringWindowToTop();
	GetParentFrame()->BringWindowToTop();
	GetParentFrame()->SetFocus();
	
	m_SelectionI->SetLayer(GetMainFrame()->m_Layers.GetCurSel());
	m_SelectionI->Paste((LPVOID)wParam, CPoint(lParam));
	OnChangeSel();
	return 0;
}

void CMapEditorView::UIUpdateStatusBar()
{
}
void CMapEditorView::UIUpdateMenuItems()
{
	// Get the main window's UI updater
	CMapEditorFrame *pMapEditorFrm = static_cast<CMapEditorFrame*>(GetParentFrame());
	CMainFrame *pMainFrm = GetMainFrame();

	CUpdateUIBase *pMapUpdateUI = pMapEditorFrm->GetUpdateUI();
	CUpdateUIBase *pMainUpdateUI = pMainFrm->GetUpdateUI();

	pMapUpdateUI->UISetCheck(ID_MAPED_ARROW, m_bMulSelection?FALSE:TRUE);
	pMapUpdateUI->UISetCheck(ID_MAPED_SELECT, m_bMulSelection?TRUE:FALSE);

	pMapUpdateUI->UISetCheck(ID_MAPED_MASK, m_bShowMasks?TRUE:FALSE);
	pMapUpdateUI->UISetCheck(ID_MAPED_BOUNDS, m_bShowBoundaries?TRUE:FALSE);
	pMapUpdateUI->UISetCheck(ID_MAPED_GRID, m_bShowGrid?TRUE:FALSE);
	pMapUpdateUI->UISetCheck(ID_MAPED_GRIDSNAP, m_bSnapToGrid?TRUE:FALSE);

	int nSelection = 0;
	if(m_SelectionI) nSelection = m_SelectionI->Count();
	pMapUpdateUI->UIEnable(ID_MAPED_TOTOP, nSelection);
	pMapUpdateUI->UIEnable(ID_MAPED_OBJUP, nSelection);
	pMapUpdateUI->UIEnable(ID_MAPED_OBJDWN, nSelection);
	pMapUpdateUI->UIEnable(ID_MAPED_TOBOTTOM, nSelection);
	pMapUpdateUI->UIEnable(ID_MAPED_FLIP, nSelection);
	pMapUpdateUI->UIEnable(ID_MAPED_MIRROR, nSelection);
	pMapUpdateUI->UIEnable(ID_MAPED_C90, nSelection);
	pMapUpdateUI->UIEnable(ID_MAPED_CC90, nSelection);
	pMapUpdateUI->UIEnable(ID_MAPED_ALTOP, (nSelection>1)?TRUE:FALSE);
	pMapUpdateUI->UIEnable(ID_MAPED_ALBOTTOM, (nSelection>1)?TRUE:FALSE);
	pMapUpdateUI->UIEnable(ID_MAPED_ALLEFT, (nSelection>1)?TRUE:FALSE);
	pMapUpdateUI->UIEnable(ID_MAPED_ALRIGHT, (nSelection>1)?TRUE:FALSE);
	pMapUpdateUI->UIEnable(ID_MAPED_ALCY, (nSelection>1)?TRUE:FALSE);
	pMapUpdateUI->UIEnable(ID_MAPED_ALCX, (nSelection>1)?TRUE:FALSE);

	if(pMainFrm->m_bLayers == FALSE) {
		pMainFrm->m_bLayers = TRUE;
		pMainFrm->m_Layers.EnableWindow(TRUE);
	}
	pMainUpdateUI->UIEnable(ID_APP_SAVE, hasChanged());	
/*
	pUpdateUI->UIEnable(ID_UNDO, m_SelectionI->CanUndo());
	pUpdateUI->UIEnable(ID_REDO, m_SelectionI->CanRedo());

	pUpdateUI->UIEnable(ID_CUT, m_SelectionI->CanCut());
	pUpdateUI->UIEnable(ID_COPY, m_SelectionI->CanCopy());
	pUpdateUI->UIEnable(ID_PASTE, m_SelectionI->CanPaste());
	pUpdateUI->UIEnable(ID_ERASE, m_SelectionI->IsSelection());

*/
}

inline bool CMapEditorView::Flip() 
{
	m_SelectionI->FlipSelection();
	Invalidate();
	OnChange();
	return true;
}
inline bool CMapEditorView::Mirror() 
{
	m_SelectionI->MirrorSelection();
	Invalidate();
	OnChange();
	return true;
}
inline bool CMapEditorView::CWRotate()
{
	m_SelectionI->CWRotateSelection();
	Invalidate();
	OnChange();
	return true;
}
inline bool CMapEditorView::CCWRotate()
{
	m_SelectionI->CCWRotateSelection();
	Invalidate();
	OnChange();
	return true;
}
inline bool CMapEditorView::Delete()
{
	m_SelectionI->DeleteSelection();
	OnChangeSel();
	return true;
}
inline bool CMapEditorView::Copy()
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
inline bool CMapEditorView::Cut()
{
	if(!Copy()) return false;
	m_SelectionI->DeleteSelection();
	Invalidate();
	OnChangeSel();
	return true;
}
inline bool CMapEditorView::Paste()
{
	CPoint Point;
	GetCursorPos(&Point);
	ScreenToClient(&Point);
	m_pGraphicsI->GetWorldPosition(&Point);
	Paste(Point);
	return true;
}
bool CMapEditorView::InsertPlayer()
{
	return true;
}
bool CMapEditorView::SingleSel()
{
	m_bMulSelection = false;
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CMapEditorView::MultipleSel()
{
	m_bMulSelection = true;
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CMapEditorView::SelectAll()
{
	Invalidate();
	OnChangeSel();
	return true;
}
bool CMapEditorView::SelectNone()
{
	m_SelectionI->CleanSelection();
	Invalidate();
	OnChangeSel();
	return true;
}
bool CMapEditorView::Zoom(float zoom)
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
	SetScrollSize((int)((float)m_szMap.cx*m_Zoom), (int)((float)m_szMap.cy*m_Zoom));
	return true;
}
bool CMapEditorView::NoZoom()
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
bool CMapEditorView::ZoomIn()
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
bool CMapEditorView::ZoomOut()
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
bool CMapEditorView::ToggleMask()
{
	m_bShowMasks = !m_bShowMasks;
	Invalidate();
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CMapEditorView::ToggleBounds()
{
	m_bShowBoundaries = !m_bShowBoundaries;
	Invalidate();
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CMapEditorView::ToggleGrid()
{
	m_bShowGrid = !m_bShowGrid;
	m_SelectionI->SetSnapSize(m_nSnapSize, m_bShowGrid);
	Invalidate();
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CMapEditorView::TogleSnap()
{
	m_bSnapToGrid = !m_bSnapToGrid;
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}

inline bool CMapEditorView::Paste(CPoint &Point)
{
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

	OnChangeSel();
	Invalidate();
	return true;
}
bool CMapEditorView::hasChanged()
{
	if(m_SelectionI == NULL) return false;
	return m_SelectionI->IsModified();
}
