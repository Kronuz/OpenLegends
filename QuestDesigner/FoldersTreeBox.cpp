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
// FoldersTreeBox.cpp : implementation of the CFoldersTreeBox class
//

#include "stdafx.h"

#include "FoldersTreeBox.h"

#include "MainFrm.h"

BOOL CFoldersTreeBox::PreTranslateMessage(MSG* pMsg)
{
	return FALSE;
}

LRESULT CFoldersTreeBox::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_ctrlToolbar.SubclassWindow( CFrameWindowImplBase<>::CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_FOLDERS, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE) );
	m_ctrlToolbar.LoadTrueColorToolBar(IDR_TB1_FOLDERS);

	int cx = ::GetSystemMetrics(SM_CXSMICON), cy = ::GetSystemMetrics(SM_CYSMICON);

	m_ImageList.Create(cx, cy, ILC_COLOR32 | ILC_MASK, IDI_END-IDI_BEGIN, 0);

	HICON hIcon = NULL;
	// NOTE: Don't Load using the LR_LOADTRANSPARENT bit, because the icon
	//  already properly deals with transparency (setting it causes problems).
	//  We will load this as LR_SHARED so that we don't have to do a DeleteIcon.

	// Load all needed icons:
	ATLASSERT(IDI_END-IDI_BEGIN < 20);
	for(int nIcoIdx=0; nIcoIdx<II_END; nIcoIdx++) {
		hIcon = (HICON)::LoadImage(
					_Module.GetResourceInstance(),
					MAKEINTRESOURCE(nIcoIdx+IDI_BEGIN),
					IMAGE_ICON, cx, cy, LR_SHARED);
		m_nIcoIndex[nIcoIdx] = m_ImageList.AddIcon(hIcon);
	}
	DWORD dwStyle = 
				WS_BORDER | WS_CHILD | WS_VISIBLE |
				TVS_HASBUTTONS | 
				TVS_HASLINES | TVS_LINESATROOT | TVS_EDITLABELS | 
				TVS_SHOWSELALWAYS;

	m_ctrlTree.Create(m_hWnd, rcDefault, NULL, dwStyle);
	// Hook up the image list to the tree view
	m_ctrlTree.UseImageList(m_ImageList);
	m_ctrlTree.AddFileType("Folder", ICO_FOLDER);
	m_ctrlTree.AddFileType("File", ICO_UNKNOWN);
	m_ctrlTree.AddFileType(".set", ICO_SPTSET);
	m_ctrlTree.AddFileType(".spt", ICO_SPTSHT);
	m_ctrlTree.AddFileType(".zes", ICO_SCRIPT);
	m_ctrlTree.AddFileType(".lnd", ICO_MAP);

	// Sound formats:
	m_ctrlTree.AddFileType(".wav", ICO_WAV);
	m_ctrlTree.AddFileType(".mp3", ICO_WAV);
	m_ctrlTree.AddFileType(".ogg", ICO_WAV);
	m_ctrlTree.AddFileType(".mp2", ICO_WAV);
	m_ctrlTree.AddFileType(".wma", ICO_WAV);
	m_ctrlTree.AddFileType(".asf", ICO_WAV);
	m_ctrlTree.AddFileType(".raw", ICO_WAV);

	// Music formats:
	m_ctrlTree.AddFileType(".mid", ICO_MIDI);
	m_ctrlTree.AddFileType(".rmi", ICO_MIDI);
	m_ctrlTree.AddFileType(".it", ICO_MIDI);
	m_ctrlTree.AddFileType(".s3m", ICO_MIDI);
	m_ctrlTree.AddFileType(".xm", ICO_MIDI);
	m_ctrlTree.AddFileType(".mod", ICO_MIDI);
	

	return 0;
}
LRESULT CFoldersTreeBox::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if( !m_ctrlToolbar.IsWindow() || !m_ctrlTree.IsWindow() ) return 0;
	RECT rc;
	GetClientRect(&rc);
	RECT rcTb = { rc.left, rc.top, rc.right, rc.top + 24 };
	if(rcTb.right-rcTb.left<100) rcTb.right = rcTb.left + 100;

	m_ctrlToolbar.SetWindowPos(NULL, &rcTb, SWP_NOZORDER | SWP_NOACTIVATE);
	m_ctrlToolbar.GetWindowRect(&rcTb);

	RECT rcTree = { rc.left, rcTb.bottom-rcTb.top, rc.right, rc.bottom };
	if(rcTree.right-rcTree.left<100) rcTree.right = rcTree.left + 100;
	if(rcTree.bottom-rcTree.top<100) rcTree.bottom = rcTree.top + 100;

	m_ctrlTree.SetWindowPos(NULL, &rcTree, SWP_NOZORDER | SWP_NOACTIVATE);
	return 0;
}

LRESULT CFoldersTreeBox::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bDestroy = m_ImageList.Destroy();

	RevokeDragDrop(m_hWnd); // calls Release()
	m_pDropTarget = NULL;

	bHandled = FALSE;
	return 0;
}
LRESULT CFoldersTreeBox::OnDelTree(WPARAM wParam, LPARAM lParam)
{
	HTREEITEM hParentItem, hItem;
	CTreeInfo *pTreeInfo = (CTreeInfo *)lParam;
	ATLASSERT(pTreeInfo);

	hItem = pTreeInfo->m_hItem;
	pTreeInfo->m_hWndParent = m_ctrlTree.m_hWnd;
	while(hItem) {
		hParentItem = m_ctrlTree.GetParentItem(hItem);
		m_ctrlTree.DeleteItem(hItem);
		if(m_ctrlTree.ItemHasChildren(hParentItem)) break;
		hItem = hParentItem;
	}

	delete pTreeInfo;

	return TRUE;
}

LRESULT CFoldersTreeBox::OnAddTree(WPARAM wParam, LPARAM lParam)
{
	static char szOldPath[MAX_PATH];
	static HTREEITEM hOldItem;

	HTREEITEM hItem;
	CTreeInfo *pTreeInfo = (CTreeInfo *)lParam;
	ATLASSERT(pTreeInfo);

	LPCSTR szItemPath = pTreeInfo->GetItemPath();
	LPCSTR szItemName = pTreeInfo->GetDisplayName();

	if(*szItemPath) {
		if(strcmp(szOldPath, szItemPath)) {
			strcpy(szOldPath, szItemPath);
			hOldItem = hItem = FindPath(NULL, szItemPath);
		} else hItem = hOldItem;
	} else {
		hItem = m_ctrlTree.GetRootItem();
	}

	if(FindLeaf(hItem, szItemName) == NULL) {
		hItem = m_ctrlTree.InsertItem(szItemName, (wParam>>8), (wParam&0xff), hItem, TVI_SORT);
		pTreeInfo->m_hItem = hItem; // set the item's handle.
		pTreeInfo->m_hWndParent = m_ctrlTree.m_hWnd;
		m_ctrlTree.SetItemData(hItem, (DWORD_PTR)pTreeInfo);
	}

	hItem = m_ctrlTree.GetRootItem();
	ATLASSERT(hItem);
	m_ctrlTree.Expand(hItem);
	m_ctrlTree.SetItemImage(hItem, (ICO_PROJECT>>8), (ICO_PROJECT&0xff));

	return TRUE;
}
HTREEITEM CFoldersTreeBox::FindPath(HTREEITEM hParent, LPCSTR _szPath)
{
	HTREEITEM hItem = NULL;
	LPSTR szPath = (LPSTR)_alloca(strlen(_szPath)+1);
	strcpy(szPath, _szPath);
	char *token = strtok( szPath, "\\" );
	while( token != NULL ) {
		if((hItem = FindLeaf(hParent, token))==NULL) {
			hItem = m_ctrlTree.InsertItem(token, (ICO_FOLDER>>8), (ICO_FOLDER&0xff), hParent, TVI_LAST);
		}
		hParent = hItem;
		token = strtok( NULL, "\\" );
	}
	return hItem;
}
HTREEITEM CFoldersTreeBox::FindLeaf(HTREEITEM hParent, LPCSTR _szName)
{
	char szBuff[_MAX_FNAME];
	HTREEITEM hLeaf = m_ctrlTree.GetChildItem(hParent);
	do {
		m_ctrlTree.GetItemText(hLeaf, szBuff, sizeof(szBuff));
		if(!strcmp(szBuff, _szName)) break;
	} while((hLeaf = m_ctrlTree.GetNextSiblingItem(hLeaf)));

	return hLeaf;
}
void CFoldersTreeBox::PopulateTree(LPCSTR szRootFolder)
{
	m_ctrlTree.SetRootFolder(szRootFolder);
	m_ctrlTree.PostMessage(WM_POPULATE_TREE);
	m_ctrlTree.Expand(m_ctrlTree.GetRootItem());
}

LRESULT CFoldersTreeBox::OnFileItemSelected(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLASSERT(wParam);
	CTreeInfo *pTreeInfo = (CTreeInfo *)wParam;
	if(pTreeInfo->GetFile() == NULL) return TRUE;

	CString sFileName = (LPCSTR)pTreeInfo->GetFile()->GetAbsFilePath();

	switch(pTreeInfo->m_cSubType) {
		case 'E': // Entity (Script)
			m_pMainFrame->ScriptFileOpen(sFileName, 0); 
			break;
		case 'W': // World
		case 'M': // Map
			break;
		case 'S': // Sprite (Sprite Sheet)
			m_pMainFrame->ScriptFileOpen(sFileName, 0);
			break;
		default:
			break;
	}
	return TRUE;
}

HTREEITEM CFoldersTreeBox::InsertRootItem( LPCSTR szFile, LPCSTR szPath, HTREEITEM hParent, DWORD Idx )
{
	return m_ctrlTree.InsertRootItem(szFile, szPath, hParent, Idx);
}
HTREEITEM CFoldersTreeBox::InsertFileItem( LPCSTR szFile, LPCSTR szPath, HTREEITEM hParent, DWORD Idx )
{
	return m_ctrlTree.InsertRootItem(szFile, szPath, hParent, Idx);
}

LRESULT CFoldersTreeBox::OnBegindrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pnmh;
	USES_CONVERSION;
	CComBSTR bstr;
	
	// Get and verify item data:
	CTreeInfo *pTreeInfo = (CTreeInfo *)m_ctrlTree.GetItemData(pnmtv->itemNew.hItem);
	if(!pTreeInfo) return 0;

	if(pTreeInfo->m_eType == titFolder) return 0; // only drag files.

	// Build the OLE interfaces:
	CIDropSource* pDropSource = new CIDropSource;
	CIDataObject* pDataObject = new CIDataObject(pDropSource);
	if(pDropSource == NULL || pDataObject == NULL) {
		delete pDropSource;
		delete pDataObject;
		return 0;
	}
	pDropSource->AddRef();
	pDataObject->AddRef();

	// Create object to drag:
	HGLOBAL hGlobal = NULL;
	BITMAP *pBitmap = NULL;
	HBITMAP hBitmapOle = NULL;
	CRect BitmapRect(0, 0, 0, 0);
	NULL;

	m_ctrlTree.GetItemText(pnmtv->itemNew.hItem, bstr.m_str);
	CString sStr = OLE2T(bstr.m_str); // Item's name

	pBitmap = pTreeInfo->GetThumbnail();// try to get the thumbnail

	// check if the object contains a valid OZ file as the data:
	LPCOZFILE pOZFile = (LPCOZFILE)(pTreeInfo->GetData());

	if(VerifyOZFile(&pOZFile)) {
		hGlobal = GlobalAlloc(GMEM_MOVEABLE, pOZFile->dwSize);
		if(!hGlobal) {
			pDropSource->Release();
			pDataObject->Release();
			return 0;
		}

		BYTE *pMem = (BYTE*)GlobalLock(hGlobal);
		ASSERT(pMem);

		memcpy(pMem, pOZFile, pOZFile->dwSize);
		GlobalUnlock(hGlobal);

		if(pOZFile->dwBitmapOffset && !pBitmap) {
			pBitmap = (BITMAP *)((LPBYTE)pOZFile + pOZFile->dwBitmapOffset);
			pBitmap->bmBits = (LPVOID)((LPBYTE)pBitmap + sizeof(BITMAP));
		}
	} else {
		// If the Tree item data is an unkown type, and there is a file path, use it:
		if(pTreeInfo->GetFile() && pTreeInfo->m_cSubType=='?') sStr = (LPCSTR)pTreeInfo->GetFile()->GetAbsFilePath();

		hGlobal = GlobalAlloc(GMEM_MOVEABLE, sStr.GetLength() + 1); // for NULL
		if(!hGlobal) {
			pDropSource->Release();
			pDataObject->Release();
			return 0;
		}

		LPSTR pMem = (LPSTR)GlobalLock(hGlobal);
		ASSERT(pMem);

		strcpy(pMem, sStr);
		GlobalUnlock(hGlobal);
	}

	FORMATETC fmtetc = {0};
	STGMEDIUM medium = {0};
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	//////////////////////////////////////
	if(pBitmap) { // If there is a thumbnail bitmap:
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

		BitmapRect.SetRect(0, 0, pBitmap->bmWidth, pBitmap->bmHeight);
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
		//////////////////////////////////////
		fmtetc.cfFormat = CF_BITMAP;
		fmtetc.tymed = TYMED_GDI;			
		medium.tymed = TYMED_GDI;
		hBitmapOle = (HBITMAP)OleDuplicateData(hBitmap, fmtetc.cfFormat, NULL);
		medium.hBitmap = hBitmapOle;
		pDataObject->SetData(&fmtetc, &medium, FALSE);
		//////////////////////////////////////
		fmtetc.cfFormat = CF_ENHMETAFILE;
		fmtetc.tymed = TYMED_ENHMF;
		medium.tymed = TYMED_ENHMF;
		HDC hMetaDC = CreateEnhMetaFile(hDC, NULL, NULL, NULL);
		::BitBlt(hMetaDC, 0, 0, BitmapRect.Width(), BitmapRect.Height(), hDCMem, 0, 0, SRCCOPY);
		medium.hEnhMetaFile = CloseEnhMetaFile(hMetaDC);
		pDataObject->SetData(&fmtetc, &medium, TRUE);
		//////////////////////////////////////
		::SelectObject(hDCMem, hOldBmp);
		::DeleteObject(hBitmap);
		::DeleteDC(hDCMem);
		ReleaseDC(hDC);
	}
	//////////////////////////////////////
	fmtetc.cfFormat = CF_TEXT;
	fmtetc.tymed = TYMED_HGLOBAL;
	medium.tymed = TYMED_HGLOBAL;
	medium.hGlobal = hGlobal;
	pDataObject->SetData(&fmtetc, &medium, TRUE);
	//////////////////////////////////////

	CDragSourceHelper dragSrcHelper;
	if(hBitmapOle) { // If there is a thumbnail bitmap:
		dragSrcHelper.InitializeFromBitmap(hBitmapOle, BitmapRect.CenterPoint(), BitmapRect, pDataObject, RGB(255,255,255)); //will own the bmp
	} else {
		// get drag image from the window through DI_GETDRAGIMAGE (treeview seems to already support it)
		dragSrcHelper.InitializeFromWindow(m_hWnd, pnmtv->ptDrag, pDataObject);
	}

	DWORD dwEffect;
	HRESULT hr = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_MOVE, &dwEffect);

	pDropSource->Release();
	pDataObject->Release();

	return 0;
}

bool CFoldersTreeBox::InitDragDrop()
{
	ATLASSERT(::IsWindow(m_ctrlTree.m_hWnd));
	m_pDropTarget = new CTreeDropTarget(m_ctrlTree.m_hWnd);
	if(m_pDropTarget == NULL) return false;
	m_pDropTarget->AddRef();

	if(FAILED(RegisterDragDrop(m_hWnd, m_pDropTarget))) { // calls AddRef()
		m_pDropTarget->Release();
		m_pDropTarget = NULL;
		return false;
	} else {
		m_pDropTarget->Release(); // I decided to AddRef explicitly after new
	}

	FORMATETC ftetc = {0};
	ftetc.cfFormat = CF_TEXT;
	ftetc.dwAspect = DVASPECT_CONTENT;
	ftetc.lindex = -1;
	ftetc.tymed = TYMED_HGLOBAL;
	m_pDropTarget->AddSuportedFormat(ftetc);
	ftetc.cfFormat=CF_HDROP;
	m_pDropTarget->AddSuportedFormat(ftetc);
	return true;
}
