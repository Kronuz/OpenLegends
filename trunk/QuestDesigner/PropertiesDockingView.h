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

#pragma once

#include "../IGame.h"

#include <PropertyList.h>
#include "ToolBarBox.h"

typedef struct tagPROPDESC {
	char Name[30];
	UINT uID;
} PROPDESC, *LPPROPDESC;

class CPropertyView : 
	public CDialogImpl<CPropertyView>,
	public CDialogResize<CPropertyView>
{
	IPropertyEnabled *m_pProperty;
	SPropertyList m_PropertyList;

	int m_nMinWidth;
	int m_nMinHeight;

	void AddProperties(SPropertyList *pPL);
	void UpdateProperties();

public:
	enum { IDD = IDD_PROPERTYVIEW };

	CToolBarBox m_ctrlToolbar;
	CPropertyListCtrl m_ctrlList;

	BEGIN_MSG_MAP(CPropertyView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_HANDLER(IDC_COMBO, CBN_SELCHANGE, OnSelChange)
		NOTIFY_HANDLER(IDC_LIST, PIN_ITEMCHANGED, OnItemChanged)
		NOTIFY_HANDLER(IDC_LIST, PIN_SELCHANGED, OnSelChanged)

		CHAIN_MSG_MAP( CDialogResize<CPropertyView> )
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CPropertyView)
		DLGRESIZE_CONTROL(IDC_COMBO, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_FRAME, DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_TITLE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_DESCRIPTION, DLSZ_SIZE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	// List:
	LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT OnSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/);

	// Combo box:
	LRESULT OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnClear(WPARAM wParam, LPARAM lParam);
	LRESULT OnAddInfo(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetProperties(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdate(WPARAM wParam, LPARAM lParam);

};

