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
// DebugBox.cpp : implementation of the CDebugBox class
//

#include "stdafx.h"

#include "DebugBox.h"

LRESULT CDebugBox::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lRes = baseClass::OnCreate(uMsg, wParam, lParam, bHandled);

	InsertColumn(0, _T(""), LVCFMT_LEFT, 36, 0);
	InsertColumn(1, _T("Name"), LVCFMT_LEFT, 100, 0);
	InsertColumn(2, _T("Value"), LVCFMT_LEFT, 300, 0);
	InsertColumn(3, _T("Type"), LVCFMT_LEFT, 80, 0);

	//m_grid.SetExtendedGridStyle(PGS_EX_SINGLECLICKEDIT);
	SetExtendedGridStyle(PGS_EX_SINGLECLICKEDIT | PGS_EX_ADDITEMATEND);

	InsertItem(0, PropCreateCheckButton(_T(""), true));
	SetSubItem(0, 1, PropCreateReadOnlyItem(_T(""), _T("raymond")));
	SetSubItem(0, 2, PropCreateSimple(_T(""), _T("157")));
	SetSubItem(0, 3, PropCreateReadOnlyItem(_T(""), _T("cell")));

	InsertItem(0, PropCreateCheckButton(_T(""), true));
	SetSubItem(0, 1, PropCreateReadOnlyItem(_T(""), _T("raymond2")));
	SetSubItem(0, 2, PropCreateSimple(_T(""), _T("158")));
	SetSubItem(0, 3, PropCreateReadOnlyItem(_T(""), _T("float")));

	return 0;
}
