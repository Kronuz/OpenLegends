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
/*! \file		FoldersTreeBox.h 
	\brief		Interface of the CFoldersTree class.
	\date		April 15, 2003
*/

#pragma once
#include "Controls.h"

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

class CFoldersTreeBox :
	public CTreeBox
{
	typedef CFoldersTreeBox thisClass;
	typedef CTreeBox baseClass;

protected:
	CImageList m_ImageList;
	int m_nIcoIndex[II_END];

public:
	DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrl::GetWndClassName())

	BOOL PreTranslateMessage(MSG* /*pMsg*/);

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CREATE, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		 
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)
		CHAIN_MSG_MAP(baseClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT DelTree(WPARAM wParam, LPARAM lParam);
	LRESULT AddTree(WPARAM wParam, LPARAM lParam);

	LRESULT OnSelChanged(int wParam, LPNMHDR lParam, BOOL& bHandled);

protected:
	HTREEITEM FindPath(HTREEITEM hParent, LPSTR szPath);
	HTREEITEM FindLeaf(HTREEITEM hParent, LPSTR szName);
};
