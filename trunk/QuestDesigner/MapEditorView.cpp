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
/*! \file		MapEditorView.cpp 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the CMapEditorView class.
	\date		April 27, 2003
				July 15, 2005:
						+ Added undo/redo support
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
	m_bShowBoundaries(false),
	
	m_nCheckPoint(0),
	m_nCheckPointLimit(0)
{
}

LRESULT CMapEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	LRESULT lResult = DefWindowProc();

	bHandled = FALSE;

	if(FAILED(CGraphicsFactory::New(&m_pGraphicsI, "GraphicsD3D9.dll"))) {
		::PostMessage(GetParent(), WM_CLOSE, 0, 0);
		exit(1);
		return ERROR_FILE_NOT_FOUND;
	}

	ASSERT(m_pGraphicsI);

	if(!m_pGraphicsI->Initialize(GetMainFrame()->m_hWnd)) {
		::PostMessage(GetParent(), WM_CLOSE, 0, 0);
	}

	m_pSoundManager = CProjectFactory::Interface()->GetSoundManager();

	return lResult;
}
LRESULT CMapEditorView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	// check if we are still the topmost window in the MDI frame, if so, stop the music:
	if(GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) {
		if(m_pSoundManager && GetMainFrame()->m_bAllowSounds)
			m_pSoundManager->SwitchMusic(NULL, 0, false);
	}

	// Now close:
	if(m_SelectionI) {
		m_SelectionI->Cancel();
		m_SelectionI->CleanSelection();
	}
	if(m_pMapGroupI) {
		m_pMapGroupI->Close(true);
	}

	::SendMessage(GetMainFrame()->m_hWnd, WMP_CLEAR, 0, (LPARAM)m_hWnd);

	CProjectFactory::Delete(&m_SelectionI); m_SelectionI = NULL;
	CGraphicsFactory::Delete(&m_pGraphicsI); m_pGraphicsI = NULL;

	bHandled = FALSE;

	return 0;
}

// Called if the layer's combo box (in the toolbar) selection changes (other layer is selected):
LRESULT CMapEditorView::OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
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
	Invalidate();

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

	OnChangeSel(OCS_AUTO);
	Invalidate();

	return 0;
}

LRESULT CMapEditorView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	if(m_pMapGroupI && m_pSoundManager && GetMainFrame()->m_bAllowSounds) {
		// If the last Map Editor window to have the focus wasn't this one, switch the music:
		if(GetMainFrame()->GetOldFocus(tMapEditor) != m_hWnd) {
			m_pSoundManager->SwitchMusic(m_pMapGroupI->GetMusic(), 0);
		}
	}

	// If the last Registerd Window to have the focus was this very same window, do nothing:
	if(GetMainFrame()->GetOldFocus(tAny) == m_hWnd) return 0;
	GetMainFrame()->SetOldFocus(tMapEditor, m_hWnd); // leave a trace of our existence

	if(!m_pMapGroupI || !m_SelectionI) { // If the map isn't initialized, reset layers:
		for(int nLayer=0; nLayer<MAX_LAYERS; nLayer++) {
			GetMainFrame()->m_ctrlLayers.SetItemState(nLayer, "1_Visible", 0);
			GetMainFrame()->m_ctrlLayers.SetItemState(nLayer, "2_Locked", 0);
		}
		GetMainFrame()->m_ctrlLayers.SetCurSel(DEFAULT_LAYER);
	}

	// Update the properties window:
	OnChangeSel(OCS_RENEW);

	return 0;
}

LRESULT CMapEditorView::OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	// check if we are still the focused window or other application's window is to be focused, 
	// if so, we just do nothing:
	if(wParam == NULL) return 0;
	if(GetMainFrame() && GetParentFrame()->m_hWnd == GetMainFrame()->m_tabbedClient.GetTopWindow()) return 0;
	if(GetParentFrame()->m_hWnd == (HWND)wParam) return 0;

	// Capture the MapGroup (to update the world editor)
	if(m_pMapGroupI && m_SelectionI) {
		if(m_SelectionI->HasChanged()) {
			BITMAP *pBitmap = m_SelectionI->Capture(m_pGraphicsI, 0.25f); // (1/4 the size)
			m_pMapGroupI->SetThumbnail(pBitmap);
			m_pMapGroupI->SettleOriginalBitmap();
		}
	}

	::SendMessage(GetMainFrame()->m_hWnd, WMP_CLEAR, 0, (LPARAM)m_hWnd);

	return 0;
}

void CMapEditorView::OnParallax()
{
	OnAdjustLimits();
}
void CMapEditorView::OnAnim()
{
	Invalidate();
}
void CMapEditorView::OnSound()
{
	if(m_pMapGroupI) {
		if(m_pSoundManager && GetMainFrame()->m_bAllowSounds) m_pSoundManager->SwitchMusic(m_pMapGroupI->GetMusic(), 0);
	}
}
void CMapEditorView::OnMerge()
{
	if(m_pMapGroupI) {
		GetMainFrame()->UIEnableToolbar(FALSE);
		GetMainFrame()->UIUpdateToolBar();
		GetMainFrame()->UpdateWindow();

		CleanSelection();

		int nObjects = m_pMapGroupI->CountObjects();
	    GetMainFrame()->StatusBar("Merging objects...", IDI_ICO_WAIT);
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		DWORD dwInitTicks = GetTickCount();
		CONSOLE_PRINTF("Merging objects in the map...\n");
		//FIXME: perhaps doing this in a separated thread:
		int nMerged = m_pMapGroupI->MergeObjects();
		
		Checkpoint();
		CONSOLE_PRINTF("Done! %d/%d objects merged (compressed to %0.02f%%). (%d milliseconds)\n", nMerged, nObjects, 100.f*(1.0f-(float)nMerged/(float)nObjects), GetTickCount()-dwInitTicks);
		
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		GetMainFrame()->UIEnableToolbar(TRUE);
	    GetMainFrame()->StatusBar("Ready", IDI_ICO_OK);
	}
}
void CMapEditorView::OnUndo()
{
	if(m_pMapGroupI && CanUndo()) {
		m_nCheckPoint--;
		CONSOLE_DEBUG("Restored state (%d)\n", m_nCheckPoint);
		if(m_pMapGroupI->RestoreState(m_nCheckPoint)) 
			CleanSelection(); //FIXME: selections should also be restored
	}
}

void CMapEditorView::OnRedo()
{
	if(m_pMapGroupI && CanRedo()) {
		m_nCheckPoint++;
		CONSOLE_DEBUG("Restored state (%d)\n", m_nCheckPoint);
		if(m_pMapGroupI->RestoreState(m_nCheckPoint))
			CleanSelection(); //FIXME: selections should also be restored
	}
}

bool CMapEditorView::DoFileOpen(LPCTSTR lpszFilePath, LPCTSTR lpszTitle, WPARAM wParam, LPARAM lParam) 
{
	if(!DoFileClose()) return false;

	m_pMapGroupI = (CMapGroup *)wParam;

	if(FAILED(CProjectFactory::New(&m_SelectionI, reinterpret_cast<CDrawableContext**>(&m_pMapGroupI)))) {
		MessageBox("Couldn't load kernel, check kernel version.", QD_MSG_TITLE);
		return false;
	}

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

	CONSOLE_DEBUG("Saved state (%d)\n", m_nCheckPoint);
	m_pMapGroupI->SaveState(m_nCheckPoint);
	m_pMapGroupI->HasChanged(); // make sure the object 
	m_pMapGroupI->WasSaved();	// is kept unchanged.

	OnChangeSel(OCS_RENEW);

	UpdateView();
	Invalidate();
	return true;
}
// must return true if the file was truly closed, false otherwise.
bool CMapEditorView::DoFileClose()
{
	if(hasChanged()) {
		CString sSave;
		sSave.Format("Save Changes to '%s'?", GetTitle());
		int ret = MessageBox(sSave, QD_MSG_TITLE, MB_YESNOCANCEL|MB_ICONWARNING);
		switch(ret) {
			case IDCANCEL: 
				return false;
			case IDYES: 
				if(!OnFileSave()) { 
					MessageBox("Couldn't save!", QD_MSG_TITLE, MB_OK|MB_ICONERROR); 
					return false; 
				}
		}
	}
	// OnDestroy closes the file.
	return true;
}
bool CMapEditorView::DoFileSave(LPCTSTR lpszFilePath)
{
	bool bRet = m_pMapGroupI->Save();
	if(bRet) {
		m_SelectionI->WasSaved(); // Let the Selection know it was saved
		m_pMapGroupI->WasSaved();
		// Capture the MapGroup (to update the world editor)
		BITMAP *pBitmap = m_SelectionI->Capture(m_pGraphicsI, 0.25f); // (1/4 the size)
		m_pMapGroupI->SetThumbnail(pBitmap);
	}
	return bRet;
}
bool CMapEditorView::DoFileSaveAs()
{
	return false;
}
bool CMapEditorView::DoFileReload()
{
	return false;
}

void CMapEditorView::RunPopUpCmd(int nCmd)
{
	CPoint Point;
	switch(nCmd) {
		case 1:		Mirror();					break;
		case 2:		Flip();						break;
		case 3:		CWRotate();					break;
		case 4:		CCWRotate();				break;
		case 5:		ToTop();					break;
		case 6:		ToBottom();					break;
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
		case 12: {
			SObjProp *pObjProp = m_SelectionI->GetFirstSelection();
			ATLASSERT(pObjProp);
			if(pObjProp && pObjProp->pContext) {
				CSprite *pSprite = static_cast<CSprite *>(pObjProp->pContext->GetDrawableObj());
				CSpriteSheet *pSpriteSheet = pSprite->GetSpriteSheet();

				GetMainFrame()->SptShtFileOpen(pSpriteSheet, (LPCSTR)pSprite->GetName());
			}
			break;
		}
		case 13: {
			SObjProp *pObjProp = m_SelectionI->GetFirstSelection();
			ATLASSERT(pObjProp);
			if(pObjProp && pObjProp->pContext) {
				CSprite *pSprite = static_cast<CSprite *>(pObjProp->pContext->GetDrawableObj());
				const IScript *pScript = GetMainFrame()->m_pOLKernel->GetScript(pSprite);
				 // if the selected sprite is an entity:
				if(pScript) {
					char szScriptFile[MAX_PATH];
					pScript->GetScriptFilePath(szScriptFile, MAX_PATH);
					if(!GetMainFrame()->ScriptFileOpen(szScriptFile)) {
						CONSOLE_PRINTF("Error: Couldn't load script.");
					}
				} else {
					CONSOLE_PRINTF("Warning: Script not found, or not an entity.");
				}
			}
			break;
		}
		case 14: {
			m_SelectionI->SelectionToGroup("NewGroup");
			Checkpoint();
			OnChangeSel(OCS_AUTO);
			break;
		}
		case 15: {
			m_SelectionI->GroupToSelection();
			Checkpoint();
			OnChangeSel(OCS_AUTO);
			break;
		}
	}
}

LRESULT CMapEditorView::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	ATLASSERT(m_SelectionI);
	if(!m_SelectionI) return 0;

	CMenu menu;
	if(!menu.CreatePopupMenu())
		return 0;

	bool bIsEntity = false;
	CSprite *pSprite = NULL;

	SObjProp *pObjProp = m_SelectionI->GetFirstSelection();
	if(pObjProp && pObjProp->pContext) {
		pSprite = static_cast<CSprite *>(pObjProp->pContext->GetDrawableObj());
		if(pSprite && pSprite->GetSpriteType() == tEntity) {
			bIsEntity = true;
		}
	}

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

	ViewToWorld(&Point);

	if(SelectedCount() == 1) {
		menu.AppendMenu(MF_STRING, 12, "Edit &Sprite"); // Edit Sprite
		if(bIsEntity) {
			menu.AppendMenu(MF_STRING, 13, "Edit Sprite S&cript"); // Edit Sprite Script
			SetMenuDefaultItem(menu.m_hMenu, 13, FALSE);
		} else {
			SetMenuDefaultItem(menu.m_hMenu, 12, FALSE);
		}
		menu.AppendMenu(MF_SEPARATOR);
	}
	if(isFloating()) {
		menu.AppendMenu(MF_STRING, 1, "&Mirror");
		menu.AppendMenu(MF_STRING, 2, "&Flip");
		menu.AppendMenu(MF_STRING, 3, "&Rotate CW 90º");
		menu.AppendMenu(MF_STRING, 4, "R&otate CCW 90º");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 7, "&Duplicate");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 11, "Cance&l");
	} else {
		menu.AppendMenu(MF_STRING, 1, "&Mirror");
		menu.AppendMenu(MF_STRING, 2, "&Flip");
		menu.AppendMenu(MF_STRING, 3, "&Rotate CW 90º");
		menu.AppendMenu(MF_STRING, 4, "R&otate CCW 90º");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 5, "Bring To &Top");
		menu.AppendMenu(MF_STRING, 6, "Send To &Bottom");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 7, "&Duplicate");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 8, "&Copy");
		menu.AppendMenu(MF_STRING, 9, "&Paste");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, 10, "De&lete");
		if(SelectedCount()) {
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, 11, "Select &None"); // Select None
		}
	}
	if(m_SelectionI->isGroup())	menu.AppendMenu(MF_STRING, 15, "&Ungroup");
	else if(m_SelectionI->Count()>1) menu.AppendMenu(MF_STRING, 14, "&Group");

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
	if(isFloating()) {
		menu.EnableMenuItem(12, MF_GRAYED);
		menu.EnableMenuItem(13, MF_GRAYED);
		menu.EnableMenuItem(7, MF_GRAYED);
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

	if(nCmd) {
		m_bIgnoreNextButton = true;
		RunPopUpCmd(nCmd);
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
	if(pMainFrm->m_PaneWindows.size() >= (ID_VIEW_PANELAST-ID_VIEW_PANEFIRST)) {
		CTabbedDockingWindow *pThumbnails = pMainFrm->m_PaneWindows[ID_PANE_THUMBNAILS-ID_VIEW_PANEFIRST];
		if(pThumbnails->IsWindowVisible() == FALSE) {
			pThumbnails->Toggle();
			pThumbnails->Activate();
		}
		m_hContainer = pThumbnails->GetClient();
	}

	pMainUpdateUI->UIEnable(ID_APP_SAVE, hasChanged());	

/* Undo features and stuff will be left pending for the next major release */
	pMainFrm->UIEnable(ID_UNDO, CanUndo());
	pMainFrm->UIEnable(ID_REDO, CanRedo());

	pMainFrm->UIEnable(ID_CUT, CanCut());
	pMainFrm->UIEnable(ID_COPY, CanCopy());
	pMainFrm->UIEnable(ID_PASTE, CanPaste());
	pMainFrm->UIEnable(ID_ERASE, IsSelection());

	pMainFrm->UIEnable(ID_MAPED_MERGE, TRUE);

}
BOOL CMapEditorView::CanUndo()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld() || isFloating()) return FALSE;
	if(m_nCheckPoint>0) return TRUE;
	return FALSE;
}
BOOL CMapEditorView::CanRedo()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld() || isFloating()) return FALSE;
	if(m_nCheckPoint<m_nCheckPointLimit) return TRUE;
	return FALSE;
}
BOOL CMapEditorView::CanCut()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld() || isFloating()) return FALSE;
	return (m_SelectionI->Count() != 0);
}
BOOL CMapEditorView::CanCopy()
{
	if(!m_SelectionI) return FALSE;
	if(isFloating()) return FALSE;
	return (m_SelectionI->Count() != 0);
}
BOOL CMapEditorView::CanPaste()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld() || isFloating()) return FALSE;
	return TRUE;
}

BOOL CMapEditorView::IsSelection()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld() || isFloating()) return FALSE;
	return (m_SelectionI->Count() != 0);
}
BOOL CMapEditorView::IsReadOnly()
{
	if(!m_SelectionI) return FALSE;
	return FALSE;
}

inline bool CMapEditorView::ToTop()
{
	m_SelectionI->SelectionToTop();
	Invalidate();
	OnChangeSel(OCS_UPDATE);
	return true;
}
inline bool CMapEditorView::ToBottom()
{
	m_SelectionI->SelectionToBottom();
	Invalidate();
	OnChangeSel(OCS_UPDATE);
	return true;
}
inline bool CMapEditorView::ObjectDown()
{
	m_SelectionI->SelectionDown();
	Invalidate();
	OnChangeSel(OCS_UPDATE);
	return true;
}
inline bool CMapEditorView::ObjectUp()
{
	m_SelectionI->SelectionUp();
	Invalidate();
	OnChangeSel(OCS_UPDATE);
	return true;
}

inline bool CMapEditorView::AlignTop()
{
	return false;
}
inline bool CMapEditorView::AlignBottom()
{
	return false;
}
inline bool CMapEditorView::AlignRight()
{
	return false;
}
inline bool CMapEditorView::AlignLeft()
{
	return false;
}
inline bool CMapEditorView::AlignCenter()
{
	return false;
}
inline bool CMapEditorView::AlignMiddle()
{
	return false;
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

	bool bModified = hasChanged();
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
bool CMapEditorView::hasChanged()
{
	bool bRet = false;
	if(m_pMapGroupI) bRet |= m_pMapGroupI->IsModified();
	if(m_SelectionI) bRet |= m_SelectionI->IsModified();
	return bRet;
}

void CMapEditorView::ViewToWorld(CPoint *_pPoint) 
{ 
	if(m_pGraphicsI) m_pGraphicsI->ViewToWorld(_pPoint); 
}

void CMapEditorView::HoldOperation()
{
	return m_SelectionI->HoldOperation();
}

void CMapEditorView::CancelOperation(bool bPropagate)
{
	if(bPropagate) {
		CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
		pMainFrm->m_ThumbnailsBox.m_sSelected = "";
	}
	m_sPasting = "";
	return m_SelectionI->Cancel();
}

bool CMapEditorView::isResizing()
{
	return m_SelectionI && m_SelectionI->isResizing();
}

bool CMapEditorView::isMoving()
{
	return m_SelectionI && m_SelectionI->isMoving();
}

bool CMapEditorView::isFloating()
{
	return m_SelectionI && m_SelectionI->isFloating();
}

bool CMapEditorView::isSelecting()
{
	return m_SelectionI && m_SelectionI->isSelecting();
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
	m_SelectionI->EndSelBoxAdd(_Point, 0);
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

// Called after a zooming:
void CMapEditorView::OnZoom()
{
	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("%4d%%"), (int)(100.0f * m_Zoom));
	pStatusBar->SetPaneText(ID_OVERTYPE_PANE, sText);
}

void CMapEditorView::CleanSelection()
{
	m_SelectionI->CleanSelection();
	OnChangeSel(OCS_AUTO);
}

int CMapEditorView::DeleteSelection()
{
	int nRet = m_SelectionI->DeleteSelection();
	OnChangeSel(OCS_AUTO);
	return nRet;
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
	int nVisibleX = (int)((float)m_szMap.cx * m_Zoom + 0.5f);
	int nVisibleY = (int)((float)m_szMap.cy * m_Zoom + 0.5f);

	if(GetMainFrame()->m_bAllowParallax) {
		CRect Rect;
		GetClientRect(&Rect);
		nVisibleX = Rect.Width() + (int)((float)(m_szMap.cx - 640) * m_Zoom + 0.5f);
		nVisibleY = Rect.Height() + (int)((float)(m_szMap.cy - 480) * m_Zoom + 0.5f);
	}

	m_rcScrollLimits.SetRect(
		0,
		0,
		nVisibleX,
		nVisibleY
	);
}

void CMapEditorView::UpdateSnapSize(int _SnapSize)
{
	m_SelectionI->SetSnapSize(_SnapSize, m_bShowGrid);
}

void CMapEditorView::DoFrame()
{
	ASSERT(m_SelectionI);

	// Update timings and stuff for the animations
	CProjectFactory::Interface()->UpdateFPS();
	
	if(m_pSoundManager && GetMainFrame()->m_bAllowSounds)
		m_pSoundManager->DoMusic();
}
void CMapEditorView::Render(WPARAM wParam)
{
	ASSERT(m_SelectionI);
	if(!m_SelectionI) return;

	if(!GetMainFrame()->m_bAllowAnimations && wParam == NULL) return;

	CRect rcView;
	m_pGraphicsI->GetVisibleRect(&rcView);

	// resolution on which the parallax will be used:
	rcView.right = rcView.left + 640;
	rcView.bottom = rcView.top + 480;

	if(GetMainFrame()->m_bAllowParallax) {
		m_pMapGroupI->CalculateParallax(&rcView);
		m_SelectionI->SetClip(&rcView, COLOR_ARGB(128, 255, 255, 255)); // Clip the window
	} else {
		m_pMapGroupI->CalculateParallax(NULL);
		m_SelectionI->SetClip(NULL);
	}

	// show entities, and if set, also sprite boundaries and masks
	WORD wFlags = GRAPHICS_FILTERS | SPRITE_ENTITIES | (m_bShowBoundaries?SPRITE_BOUNDS:0) | (m_bShowMasks?SPRITE_MASKS:0);
	m_SelectionI->Paint(m_pGraphicsI, wFlags);
}

void CMapEditorView::UpdateView()
{
	// We need to recalculate the window's size and position:
	CRect rcClient, rcClip;
	GetClientRect(&rcClient);
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	rcClip.SetRect(0, 0, m_szMap.cx, m_szMap.cy);
	m_pGraphicsI->SetWindowView(m_hWnd, m_Zoom, &rcClient, &rcClip);
}

void CMapEditorView::Checkpoint()
{
	if(m_pMapGroupI->HasChanged()) {
		m_nCheckPoint++;
		if(m_pMapGroupI->SaveState(m_nCheckPoint)) {
			CONSOLE_DEBUG("Saved state (%d)\n", m_nCheckPoint);
			m_nCheckPointLimit = m_nCheckPoint;
		} else m_nCheckPoint--; // nothing was saved anyway.
	}
}

void CMapEditorView::OnChangeSel(int type, IPropertyEnabled *pPropObj)
{
	if(!m_SelectionI || !m_pMapGroupI) return;

	HWND hWnd = GetMainFrame()->m_hWnd;

	if( isHeld() && type == OCS_AUTO || 
		type == OCS_UPDATE ) {
		::SendMessage(hWnd, WMP_UPDATE, 0, 0);
		return;
	}

	int nLayer;
	for(nLayer=0; nLayer<MAX_LAYERS; nLayer++) {
		bool bVisible = m_pMapGroupI->isVisible(nLayer);
		bool bLocked = m_SelectionI->isLocked(nLayer);

		GetMainFrame()->m_ctrlLayers.SetItemState(nLayer, "1_Visible", bVisible?0:1);
		GetMainFrame()->m_ctrlLayers.SetItemState(nLayer, "2_Locked", bLocked?1:0);
	}
	nLayer = m_SelectionI->GetLayer();
	if(nLayer!=-1) GetMainFrame()->m_ctrlLayers.SetCurSel(nLayer);

	::SendMessage(hWnd, WMP_CLEAR, 0, (LPARAM)m_hWnd);

	SInfo Information;

	// Add te map group to the properties window:
	m_pMapGroupI->GetInfo(&Information);
	m_pMapGroupI->Flag(m_SelectionI->Count() == 0 || m_SelectionI->isFloating());
	::SendMessage(hWnd, WMP_ADDINFO, NULL, (LPARAM)&Information);

	if(!m_SelectionI->isFloating()) {
		// add every selected object to the properties window:
		SObjProp *pOP = m_SelectionI->GetFirstSelection();
		while(pOP) {
			pOP->GetInfo(&Information);
			::SendMessage(hWnd, WMP_ADDINFO, (WPARAM)pPropObj, (LPARAM)&Information);
			pOP = m_SelectionI->GetNextSelection();
		}
	}

	::SendMessage(hWnd, WMP_SETPROP, 0, 0);
}

LRESULT CMapEditorView::OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if( wParam >= '0' || wParam <= '9' ) {
		if(GetMainFrame()->m_ctrlLayers.SetCurSel(wParam - '0') != CB_ERR) {
			BOOL bDummy;
			OnSelChange(0,0,0,bDummy);
		}
	}

	bHandled = FALSE;
	return 0;
}
LRESULT CMapEditorView::OnLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CDrawableContext *pDrawableContext = m_SelectionI->GetLastSelected();

	if(pDrawableContext) {
		SObjProp *pObjProp = m_SelectionI->GetFirstSelection();
		if(pObjProp && pObjProp->pContext) {
			CSprite *pSprite = static_cast<CSprite *>(pObjProp->pContext->GetDrawableObj());
			if(pSprite && pSprite->GetSpriteType() == tEntity) {
				RunPopUpCmd(13);
			}
			else {
				RunPopUpCmd(12);
			}
		}
	}

	return 0;
}
LRESULT CMapEditorView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CPoint Point(lParam);
	ViewToWorld(&Point);

	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	if(m_SelectionI && !m_bPanning) {
		bool bNewSelected = (m_sPasting != pMainFrm->m_ThumbnailsBox.m_sSelected);
		if( bNewSelected || !m_SelectionI->isFloating()) {
			if(bNewSelected || !m_sPasting.IsEmpty()) {
				m_SelectionI->Cancel();
				m_SelectionI->CleanSelection();
				OnChangeSel(OCS_AUTO);
			}
			m_sPasting = pMainFrm->m_ThumbnailsBox.m_sSelected;
			if(!m_sPasting.IsEmpty()) {
				//LPCSTR szToPaste = pMainFrm->m_ThumbnailsBox.FindSpriteSet(m_sPasting);
				if(!m_SelectionI->Paste((LPVOID)(LPCSTR)m_sPasting, Point))
					pMainFrm->m_ThumbnailsBox.m_sSelected = ""; // there was an error pasting the sprite
			}
		}
		if(!m_sPasting.IsEmpty()) ReleaseCapture();
	}

	::CGEditorView::OnMouseMove(uMsg, wParam, lParam, bHandled);

	CMultiPaneStatusBarCtrl *pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	CString sText;
	sText.Format(_T("X: %3d, Y: %3d"), Point.x, Point.y);
	pStatusBar->SetPaneText(ID_POSITION_PANE, sText);

	//if(::GetFocus() != m_hWnd && ::GetFocus()) ::SetFocus(m_hWnd);

	return 0;
}
