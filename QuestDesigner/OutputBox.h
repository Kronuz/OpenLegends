/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		OutputBox.h 
	\brief		Interface of the COutputBox class.
	\date		April 15, 2003
*/

#pragma once

class COutputBox :	public CWindowImpl<COutputBox, CRichEditCtrl>, 
					public CRichEditCommands<COutputBox>
{
protected:
	typedef COutputBox thisClass;
	typedef CWindowImpl<COutputBox, CRichEditCtrl> baseClass;
public:

	DECLARE_WND_SUPERCLASS(NULL, CRichEditCtrl::GetWndClassName())

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(COutputBox)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)  
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		CHAIN_MSG_MAP_ALT(CRichEditCommands<COutputBox>, 1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnRequestScrollToEnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void WriteMsg(LPCTSTR lpszNewText, CHARFORMAT2 *pcFmt=NULL);
	void Empty();
};

