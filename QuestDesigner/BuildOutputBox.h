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
/*! \file		BuildOutputBox.h 
	\brief		Interface of the CBuildOutputBox class.
	\date		April 15, 2003
*/

#pragma once

#include "OutputBox.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMainFrame;

class CBuildOutputBox :	public COutputBox
{
	int m_Errors;
	int m_Warnings;
	CString m_sProject;
public:
	// Pointer to main frame
	CMainFrame *m_pMainFrame;

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(COutputBox)
    
		MESSAGE_HANDLER(WM_CREATE, OnCreate)  

		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
	
		CHAIN_MSG_MAP(COutputBox)
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnWriteMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void WriteMsg(LPCTSTR lpszNewText, CHARFORMAT2 *pcFmt=NULL);
	LRESULT BeginBuildMsg(WPARAM wParam, LPARAM lParam);
	LRESULT EndBuildMsg(WPARAM wParam, LPARAM lParam);
};
