/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.
   Author: Leon Finker  1/2001
**************************************************************************/
//controls.h
#ifndef __CONTROLS_H__
#define __CONTROLS_H__

#include <WtlFileTreeCtrl.h>

#include "DragDropImpl.h"

#include "EditDropTarget.h"
#include "ListviewDropTarget.h"
#include "StaticDropTarget.h"
#include "TreeDropTarget.h"

#include "../IGame.h"
/*
class CEditBox : public CWindowImpl<CEditBox, WTL::CEdit>, public WTL::CEditCommands<CEditBox>
{
	CIDropTarget* m_pDropTarget;
public:
	CEditBox():m_pDropTarget(NULL){}

	BEGIN_MSG_MAP(CEditBox)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP_ALT(CEditCommands<CEditBox>, 1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RevokeDragDrop(m_hWnd); //calls release
		m_pDropTarget=NULL;
		return 0;
	}

	bool InitDragDrop()
	{
			m_pDropTarget = new CEditDropTarget(m_hWnd);
			if(m_pDropTarget == NULL)
				return false;
			m_pDropTarget->AddRef();

			if(FAILED(RegisterDragDrop(m_hWnd,m_pDropTarget))) //calls addref
			{
				m_pDropTarget = NULL;
				return false;
			}
			else
				m_pDropTarget->Release(); //i decided to AddRef explicitly after new

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
		return true;
	}

};
/*
class CTreeBox : 
	public CWindowImpl<CTreeBox, CWtlFileTreeCtrl>
{
	CIDropTarget *m_pDropTarget;
public:
	
	BEGIN_MSG_MAP(CTreeBox)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_BEGINDRAG, OnBegindrag)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	CTreeBox() : 
		CWtlFileTreeCtrl(), 
		m_pDropTarget(NULL) 
	{
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RevokeDragDrop(m_hWnd); // calls Release()
		m_pDropTarget = NULL;
		return 0;
	}

	LRESULT OnBegindrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		NMTREEVIEW* pnmtv = (NMTREEVIEW*)pnmh;
		USES_CONVERSION;
		CComBSTR bstr;
		
		// Get and verify item data:
		CTreeInfo *pTreeInfo = (CTreeInfo *)GetItemData(pnmtv->itemNew.hItem);
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

		GetItemText(pnmtv->itemNew.hItem, bstr.m_str);
		CString sStr = OLE2T(bstr.m_str); // Item's name

		pBitmap = pTreeInfo->GetThumbnail();// try to get the thumbnail

		// check if the object contains a valid OZ file as the data:
		_OpenZeldaFile *pOZFile = (_OpenZeldaFile*)(pTreeInfo->GetData());
		if(pOZFile) if(LOWORD(pOZFile->dwSignature) != OZF_SIGNATURE) pOZFile = NULL;
		if(pOZFile) {
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
			if(pTreeInfo->GetFile() && pTreeInfo->m_cSubType=='?') sStr = (LPCSTR)pTreeInfo->GetFile()->GetFilePath();

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
		HRESULT hr = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);

		pDropSource->Release();
		pDataObject->Release();

		return 0;
	}

	bool InitDragDrop()
	{
		m_pDropTarget = new CTreeDropTarget(m_hWnd);
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
};
*/
/*
class CStaticBox : public CWindowImpl<CStatic,WTL::CStatic>
{
	CIDropTarget* m_pDropTarget;
	bool m_bDragMode;
public:
	BEGIN_MSG_MAP(CStaticBox)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	CStaticBox():m_bDragMode(false), m_pDropTarget(NULL){}
	
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RevokeDragDrop(m_hWnd); //calls release
		m_pDropTarget=NULL;
		return 0;
	}

	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CMenu menu;
		if(!menu.CreatePopupMenu())
			return 0;
		menu.AppendMenu(MF_STRING, 1, "Copy");
		int nCmd = menu.TrackPopupMenu(TPM_RETURNCMD,LOWORD(lParam),HIWORD(lParam),m_hWnd);
		if(nCmd == 1)
		{
			CIDataObject* pdobj = new CIDataObject(NULL);
			if(pdobj == NULL)
				return 0;
			pdobj->AddRef();
			BITMAP bmap;
			HBITMAP hBitmap = (HBITMAP)OleDuplicateData(GetBitmap(), CF_BITMAP, NULL);
			GetObject(hBitmap, sizeof(BITMAP), &bmap);
			RECT rc={0,0,bmap.bmWidth,bmap.bmHeight};
			HDC hdc = GetDC();
			FORMATETC fmtetc = {0};
			STGMEDIUM medium = {0};
			fmtetc.dwAspect = DVASPECT_CONTENT;
			fmtetc.lindex = -1;
			fmtetc.cfFormat = CF_ENHMETAFILE;
			fmtetc.tymed = TYMED_ENHMF;
			HDC hMetaDC = CreateEnhMetaFile(hdc, NULL, NULL, NULL);
			HDC hdcMem = CreateCompatibleDC(hdc);
			HGDIOBJ hOldBmp = ::SelectObject(hdcMem, hBitmap);
			::BitBlt(hMetaDC, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
			::SelectObject(hdcMem, hOldBmp);
			medium.hEnhMetaFile = CloseEnhMetaFile(hMetaDC);
			DeleteDC(hdcMem);
			ReleaseDC(hdc);
			medium.tymed = TYMED_ENHMF;
			pdobj->SetData(&fmtetc,&medium,TRUE);
			OleSetClipboard(pdobj);
			OleFlushClipboard(); //render the data
			pdobj->Release();
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bDragMode = true;
		return 0;
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(m_bDragMode && wParam == MK_LBUTTON)
		{
			CIDropSource* pdsrc = new CIDropSource;
			if(pdsrc == NULL)
				return 0;
			pdsrc->AddRef();

			CIDataObject* pdobj = new CIDataObject(pdsrc);
			if(pdobj == NULL)
				return 0;
			pdobj->AddRef();

			FORMATETC fmtetc = {0};
			STGMEDIUM medium = {0};
			fmtetc.dwAspect = DVASPECT_CONTENT;
			fmtetc.lindex = -1;
			//////////////////////////////////////
			fmtetc.cfFormat = CF_BITMAP;
			fmtetc.tymed = TYMED_GDI;			
			medium.tymed = TYMED_GDI;
			HBITMAP hBitmap = (HBITMAP)OleDuplicateData(GetBitmap(), fmtetc.cfFormat, NULL);
			medium.hBitmap = hBitmap;
			pdobj->SetData(&fmtetc,&medium,FALSE);
			//////////////////////////////////////
			BITMAP bmap;
			GetObject(hBitmap, sizeof(BITMAP), &bmap);
			RECT rc={0,0,bmap.bmWidth,bmap.bmHeight};
			HDC hdc = GetDC();
			fmtetc.cfFormat = CF_ENHMETAFILE;
			fmtetc.tymed = TYMED_ENHMF;
			HDC hMetaDC = CreateEnhMetaFile(hdc, NULL, NULL, NULL);
			HDC hdcMem = CreateCompatibleDC(hdc);
			HGDIOBJ hOldBmp = ::SelectObject(hdcMem, hBitmap);
			::BitBlt(hMetaDC, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
			::SelectObject(hdcMem, hOldBmp);
			medium.hEnhMetaFile = CloseEnhMetaFile(hMetaDC);
			DeleteDC(hdcMem);
			ReleaseDC(hdc);
			medium.tymed = TYMED_ENHMF;
			pdobj->SetData(&fmtetc,&medium,TRUE);
			//////////////////////////////////////
			CDragSourceHelper dragSrcHelper;
			POINT pt={LOWORD(lParam), HIWORD(lParam)};
			//rc is off because i used static box with style where bmp doesn't take whole window
			dragSrcHelper.InitializeFromBitmap(hBitmap, pt, rc, pdobj); //will own the bmp
			DWORD dwEffect;
			HRESULT hr = ::DoDragDrop(pdobj, pdsrc, DROPEFFECT_COPY|DROPEFFECT_MOVE, &dwEffect);
			pdsrc->Release();
			pdobj->Release();
			m_bDragMode = false;
		}
		return 0;
	}
	
	bool InitDragDrop()
	{
			m_pDropTarget = new CStaticDropTarget(m_hWnd);
			if(m_pDropTarget == NULL)
				return false;
			m_pDropTarget->AddRef();

			if(FAILED(RegisterDragDrop(m_hWnd,m_pDropTarget))) //calls addref
			{
				m_pDropTarget = NULL;
				return false;
			}
			else
				m_pDropTarget->Release(); //i decided to AddRef explicitly after new

			FORMATETC ftetc={0};
			ftetc.cfFormat = CF_BITMAP;
			ftetc.dwAspect = DVASPECT_CONTENT;
			ftetc.lindex = -1;
			ftetc.tymed = TYMED_GDI;
			m_pDropTarget->AddSuportedFormat(ftetc);
			ftetc.cfFormat = CF_DIB;
			ftetc.tymed = TYMED_HGLOBAL;
			m_pDropTarget->AddSuportedFormat(ftetc);
			ftetc.cfFormat = CF_HDROP;
			ftetc.tymed = TYMED_HGLOBAL;
			m_pDropTarget->AddSuportedFormat(ftetc);
			ftetc.cfFormat = CF_ENHMETAFILE;
			ftetc.tymed = TYMED_ENHMF;
			m_pDropTarget->AddSuportedFormat(ftetc);
		return true;
	}
};

class CListViewBox : public CWindowImpl<CListViewBox,WTL::CListViewCtrl>
{
	CIDropTarget* m_pDropTarget;
public:	
	BEGIN_MSG_MAP(CTreeBox)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_BEGINDRAG, OnBegindragListview)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	CListViewBox():m_pDropTarget(NULL){}

	bool InitDragDrop()
	{
			m_pDropTarget = new CListViewDropTarget(m_hWnd);
			if(m_pDropTarget == NULL)
				return false;
			m_pDropTarget->AddRef();

			if(FAILED(RegisterDragDrop(m_hWnd,m_pDropTarget))) //calls addref
			{
				m_pDropTarget = NULL;
				return false;
			}
			else
				m_pDropTarget->Release(); //i decided to AddRef explicitly after new

			FORMATETC ftetc={0};
			ftetc.cfFormat = CF_TEXT;
			ftetc.dwAspect = DVASPECT_CONTENT;
			ftetc.lindex = -1;
			ftetc.tymed = TYMED_HGLOBAL;
			m_pDropTarget->AddSuportedFormat(ftetc);
			ftetc.cfFormat=CF_HDROP;
			m_pDropTarget->AddSuportedFormat(ftetc);
		return true;
	}

	LRESULT OnBegindragListview(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		USES_CONVERSION;
		NMLISTVIEW* pnmlv = (NMLISTVIEW*)pnmh;
		CComBSTR bstr;
		GetItemText(pnmlv->iItem, 0, bstr.m_str);
		
		CIDropSource* pdsrc = new CIDropSource;
		if(pdsrc == NULL)
			return 0;
		pdsrc->AddRef();

		CIDataObject* pdobj = new CIDataObject(pdsrc);
		if(pdobj == NULL)
			return 0;
		pdobj->AddRef();

		FORMATETC fmtetc = {0};
		fmtetc.cfFormat = CF_TEXT;
		fmtetc.dwAspect = DVASPECT_CONTENT;
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;

		STGMEDIUM medium = {0};
		medium.tymed = TYMED_HGLOBAL;
		TCHAR* str = OLE2T(bstr.m_str);
		
		medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE, strlen(str)+1); //for NULL
		TCHAR* pMem = (TCHAR*)GlobalLock(medium.hGlobal);
		strcpy(pMem,str);
		GlobalUnlock(medium.hGlobal);
		pdobj->SetData(&fmtetc,&medium,TRUE);
		/////////////////////////////////////////
		HRESULT hr;
		IStream* pStream;
//		IStream* pStream;
//		//ie5.0 API
//		hr = SHCreateStreamOn("readme.txt",STGM_READ,&pStream);
//		medium.pstm = pStream;
//		fmtetc.tymed = TYMED_ISTREAM;
//		medium.tymed = TYMED_ISTREAM;
//		pdobj->SetData(&fmtetc,&medium,TRUE);
		/////////////////////////////////////////
		CreateStreamOnHGlobal(NULL, TRUE, &pStream);
		DWORD dwWritten;
		pStream->Write(str, strlen(str)+1, &dwWritten);
		LARGE_INTEGER dlibMove; dlibMove.QuadPart = 0;
		pStream->Seek(dlibMove,STREAM_SEEK_SET,NULL);
		medium.pstm = pStream;
		fmtetc.tymed = TYMED_ISTREAM;
		medium.tymed = TYMED_ISTREAM;
		pdobj->SetData(&fmtetc,&medium,TRUE);
		/////////////////////////////////////////
		
		CDragSourceHelper dragSrcHelper;
		if(1)	// get drag image from the window through DI_GETDRAGIMAGE (listview seems to already support it)
			dragSrcHelper.InitializeFromWindow(m_hWnd, pnmlv->ptAction, pdobj);
		else   // load drag image from bitmap
		{
		RECT rc;
		ListView_GetItemRect(m_hWnd, pnmlv->iItem, &rc, LVIR_SELECTBOUNDS);
		HBITMAP hBitmap = LoadBitmap(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_DRAGIMG));
		dragSrcHelper.InitializeFromBitmap(hBitmap, pnmlv->ptAction, rc, pdobj);
		}
		DWORD dwEffect;
		hr = ::DoDragDrop(pdobj, pdsrc, DROPEFFECT_COPY, &dwEffect);
		pdsrc->Release();
		pdobj->Release();
		//ReleaseStgMedium(&medium); // SetData fRelease flag is set, so DataObject owns the data don't free the data
		return 0;
	}
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RevokeDragDrop(m_hWnd); //calls release
		m_pDropTarget=NULL;
		return 0;
	}
};
*/
#endif //__CONTROLS_H__