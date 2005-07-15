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
/*! \file		FoldersTreeBox.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the CFoldersTree class.
	\date		April 15, 2003
*/

#pragma once

#include "Controls.h"

#include <WtlFileTreeCtrl.h>

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMainFrame;

class CFoldersTreeBox :
	public CWindowImpl < CFoldersTreeBox > 
{
	CIDropTarget *m_pDropTarget;
protected:
	CTrueColorToolBarCtrl m_ctrlToolbar;
	CWtlFileTreeCtrl m_ctrlTree;

	CImageList m_ImageList;
	int m_nIcoIndex[II_END];

public:
	// Pointer to main frame
	CMainFrame *m_pMainFrame;

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CFoldersTreeBox)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SETREDRAW, OnSetRedraw)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ITEM_SELECTED, OnFileItemSelected )

		NOTIFY_CODE_HANDLER(TVN_BEGINDRAG, OnBegindrag)

		// Not entirely sure why this filter "should be" needed...
		//if( uMsg==WM_NOTIFY && ((LPNMHDR)lParam)->hwndFrom == m_ctrlToolbar ) 
			REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnFileItemSelected(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnSetRedraw(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		::SendMessage(m_ctrlTree, WM_SETREDRAW, wParam, 0);
		return 0;
	}

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnDelTree(WPARAM wParam, LPARAM lParam);
	LRESULT OnAddTree(WPARAM wParam, LPARAM lParam);
	void PopulateTree(LPCSTR szRootFolder);

	HTREEITEM InsertRootItem( LPCSTR szFile, LPCSTR szPath, HTREEITEM hParent, DWORD Idx=-1 );
	HTREEITEM InsertFileItem( LPCSTR szFile, LPCSTR szPath, HTREEITEM hParent, DWORD Idx=-1 );

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnBegindrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	bool InitDragDrop();

protected:
	HTREEITEM FindPath(HTREEITEM hParent, LPCSTR _szPath);
	HTREEITEM FindLeaf(HTREEITEM hParent, LPCSTR _szName);
};
