/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.
   Author: Leon Finker  1/2001
**************************************************************************/
#ifndef __TREEDROPTARGET_H__
#define __TREEDROPTARGET_H__

#include "../IGame.h"

class CTreeDropTarget : public CIDropTarget
{
	DWORD _FindIcon(LPCOZFILE pOZFile)
	{
		if(!pOZFile) return ICO_UNKNOWN;
		// Find out what icon should the pOZFile use:
		if(VerifyOZFile(&pOZFile) == FALSE) return ICO_UNKNOWN;

		DWORD dwIcon = ICO_UNKNOWN;
		switch(pOZFile->dwSignature) {
			case OZF_SPRITE_SET_SIGNATURE:
				dwIcon = ICO_SPTSET;
				break;
			case OZF_MAP_GROUP_SIGNATURE:
				dwIcon = ICO_MAP;
				break;
			case OZF_WORLD_SIGNATURE:
				dwIcon = ICO_WORLD;
				break;
			case OZF_SPRITE_SHEET_SIGNATURE:
				dwIcon = ICO_SPTSHT;
				break;
		}
		return dwIcon;
	}
	CTreeInfo* _DropFile(HTREEITEM hParent, CTreeInfo *pTreeInfo) 
	{
		ATLASSERT(pTreeInfo);
		if(!pTreeInfo) return NULL;

		DWORD dwIcon = _FindIcon((LPCOZFILE)pTreeInfo->GetData());
		// If we can't recognize the dropped file, we don't use it.
		if(dwIcon == ICO_UNKNOWN) {
			delete pTreeInfo;
			return NULL;
		}

		CString sPath;

		// Get and verify parent's directory:
		TVITEM item;
		item.hItem = hParent;
		item.mask = TVIF_PARAM;
		if(TreeView_GetItem(m_hTargetWnd, &item) == TRUE && item.lParam != 0) {
			CTreeInfo *pTreeInfo = (CTreeInfo *)item.lParam;
			sPath = CString((LPCSTR)pTreeInfo->GetFile()->GetAbsFilePath()) + "\\";
			if(sPath.Right(1) != "\\" && !sPath.IsEmpty()) sPath += "\\";
		} else {
			delete pTreeInfo;
			return NULL;
		}
		sPath += pTreeInfo->GetDisplayName();

		switch(dwIcon) {
			case ICO_SPTSET:
				sPath += ".set";
				break;
			default:
				sPath += ".oz";
				break;
		}
		if(!pTreeInfo->Save(sPath)) {
			delete pTreeInfo;
			return NULL;
		}

		int nIconIndex = (int)(dwIcon >> 8);
		int nSelIconIndex = (int)(dwIcon & 0xff);

		TVINSERTSTRUCT tvis;
		ZeroMemory( &tvis, sizeof(TVINSERTSTRUCT) );
		tvis.hParent		= hParent;
		tvis.hInsertAfter	= TVI_LAST;
		tvis.item.mask		= TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
		tvis.item.pszText	= (LPSTR)pTreeInfo->GetDisplayName();
		tvis.item.iImage	= nIconIndex;
		tvis.item.iSelectedImage = nSelIconIndex;
		tvis.item.lParam	= (LPARAM)pTreeInfo;

		HTREEITEM hItem = TreeView_InsertItem(m_hTargetWnd, &tvis);
		pTreeInfo->m_hItem = hItem;
		pTreeInfo->m_hWndParent = m_hTargetWnd;
		ATLASSERT(hItem);
		if(!hItem) {
			delete pTreeInfo;
			return NULL;
		}
		return pTreeInfo;
	}

public:
	CTreeDropTarget(HWND hTargetWnd) : CIDropTarget(hTargetWnd) {}
	
	virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
	{
		// Get and verify Drop Hilight. (find the new item's parent):
		HTREEITEM hParent = TreeView_GetDropHilight(m_hTargetWnd);
		if(hParent) {
			TVITEM item;
			item.hItem = hParent;
			item.mask = TVIF_PARAM;
			if(TreeView_GetItem(m_hTargetWnd, &item) == TRUE && item.lParam != 0) {
				if(((CTreeInfo *)item.lParam)->m_eType == titFile) {
					hParent = TreeView_GetParent(m_hTargetWnd, hParent);
				}
			}
		}
		if(!hParent) hParent = TreeView_GetRoot(m_hTargetWnd);
		if(!hParent) hParent = TVI_ROOT;

		CTreeInfo *pTreeInfo = NULL;

		if(pFmtEtc->cfFormat == CF_TEXT && medium.tymed == TYMED_HGLOBAL) {
			BYTE *pData = (BYTE*)GlobalLock(medium.hGlobal);
			if(pData != NULL) {
				pTreeInfo = NULL;

				// check if the object contains a valid OZ file as the data:
				LPCOZFILE pOZFile = (LPCOZFILE)pData;
				
				if(VerifyOZFile(&pOZFile)) {
					CString sName;
					DWORD dwSize = 0;

					dwSize = pOZFile->dwSize;
					pData = new BYTE[dwSize];
					memcpy(pData, pOZFile, dwSize);

					// Find the name of the OZ file, to be shown in the tree:
					char tmp[sizeof(pOZFile->ID)];
					strcpy(tmp, pOZFile->ID);
					strtok(tmp, "\n");								// signature
					LPSTR name = strtok(NULL, "\n");				// name
					if(name) sName = name;
					BYTE cType = '?';
					pTreeInfo = new CTreeInfo(cType, (LPCSTR)sName, pData, dwSize);

				} else {
					pTreeInfo = new CTreeInfo('?', (LPCSTR)pData, (LPCSTR)pData);
					pTreeInfo->Load(); // if the file is found, the titFile type is set.
				}
				pTreeInfo = _DropFile(hParent, pTreeInfo);
			}
			GlobalUnlock(medium.hGlobal);
		}
		if(pFmtEtc->cfFormat == CF_HDROP && medium.tymed == TYMED_HGLOBAL) {
			HDROP hDrop = (HDROP)GlobalLock(medium.hGlobal);
			if(hDrop != NULL) {
				char szFileName[MAX_PATH];

				UINT cFiles = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0); 
				for(UINT i = 0; i < cFiles; i++) {
					DragQueryFileA(hDrop, i, szFileName, sizeof(szFileName));

					pTreeInfo = new CTreeInfo('?', szFileName, szFileName);
					pTreeInfo->Load(); // if the file is found, the titFile type is set.
					pTreeInfo = _DropFile(hParent, pTreeInfo);
				}  
				//DragFinish(hDrop); // base class calls ReleaseStgMedium
			}
			GlobalUnlock(medium.hGlobal);
		}
		TreeView_SelectDropTarget(m_hTargetWnd, NULL);
		return true; //let base free the medium
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver( 
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD __RPC_FAR *pdwEffect)
	{
		TVHITTESTINFO hit;
		hit.pt = (POINT&)pt;
		ScreenToClient(m_hTargetWnd,&hit.pt);
		hit.flags = TVHT_ONITEM;
		HTREEITEM hItem = TreeView_HitTest(m_hTargetWnd,&hit);
		if(hItem != NULL)
		{
			TreeView_SelectDropTarget(m_hTargetWnd, hItem);
		}
		return CIDropTarget::DragOver(grfKeyState, pt, pdwEffect);
	}
	virtual HRESULT STDMETHODCALLTYPE DragLeave(void)
	{
		TreeView_SelectDropTarget(m_hTargetWnd, NULL);
		return CIDropTarget::DragLeave();
	}

};

#endif //__TREEDROPTARGET_H__