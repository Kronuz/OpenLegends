/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003 Kronuz
   Copyright (C) 2001/2003 Open Zelda's Project
 
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
/*! \file		FoldersTreeView.h 
	\brief		Interface of the CFoldersTree class.
	\date		April 15, 2003
*/

#pragma once

class CTreeInfo {
public:
	LPTSTR m_szPath;
	DWORD_PTR m_dwData;
	CTreeInfo(LPCSTR szPath, DWORD_PTR dwData) {
		m_szPath = new char[strlen(szPath)+1];
		strcpy(m_szPath, szPath);
		m_dwData = dwData;
	}
	~CTreeInfo() {
		delete []m_szPath;
	}
};

class CFoldersTreeView :
	public CWindowImpl<CFoldersTreeView, CTreeViewCtrl>
{
	typedef CFoldersTreeView thisClass;
	typedef CWindowImpl<CFoldersTreeView, CTreeViewCtrl> baseClass;
protected:
public:
	DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrl::GetWndClassName())

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CREATE, OnDestroy)

		//MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)

	END_MSG_MAP()

	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

		CImageList images;
		images.Create(IDB_FOLDERS, 16, 0, RGB(255,255,255));
		bHandled = TRUE;
		return 0;
	}
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT DelTree(WPARAM wParam, LPARAM lParam)
	{
		HTREEITEM hParentItem, hItem;
		CTreeInfo *pTI = (CTreeInfo *)lParam;
		ATLASSERT(pTI);

		hItem = FindPath(NULL, pTI->m_szPath);
		while(hItem) {
			hParentItem = GetParentItem(hItem);
			DeleteItem(hItem);
			if(ItemHasChildren(hParentItem)) break;
			hItem = hParentItem;
		}

		delete pTI;

		return TRUE;
	}

	LRESULT AddTree(WPARAM wParam, LPARAM lParam)
	{
		HTREEITEM hItem;
		CTreeInfo *pTI = (CTreeInfo *)lParam;
		ATLASSERT(pTI);

		char *szAux;
		char *szTmp = pTI->m_szPath;
		szAux = strrchr(szTmp, '\\');
		if(szAux) {
			*szAux++='\0';
			hItem = FindPath(NULL, szTmp);
		} else {
			szAux = szTmp;
			hItem = GetRootItem();
		}

		if(FindLeaf(hItem, szAux) == NULL) {
			hItem = InsertItem(szAux, (wParam>>8), (wParam&0xff), hItem, TVI_LAST);
			SetItemData(hItem, pTI->m_dwData);
		}

		hItem = GetRootItem();
		ATLASSERT(hItem);
		SetItemImage(hItem, (ICO_PROJECT>>8), (ICO_PROJECT&0xff));

		delete pTI;

		return TRUE;
	}
	HTREEITEM FindPath(HTREEITEM hParent, LPSTR szPath)
	{
		HTREEITEM hItem = NULL;
		char *token = strtok( szPath, "\\" );
		while( token != NULL ) {
			if((hItem = FindLeaf(hParent, token))==NULL) {
				hItem = InsertItem(token, (ICO_FOLDER>>8), (ICO_FOLDER&0xff), hParent, TVI_LAST);
			}
			hParent = hItem;
			token = strtok( NULL, "\\" );
		}
		return hItem;
	}
	HTREEITEM FindLeaf(HTREEITEM hParent, LPSTR szName)
	{
		char szBuff[_MAX_FNAME];
		HTREEITEM hLeaf = GetChildItem(hParent);
		do {
			GetItemText(hLeaf, szBuff, sizeof(szBuff));
			if(!strcmp(szBuff, szName)) break;
		} while((hLeaf=GetNextSiblingItem(hLeaf)));

		return hLeaf;
	}

};
