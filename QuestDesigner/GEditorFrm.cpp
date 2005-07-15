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
/*! \file		GEditorFrm.cpp 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the CGEditorFrame virtual base class.
	\date		April 27, 2003:
						* Creation date.
				September 02, 2003
*/

#include "stdafx.h"
#include "GEditorFrm.h"
#include "MainFrm.h"

CGEditorFrame::CGEditorFrame(CMainFrame *pMainFrame, _child_type ChildType) :
	CChildFrame(pMainFrame, ChildType),
	m_pGEditorView(NULL)
{ 
}	
LRESULT CGEditorFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLASSERT(m_pGEditorView);  // this should have been set by the user class
	m_pChildView = m_pGEditorView;

	bHandled = FALSE;

	return TRUE;
}
LRESULT CGEditorFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(m_pGEditorView->DoFileClose()) {
		DefWindowProc();
		return 0;
	}
	return 1;
}
