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

#include "FoldersTreeBox.h"
#include "BuildOutputBox.h"
#include "PropertiesDockingView.h"

#include "ProjectFactory.h"

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

public:
	CTabbedMDIClient< CDotNetTabCtrl<CTabViewTabItem> > m_tabbedClient;
protected:
	sstate::CWindowStateMgr	m_stateMgr;
	
	CSimpleArray<CChildFrame*> m_ChildList;

	// The windows status bar
	CMultiPaneStatusBarCtrl m_wndStatusBar;
	CTabbedMDICommandBarCtrl m_CmdBar;

	std::vector<CTabbedDockingWindow*> m_PaneWindows;
	typedef std::vector<CTabbedDockingWindow*>::iterator _PaneWindowIter;

	std::vector<HICON> m_PaneWindowIcons;
	typedef std::vector<HICON>::iterator _PaneWindowIconsIter;

////////////////////////////////////////////////////////
	CFoldersTreeBox m_SpriteSets;
	CFoldersTreeBox m_GameProject;
	CFoldersTreeBox m_Quest;

	CPlainTextView m_TaskListView;
	CPlainTextView m_DescriptionView;
	CBuildOutputBox m_OutputBox;

	CPropertyView m_PropertiesView;

////////////////////////////////////////////////////////

public:
	BOOL m_bLayers;
	CComboBox m_Layers;

////////////////////////////////////////////////////////

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_PROJECT_OPEN, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)

		UPDATE_ELEMENT(ID_QUEST_NEW, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_QUEST_OPEN, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)

		UPDATE_ELEMENT(ID_MAPED_LAYER, UPDUI_TOOLBAR)

		UPDATE_ELEMENT(ID_APP_NEW, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_OPEN, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_RELOAD, UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_APP_SAVE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_SAVE_ALL, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_SAVE_AS, UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_APP_PRINT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_BUILD, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_STOPBUILD, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_PREFERENCES, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_HELP, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_ABOUT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)

		UPDATE_ELEMENT(ID_APP_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_APP_STATUS_BAR, UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_APP_WORLDED, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_MAPED, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_SPTSHTED, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_SCRIPTED, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)

		UPDATE_ELEMENT(ID_SCRIPTED_READ_ONLY, UPDUI_MENUPOPUP)
		
		UPDATE_ELEMENT(ID_SCRIPTED_CODELIST, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_SCRIPTED_CODETIP1, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)		
		UPDATE_ELEMENT(ID_SCRIPTED_CODETIP2, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)		

		UPDATE_ELEMENT(ID_SCRIPTED_TAB, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_SCRIPTED_UNTAB, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)		
		
		UPDATE_ELEMENT(ID_SCRIPTED_FIND_SELECTION, UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_SCRIPTED_UPPERCASE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SCRIPTED_LOWERCASE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SCRIPTED_TABIFY, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SCRIPTED_UNTABIFY, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SCRIPTED_SHOW_WHITE_SPACE, UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_SCRIPTED_TOGGLE_BOOKMARK, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SCRIPTED_GOTO_NEXT_BOOKMARK, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SCRIPTED_GOTO_PREV_BOOKMARK, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SCRIPTED_CLEAR_ALL_BOOKMARKS, UPDUI_MENUPOPUP)		

		UPDATE_ELEMENT(ID_PANE_OUTPUTWINDOW        , UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PANE_PROPERTIES          , UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PANE_PROJECTDESCRIPTION  , UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PANE_THINGSTODO          , UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PANE_PROJECTEXPLORER     , UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PANE_QUESTEXPLORER       , UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PANE_SPRITESETEXPLORER   , UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_UNDO, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_REDO, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CUT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_COPY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PASTE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_ERASE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)		

	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(CWM_INITIALIZE, OnInitialize)

		COMMAND_ID_HANDLER(ID_PROJECT_OPEN, OnProjectOpen)

		COMMAND_ID_HANDLER(ID_QUEST_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_QUEST_OPEN, OnFileOpen)

		COMMAND_ID_HANDLER(ID_APP_NEW, OnScriptFileNew)
		COMMAND_ID_HANDLER(ID_APP_OPEN, OnScriptFileOpen)

		COMMAND_ID_HANDLER(ID_APP_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_APP_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)

		COMMAND_ID_HANDLER(ID_APP_WORLDED, OnViewWorldEditor)
		COMMAND_ID_HANDLER(ID_APP_MAPED, OnViewMapEditor)
		COMMAND_ID_HANDLER(ID_APP_SPTSHTED, OnViewSpriteEditor)

		COMMAND_ID_HANDLER(ID_APP_BUILD, OnBuildProject)
		//MESSAGE_HANDLER(WMQD_BEGIN, ??)
		MESSAGE_HANDLER(WMQD_MESSAGE, m_OutputBox.OnWriteMsg)

		SIMPLE_MESSAGE_HANDLER(WMQD_STEPEND, CProjectFactory::Instance()->BuildNextStep)
		SIMPLE_MESSAGE_HANDLER(WMQD_BUILDBEGIN, m_OutputBox.BeginBuildMsg)
		SIMPLE_MESSAGE_HANDLER(WMQD_BUILDEND, m_OutputBox.EndBuildMsg)

		SIMPLE_MESSAGE_HANDLER(WMQD_ADDTREE, m_GameProject.OnAddTree)
		SIMPLE_MESSAGE_HANDLER(WMQD_DELTREE, m_GameProject.OnDelTree)

		if(uMsg == WM_COMMAND && (LOWORD(wParam) >= ID_VIEW_PANEFIRST && LOWORD(wParam) <= ID_VIEW_PANELAST)) {
			ATLASSERT(m_PaneWindows.size() >= (ID_VIEW_PANELAST-ID_VIEW_PANEFIRST));
			m_PaneWindows[LOWORD(wParam) - ID_VIEW_PANEFIRST]->Toggle();
		}

		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_APP_PREFERENCES, OnAppConfig)
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
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	void InitializeDefaultPanes();
	LRESULT OnInitialize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnProjectOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	LRESULT OnScriptFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnScriptFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnViewWorldEditor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewMapEditor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewSpriteEditor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnViewInformationWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewPropertiesWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowCascade(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowTile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowArrangeIcons(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnBuildProject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	HWND CreatePane(HWND hWndClient, LPCTSTR sName, HICON hIcon, CRect& rcDock, HWND hDockTo, dockwins::CDockingSide side);
public:
	int CountChilds(_child_type ChildType = tAny);
	CChildFrame* FindChild(LPCSTR lpszName);

	int Select(LPCTSTR szFilename, LPARAM lParam);
	int FileOpen(LPCTSTR szFilename, LPARAM lParam=0, BOOL bReadOnly=FALSE);
	int ScriptFileOpen(LPCTSTR szFilename, LPARAM lParam=0, BOOL bReadOnly=FALSE);
	int MapCreate(CPoint &Point);
	int MapFileOpen(CPoint &Point);

	void UIUpdateMenuItems();
	void UIEnableToolbar(BOOL bEnable = TRUE);

	// Return a the window's UI updater
	CUpdateUIBase* GetUpdateUI() {
		return static_cast<CUpdateUIBase *>(this);
	}

	// The windows status bar
	CMultiPaneStatusBarCtrl* GetMultiPaneStatusBarCtrl() {
		return &m_wndStatusBar;
	}

};
