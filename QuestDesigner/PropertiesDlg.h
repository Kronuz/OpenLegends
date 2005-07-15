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
/*! \file		PropertiesDlg.h
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the CPropertiesDlg class.
	\date		June 10, 2003
*/

#pragma once
#include "resource.h"

#include "DlgContainer.h"
#include "PropertiesSheet.h"

#define PROPERTIES_SHEET_NOTIMPLEMENTED  0

class CPropertiesDlg : 
	public CDialogImpl<CPropertiesDlg>,
	public CDialogResize<CPropertiesDlg>
{
private:
	CTreeViewCtrl	m_PropertiesTree;
	CSimpleArray<CPropertiesSheet*> m_PropertiesSheets;
	CPropertiesSheet *m_pActiveSheet;

	CDlgContainerCtrl m_CtrlContainer;
	CNotImplementedSheet m_NotImplementedSheet;
	CGeneralSheet m_GeneralSheet;

	CStatic m_OptionCtrl;
	CStatic m_InfoCtrl;

	int m_DefaultSheet;

public:
	CPropertiesDlg(int SheetNum) {
		m_DefaultSheet = SheetNum;
		m_pActiveSheet = NULL;
	}
	void ShowSheet(int SheetNum);

	enum { IDD = IDD_PROPERTIES };
	BEGIN_MSG_MAP(CPropertiesDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOKCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

		NOTIFY_HANDLER(IDC_PROPERTIES_TREE, TVN_SELCHANGING, OnTvnSelchangingPropertiestree)
		NOTIFY_HANDLER(IDC_PROPERTIES_TREE, TVN_SELCHANGED, OnTreeSelectChange)    

		CHAIN_MSG_MAP(CDialogResize<CPropertiesDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CPropertiesDlg)
		DLGRESIZE_CONTROL(IDC_PROPERTIES_TREE,			DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_INFO,				DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_PROPERTIES_PLACEHOLDER,	DLSZ_SIZE_X | DLSZ_SIZE_Y)

		DLGRESIZE_CONTROL(IDOK,							DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL,						DLSZ_MOVE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()


// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOKCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnTreeSelectChange(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnTvnSelchangingPropertiestree(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};
