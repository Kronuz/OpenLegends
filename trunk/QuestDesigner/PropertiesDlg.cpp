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
/*! \file		PropertiesDlg.cpp
	\brief		Implementation of the CPropertiesDlg class.
	\date		June 10, 2003
*/

#include "stdafx.h"

#include "PropertiesDlg.h"

LRESULT CPropertiesDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{  
	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	HTREEITEM hItem, hParent;

	CenterWindow(GetParent());
	DlgResize_Init();

	m_OptionCtrl.Attach(GetDlgItem(IDC_STATIC_OPTION));
	m_InfoCtrl.Attach(GetDlgItem(IDC_STATIC_INFO));
	m_PropertiesTree.Attach(GetDlgItem(IDC_PROPERTIES_TREE));

	// Add the tree items
	hParent=m_PropertiesTree.InsertItem(TVIF_TEXT | TVIF_PARAM, "Open Zelda Core", 0, 0, 0, 0, PROPERTIES_SHEET_NOTIMPLEMENTED, TVI_ROOT,TVI_LAST);
	hItem=m_PropertiesTree.InsertItem(TVIF_TEXT | TVIF_PARAM, "Sprites", 0, 0, 0, 0, PROPERTIES_SHEET_NOTIMPLEMENTED, hParent, TVI_LAST);
	hItem=m_PropertiesTree.InsertItem(TVIF_TEXT | TVIF_PARAM, "Entities", 0, 0, 0, 0, PROPERTIES_SHEET_NOTIMPLEMENTED, hParent, TVI_LAST);
	hItem=m_PropertiesTree.InsertItem(TVIF_TEXT | TVIF_PARAM, "Scripts", 0, 0, 0, 0, PROPERTIES_SHEET_NOTIMPLEMENTED, hParent, TVI_LAST);
	m_PropertiesTree.Expand(hParent,TVE_EXPAND);

	m_CtrlContainer.SubclassWindow(GetDlgItem(IDC_PROPERTIES_PLACEHOLDER));

	// then add the sheet classes to the list of sheets.
	m_NotImplementedSheet.Create(m_CtrlContainer);
	m_NotImplementedSheet.SetDetails("Properties under construction...", "Sorry!");
	m_PropertiesSheets.Add(&m_NotImplementedSheet);
	m_CtrlContainer.AddItem(m_NotImplementedSheet); // 0

	// and select a sheet...
	if(m_DefaultSheet == -1) { // this lets us jump to a particular sheet..
		m_PropertiesTree.SelectItem(m_PropertiesTree.GetRootItem());
	} else {
		ShowSheet(m_DefaultSheet);
	}
	return TRUE;
}

LRESULT CPropertiesDlg::OnOKCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Validate the current sheet!
	if(m_pActiveSheet->OnSheetValidate()) {
		m_pActiveSheet->OnSheetDone();
		EndDialog(1);    
	}
	return 0;
}

LRESULT CPropertiesDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// end the current sheet
	m_pActiveSheet->OnSheetDone();
	EndDialog(0);    
	return 0;
}

LRESULT CPropertiesDlg::OnTvnSelchangingPropertiestree(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// validate the current dialog...

	if(!m_pActiveSheet)
		return 0; // OK, allow change..

	if(m_pActiveSheet->OnSheetValidate()) {
		m_pActiveSheet->OnSheetDone();
		m_pActiveSheet = NULL;
		return 0; // OK, allow change..
	} else {
		return 1; // SHEET NOT VALIDATED!  Disallow change!
	}
}

LRESULT CPropertiesDlg::OnTreeSelectChange(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	// when a user clicks on an item in the server treeview, we get a message
	// the message contains a pointer to the new and old selections

	LPNMTREEVIEW selectioninfo = (LPNMTREEVIEW)pnmh;  

	ShowSheet((int) m_PropertiesTree.GetItemData(selectioninfo->itemNew.hItem));

	bHandled = FALSE;
	return 0;
}

void CPropertiesDlg::ShowSheet(int SheetNum)
{
	if(m_pActiveSheet) {
		if(m_pActiveSheet->OnSheetValidate()) {
			m_pActiveSheet->OnSheetDone();
		} else {
			// TODO: select the still active Sheet's (m_pActiveSheet) corresponding item
			// in the treeview.
			return;
		}
	}

	m_CtrlContainer.SetCurSel(SheetNum);

	m_pActiveSheet = m_PropertiesSheets[SheetNum];

	m_OptionCtrl.SetWindowText(m_pActiveSheet->GetName());
	m_InfoCtrl.SetWindowText(m_pActiveSheet->GetInfo());

	// TODO: select the corresponding item in the treeview.
	// e.g. if SheetNum is set to PROPERTIES_SHEET_IDENTITIES then select/highlight "User Identities"

	m_pActiveSheet->OnSheetDisplay();
}
