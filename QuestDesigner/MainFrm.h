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
/*! \file		MainFrm.h 
	\brief		Interface of the CMainFrame class.
	\date		April 15, 2003
*/

#pragma once

#include <dbstate.h>
#include <DockingFrame.h>
#include <TabbedDockingWindow.h>

#define CWM_INITIALIZE	(WMDF_LAST+1)

/////////////////////////////////////////////////////////////////////////////

#include <PlainTextView.h>

#include "ChildFrm.h"

#include "FoldersTreeView.h"
#include "BuildOutputView.h"

#include "ProjectManager.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CIdleHandlerPump;

/////////////////////////////////////////////////////////////////////////////
/*! \class		CMainFrame
	\brief		This class is the main frame for the program.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	CMainFrame, derivated from the public library CMDIDockingFrameImpl,
	is a MDI frame window with docking capabilities.
	This window maintain a list of children windows. These children must
	be derived from the class CChildFrame.

	In addition, the Main Frame also has one tabbed docking window
	to keep detailed information about the project all the time,
	other tabbed docking window to keep the project tree, and yet another
	one to keep the properties of the selected object.

	The information tabbed docking window itself, contains three windows on each
	of its tabs: "Project Description", "Things To Do", and "Output Window".

	These specifications are subjected to change at any time as the project
	evolves and makes new or retire requirements.

	\sa CChildFrame, CIOCompressedMode.
	\todo Write the implementation of this class.
*/
class CMainFrame : 
	public dockwins::CMDIDockingFrameImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, 
	public CIdleHandlerPump
{
	friend CChildFrame;
protected:
	typedef CMainFrame thisClass;
	typedef dockwins::CMDIDockingFrameImpl<CMainFrame> baseClass;

protected:
	CTabbedMDIClient<CDotNetTabCtrl<CTabViewTabItem> > m_tabbedClient;
	sstate::CWindowStateMgr	m_stateMgr;
	
	CSimpleArray<CChildFrame*> m_ChildList;

	// The windows status bar
	CMultiPaneStatusBarCtrl m_wndStatusBar;

	CTabbedMDICommandBarCtrl m_CmdBar;

	// Information docking window:
	CTabbedDockingWindow m_InfoFrame;
	// Properties docking window:
	CTabbedDockingWindow m_PropFrame;
	// World list docking window:
	CTabbedDockingWindow m_ListFrame;

////////////////////////////////////////////////////////
	CFoldersTreeView m_FoldersView;

	CPlainTextView m_TaskListView;
	CPlainTextView m_DescriptionView;
	CBuildOutputView m_OutputView;
////////////////////////////////////////////////////////
	CProjectManager m_ProjectManager;
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT ( ID_FILE_RELOAD, UPDUI_MENUPOPUP )
		UPDATE_ELEMENT ( ID_FILE_SAVE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT ( ID_FILE_SAVE_ALL, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT ( ID_FILE_SAVE_AS, UPDUI_MENUPOPUP )
		UPDATE_ELEMENT ( ID_FILE_PRINT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT ( ID_FILE_PRINT_SETUP, UPDUI_MENUPOPUP )

		UPDATE_ELEMENT ( ID_EDIT_UNDO, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT ( ID_EDIT_REDO, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT ( ID_EDIT_CUT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT ( ID_EDIT_COPY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT ( ID_EDIT_PASTE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT ( ID_EDIT_CLEAR, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )		
		
		UPDATE_ELEMENT ( ID_EDIT_READ_ONLY, UPDUI_MENUPOPUP )
		
		UPDATE_ELEMENT ( ID_EDIT_TAB, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )
		UPDATE_ELEMENT ( ID_EDIT_UNTAB, UPDUI_MENUPOPUP | UPDUI_TOOLBAR )		
		
		UPDATE_ELEMENT ( ID_EDIT_FIND_SELECTION, UPDUI_MENUPOPUP )

		UPDATE_ELEMENT ( ID_EDIT_UPPERCASE, UPDUI_MENUPOPUP )
		UPDATE_ELEMENT ( ID_EDIT_LOWERCASE, UPDUI_MENUPOPUP )
		UPDATE_ELEMENT ( ID_EDIT_TABIFY, UPDUI_MENUPOPUP )
		UPDATE_ELEMENT ( ID_EDIT_UNTABIFY, UPDUI_MENUPOPUP )
		UPDATE_ELEMENT ( ID_EDIT_SHOW_WHITE_SPACE, UPDUI_MENUPOPUP )

		UPDATE_ELEMENT ( ID_EDIT_TOGGLE_BOOKMARK, UPDUI_MENUPOPUP )
		UPDATE_ELEMENT ( ID_EDIT_GOTO_NEXT_BOOKMARK, UPDUI_MENUPOPUP )
		UPDATE_ELEMENT ( ID_EDIT_GOTO_PREV_BOOKMARK, UPDUI_MENUPOPUP )
		UPDATE_ELEMENT ( ID_EDIT_CLEAR_ALL_BOOKMARKS, UPDUI_MENUPOPUP )		

		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_VIEW_INFO, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_PROP, UPDUI_MENUPOPUP)

	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(CWM_INITIALIZE, OnInitialize)

		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_VIEW_INFO, OnViewInfoWindow)

		COMMAND_ID_HANDLER(ID_BUILD, OnBuildProject)
		//MESSAGE_HANDLER(WMQD_BEGIN, ??)
		MESSAGE_HANDLER(WMQD_MESSAGE, m_OutputView.OnWriteMsg)

		SIMPLE_MESSAGE_HANDLER(WMQD_STEPEND, m_ProjectManager.BuildNextStep)
		SIMPLE_MESSAGE_HANDLER(WMQD_BUILDBEGIN, m_OutputView.BeginBuildMsg)
		SIMPLE_MESSAGE_HANDLER(WMQD_BUILDEND, m_OutputView.EndBuildMsg)

		SIMPLE_MESSAGE_HANDLER(WMQD_ADDTREE, m_FoldersView.AddTree)

		COMMAND_TOGGLE_MEMBER_HANDLER(ID_VIEW_INFO, m_InfoFrame)

		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_WINDOW_CASCADE, OnWindowCascade)
		COMMAND_ID_HANDLER(ID_WINDOW_TILE_HORZ, OnWindowTile)
		COMMAND_ID_HANDLER(ID_WINDOW_ARRANGE, OnWindowArrangeIcons)

		// Pass all unhandled WM_COMMAND messages to the active child window
		CHAIN_MDI_CHILD_COMMANDS()
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(baseClass)

	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void InitializeDefaultPanes();
	LRESULT OnInitialize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewInfoWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowCascade(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowTile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowArrangeIcons(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnBuildProject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	int Select(LPCTSTR szFilename, LPARAM lParam);
	int FileOpen(LPCTSTR szFilename, LPARAM lParam=0, BOOL bReadOnly=FALSE);

	void UIUpdateMenuItems();

};
