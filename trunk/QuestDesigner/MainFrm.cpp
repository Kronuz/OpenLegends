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
// MainFrm.cpp : implmentation of the CMainFrame class
//

#include "stdafx.h"
#include "QuestDesigner.h"

#include "aboutdlg.h"

#include <ssfiledialog.h>

static int nStatusBarPanes [] =
{
	ID_DEFAULT_PANE,
	ID_ICON_PANE,
	ID_POSITION_PANE,
	ID_OVERTYPE_PANE,
	ID_READONLY_PANE
};

static LPCTSTR lpcszQuestDesignerRegKey = _T("Software\\OpenZelda\\QuestDesigner");

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(baseClass::PreTranslateMessage(pMsg))
		return TRUE;

	HWND hWndFocus = ::GetFocus();
	HWND hWndMDIActive = MDIGetActive();

	if((hWndMDIActive == hWndFocus) || (::IsChild(hWndMDIActive, hWndFocus))) {
		if(hWndMDIActive != NULL)
			return (BOOL)::SendMessage(hWndMDIActive, WM_FORWARDMSG, 0, (LPARAM)pMsg);
	} 
	else if(m_InfoFrame.IsWindow() && m_InfoFrame.IsChild(hWndFocus)) {
		if(m_InfoFrame.PreTranslateMessage(pMsg))
			return TRUE;
	}
	return FALSE;
}

BOOL CMainFrame::OnIdle()
{
	// Update all the toolbar items
	UIUpdateToolBar();
	// Update all the menu items
	UIUpdateMenuItems();

	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// updating the window handler of the project's console
	CConsole::m_shWnd = m_hWnd;

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach the menu to the command bar
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove the old style menu
	SetMenu(NULL);

	// create a toolbar
	HWND hScriptToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_SCRIPTS, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hScriptToolBar);

	// create a toolbar
	HWND hProjectToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_PROJECT, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hProjectToolBar);

	// create a toolbar
	HWND hMapEdToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_MAPEDITOR, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hMapEdToolBar);

	// create a rebat to hold both: the command bar and the toolbar
	if(!CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE)) {
		ATLTRACE("Failed to create applications rebar\n");
		return -1;      // fail to create
	}	

	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hProjectToolBar, NULL, TRUE, 144);
	AddSimpleReBarBand(hScriptToolBar, "Scripts", FALSE);
	AddSimpleReBarBand(hMapEdToolBar, "Map Editor", TRUE);

	// create a status bar
	if(!CreateSimpleStatusBar(_T("Ready")) ||
		 !m_wndStatusBar.SubclassWindow(m_hWndStatusBar) ||
		 !m_wndStatusBar.SetPanes(nStatusBarPanes, 
		 sizeof(nStatusBarPanes)/sizeof(int), false) ) {
		
		ATLTRACE("Failed to create status bar\n");
		return -1;      // fail to create
	}

	HICON hIcon = AtlLoadIconImage(IDI_ICO_OK, LR_DEFAULTCOLOR);
	m_wndStatusBar.SetPaneIcon(ID_ICON_PANE, hIcon);

	// finally we create the MDI client
	if(!CreateMDIClient()) {
		ATLTRACE("Failed to create MDI client\n");
		return -1;      // fail to create
	}

	m_tabbedClient.SetTabOwnerParent(m_hWnd);
	BOOL bSubclass = m_tabbedClient.SubclassWindow(m_hWndMDIClient);
	m_CmdBar.UseMaxChildDocIconAndFrameCaptionButtons(false);
	m_CmdBar.SetMDIClient(m_hWndMDIClient);

	UISetCheck(ID_APP_TOOLBAR, TRUE);
	UISetCheck(ID_APP_STATUS_BAR, TRUE);

	UISetCheck(ID_APP_WORLDED, FALSE);
	UISetCheck(ID_APP_MAPED, FALSE);
	UISetCheck(ID_APP_SPTSHTED, FALSE);

	UISetCheck(ID_APP_INFORMATION, TRUE);
	UISetCheck(ID_APP_PROPERTIES, FALSE);

	// Update all the menu items
	UIUpdateMenuItems();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	InitializeDockingFrame();
	InitializeDefaultPanes();

	PostMessage(CWM_INITIALIZE);

	m_InfoFrame.DisplayTab(m_OutputView.m_hWnd);
	CProjectManager::Instance()->Load(_T("Kronuz Project"));

	return 0;
}
void CMainFrame::InitializeDefaultPanes()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	CRect rcFloat(0,0,400,200);
	CRect rcDock(0,0,200,rcClient.Width()-200);

	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	m_InfoFrame.Create(m_hWnd, rcFloat, _T("Information Window"), dwStyle);
	DockWindow(
		m_InfoFrame,
		dockwins::CDockingSide(dockwins::CDockingSide::sBottom),
		0 /*nBar*/,
		float(0.0)/*fPctPos*/,
		rcDock.Width() /* nWidth*/,
		rcDock.Height() /* nHeight*/);

	dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL;
	m_OutputView.Create(m_InfoFrame, rcDefault, NULL, dwStyle, WS_EX_CLIENTEDGE);
	m_TaskListView.Create(m_InfoFrame, rcDefault, NULL, dwStyle, WS_EX_CLIENTEDGE);
	m_DescriptionView.Create(m_InfoFrame, rcDefault, NULL, dwStyle, WS_EX_CLIENTEDGE);
	m_OutputView.m_pMainFrame = this;

	int nFirstIndex = m_InfoFrame.AddBitmap(IDB_MSDEV_TAB_ICONS, RGB(0,255,0));

	m_InfoFrame.AddTab(m_DescriptionView,	_T("Project Description"),	1 + nFirstIndex);
	m_InfoFrame.AddTab(m_TaskListView,		_T("Things To Do"),			6 + nFirstIndex);
	m_InfoFrame.AddTab(m_OutputView,		_T("Output Window"),		3 + nFirstIndex);


	m_ListFrame.Create(m_hWnd, rcFloat, _T("World List Window"), dwStyle);
	DockWindow(
		m_ListFrame,
		dockwins::CDockingSide(dockwins::CDockingSide::sLeft),
		0 /*nBar*/,
		float(0.0)/*fPctPos*/,
		rcDock.Width() /* nWidth*/,
		rcDock.Height() /* nHeight*/);

	m_FoldersView.Create(m_ListFrame, 
		rcDefault, 
		NULL, 
		WS_CHILD | WS_VISIBLE |
		TVS_LINESATROOT |TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS, 
		WS_EX_CLIENTEDGE);
	m_ListFrame.AddTab(m_FoldersView,		_T("Project"),	NULL);

}
LRESULT CMainFrame::OnInitialize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	sstate::CDockWndMgr mgrDockWnds;

	mgrDockWnds.Add(sstate::CDockingWindowStateAdapter<CTabbedDockingWindow>(m_InfoFrame));

	m_stateMgr.Initialize(lpcszQuestDesignerRegKey, m_hWnd);
	m_stateMgr.Add(sstate::CRebarStateAdapter(m_hWndToolBar));
	m_stateMgr.Add(sstate::CToggleWindowAdapter(m_hWndStatusBar));
	m_stateMgr.Add(mgrDockWnds);
	m_stateMgr.Restore();
	UpdateLayout();
	return 0;
}
LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}
LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// create a new MDI child window
//	CreateNewChildWnd();

	CHtmlFrame *pChild = new CHtmlFrame(this);
	pChild->CreateEx(m_hWndClient);

	// TODO: add code to initialize document

	return 0;
}
LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	static TCHAR szFilter[] = "OZ Script files (*.zes;*.inc)|*.zes; *.inc|All Files (*.*)|*.*||";
	CSSFileDialog wndFileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter, m_hWnd);
	
	if(IDOK == wndFileDialog.DoModal()) {
		FileOpen(wndFileDialog.m_ofn.lpstrFile, 0, (wndFileDialog.m_ofn.Flags&OFN_READONLY)?TRUE:FALSE);
	}
	return TRUE;
}
LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_APP_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}
LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_APP_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}
int CMainFrame::CountChilds(_child_type ChildType)
{
	int cnt = 0;
	CChildFrame *pChildFrame;
	for(int i=0; i<m_ChildList.GetSize(); i++) {
		pChildFrame = m_ChildList[i];
		if(pChildFrame->m_ChildType == ChildType || tAny == ChildType) cnt++;
	}
	return cnt;
}
CChildFrame* CMainFrame::FindChild(LPCSTR lpszName)
{
	int cnt = 0;
	CChildFrame *pChildFrame;
	for(int i=0; i<m_ChildList.GetSize(); i++) {
		pChildFrame = m_ChildList[i];
		if(pChildFrame->m_sChildName == lpszName) {
			return pChildFrame;
		}
	}
	return NULL;
}
LRESULT CMainFrame::OnViewWorldEditor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(CountChilds(tWorldEditor)) {
		CChildFrame *pChild = FindChild(_T("World Editor"));
		ATLASSERT(pChild);
		SendMessage(pChild->m_hWnd, WM_CLOSE, 0, 0);
	} else {
		CWorldEditorFrame *pChild = new CWorldEditorFrame(this);
		HWND hChildWnd = pChild->CreateEx(m_hWndClient);
	}
	OnIdle(); // Force idle processing to update the toolbar.
	return 0;
}
LRESULT CMainFrame::OnViewMapEditor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CMapEditorFrame *pChild = new CMapEditorFrame(this);
	HWND hChildWnd = pChild->CreateEx(m_hWndClient);

	OnIdle(); // Force idle processing to update the toolbar.
	return 0;
}
LRESULT CMainFrame::OnViewSpriteEditor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}
LRESULT CMainFrame::OnWindowCascade(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MDICascade();
	return 0;
}
LRESULT CMainFrame::OnWindowTile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MDITile();
	return 0;
}
LRESULT CMainFrame::OnWindowArrangeIcons(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MDIIconArrange();
	return 0;
}

LRESULT CMainFrame::OnBuildProject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_InfoFrame.DisplayTab(m_OutputView.m_hWnd);
	CProjectManager::Instance()->StartBuild();
	return 0;
}

int CMainFrame::Select(LPCTSTR szFilename, LPARAM lParam)
{
	CChildFrame *pChildFrame;
	for(int i=0; i<m_ChildList.GetSize(); i++) {
		pChildFrame = m_ChildList[i];
		if(pChildFrame->m_sChildName == szFilename) {
			pChildFrame->BringWindowToTop();
			if(lParam!=0) SendMessage(pChildFrame->m_hWnd, WMQD_SELECT, 0, lParam);
			return 1;
		}
	}
	return 0;
}
int CMainFrame::FileOpen(LPCTSTR szFilename, LPARAM lParam, BOOL bReadOnly)
{
	// Searching for an open child with the same file:
	if(Select(szFilename, lParam)) return 1;

	// make the title for the child window
	TCHAR szTitle [ _MAX_FNAME ], szExt [ _MAX_FNAME ];
	_splitpath(szFilename, NULL, NULL, szTitle, szExt);
	lstrcat(szTitle, szExt);

	// create a hiden MDI child
	// create a new MDI child window
	CScriptEditorFrame *pChild = new CScriptEditorFrame(this);
	HWND hChildWnd = pChild->CreateEx(m_hWndClient);

	// get the child's 'view' (actually child's client control)
	CScriptEditorView *pView = pChild->GetView();

	// open the requested file
	if(!pView->DoFileOpen(szFilename, szTitle)) {
		// kill the failed window (will delete itself)
		pChild->DestroyWindow();
		return 0;
	} else {
		pView->SetReadOnly(bReadOnly);
		if(lParam!=0) SendMessage(pChild->m_hWnd, WMQD_SELECT, 0, lParam);
	}
	return 1;
}

void CMainFrame::UIUpdateMenuItems ()
{
	_child_type ActiveChildType = tAny;
	CChildFrame *pChildFrame;
	HWND hWndMDIActive = MDIGetActive();
	int OpenScripts=0;
	for(int i=0; i<m_ChildList.GetSize(); i++) {
		pChildFrame = m_ChildList[i];
		if(pChildFrame->m_ChildType == tScriptEditor) OpenScripts++;
		if(pChildFrame->m_hWnd==hWndMDIActive) ActiveChildType=tScriptEditor;
	}
	
	int nChildWindows = m_ChildList.GetSize();

	UISetCheck(ID_APP_INFORMATION, m_InfoFrame.IsWindow() && m_InfoFrame.IsWindowVisible());
	UISetCheck(ID_APP_WORLDED, CountChilds(tWorldEditor));

	if( ActiveChildType!=tScriptEditor ) {
		UIEnable(ID_SCRIPTED_RELOAD, FALSE);
		UIEnable(ID_SCRIPTED_SAVE, FALSE);
		UIEnable(ID_SCRIPTED_SAVE_ALL, FALSE);
		UIEnable(ID_SCRIPTED_SAVE_AS, FALSE);
		UIEnable(ID_SCRIPTED_PRINT, FALSE);
		UIEnable(ID_SCRIPTED_PRINT_SETUP, FALSE);

		UIEnable(ID_SCRIPTED_UNDO, FALSE);
		UIEnable(ID_SCRIPTED_REDO, FALSE);	
		UIEnable(ID_SCRIPTED_CUT, FALSE);
		UIEnable(ID_SCRIPTED_COPY, FALSE);
		UIEnable(ID_SCRIPTED_PASTE, FALSE);
		UIEnable(ID_SCRIPTED_ERASE, FALSE);

		UIEnable(ID_SCRIPTED_TAB, FALSE);
		UIEnable(ID_SCRIPTED_UNTAB, FALSE);
		
		UIEnable(ID_SCRIPTED_FIND_SELECTION, FALSE);
		
		UIEnable(ID_SCRIPTED_READ_ONLY, FALSE);
		UISetCheck(ID_SCRIPTED_READ_ONLY, 0);
		
		UIEnable(ID_SCRIPTED_UPPERCASE, FALSE);
		UIEnable(ID_SCRIPTED_LOWERCASE, FALSE);
		UIEnable(ID_SCRIPTED_TABIFY, FALSE);
		UIEnable(ID_SCRIPTED_UNTABIFY, FALSE);
		
		UIEnable(ID_SCRIPTED_SHOW_WHITE_SPACE, FALSE);
		UISetCheck(ID_SCRIPTED_SHOW_WHITE_SPACE, 0);
		
		UIEnable(ID_SCRIPTED_GOTO_NEXT_BOOKMARK, FALSE);
		UIEnable(ID_SCRIPTED_GOTO_PREV_BOOKMARK, FALSE);
		UIEnable(ID_SCRIPTED_CLEAR_ALL_BOOKMARKS, FALSE);
	} 
	else {
		if(OpenScripts > 1) {
			UIEnable(ID_SCRIPTED_SAVE_ALL, TRUE);
		} else {
			UIEnable(ID_SCRIPTED_SAVE_ALL, FALSE);
		}
	}

}
