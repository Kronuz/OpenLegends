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
/*! \file		FoldersTree.h 
	\brief		Interface of the CFoldersTree class.
	\date		April 15, 2003
*/

#pragma once

#include <sstream>
#include <ExtDockingWindow.h>

class CFoldersTree :
	public dockwins::CTitleDockingWindowImpl< CFoldersTree,CWindow,dockwins::COutlookLikeTitleDockingWindowTraits>
{
	typedef CFoldersTree	thisClass;
	typedef dockwins::CTitleDockingWindowImpl< CFoldersTree,CWindow,dockwins::COutlookLikeTitleDockingWindowTraits> baseClass;
protected:
	void FillTree(CTreeViewCtrl& tree)
	{/*
		for( int i =0; i < 10; i++)
		{
			std::basic_stringstream<TCHAR> text;
			text<<_T("item ")<<i;
			HTREEITEM hItem=tree.InsertItem(text.str().c_str(),0,1,TVI_ROOT,TVI_LAST);
			for( int j = 0; j < 5; j++)
			{
				std::basic_stringstream<TCHAR> text;
				text<<_T("sub item ")<<j;
				tree.InsertItem(text.str().c_str(),0,1,hItem,TVI_LAST);
			}
		}
*/
	}
public:
	LRESULT AddTree(WPARAM wParam, LPARAM lParam)
	{
		HTREEITEM hItem = m_tree.InsertItem((LPCSTR)lParam, 0, 1, TVI_ROOT, TVI_LAST);
		return TRUE;
	}




	void OnDocked(HDOCKBAR hBar,bool bHorizontal)
	{
		DWORD dwStyle = GetWindowLong(GWL_STYLE)&(~WS_SIZEBOX);
		SetWindowLong( GWL_STYLE, dwStyle);

		baseClass::OnDocked(hBar,bHorizontal);
	}
	void OnUndocked(HDOCKBAR hBar)
	{
		DWORD dwStyle = GetWindowLong(GWL_STYLE) | WS_SIZEBOX;
		SetWindowLong( GWL_STYLE , dwStyle);

		baseClass::OnUndocked(hBar);
	}

    DECLARE_WND_CLASS(_T("CFoldersTree"))
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_images.Create(16, 16, TRUE, 3, 0);

		assert(m_images.m_hImageList);
		CBitmap bmp;

		bmp.LoadBitmap(IDB_FOLDERS);

		m_images.Add( bmp, RGB(255,255,255));

		m_tree.Create(m_hWnd,NULL,NULL,
						TVS_SHOWSELALWAYS | TVS_HASBUTTONS |
						TVS_LINESATROOT | TVS_HASLINES |
						TVS_EDITLABELS|TVS_SHOWSELALWAYS|
						TVS_DISABLEDRAGDROP|
						WS_CHILD | WS_VISIBLE);
		m_tree.SetImageList(m_images, TVSIL_NORMAL);
		FillTree(m_tree);
		return 0;
	}
	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED )
		{
			RECT rc;
			GetClientRect(&rc);
			::SetWindowPos(m_tree.m_hWnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top ,SWP_NOZORDER | SWP_NOACTIVATE);
		}
		bHandled = FALSE;
		return 1;
	}
protected:
	CTreeViewCtrl	m_tree;
	CImageList		m_images;
};
