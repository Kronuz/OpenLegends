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
// FoldersTreeBox.cpp : implementation of the CFoldersTreeBox class
//

#include "stdafx.h"

#include "FoldersTreeBox.h"

BOOL CFoldersTreeBox::PreTranslateMessage(MSG* /*pMsg*/)
{
	return FALSE;
}

LRESULT CFoldersTreeBox::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CFoldersTreeBox::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT nResult = DefWindowProc(uMsg, wParam, lParam);

	int cx = ::GetSystemMetrics(SM_CXSMICON), cy = ::GetSystemMetrics(SM_CYSMICON);

	m_ImageList.Create(cx, cy, ILC_COLOR32 | ILC_MASK, 4, 4);

	HICON hIcon = NULL;
	// NOTE: Don't Load using the LR_LOADTRANSPARENT bit, because the icon
	//  already properly deals with transparency (setting it causes problems).
	//  We will load this as LR_SHARED so that we don't have to do a DeleteIcon.

	// Load all needed icons:
	ATLASSERT(IDI_END - IDI_BEGIN < 20);
	for(int nIcoIdx=0; nIcoIdx<II_END; nIcoIdx++) {
		hIcon = (HICON)::LoadImage(
					_Module.GetResourceInstance(),
					MAKEINTRESOURCE(nIcoIdx+IDI_BEGIN),
					IMAGE_ICON, cx, cy, LR_SHARED);
		m_nIcoIndex[nIcoIdx] = m_ImageList.AddIcon(hIcon);
	}

	// Hook up the image list to the tree view
	SetImageList(m_ImageList, TVSIL_NORMAL);

	InitDragDrop();

	return nResult;
}
LRESULT CFoldersTreeBox::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bDestroy = m_ImageList.Destroy();
	bHandled = FALSE;
	return 0;
}
LRESULT CFoldersTreeBox::OnSelChanged(int wParam, LPNMHDR lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CFoldersTreeBox::DelTree(WPARAM wParam, LPARAM lParam)
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

LRESULT CFoldersTreeBox::AddTree(WPARAM wParam, LPARAM lParam)
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
HTREEITEM CFoldersTreeBox::FindPath(HTREEITEM hParent, LPSTR szPath)
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
HTREEITEM CFoldersTreeBox::FindLeaf(HTREEITEM hParent, LPSTR szName)
{
	char szBuff[_MAX_FNAME];
	HTREEITEM hLeaf = GetChildItem(hParent);
	do {
		GetItemText(hLeaf, szBuff, sizeof(szBuff));
		if(!strcmp(szBuff, szName)) break;
	} while((hLeaf=GetNextSiblingItem(hLeaf)));

	return hLeaf;
}
