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
/*! \file		ChildFrm.h 
	\brief		Interface of the CChildFrame class.
	\date		April 15, 2003
*/

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMainFrame;
class CChildView;

enum _child_type { tAny=0, tScriptEditor, tSpriteEditor, tWorldEditor, tMapEditor };
/////////////////////////////////////////////////////////////////////////////
// This is the common childs class
class CChildFrame : 
	public CTabbedMDIChildWindowImpl<CChildFrame>,
	public CIdleHandlerPump
{
	typedef CTabbedMDIChildWindowImpl<CChildFrame> baseClass;

protected:

	CChildView *m_pChildView;

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

	bool hasChanged();

	virtual BOOL OnIdle();
	virtual void OnFinalMessage(HWND /*hWnd*/);

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MDICHILD)

	BEGIN_MSG_MAP(CChildFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)

		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		//MESSAGE_HANDLER(UWM_MDICHILDSHOWTABCONTEXTMENU, OnShowTabContextMenu)

		CHAIN_MSG_MAP(baseClass)

		// Pass all unhandled WM_COMMAND messages to the client window or 'view'
		CHAIN_CLIENT_COMMANDS()
		// Reflect all the WM_NOTIFY messages to the client window
		REFLECT_NOTIFICATIONS()

	END_MSG_MAP()

	LRESULT Register(_child_type ChildType);
	LRESULT Unregister();

	// Return the main frame
	CMainFrame* GetMainFrame() { ATLASSERT(m_pMainFrame); return m_pMainFrame; }
	LRESULT OnShowTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
};

