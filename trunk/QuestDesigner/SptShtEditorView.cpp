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
/*! \file		SptShtEditorView.cpp 
	\brief		Implementation of the CSptShtEditorView class.
	\date		April 27, 2003
*/

#include "stdafx.h"
#include "resource.h"

#include "MainFrm.h"
#include "SptShtEditorView.h"
#include "SptShtEditorFrm.h"

CSptShtEditorView::CSptShtEditorView(CSptShtEditorFrame *pParentFrame) :
	CGEditorView(pParentFrame),
	m_pSoundManager(NULL),
	m_pSpriteSheet(NULL),
	m_szSptSht(0, 0),
	m_nStep(16)
{
}

LRESULT CSptShtEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	LRESULT nResult = DefWindowProc();

	m_pSoundManager = CProjectFactory::Interface()->GetSoundManager();

	bHandled = FALSE;

	return nResult;
}
LRESULT CSptShtEditorView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	bHandled = FALSE;
	return 0;
}


LRESULT CSptShtEditorView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	// If the last Sprite Editor window to have the focus wasn't this one, do something...
	if(GetMainFrame()->GetOldFocus(tSpriteEditor) != m_hWnd) {
	}

	// If the last Registerd Window to have the focus was this very same window, do nothing:
	if(GetMainFrame()->GetOldFocus(tAny) == m_hWnd) return 0;
	GetMainFrame()->SetOldFocus(tSpriteEditor, m_hWnd); // leave a trace of our existence

	// Update the properties window:
	OnChangeSel(OCS_RENEW);

	return 0;
}

LRESULT CSptShtEditorView::OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	// check if we are still the focused window or other application's window is to be focused, 
	// if so, we just do nothing:
	if(wParam == NULL) return 0;
	if(GetParentFrame()->m_hWnd == (HWND)wParam) return 0;

	// we don't do anything anyway :P ... 
	return 0;
}


void CSptShtEditorView::OnParallax()
{
}
void CSptShtEditorView::OnAnim()
{
}
void CSptShtEditorView::OnSound()
{
}

bool CSptShtEditorView::DoFileOpen(LPCTSTR lpszFilePath, LPCTSTR lpszTitle, WPARAM wParam, LPARAM lParam)
{
	if(!DoFileClose()) return false;

	bool bRet = false;

	CSpriteSheet *pSpriteSheet = (CSpriteSheet *)wParam;

	char szFilePath[MAX_PATH];
	pSpriteSheet->GetFilePath(szFilePath, sizeof(szFilePath));

	m_fnFile = pSpriteSheet->GetFile();
	m_fScale = 1.0f;
	m_fnFile.SetFileExt(".png");
	if(!m_fnFile.FileExists()) {
		m_fScale = 2.0f;
		m_fnFile.SetFileExt(".bmp");
		if(!m_fnFile.FileExists()) {
			return false;
		}
	}
	int filesize = m_fnFile.GetFileSize();
	LPCVOID pData = m_fnFile.ReadFile();
	if(!pData) return false;

	LoadImage(&m_Image, pData, filesize);

	m_szSptSht.SetSize(m_Image.GetWidth(), m_Image.GetHeight());

	// Save file name for later
	m_sFilePath = szFilePath;

	// Save the title for later
	m_sTitle = lpszTitle;

	m_sInitSprite = (LPCSTR)lParam;

	m_pSpriteSheet = pSpriteSheet;

	m_pParentFrame->m_sChildName = m_sFilePath;
	m_pParentFrame->SetTitle(m_sFilePath);
	m_pParentFrame->SetTabText(m_sTitle);

	CalculateLimits();
	SetScrollSize(m_rcScrollLimits.Size());

	OnChangeSel(OCS_RENEW);

	UpdateView();
	Invalidate();
	return true;
}
// must return true if the file was truly closed, false otherwise.
bool CSptShtEditorView::DoFileClose()
{
	if(hasChanged()) {
		CString sSave;
		sSave.Format("Save Changes to %s?", GetTitle());
		int ret = MessageBox(sSave, _T("Quest Designer"), MB_YESNOCANCEL|MB_ICONWARNING);
		switch(ret) {
			case IDCANCEL: 
				return false;
			case IDYES: 
				if(!OnFileSave()) { 
					MessageBox("Couldn't save!", "Quest Designer", MB_OK|MB_ICONERROR); 
					return false; 
				}
			case IDNO: 
				return true;
		}
	}
	return true;
}
bool CSptShtEditorView::DoFileSave(LPCTSTR lpszFilePath)
{
	return false;
}
bool CSptShtEditorView::DoFileSaveAs()
{
	return false;
}
bool CSptShtEditorView::DoFileReload()
{
	return false;
}

LRESULT CSptShtEditorView::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return 0;
}

void CSptShtEditorView::UIUpdateStatusBar()
{
}
void CSptShtEditorView::UIUpdateMenuItems()
{
	// Get the main window's UI updater
	CMainFrame *pMainFrm = GetMainFrame();
	CUpdateUIBase *pMainUpdateUI = pMainFrm->GetUpdateUI();

	pMainUpdateUI->UIEnable(ID_APP_SAVE, hasChanged());	

/* Undo features and stuff will be left pending for the next major release */
	pMainFrm->UIEnable(ID_UNDO, CanUndo());
	pMainFrm->UIEnable(ID_REDO, CanRedo());

	pMainFrm->UIEnable(ID_CUT, CanCut());
	pMainFrm->UIEnable(ID_COPY, CanCopy());
	pMainFrm->UIEnable(ID_PASTE, CanPaste());
	pMainFrm->UIEnable(ID_ERASE, IsSelection());

}
BOOL CSptShtEditorView::CanUndo()
{
	return FALSE;
}
BOOL CSptShtEditorView::CanRedo()
{
	return FALSE;
}
BOOL CSptShtEditorView::CanCut()
{
	return FALSE;
}
BOOL CSptShtEditorView::CanCopy()
{
	return FALSE;
}
BOOL CSptShtEditorView::CanPaste()
{
	return FALSE;
}

BOOL CSptShtEditorView::IsSelection()
{
	return FALSE;
}
BOOL CSptShtEditorView::IsReadOnly()
{
	return FALSE;
}

// Called to do idle processing
BOOL CSptShtEditorView::OnIdle()
{
	// Update all the menu items
	UIUpdateMenuItems();
	
	// Update position display in the status bar
	UIUpdateStatusBar();

	CMainFrame *pMainFrm = GetMainFrame();
	CUpdateUIBase *pMainUpdateUI = pMainFrm->GetUpdateUI();

	// Update all the toolbar items
	pMainUpdateUI->UIUpdateToolBar();

	return CGEditorView::OnIdle();
}
bool CSptShtEditorView::hasChanged()
{
	return false;
}

void CSptShtEditorView::GetWorldPosition(CPoint *_pPoint) 
{ 
	_pPoint->x += GetScrollPos(SB_HORZ);
	_pPoint->y += GetScrollPos(SB_VERT);

	_pPoint->x = (int)((float)_pPoint->x / m_Zoom);
	_pPoint->y = (int)((float)_pPoint->y / m_Zoom);
}

void CSptShtEditorView::HoldOperation()
{
}

void CSptShtEditorView::CancelOperation()
{
}

bool CSptShtEditorView::isResizing()
{
	return false;
}

bool CSptShtEditorView::isMoving()
{
	return false;
}

bool CSptShtEditorView::isFloating()
{
	return false;
}

bool CSptShtEditorView::isSelecting()
{
	return false;
}

void CSptShtEditorView::StartSelBox(const CPoint &_Point, CURSOR *_pCursor)
{
}

void CSptShtEditorView::SizeSelBox(const CPoint &_Point, CURSOR *_pCursor)
{
}

IPropertyEnabled* CSptShtEditorView::EndSelBoxRemove(const CPoint &_Point, LPARAM lParam)
{
	return NULL;
}

IPropertyEnabled* CSptShtEditorView::EndSelBoxAdd(const CPoint &_Point, LPARAM lParam)
{
	return NULL;
}

void CSptShtEditorView::CancelSelBox()
{
}

IPropertyEnabled* CSptShtEditorView::SelectPoint(const CPoint &_Point, CURSOR *_pCursor)
{
	OnChangeSel(OCS_AUTO);
	*_pCursor = eIDC_SIZEALL;
	return NULL;
}

void CSptShtEditorView::GetSelectionBounds(CRect *_pRect)
{
}
void CSptShtEditorView::StartMoving(const CPoint &_Point, CURSOR *_pCursor)
{
}

void CSptShtEditorView::MoveTo(const CPoint &_Point, CURSOR *_pCursor)
{
}

void CSptShtEditorView::EndMoving(const CPoint &_Point, LPARAM lParam)
{
}

void CSptShtEditorView::StartResizing(const CPoint &_Point, CURSOR *_pCursor)
{
}

void CSptShtEditorView::ResizeTo(const CPoint &_Point, CURSOR *_pCursor)
{
}

void CSptShtEditorView::EndResizing(const CPoint &_Point, LPARAM lParam)
{
}

bool CSptShtEditorView::SelectedAt(const CPoint &_Point)
{
	return false;
}

int CSptShtEditorView::SelectedCount()
{
	return 0;
}

void CSptShtEditorView::PasteSelection(LPVOID _pBuffer, const CPoint &_Point)
{
}

HGLOBAL CSptShtEditorView::CopySelection(BITMAP **ppBitmap, bool bDeleteBitmap)
{
	return NULL;
}

BITMAP* CSptShtEditorView::CaptureSelection(float _fZoom)
{
	return NULL;
}

void CSptShtEditorView::CleanSelection()
{
}

int CSptShtEditorView::DeleteSelection()
{
	return 0;
}
void CSptShtEditorView::HoldSelection(bool bHold)
{
}
bool CSptShtEditorView::isHeld()
{
	return false;
}
bool CSptShtEditorView::GetMouseStateAt(const CPoint &_Point, CURSOR *_pCursor)
{
	return false;
}

void CSptShtEditorView::CalculateLimits()
{
	m_rcScrollLimits.SetRect(
		0,
		0,
		(int)((float)m_szSptSht.cx*m_Zoom*m_fScale + 0.5f),
		(int)((float)m_szSptSht.cy*m_Zoom*m_fScale + 0.5f) 
	);
}

void CSptShtEditorView::UpdateSnapSize(int _SnapSize)
{
}

void CSptShtEditorView::Render(WPARAM wParam)
{
	if(!wParam) return; 

	RECT rcSrc = {0, 0, m_szSptSht.cx, m_szSptSht.cy};

	CMemDC memdc( wParam ? (HDC)wParam : GetDC(), NULL );

	memdc.FillSolidRect(&memdc.m_rc, ::GetSysColor(COLOR_APPWORKSPACE));

	RECT rc = {0,0,0,0};
	for(int i=m_rcScrollLimits.top; i<m_rcScrollLimits.bottom; i+=m_nStep) {
		rc.top = rc.bottom;
		rc.bottom += m_nStep;
		rc.right = 0;
		int color = i / m_nStep;
		for(int j=m_rcScrollLimits.left; j<m_rcScrollLimits.right; j+=m_nStep) {
			rc.left = rc.right;
			rc.right += m_nStep;
			color++;
			if(rc.right > m_rcScrollLimits.right) rc.right = m_rcScrollLimits.right;
			if(rc.bottom > m_rcScrollLimits.bottom) rc.bottom = m_rcScrollLimits.bottom;
			if( (color&1)==1 ) memdc.FillRect(&rc, (HBRUSH)::GetStockObject(WHITE_BRUSH));
			else memdc.FillRect(&rc, (HBRUSH)::GetStockObject(LTGRAY_BRUSH));
		}
	}

	m_Image.AlphaBlend(memdc, m_rcScrollLimits, rcSrc);
}

void CSptShtEditorView::UpdateView()
{
	// We need to recalculate the window's size and position:
	CRect rcClient, rcClip;
	GetClientRect(&rcClient);
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	rcClip.SetRect(0, 0, m_szSptSht.cx, m_szSptSht.cy);
}

void CSptShtEditorView::OnChangeSel(int type, IPropertyEnabled *pPropObj)
{
	HWND hWnd = GetMainFrame()->m_hWnd;
	::SendMessage(hWnd, WMP_CLEAR, 0, (LPARAM)m_hWnd);
}

