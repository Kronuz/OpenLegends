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
/*! \file		ChildFrm.h 
	\brief		Interface of the CChildFrame class.
	\date		April 15, 2003
*/

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMainFrame;
class CChildView;

enum _child_type { tAny=0, tScriptEditor, tHtmlView, tWorldEditor, tMapEditor };
/////////////////////////////////////////////////////////////////////////////
// This is the common childs class
typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MAXIMIZE, WS_EX_MDICHILD> CChildFrameTraits;
class CChildFrame : public 
	CTabbedMDIChildWindowImpl<CChildFrame, CMDIWindow, CChildFrameTraits>
{
	typedef CTabbedMDIChildWindowImpl<CChildFrame, CMDIWindow, CChildFrameTraits> baseClass;
protected:

	// Pointer to main frame
	CMainFrame *m_pMainFrame;

	CTabbedMDICommandBarCtrl *m_pCmdBar;

	CChildFrame(CMainFrame *pMainFrame, _child_type ChildType);
	void SetCommandBarCtrlForContextMenu(CTabbedMDICommandBarCtrl* pCmdBar);

public:
	// Type of the child window
	_child_type m_ChildType;
	// Name given to the child window
	CString m_sChildName;

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MDIMAPED)

	BEGIN_MSG_MAP(CScriptEditorFrame)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT Register(_child_type ChildType);
	LRESULT Unregister();

	// Return the main frame
	CMainFrame* GetMainFrame() { ATLASSERT(m_pMainFrame); return m_pMainFrame; }

};
