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
/*! \file		ChildView.h 
	\brief		Interface of the CChildView class.
	\date		April 26, 2003
*/

#pragma once

#include "Console.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CChildFrame;

class CChildView :
	public CConsole
{
protected:
	// Pointer to parent frame
	CChildFrame *m_pParentFrame;
	// The view's file name
	CString m_sFilePath;
	// The view's title
	CString m_sTitle;

	// Construction/Destruction
	CChildView(CChildFrame *pParentFrame);
public:
	CChildFrame *GetParentFrame() { ATLASSERT(m_pParentFrame); return m_pParentFrame; }
	CMainFrame* GetMainFrame() { ATLASSERT(m_pParentFrame); return m_pParentFrame->GetMainFrame(); }

	CString& GetFilePath() { return m_sFilePath; }
	CString& GetTitle() { return m_sTitle; }
};