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
/*! \file		ThumbnailsBox.cpp 
	\brief		Implementation of the CThumbnailsBox class.
	\date		April 27, 2003
*/

#include "stdafx.h"
#include "resource.h"

#include "MainFrm.h"
#include "ThumbnailsBox.h"

#include "GraphicsFactory.h"


CThumbnailsBox::CThumbnailsBox() :
	CGEditorView(NULL),
	m_pGraphicsI(NULL),
	m_SelectionI(NULL),

	m_szMap(0, 0),

	m_bShowMasks(false),
	m_bShowBoundaries(false),

	m_pMainFrame(NULL)
{
	m_Zoom = 0.5f;
	m_bMulSelection = false;
	m_bSnapToGrid = false;
	m_bAllowSnapOverride = false;
	m_bAllowAutoScroll = false;
}
LRESULT CThumbnailsBox::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	LRESULT lResult = DefWindowProc();

	bHandled = FALSE;

	if(FAILED(CGraphicsFactory::New(&m_pGraphicsI, "GraphicsD3D9.dll"))) {
		PostMessage(WM_CLOSE);
		exit(1);
		return ERROR_FILE_NOT_FOUND;
	}

	ASSERT(m_pGraphicsI);

	if(!m_pGraphicsI->Initialize(m_hWnd)) {
		PostMessage(WM_CLOSE);
	}

	return lResult;
}

int CALLBACK CThumbnailsBox::WalkSpriteSetsDir(LPCTSTR szFile, DWORD dwFileAttributes, LPARAM lParam)
{
	int nRet = 0;
	CThumbnailsBox *This = (CThumbnailsBox *)lParam;

	// if it is a directory:
	if((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
		CString sWildcards;
		sWildcards.Format("%s\\*.*", szFile);
		CVFile vFile(sWildcards);
		nRet += vFile.ForEachFile(CThumbnailsBox::WalkSpriteSetsDir, lParam);
		This->m_pCurrentSheet = (CSpriteSheet*)-1; // AddSpriteSet uses this as a flag.
		nRet += vFile.ForEachFile(CThumbnailsBox::AddSpriteSet, lParam);
	}
	return nRet;
}

int CALLBACK CThumbnailsBox::AddSpriteSet(LPCTSTR szFile, DWORD dwFileAttributes, LPARAM lParam)
{
	CThumbnailsBox *This = (CThumbnailsBox *)lParam;

	// Height of the sprites' row:
	const int nHeight	= 136;

	// if it is a directory:
	if((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
		return 0;
	}

	CSize Size;
	if(!This->m_SelectionI->GetPastedSize((LPCVOID)szFile, &Size)) return 0;

	// Now that we know that the new sprite is going to be pasted:
	if(This->m_pCurrentSheet != NULL || This->m_Point.x > 2000) {
		This->m_pCurrentSheet = NULL;
		if(This->m_Point.x <= 2000) This->m_nMaxHeight += 32;
		This->m_Point.y = This->m_Rect.bottom = This->m_nMaxHeight;
		This->m_Point.x = This->m_Rect.right = 32;
		This->m_nColWidth = 0;
		This->m_nMaxHeight = 0;
	}

	// Save the column width of the forst sprite in the column:
	if(This->m_nColWidth == 0) This->m_nColWidth = Size.cx;

	// Make sure the sprite has the same or less width than all of the sprites above:
	if(Size.cx > This->m_nColWidth) {
		// Start a new column
		This->m_nColWidth = Size.cx;
		This->m_Point.y = This->m_Rect.bottom;
		This->m_Point.x = This->m_Rect.right;
	} else {
		// Make sure the new sprite wont go off the bottom (more than one sprite)
		if( This->m_Point.y + Size.cy > This->m_Rect.bottom + nHeight) {
			// Start a new column
			This->m_nColWidth = Size.cx;
			This->m_Point.y = This->m_Rect.bottom;
			This->m_Point.x = This->m_Rect.right;
		}
	}

	// now paste the sprite:
	if(!This->m_SelectionI->FastPaste((LPCVOID)szFile, This->m_Point)) return 0;
	char szName[MAX_PATH]; // pasted name
	This->m_SelectionI->GetSelectionName(szName, sizeof(szName));
	if(*szName == '@') {
		This->m_SelectionI->SetSelectionName(szFile);
	}

	This->m_Point.y += (Size.cy + 4);
	This->m_Rect.right = This->m_Point.x + This->m_nColWidth + 4;
	if(This->m_Point.y > This->m_nMaxHeight) This->m_nMaxHeight = This->m_Point.y;

	if(This->m_nMaxHeight >This->m_szMap.cy) This->m_szMap.cy = This->m_nMaxHeight;
	if(This->m_Rect.Width() > This->m_szMap.cx) This->m_szMap.cx = This->m_Rect.Width();

	return 1;
}
int CALLBACK CThumbnailsBox::AddSprite(LPVOID Interface, LPARAM lParam)
{
	CThumbnailsBox *This = (CThumbnailsBox *)lParam;

	CSprite *pSprite = (CSprite *)Interface;

	// Height of the sprites' row:
	const int nHeight = 136;

	// check if the sprite is to be shown in the catalog:
	if(pSprite->GetCatalogOrder() == -1) return 0;

	if(This->m_pCurrentSheet != pSprite->GetSpriteSheet() || This->m_Point.x > 2000) {
		This->m_pCurrentSheet = pSprite->GetSpriteSheet();
		if(This->m_Point.x <= 2000) This->m_nMaxHeight += 32;
		This->m_Point.y = This->m_Rect.bottom = This->m_nMaxHeight;
		This->m_Point.x = This->m_Rect.right = 32;
		This->m_nColWidth = 0;
		This->m_nMaxHeight = 0;
	}

	CSize Size;
	if(!This->m_SelectionI->GetPastedSize(pSprite, &Size)) return 0;

	if(This->m_nColWidth == 0) This->m_nColWidth = Size.cx;

	// Make sure the sprite has the same or less width than all of the sprites above:
	if(Size.cx > This->m_nColWidth) {
		// Start a new column
		This->m_nColWidth = Size.cx;
		This->m_Point.y = This->m_Rect.bottom;
		This->m_Point.x = This->m_Rect.right;
	} else {
		// Make sure the new tile wont go off the bottom (more than one sprite)
		if( This->m_Point.y + Size.cy > This->m_Rect.bottom + nHeight) {
			// Start a new column
			This->m_nColWidth = Size.cx;
			This->m_Point.y = This->m_Rect.bottom;
			This->m_Point.x = This->m_Rect.right;
		}
	}
	// now paste the sprite:
	if(!This->m_SelectionI->FastPaste(pSprite, This->m_Point)) return 0;

	This->m_Point.y += (Size.cy + 4);
	This->m_Rect.right = This->m_Point.x + This->m_nColWidth + 4;
	if(This->m_Point.y > This->m_nMaxHeight) This->m_nMaxHeight = This->m_Point.y;

	if(This->m_nMaxHeight >This->m_szMap.cy) This->m_szMap.cy = This->m_nMaxHeight;
	if(This->m_Rect.Width() > This->m_szMap.cx) This->m_szMap.cx = This->m_Rect.Width();

	return 1;
}

void CThumbnailsBox::OnRefresh()
{
    m_pMainFrame->StatusBar("Building thumbnails...", IDI_ICO_WAIT);

	m_szMap.SetSize(0, 0);
	m_Point.SetPoint(0, 0);
	m_Rect.SetRectEmpty();
	m_nColWidth = 0;
	m_nMaxHeight = 0;

	m_sSelected = "";
	
	m_SelectionI->CleanSelection();
	m_SelectionI->CleanPasteGroups();
	m_pThumbnails->CleanThumbnails();

	CVFile vFile("Sprite Sets\\*.*");
	vFile.ForEachFile(CThumbnailsBox::WalkSpriteSetsDir, (LPARAM)this);
	m_pCurrentSheet = (CSpriteSheet*)-1; // AddSpriteSet uses this as a flag.
	vFile.ForEachFile(CThumbnailsBox::AddSpriteSet, (LPARAM)this);

	m_pCurrentSheet = NULL;
	CProjectFactory::Interface()->ForEachSprite(CThumbnailsBox::AddSprite, (LPARAM)this);

	m_szMap.cx += 16;
	m_szMap.cy += 16;

	CalculateLimits();
	SetScrollSize(m_rcScrollLimits.Size());

    m_pMainFrame->StatusBar("Ready", IDI_ICO_OK);
}

LRESULT CThumbnailsBox::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	if(m_SelectionI) {
		m_SelectionI->CleanSelection();
		CProjectFactory::Delete(&m_SelectionI); m_SelectionI = NULL;
	}

	CGraphicsFactory::Delete(&m_pGraphicsI); m_pGraphicsI = NULL;


	bHandled = FALSE;

	return 0;
}

// Called if the layer's combo box (in the toolbar) selection changes (other layer is selected):
LRESULT CThumbnailsBox::OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	return 0;
}

// Called if the status of an item in the layers's combo box (in the toolbar) is changed:
LRESULT CThumbnailsBox::OnStateChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled)
{
	return 0;
}

LRESULT CThumbnailsBox::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	return 0;
}

LRESULT CThumbnailsBox::OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	// check if we are still the focused window or other application's window is to be focused, 
	// if so, we just do nothing:
	if(wParam == NULL) return 0;

	return 0;
}

void CThumbnailsBox::OnParallax()
{
	OnAdjustLimits();
}
void CThumbnailsBox::OnAnim()
{
	Invalidate();
}
void CThumbnailsBox::OnSound()
{
}

bool CThumbnailsBox::DoFileOpen(LPCTSTR lpszFilePath, LPCTSTR lpszTitle, WPARAM wParam, LPARAM lParam) 
{
	if(!DoFileClose()) return false;

	m_pThumbnails = CProjectFactory::Interface()->GetThumbnails();

	ASSERT(m_pThumbnails);
	if(!m_pThumbnails) return false;

	m_pThumbnails->SetBkColor(COLOR_ARGB(255,255,255,255));

	if(FAILED(CProjectFactory::New(&m_SelectionI, reinterpret_cast<CDrawableContext**>(&m_pThumbnails)))) {
		MessageBox("Couldn't load kernel, check kernel version.", "Quest Designer");
		return false;
	}
	m_SelectionI->SetLayer(0);

	OnRefresh();

	m_SelectionI->SetSnapSize(m_nSnapSize, m_bShowGrid);
	m_SelectionI->SetHighlightMode();

	UpdateView();
	return true;
}
// must return true if the file was truly closed, false otherwise.
bool CThumbnailsBox::DoFileClose()
{
	//close here
	return true;
}
bool CThumbnailsBox::DoFileSave(LPCTSTR lpszFilePath)
{
	if(m_sFilePath.IsEmpty()) return DoFileSaveAs();
	return false;
}
bool CThumbnailsBox::DoFileSaveAs()
{
	return false;
}
bool CThumbnailsBox::DoFileReload()
{
	return false;
}

LRESULT CThumbnailsBox::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return 0;
}

BOOL CThumbnailsBox::CanUndo()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld()) return FALSE;
	return FALSE;
}
BOOL CThumbnailsBox::CanRedo()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld()) return FALSE;
	return FALSE;
}
BOOL CThumbnailsBox::CanCut()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld()) return FALSE;
	return (m_SelectionI->Count() != 0);
}
BOOL CThumbnailsBox::CanCopy()
{
	if(!m_SelectionI) return FALSE;
	return (m_SelectionI->Count() != 0);
}
BOOL CThumbnailsBox::CanPaste()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld()) return FALSE;
	return TRUE;
}

BOOL CThumbnailsBox::IsSelection()
{
	if(!m_SelectionI) return FALSE;
	if(isHeld()) return FALSE;
	return (m_SelectionI->Count() != 0);
}
BOOL CThumbnailsBox::IsReadOnly()
{
	if(!m_SelectionI) return FALSE;
	return FALSE;
}

inline bool CThumbnailsBox::Flip() 
{
	return true;
}
inline bool CThumbnailsBox::Mirror() 
{
	return true;
}
inline bool CThumbnailsBox::CWRotate()
{
	return true;
}
inline bool CThumbnailsBox::CCWRotate()
{
	return true;
}
bool CThumbnailsBox::InsertPlayer()
{
	return true;
}
bool CThumbnailsBox::ToggleMask()
{
	m_bShowMasks = !m_bShowMasks;
	Invalidate();
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}
bool CThumbnailsBox::ToggleBounds()
{
	m_bShowBoundaries = !m_bShowBoundaries;
	Invalidate();
	OnIdle(); // Force idle processing to update the toolbar.
	return true;
}

// Called to do idle processing
BOOL CThumbnailsBox::OnIdle()
{
	if(IsWindow()&& m_SelectionI && m_sSelected == "") {
		m_SelectionI->CleanSelection();
		Invalidate();
	}
	return FALSE;
}
bool CThumbnailsBox::hasChanged()
{
	if(m_SelectionI == NULL) return false;
	return m_SelectionI->IsModified();
}

void CThumbnailsBox::ViewToWorld(CPoint *_pPoint) 
{ 
	if(m_pGraphicsI) m_pGraphicsI->ViewToWorld(_pPoint); 
}

void CThumbnailsBox::HoldOperation()
{
	return m_SelectionI->HoldOperation();
}

void CThumbnailsBox::CancelOperation(bool bPropagate)
{
	return m_SelectionI->Cancel();
}

bool CThumbnailsBox::isResizing()
{
	return m_SelectionI->isResizing();
}

bool CThumbnailsBox::isMoving()
{
	return m_SelectionI->isMoving();
}

bool CThumbnailsBox::isFloating()
{
	return m_SelectionI->isFloating();
}

bool CThumbnailsBox::isSelecting()
{
	return m_SelectionI->isSelecting();
}

void CThumbnailsBox::StartSelBox(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->StartSelBox(_Point);
}

void CThumbnailsBox::SizeSelBox(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->SizeSelBox(_Point);
}

IPropertyEnabled* CThumbnailsBox::EndSelBoxRemove(const CPoint &_Point, LPARAM lParam)
{
	m_SelectionI->EndSelBoxRemove(_Point);
	if(m_SelectionI->Count() == 0) m_sSelected = "";

	return NULL;
}

IPropertyEnabled* CThumbnailsBox::EndSelBoxAdd(const CPoint &_Point, LPARAM lParam)
{
	IPropertyEnabled *ret = m_SelectionI->EndSelBoxAdd(_Point, (int)lParam);
	if(m_SelectionI->Count() > 0) {
		char szName[MAX_PATH];
		m_sSelected = (LPCSTR)m_SelectionI->GetSelectionName(szName, sizeof(szName));
	} else m_sSelected = "";
	return ret;
}

void CThumbnailsBox::CancelSelBox()
{
	m_SelectionI->CancelSelBox();
}

IPropertyEnabled* CThumbnailsBox::SelectPoint(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->CleanSelection();
	m_SelectionI->StartSelBox(_Point);
	IPropertyEnabled *ret = m_SelectionI->EndSelBoxAdd(_Point, 0);
	if(m_SelectionI->Count() > 0) {
		char szName[MAX_PATH];
		m_sSelected = (LPCSTR)m_SelectionI->GetSelectionName(szName, sizeof(szName));
	} else m_sSelected = "";

	*_pCursor = eIDC_SIZEALL;
	return NULL;
}

void CThumbnailsBox::GetSelectionBounds(CRect *_pRect)
{
	m_SelectionI->GetSelBounds(_pRect);
}
void CThumbnailsBox::StartMoving(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->StartMoving(_Point);
}

void CThumbnailsBox::MoveTo(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->MoveTo(_Point);
}

void CThumbnailsBox::EndMoving(const CPoint &_Point, LPARAM lParam)
{
	m_SelectionI->Cancel();
}

void CThumbnailsBox::StartResizing(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->StartResizing(_Point);
}

void CThumbnailsBox::ResizeTo(const CPoint &_Point, CURSOR *_pCursor)
{
	m_SelectionI->ResizeTo(_Point);
}

void CThumbnailsBox::EndResizing(const CPoint &_Point, LPARAM lParam)
{
	m_SelectionI->Cancel();
}

bool CThumbnailsBox::SelectedAt(const CPoint &_Point)
{
	return m_SelectionI->SelectedAt(_Point);
}

int CThumbnailsBox::SelectedCount()
{
	return m_SelectionI->Count();
}

void CThumbnailsBox::PasteSelection(LPVOID _pBuffer, const CPoint &_Point)
{
}

HGLOBAL CThumbnailsBox::CopySelection(BITMAP **ppBitmap, bool bDeleteBitmap)
{
	return m_SelectionI->Copy(ppBitmap, bDeleteBitmap);
}

BITMAP* CThumbnailsBox::CaptureSelection(float _fZoom)
{
	BITMAP *pBitmap = m_SelectionI->CaptureSelection(m_pGraphicsI, _fZoom);
	m_SelectionI->SetThumbnail(pBitmap);
	return pBitmap;
}

// Called after a zooming:
void CThumbnailsBox::OnZoom()
{
}

void CThumbnailsBox::CleanSelection()
{
	m_SelectionI->CleanSelection();
}

int CThumbnailsBox::DeleteSelection()
{
	m_SelectionI->Cancel();
	return 0;
}
void CThumbnailsBox::HoldSelection(bool bHold)
{
	m_SelectionI->HoldSelection(bHold);
}
bool CThumbnailsBox::isHeld()
{
	return m_SelectionI->isHeld();
}
bool CThumbnailsBox::GetMouseStateAt(const CPoint &_Point, CURSOR *_pCursor)
{
	return m_SelectionI->GetMouseStateAt(m_pGraphicsI, _Point, _pCursor);
}

void CThumbnailsBox::CalculateLimits()
{
	int nVisibleX = (int)((float)m_szMap.cx * m_Zoom + 0.5f);
	int nVisibleY = (int)((float)m_szMap.cy * m_Zoom + 0.5f);

	m_rcScrollLimits.SetRect(
		0,
		0,
		nVisibleX,
		nVisibleY
	);
}

void CThumbnailsBox::UpdateSnapSize(int _SnapSize)
{
	m_SelectionI->SetSnapSize(_SnapSize, m_bShowGrid);
}

void CThumbnailsBox::DoFrame()
{
	ASSERT(m_SelectionI);

	// Update timings and stuff for the animations
	CProjectFactory::Interface()->UpdateFPS();
}
void CThumbnailsBox::Render(WPARAM wParam)
{
	if(!m_SelectionI) {
		m_pGraphicsI->SetClearColor(COLOR_ARGB(255,255,255,255));
		m_pGraphicsI->BeginPaint();
		m_pGraphicsI->EndPaint();
		return;
	}

	CRect rcView;
	m_pGraphicsI->GetVisibleRect(&rcView);

	// resolution on which the parallax will be used:
	rcView.right = rcView.left + 640;
	rcView.bottom = rcView.top + 480;

	// show entities, and if set, also sprite boundaries and masks
	WORD wFlags = SPRITE_ENTITIES | (m_bShowBoundaries?SPRITE_BOUNDS:0) | (m_bShowMasks?SPRITE_MASKS:0);
	m_SelectionI->Paint(m_pGraphicsI, wFlags);
}

void CThumbnailsBox::UpdateView()
{
	if(!m_pGraphicsI) return;
	// We need to recalculate the window's size and position:
	CRect rcClient, rcClip;
	GetClientRect(&rcClient);
	rcClient.OffsetRect(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	rcClip.SetRect(0, 0, m_szMap.cx, m_szMap.cy);
	m_pGraphicsI->SetWindowView(m_hWnd, m_Zoom, &rcClient, &rcClip);
}

void CThumbnailsBox::OnChangeSel(int type, IPropertyEnabled *pPropObj)
{
}
LRESULT CThumbnailsBox::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect Rect;
	GetClientRect(&Rect);
	CSize Size = Rect.Size();
	Size.cx = (int)((float)Size.cx * 0.8f);
	Size.cy = (int)((float)Size.cy * 0.8f);
	SetScrollPage(Size);

	switch(wParam) {
		case VK_F5:		OnRefresh();		UpdateView(); Invalidate(); break;
		case VK_UP:		ScrollLineUp();		UpdateView(); Invalidate(); break;
		case VK_DOWN:	ScrollLineDown();	UpdateView(); Invalidate(); break;
		case VK_LEFT:	ScrollLineLeft();	UpdateView(); Invalidate(); break;
		case VK_RIGHT:	ScrollLineRight();	UpdateView(); Invalidate(); break;
		case VK_PRIOR:	ScrollPageUp();		UpdateView(); Invalidate(); break;
		case VK_NEXT:	ScrollPageDown();	UpdateView(); Invalidate(); break;
		case VK_ESCAPE:	m_sSelected = "";	CancelOperation(); UpdateView(); Invalidate(); break;
		default:		return CGEditorView::OnKeyDown(uMsg, wParam, lParam, bHandled);
	}
	return 0;
}
LRESULT CThumbnailsBox::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CRect Rect;
	GetClientRect(&Rect);
	CSize Size = Rect.Size();
	Size.cx = (int)((float)Size.cx * 0.8f);
	Size.cy = (int)((float)Size.cy * 0.8f);
	SetScrollPage(Size);

	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	if(zDelta > 0) {
		if((wParam & MK_CONTROL) == MK_CONTROL) ScrollPageLeft();
		else ScrollPageUp();
	} else if(zDelta < 0) {
		if((wParam & MK_CONTROL) == MK_CONTROL) ScrollPageRight();
		else ScrollPageDown();
	}
	UpdateView();
	Invalidate();
	return 0;
}
LRESULT CThumbnailsBox::OnLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CDrawableContext *pDrawableContext = m_SelectionI->GetLastSelected();
	if(pDrawableContext) {
		CSprite *pSprite = (CSprite *)pDrawableContext->GetDrawableObj();
		CSpriteSheet *pSpriteSheet = pSprite->GetSpriteSheet();

		m_pMainFrame->SptShtFileOpen(pSpriteSheet, (LPCSTR)pSprite->GetName());
	}
	return 0;
}
LRESULT CThumbnailsBox::OnMouseStuff(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if(!m_SelectionI) return 0;

	if(::GetFocus() != m_hWnd && ::GetFocus()) ::SetFocus(m_hWnd);

	bHandled = FALSE;
	return 0;
}