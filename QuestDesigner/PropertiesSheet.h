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

/////////////////////////////////////////////////////////////////////////////
/*! \file		PropertiesSheet.h
	\brief		Interface of the CPropertiesSheet class.
	\date		June 10, 2003
*/

#pragma once

class CPropertiesSheet
{
private:
	char *m_Name;
	char *m_Info;

public:
	void SetDetails(char *Name, char *Info) {
		m_Name = Name;
		m_Info = Info;
	}

	char *GetName(void) { return m_Name; }
	char *GetInfo(void) { return m_Info; }
	virtual void OnSheetDisplay(void) = 0;
	virtual void OnSheetDone(void) = 0;
	virtual BOOL OnSheetValidate(void) = 0;

};

class CNotImplementedSheet : 
	public CDialogImpl<CNotImplementedSheet>,
	public CDialogResize<CNotImplementedSheet>,
	public CPropertiesSheet
{
public:
	enum { IDD = IDD_PROPERTIES_NOTIMPLEMENTED };

	BEGIN_MSG_MAP(CNotImplementedSheet)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CDialogResize<CNotImplementedSheet>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CNotImplementedSheet)
		DLGRESIZE_CONTROL(IDC_STATIC_INFO, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		DlgResize_Init(false, true, 0);
		return 0;
	}

	void OnSheetDisplay(void) {}
	void OnSheetDone(void) {}
	BOOL OnSheetValidate(void) { return TRUE; }
};
