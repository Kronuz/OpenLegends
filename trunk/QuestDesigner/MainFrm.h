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
/*! \file		MainFrm.h 
	\brief		Interface of the CMainFrame class.
	\date		April 15, 2003
*/

#pragma once

#include <dbstate.h>
#include <DockingFrame.h>
#include <TabbedDockingWindow.h>

#include "SuperCombo.h"

#define CWM_INITIALIZE	(WMDF_LAST+1)

/////////////////////////////////////////////////////////////////////////////

#include <PlainTextView.h>

#include "ChildFrm.h"

#include "FoldersTreeBox.h"
#include "BuildOutputBox.h"
#include "ThumbnailsBox.h"
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

	CProjectFactory *m_pProjectFactory;
	IGame *m_pOZKernel;
protected:
	typedef CMainFrame thisClass;
	typedef dockwins::CMDIDockingFrameImpl<CMainFrame> baseClass;

public:
	CTabbedMDIClient< CDotNetTabCtrl<CTabViewTabItem> > m_tabbedClient;
	CMainFrame() : 
		m_pProjectFactory(NULL), 
		m_pOZKernel(NULL), 
		m_bProjectLoaded(false), 
		m_bQuestLoaded (false),
		m_bAllowSounds(false),
		m_bAllowAnimations(true),
		m_bAllowParallax(false) 
	{
	}

protected:
	sstate::CWindowStateMgr	m_stateMgr;
	
	CSimpleArray<CChildFrame*> m_ChildList;

	// The windows status bar
	CMultiPaneStatusBarCtrl m_wndStatusBar;
	CTabbedMDICommandBarCtrl m_CmdBar;

public:
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
	CThumbnailsBox m_ThumbnailsBox;

	CPropertyView m_PropertiesView;

////////////////////////////////////////////////////////

	BOOL m_bLayers;
	CSuperComboBoxCtrl m_ctrlLayers;
	CTrueColorToolBarCtrl m_ctrlProjectToolBar;
	CTrueColorToolBarCtrl m_ctrlMainToolBar;
	CTrueColorToolBarCtrl m_ctrlDebugToolBar;

	bool m_bProjectLoaded;
	bool m_bQuestLoaded;
	bool m_bAllowSounds;
	bool m_bAllowAnimations;
	bool m_bAllowParallax;
	HWND m_ahWnd[10];

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
		
		UPDATE_ELEMENT(ID_APP_SOUND, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_ANIM, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_APP_PARALLAX, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)

		UPDATE_ELEMENT(ID_DBG_DEBUG, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DBG_PAUSE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DBG_STOP, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DBG_NEXT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DBG_STEPIN, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DBG_STEPOVER, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DBG_STEPOUT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DBG_PROFILER, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DBG_BREAKPOINT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DBG_CONTINUE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)

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
		
		MESSAGE_HANDLER(WM_USER, OnDebugCommand);

		MENU_COMMAND_HANDLER(ID_PROJECT_OPEN, OnProjectOpen)

		MENU_COMMAND_HANDLER(ID_QUEST_NEW, OnFileNew)
		MENU_COMMAND_HANDLER(ID_QUEST_OPEN, OnFileOpen)

		MENU_COMMAND_HANDLER(ID_DBG_STEPIN, OnStepInto)
		MENU_COMMAND_HANDLER(ID_DBG_STEPOUT, OnStepOut)
		MENU_COMMAND_HANDLER(ID_DBG_STEPOVER, OnStepOver)
		MENU_COMMAND_HANDLER(ID_DBG_PAUSE, OnDbgBreak)
		MENU_COMMAND_HANDLER(ID_DBG_STOP, OnDbgStop)
		MENU_COMMAND_HANDLER(ID_DBG_NEXT, OnShowNext)
		MENU_COMMAND_HANDLER(ID_DBG_CONTINUE, OnContinue)

		MENU_COMMAND_HANDLER(ID_APP_NEW, OnScriptFileNew)
		MENU_COMMAND_HANDLER(ID_APP_OPEN, OnScriptFileOpen)

		MENU_COMMAND_HANDLER(ID_APP_TOOLBAR, OnViewToolBar)
		MENU_COMMAND_HANDLER(ID_APP_STATUS_BAR, OnViewStatusBar)
		MENU_COMMAND_HANDLER(ID_APP_EXIT, OnFileExit)

		COMMAND_ID_HANDLER(ID_APP_SOUND, OnSound)
		COMMAND_ID_HANDLER(ID_APP_ANIM, OnAnim)
		COMMAND_ID_HANDLER(ID_APP_PARALLAX, OnParallax)

		MENU_COMMAND_HANDLER(ID_APP_WORLDED, OnViewWorldEditor)
		MENU_COMMAND_HANDLER(ID_APP_MAPED, OnViewMapEditor)
		MENU_COMMAND_HANDLER(ID_APP_SCRIPTED, OnViewScriptEditor)
		MENU_COMMAND_HANDLER(ID_APP_SPTSHTED, OnViewSpriteEditor)

		MENU_COMMAND_HANDLER(ID_APP_BUILD, OnBuildProject)
		MENU_COMMAND_HANDLER(ID_APP_STOPBUILD, OnStopBuild)

		MENU_COMMAND_HANDLER(ID_DBG_DEBUG, OnRunProject)

		//MESSAGE_HANDLER(WMQD_BEGIN, ??)
		MESSAGE_HANDLER(WMQD_MESSAGE, m_OutputBox.OnWriteMsg)

		SIMPLE_MESSAGE_HANDLER(WMQD_STEPEND, CProjectFactory::Instance()->BuildNextStep)
		SIMPLE_MESSAGE_HANDLER(WMQD_BUILDBEGIN, m_OutputBox.BeginBuildMsg)
		SIMPLE_MESSAGE_HANDLER(WMQD_BUILDEND, m_OutputBox.EndBuildMsg)

		SIMPLE_MESSAGE_HANDLER(WMGP_ADDTREE, m_GameProject.OnAddTree)
		SIMPLE_MESSAGE_HANDLER(WMGP_DELTREE, m_GameProject.OnDelTree)

		SIMPLE_MESSAGE_HANDLER(WMQ_ADDTREE, m_Quest.OnAddTree)
		SIMPLE_MESSAGE_HANDLER(WMQ_DELTREE, m_Quest.OnDelTree)

		SIMPLE_MESSAGE_HANDLER(WMP_CLEAR, m_PropertiesView.OnClear)
		SIMPLE_MESSAGE_HANDLER(WMP_ADDINFO, m_PropertiesView.OnAddInfo)
		SIMPLE_MESSAGE_HANDLER(WMP_SETPROP, m_PropertiesView.OnSetProperties)
		SIMPLE_MESSAGE_HANDLER(WMP_UPDATE, m_PropertiesView.OnUpdate)

		if(uMsg == WM_COMMAND && (LOWORD(wParam) >= ID_VIEW_PANEFIRST && LOWORD(wParam) <= ID_VIEW_PANELAST)) {
			ATLASSERT(m_PaneWindows.size() >= (ID_VIEW_PANELAST-ID_VIEW_PANEFIRST));
			m_PaneWindows[LOWORD(wParam) - ID_VIEW_PANEFIRST]->Toggle();
		}

		MENU_COMMAND_HANDLER(ID_APP_HELP, OnAppHelp)
		MENU_COMMAND_HANDLER(ID_APP_ABOUT, OnAppAbout)
		MENU_COMMAND_HANDLER(ID_APP_PREFERENCES, OnAppConfig)
		MENU_COMMAND_HANDLER(ID_WINDOW_CASCADE, OnWindowCascade)
		MENU_COMMAND_HANDLER(ID_WINDOW_TILE_HORZ, OnWindowTile)
		MENU_COMMAND_HANDLER(ID_WINDOW_ARRANGE, OnWindowArrangeIcons)

		// Pass all unhandled WM_COMMAND messages to the active child window
		CHAIN_MDI_CHILD_COMMANDS()
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(baseClass)

		REFLECT_NOTIFICATIONS()

	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT OnDebugCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void InitializeDefaultPanes();
	LRESULT OnInitialize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnAnim(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL &bHandled);
	LRESULT OnParallax(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL &bHandled);
	LRESULT OnSound(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL &bHandled);

	LRESULT OnFileExit();
	LRESULT OnFileNew();
	LRESULT OnFileOpen();

	LRESULT OnProjectOpen();
	
	LRESULT OnScriptFileNew();
	LRESULT OnScriptFileOpen();

	LRESULT OnViewToolBar();
	LRESULT OnViewStatusBar();
	
	LRESULT OnViewWorldEditor();
	LRESULT OnViewMapEditor();
	LRESULT OnViewSpriteEditor();
	LRESULT OnViewScriptEditor();

	LRESULT OnViewInformationWindow();
	LRESULT OnViewPropertiesWindow();

	LRESULT OnAppHelp();
	LRESULT OnAppAbout();
	LRESULT OnAppConfig();
	LRESULT OnWindowCascade();
	LRESULT OnWindowTile();
	LRESULT OnWindowArrangeIcons();

	int OnSaveAll();
	HRESULT OnBuildProject();
	HRESULT OnRunProject();
	HRESULT OnStopBuild();

	void OnDbgBreak();
	void OnDbgStop();
	void OnShowNext();
	void OnStepInto();
	void OnStepOut();
	void OnStepOver();
	void OnContinue();

	HWND CreatePane(HWND hWndClient, LPCTSTR sName, HICON hIcon, CRect& rcDock, HWND hDockTo, dockwins::CDockingSide side);
public:
	HWND GetOldFocus(_child_type ChildType);
	void SetOldFocus(_child_type ChildType, HWND hWnd);

	int CountChilds(_child_type ChildType = tAny);
	CChildFrame* FindChild(LPCSTR lpszName);

	void StatusBar(LPCSTR szMessage, UINT Icon); // used to change the status bar message and icon.

	int Select(LPCTSTR szFilename, LPARAM lParam);
	int FileOpen(LPCTSTR szFilename, LPARAM lParam=0, BOOL bReadOnly=FALSE);
	int ScriptFileOpen(LPCTSTR szFilename, LPARAM lParam=0, BOOL bReadOnly=FALSE);
	int MapCreate(CPoint &Point);
	int MapFileOpen(CPoint &Point);
	int SptShtFileOpen(CSpriteSheet *pSpriteSheet, LPCSTR lpszSprite);

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
