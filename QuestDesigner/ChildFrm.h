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

#define DECLARE_FRAME_CHILD_CLASS(WndClassName, uCommonResourceID) \
virtual CFrameWndClassInfo& GetChildClassInfo() \
{ \
	static CFrameWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), 0, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL, WndClassName, NULL }, \
		NULL, NULL, IDC_ARROW, TRUE, 0, _T(""), uCommonResourceID \
	}; \
	return wc; \
}

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

	virtual CFrameWndClassInfo& GetChildClassInfo() = 0;
public:
	// Type of the child window
	_child_type m_ChildType;
	// Name given to the child window
	CString m_sChildName;

	BEGIN_MSG_MAP(CChildFrame)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT Register(_child_type ChildType);
	LRESULT Unregister();

	// Return the main frame
	CMainFrame* GetMainFrame() { ATLASSERT(m_pMainFrame); return m_pMainFrame; }

	HWND CreateChild(HWND hWndParent = NULL) {
		GetChildClassInfo().Register(&m_pfnSuperWindowProc);

		TCHAR szWindowName[256];
		szWindowName[0] = 0;
		::LoadString(_Module.GetResourceInstance(), GetChildClassInfo().m_uCommonResourceID, szWindowName, 256);

		HMENU hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(GetChildClassInfo().m_uCommonResourceID));

		HWND hWnd = Create(hWndParent, 0, szWindowName, 0, 0, (UINT)hMenu, 0);

		if(hWnd != NULL)
			m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(GetChildClassInfo().m_uCommonResourceID));
		return hWnd;

	}
};
