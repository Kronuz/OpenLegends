/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		DebugBox.h 
	\brief		Interface of the CDebugBox class.
	\date		November 29, 2003

	This class is in harge of maintaining the multiple windows that are needed
	during script debugging time, such as the current local and global variables,
	active breakpoints, etc.
*/
#pragma once

#include "PropertyGrid.h"

class CDebugBox : public CPropertyGridImpl<CDebugBox>
{
protected:
	typedef CDebugBox thisClass;
	typedef CPropertyGridImpl<CDebugBox> baseClass;
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_DebugBox"), GetWndClassName())

	BEGIN_MSG_MAP(CDebugBox)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

};
