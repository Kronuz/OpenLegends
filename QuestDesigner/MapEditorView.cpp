/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germ�n M�ndez Bravo)
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


CMapEditorView::CMapEditorView(CMapEditorFrame *pParentFrame) :
	CGEditorView(pParentFrame),
	m_pGraphicsI(NULL),
	m_pMapGroupI(NULL),
	m_pSoundManager(NULL),
	m_SelectionI(NULL),

	m_szMap(0, 0),

	m_bShowMasks(false),
	m_bShowBoundaries(false)
{
}

// Called to translate window messages before they are dispatched 
BOOL CMapEditorView::PreTranslateMessage(MSG *pMsg)
{
	return FALSE;
}

// Called to clean up after window is destroyed (called when WM_NCDESTROY is sent)
void CMapEditorView::OnFinalMessage(HWND /*hWnd*/)
{
	CProjectFactory::Delete(&m_SelectionI);
	CGraphicsFactory::Delete(&m_pGraphicsI);
	delete this;
}

LRESULT CMapEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	LRESULT nResult = DefWindowProc();

	if(FAILED(CGraphicsFactory::New(&m_pGraphicsI, "GraphicsD3D8.dll"))) {
		MessageBox("Couldn't load graphics plugin, check plugin version.", "Quest Designer");
	}

	ASSERT(m_pGraphicsI);

	m_pGraphicsI->Initialize(GetMainFrame()->m_hWnd);

	m_pSoundManager = CProjectFactory::Interface()->GetSoundManager();

	bHandled = FALSE;

	return nResult;
}
LRESULT CMapEditorView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	if(GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) {
		if(m_pSoundManager && GetMainFrame()->m_bAllowSounds)
			m_pSoundManager->SwitchMusic(NULL, 0, false);
	}

	bHandled = FALSE;

	return 0;
}

// Called if the layer's combo box (in the toolbar) selection changes (other layer is selected):
LRESULT CMapEditorView::OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled)
{
	if(!m_pMapGroupI || !m_SelectionI) return 0;
	
	int nLayer = GetMainFrame()->m_ctrlLayers.GetCurSel();
	if(nLayer == CB_ERR) return 0;

	if(m_SelectionI->isLocked(nLayer) && m_SelectionI->Count()) {
		CONSOLE_PRINTF("Warning: Couldn't move the selection to the specified layer. \n  First unlock layer %s.\n", g_szLayerNames[nLayer]);
		MessageBeep((UINT)-1);
		return 0;
	}

	m_SelectionI->SetLayerSelection(nLayer);

	OnChangeSel(OCS_UPDATE); // was OnChangeSel(OCS_AUTO)
	UpdateView();

	return 0;
}

// Called if the status of an item in the layers's combo box (in the toolbar) is changed:
LRESULT CMapEditorView::OnStateChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled)
{
	if(!m_pMapGroupI || !m_SelectionI) return 0;

	int nLayer = GetMainFrame()->m_ctrlLayers.GetChanged();
	if(nLayer == CB_ERR) return 0;

	int nVisibleState = GetMainFrame()->m_ctrlLayers.GetItemState(nLayer, "1_Visible");
	int nLockedState = GetMainFrame()->m_ctrlLayers.GetItemState(nLayer, "2_Locked");
	if(nVisibleState == CB_ERR || nLockedState == CB_ERR) return 0;

	if(m_SelectionI->isHeld() && (nLockedState == 1)) {
		CONSOLE_PRINTF("Warning: Can not lock layers while there are held selections, release the selection first.\n");
		GetMainFrame()->m_ctrlLayers.SetItemState(nLayer, "2_Locked", 0);
		nLockedState = 0;
	}

	m_pMapGroupI->ShowLayer(nLayer, (nVisibleState == 0));
	m_SelectionI->LockLayer(nLayer, (nLockedState == 1));

	UpdateView();

	return 0;
}

LRESULT CMapEditorView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if(GetMainFrame()->GetOldFocus(tMapEditor) != m_hWnd) {
		if(m_pMapGroupI) {
			if(m_pSoundManager && GetMainFrame()->m_bAllowSounds)
				m_pSoundManager->SwitchMusic(m_pMapGroupI->GetMusic(), 0);
		}
	}

	if(GetMainFrame()->GetOldFocus(tAny) == m_hWnd) return 0;
	GetMainFrame()->SetOldFocus(tMapEditor, m_hWnd);


	if(!m_pMapGroupI || !m_SelectionI) {
		for(int nLayer=0; nLayer<MAX_LAYERS; nLayer++) {
			GetMainFrame()->m_ctrlLayers.SetItemState(nLayer, "1_Visible", 0);
			GetMainFrame()->m_ctrlLayers.SetItemState(nLayer, "2_Locked", 0);
		}
		GetMainFrame()->m_ctrlLayers.SetCurSel(0);
	}

	OnChangeSel(OCS_RENEW);

	bHandled = FALSE;

	return 0;
}

LRESULT CMapEditorView::OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	// stop the animations
	if(GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) return 0;

	// Capture the MapGroup (to update the world editor)
	if(m_pMapGroupI && m_SelectionI) {
		if(m_SelectionI->HasChanged()) {
			BITMAP *pBitmap = m_SelectionI->Capture(m_pGraphicsI, 0.25f);
			m_pMapGroupI->SetThumbnail(pBitmap);
		}
	}
	
	bHandled = FALSE;

	return 0;
}

void CMapEditorView::OnNoSound()
{
	if(m_pMapGroupI) {
		if(m_pSoundManager && GetMainFrame()->m_bAllowSounds) m_pSoundManager->SwitchMusic(m_pMapGroupI->GetMusic(), 0);
	}
}

bool CMapEditorView::DoMapOpen(CMapGroup *pMapGroupI, LPCTSTR lpszTitle) 
{
	m_pMapGroupI = pMapGroupI;
	if(FAILED(CProjectFactory::New(&m_SelectionI, reinterpret_cast<CDrawableContext**>(&m_pMapGroupI))))
		MessageBox("Couldn't load kernel, check kernel version.", "Quest Designer");

	ASSERT(m_pMapGroupI);
	if(!m_pMapGroupI) return false;

	m_pMapGroupI->GetSize(m_szMap);

	// Save file name for later
	m_sFilePath = m_pMapGroupI->GetMapGroupID();

	if(m_pSoundManager && GetMainFrame()->m_bAllowSounds)
		m_pSoundManager->SwitchMusic(m_pMapGroupI->GetMusic(), 0);

	// Save the title for later
	m_sTitle = lpszTitle;

	m_pParentFrame->m_sChildName = m_sFilePath;
	m_pParentFrame->SetTitle(m_sFilePath);
	m_pParentFrame->SetTabText(m_sTitle);

	CalculateLimits();
	SetScrollSize(m_rcScrollLimits.Size());

	m_SelectionI->SetSnapSize(m_nSnapSize, m_bShowGrid);

	BITMAP *pBitmap = m_SelectionI->Capture(m_pGraphicsI, 0.25f);
	m_pMapGroupI->SetThumbnail(pBitmap);

	OnChangeSel(OCS_RENEW);

	UpdateView();
	return true;
}

LRESULT CMapEditorView::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CMenu menu;
	if(!menu.CreatePopupMenu())
		return 0;

	CPoint Point(lParam);
	CPoint PopUpPoint(lParam);
	if(lParam == -1) {
		GetCursorPos(&Point);
		PopUpPoint = Point;
	} 
	::ScreenToClient((HWND)wParam, &Point);

	CRect Rect;
	::GetClientRect((HWND)wParam, &Rect);
	if(!Rect.PtInRect(Point)) {
		PopUpPoint = Rect.CenterPoint();
		::ClientToScreen((HWND)wParam, &PopUpPoint);
	}

	GetWorldPosition(&Point);

	if(isFloating()) {
		menu.AppendMenu(MF_STRING, 1, "&Mirror");
		menu.AppendMenu(MF_STRING, 2, "&Flip");
		menu.AppendMenu(MF_STRING, 3, "&Rotate CW 90�");
		menu.AppendMenu(MF_STRING, 4, "R&otate CCW 90�");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 7, "&Duplicate");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 11, "Cance&l");
	} else {
		menu.AppendMenu(MF_STRING, 1, "&Mirror");
		menu.AppendMenu(MF_STRING, 2, "&Flip");
		menu.AppendMenu(MF_STRING, 3, "&Rotate CW 90�");
		menu.AppendMenu(MF_STRING, 4, "R&otate CCW 90�");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 5, "Move &Top");
		menu.AppendMenu(MF_STRING, 6, "Move &Bottom");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 7, "&Duplicate");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 8, "&Copy");
		menu.AppendMenu(MF_STRING, 9, "&Paste");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 10, "De&lete");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 11, "Select &None");
	}

	menu.EnableMenuItem(9, MF_GRAYED);
	// are there more than one object selected?
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

	if(isHeld()) {
		menu.EnableMenuItem(1, MF_GRAYED);
		menu.EnableMenuItem(2, MF_GRAYED);
		menu.EnableMenuItem(3, MF_GRAYED);
		menu.EnableMenuItem(4, MF_GRAYED);
		menu.EnableMenuItem(5, MF_GRAYED);
		menu.EnableMenuItem(6, MF_GRAYED);
		menu.EnableMenuItem(7, MF_GRAYED);
		menu.EnableMenuItem(9, MF_GRAYED);
		menu.EnableMenuItem(10, MF_GRAYED);
	}
	
	int nCmd = menu.TrackPopupMenu(TPM_RETURNCMD, PopUpPoint.x, PopUpPoint.y, (HWND)wParam);

	switch(nCmd) {
		case 1:		Mirror();					break;
		case 2:		Flip();						break;
		case 3:		CWRotate();					break;
		case 4:		CCWRotate();				break;
		case 5:									break;
		case 6:									break;
		case 7:		
			if(isFloating()) {
				EndMoving(Point, NULL);
				Duplicate(Point);
				OnChangeSel(OCS_UPDATE);
			} else {
				Duplicate(Point);	
			}
			break;
		case 8:		Copy();						break;
		case 9:		Paste(Point);				break;
		case 10:	Delete();					break;
		case 11:	
			if(isFloating()) {
				CancelOperation();
				m_DragState = tNone;
				m_bDuplicating = false;
				ReleaseCapture();
				OnChangeSel(OCS_AUTO);
			} else {
				SelectNone();
			}
			break;
	}

	if(isFloating()) SetCapture();

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
	if(m_SelectionI && !isFloating()) nSelection = m_SelectionI->Count();
	if(m_SelectionI) pMapUpdateUI->UISetCheck(ID_MAPED_SELHOLD, m_SelectionI->isHeld());
	pMapUpdateUI->UIEnable(ID_MAPED_SELHOLD, (nSelection>1)?TRUE:FALSE);

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
		pMainFrm->m_ctrlLayers.EnableWindow(TRUE);
	}
	pMainUpdateUI->UIEnable(ID_APP_SAVE, hasChanged());	

/* Undo features and stuff will be left pending for the next major release
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
	OnChangeSel(OCS_UPDATE);
	return true;
}
inline bool CMapEditorView::Mirror() 
{
	m_SelectionI->MirrorSelection();
	Invalidate();
	OnChangeSel(OCS_UPDATE);
	return true;
}
inline bool CMapEditorView::CWRotate()
{
	m_SelectionI->CWRotateSelection();
	Invalidate();
	OnChangeSel(OCS_UPDATE);
	return true;
}
inline bool CMapEditorView::CCWRotate()
{
	m_SelectionI->CCWRotateSelection();
	Invalidate();
	OnChangeSel(OCS_UPDATE);
	return true;
}
bool CMapEditorView::InsertPlayer()
{
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

	return CGEditorView::OnIdle();
}
bool CMapEditorView::hasChanged()
{
	if(m_SelectionI == NULL) return false;
	return m_SelectionI->IsModified();
}

void CMapEditorView::GetWorldPosition(CPoint *_pPoint) 
{ 
	if(m_pGraphicsI) m_pGraphicsI->GetWorldPosition(_pPoint); 
}

void CMapEditorView::HoldOperation()
{
	return m_SelectionI->HoldOperation();
}

void CMapEditorView::CancelOperation()
{
	return m_SelectionI->Cancel();
}

bool CMapEditorView::isResizing()
{
	return m_SelectionI->isResizing();
}

bool CMapEditorView::isMoving()
{
	return m_SelectionI->isMoving();
}

bool CMapEditorView::isFloating()
{
	return m_SelectionI->isFloating();
}

bool CMapEditorView::isSelecting()
{
	return m_SelectionI->isSelecting();
}

void CMapEditorView::StartSelBox(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->StartSelBox(_Point);
}

void CMapEditorView::SizeSelBox(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->SizeSelBox(_Point);
}

IPropertyEnabled* CMapEditorView::EndSelBoxRemove(const CPoint &_Point, LPARAM lParam)
{
	m_SelectionI->EndSelBoxRemove(_Point);
	return NULL;
}

IPropertyEnabled* CMapEditorView::EndSelBoxAdd(const CPoint &_Point, LPARAM lParam)
{
	IPropertyEnabled *ret = m_SelectionI->EndSelBoxAdd(_Point, (int)lParam);
	return ret;
}

void CMapEditorView::CancelSelBox()
{
	m_SelectionI->CancelSelBox();
}

IPropertyEnabled* CMapEditorView::SelectPoint(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->CleanSelection();
	m_SelectionI->StartSelBox(_Point);
	m_SelectionI->EndSelBoxAdd(_Point,0);
	OnChangeSel(OCS_AUTO);
	*_pCursor = eIDC_SIZEALL;
	return NULL;
}

void CMapEditorView::GetSelectionBounds(CRect *_pRect)
{
	m_SelectionI->GetSelBounds(_pRect);
}
void CMapEditorView::StartMoving(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->StartMoving(_Point);
}

void CMapEditorView::MoveTo(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->MoveTo(_Point);
}

void CMapEditorView::EndMoving(const CPoint &_Point, LPARAM lParam)
{
	m_SelectionI->EndMoving(_Point);
}

void CMapEditorView::StartResizing(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->StartResizing(_Point);
}

void CMapEditorView::ResizeTo(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->ResizeTo(_Point);
}

void CMapEditorView::EndResizing(const CPoint &_Point, LPARAM lParam)
{
	m_SelectionI->EndResizing(_Point);
}

bool CMapEditorView::SelectedAt(const CPoint &_Point)
{
	return m_SelectionI->SelectedAt(_Point);
}

int CMapEditorView::SelectedCount()
{
	return m_SelectionI->Count();
}

void CMapEditorView::PasteSelection(LPVOID _pBuffer, const CPoint &_Point)
{
	m_SelectionI->SetLayer(GetMainFrame()->m_ctrlLayers.GetCurSel());
	m_SelectionI->Paste(_pBuffer, _Point);
}

HGLOBAL CMapEditorView::CopySelection(BITMAP **ppBitmap, bool bDeleteBitmap)
{
	return m_SelectionI->Copy(ppBitmap, bDeleteBitmap);
}

BITMAP* CMapEditorView::CaptureSelection(float _fZoom)
{
	BITMAP *pBitmap = m_SelectionI->CaptureSelection(m_pGraphicsI, _fZoom);
	m_SelectionI->SetThumbnail(pBitmap);
	return pBitmap;
}

void CMapEditorView::CleanSelection()
{
	m_SelectionI->CleanSelection();
}

int CMapEditorView::DeleteSelection()
{
	return m_SelectionI->DeleteSelection();
}
void CMapEditorView::HoldSelection(bool bHold)
{
	m_SelectionI->HoldSelection(bHold);
}
bool CMapEditorView::isHeld()
{
	return m_SelectionI->isHeld();
}
bool CMapEditorView::GetMouseStateAt(const CPoint &_Point, CURSOR *_pCursor)
{
	return m_SelectionI->GetMouseStateAt(m_pGraphicsI, _Point, _pCursor);
}

void CMapEditorView::CalculateLimits()
{
	m_rcScrollLimits.SetRect(
		0,
		0,
		(int)((float)m_szMap.cx*m_Zoom + 0.5f),
		(int)((float)m_szMap.cy*m_Zoom + 0.5f) 
	);
}

void CMapEditorView::UpdateSnapSize(int _SnapSize)
{
	m_SelectionI->SetSnapSize(_SnapSize, m_bShowGrid);
}

void CMapEditorView::Render()
{
	ASSERT(m_SelectionI);

	// Update timings and stuff for the animations
	CProjectFactory::Interface()->UpdateFPS();

	if(m_pSoundManager && GetMainFrame()->m_bAllowSounds)
		m_pSoundManager->DoMusic();

	// show entities, and if set, also sprite boundaries and masks
	WORD wFlags = SPRITE_ENTITIES | (m_bShowBoundaries?SPRITE_BOUNDS:0) | (m_bShowMasks?SPRITE_MASKS:0);
	m_SelectionI->Paint(m_pGraphicsI, wFlags);
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

void CMapEditorView::OnChangeSel(int type, IPropertyEnabled *pPropObj)
{
	if(!m_SelectionI || !m_pMapGroupI) return;

	HWND hWnd = GetMainFrame()->m_hWnd;

	if(isHeld() && type == OCS_AUTO || type == OCS_UPDATE) {
		::SendMessage(hWnd, WMP_UPDATE, 0, 0);
		return;
	}

	for(int nLayer=0; nLayer<MAX_LAYERS; nLayer++) {
		bool bVisible = m_pMapGroupI->isVisible(nLayer);
		bool bLocked = m_SelectionI->isLocked(nLayer);

		GetMainFrame()->m_ctrlLayers.SetItemState(nLayer, "1_Visible", bVisible?0:1);
		GetMainFrame()->m_ctrlLayers.SetItemState(nLayer, "2_Locked", bLocked?1:0);
	}
	GetMainFrame()->m_ctrlLayers.SetCurSel(m_SelectionI->GetLayer());

	::SendMessage(hWnd, WMP_CLEAR, 0, 0);

	SInfo Information;
	SObjProp *pOP = m_SelectionI->GetFirstSelection();
	while(pOP) {
		pOP->GetInfo(&Information);
		::SendMessage(hWnd, WMP_ADDINFO, (WPARAM)pPropObj, (LPARAM)&Information);
		pOP = m_SelectionI->GetNextSelection();
	}
	::SendMessage(hWnd, WMP_SETPROP, 0, 0);
}

