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

#include "AboutDlg.h"
#include "PropertiesDlg.h"

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
	// Update all the menu items
	UIUpdateMenuItems();

	// Update all the toolbar items
	UIUpdateToolBar();

	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// updating the window handler of the project's console
	CConsole::ms_hWnd = m_hWnd;

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach the menu to the command bar
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove the old style menu
	SetMenu(NULL);

	// create a toolbar
	HWND hScriptToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_SCRIPTED, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_WRAPABLE | CCS_ADJUSTABLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hScriptToolBar);

	// create a toolbar
	HWND hProjectToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_PROJECT, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_WRAPABLE | CCS_ADJUSTABLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hProjectToolBar);

	// create a toolbar
	HWND hMapEdToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_MAPED_MAIN, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_WRAPABLE | CCS_ADJUSTABLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hMapEdToolBar);

	// create a rebat to hold both: the command bar and the toolbar
	if(!CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE | CCS_ADJUSTABLE)) {
		ATLTRACE("Failed to create applications rebar\n");
		return -1;      // fail to create
	}	

	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hProjectToolBar, NULL, TRUE, 285, TRUE);
	AddSimpleReBarBand(hMapEdToolBar, "Map Editor", TRUE, 200, TRUE);
	AddSimpleReBarBand(hScriptToolBar, "Script Editor", FALSE, 375, TRUE);

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

	m_tabbedClient.UseMDIChildIcon();
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

	UIEnableToolbar(FALSE);

	// Update all the toolbar items
	UIUpdateToolBar();

	UIEnableToolbar(TRUE);
	UIUpdateMenuItems();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	InitializeDockingFrame();
	InitializeDefaultPanes();

	PostMessage(CWM_INITIALIZE);

	m_InfoFrame.DisplayTab(m_OutputBox.m_hWnd);
	CVFile vfFile("Kronuz Project");

	ShowWindow(true);
	Invalidate();
	UpdateWindow();
	CProjectFactory::Instance(m_hWnd)->Interface()->Load(vfFile);

	return 0;
}

void CMainFrame::InitializeDefaultPanes()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	CRect rcFloat(0,0,400,200);
	//CRect rcDock(0,0,200,200);
	CRect rcDock(0, 0, 200, rcClient.Height()/3);

	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	// Create docking windows:
	m_InfoFrame.Create(m_hWnd, rcFloat, _T("Information Window"), dwStyle);
	m_ListFrame.Create(m_hWnd, rcFloat, _T("World List Window"), dwStyle);

	m_PropFrame.Create(m_hWnd, rcFloat, _T("Properties Window"), dwStyle,	WS_EX_CLIENTEDGE);

	// Add bitmaps to the docking window's tabs
	int nFirstIndex = m_InfoFrame.AddBitmap(IDB_MSDEV_TAB_ICONS, RGB(0,255,0));

	// Dock the windows.
	DockWindow (
		m_PropFrame,
		dockwins::CDockingSide(dockwins::CDockingSide::sRight),
		0,				// nBar
		float(0.0),		// fPctPos
		rcDock.Width(),	// nWidth
		rcDock.Height()	// nHeight
	);
	DockWindow (
		m_ListFrame,
		dockwins::CDockingSide(dockwins::CDockingSide::sRight),
		0,				// nBar
		float(0.0),		// fPctPos
		rcDock.Width(),	// nWidth
		rcDock.Height()*2	// nHeight
	);
	DockWindow (
		m_InfoFrame,
		dockwins::CDockingSide(dockwins::CDockingSide::sBottom),
		0,				// nBar
		float(0.0),		// fPctPos
		rcDock.Width(),	// nWidth
		rcDock.Height()	// nHeight
	);

	////////////////////////////////////////////////////////////////////
	// Create the content for the docking windows:
	dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL;
	m_TaskListView.Create(m_InfoFrame,		rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);
	m_DescriptionView.Create(m_InfoFrame,	rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);
	m_OutputBox.Create(m_InfoFrame,		rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);
	m_OutputBox.m_pMainFrame = this;

	dwStyle = WS_CHILD | WS_VISIBLE | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS;
	m_GameProject.Create(m_ListFrame,		rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);
	m_Quest.Create(m_ListFrame,				rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);

	// Add the created windows to the docking frame:
	m_InfoFrame.AddTab(m_DescriptionView,	_T("Project Description"),	1 + nFirstIndex);
	m_InfoFrame.AddTab(m_TaskListView,		_T("Things To Do"),			6 + nFirstIndex);
	m_InfoFrame.AddTab(m_OutputBox,			_T("Output Window"),		3 + nFirstIndex);

	m_ListFrame.AddTab(m_Quest,				_T("Quest"),				NULL);
	m_ListFrame.AddTab(m_GameProject,		_T("Game Project"),			NULL);

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
LRESULT CMainFrame::OnScriptFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// create a new MDI child window
//	CreateNewChildWnd();

	CHtmlFrame *pChild = new CHtmlFrame(this);
	HWND hChildWnd = pChild->CreateEx(m_hWndClient);
	ATLASSERT(hChildWnd);

	// TODO: add code to initialize document

	return 0;
}

LRESULT CMainFrame::OnScriptFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static TCHAR szFilter[] = "OZ Script files (*.zes;*.inc)|*.zes; *.inc|All Files (*.*)|*.*||";
	CSSFileDialog wndFileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter, m_hWnd);
	
	if(IDOK == wndFileDialog.DoModal()) {
		ScriptFileOpen(wndFileDialog.m_ofn.lpstrFile, 0, (wndFileDialog.m_ofn.Flags&OFN_READONLY)?TRUE:FALSE);
	}
	return TRUE;
}
LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static TCHAR szFilter[] = "OZ Quest files (*.qss;*.qsz)|*.qss; *.qsz|All Files (*.*)|*.*||";
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
		ATLASSERT(hChildWnd);
	}
	OnIdle(); // Force idle processing to update the toolbar.
	return 0;
}
LRESULT CMainFrame::OnViewMapEditor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MapCreate(CPoint(0,0));
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
LRESULT CMainFrame::OnAppConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CPropertiesDlg dlg(0);
	int result = dlg.DoModal();

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
	m_InfoFrame.DisplayTab(m_OutputBox.m_hWnd);
	CProjectFactory::Instance()->StartBuild();
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

int CMainFrame::MapCreate(CPoint &Point)
{
	CMapGroup *pMapGroupI;
	if((pMapGroupI = CProjectFactory::Interface()->FindMapGroup(Point.x, Point.y))==NULL) {
		if((pMapGroupI = CProjectFactory::Interface()->BuildMapGroup(Point.x, Point.y, 1, 1))==NULL) return 0;
	}

	// Searching for an open child with the same file:
	LPCSTR szTitle = pMapGroupI->GetMapGroupID();

	CMapEditorFrame *pChild = new CMapEditorFrame(this);
	HWND hChildWnd = pChild->CreateEx(m_hWndClient);
	ATLASSERT(hChildWnd);

	// get the child's 'view' (actually child's client control)
	CMapEditorView *pView = pChild->GetView();
	if(!pView->DoMapOpen(pMapGroupI, szTitle)) {
		// kill the failed window (will delete itself)
		::SendMessage(pChild->m_hWnd,WM_CLOSE,0,0);
		return 0;
	}
	return 1;

	return 0;
}

int CMainFrame::MapFileOpen(CPoint &Point)
{
	CMapGroup *pMapGroupI;
	if((pMapGroupI = CProjectFactory::Interface()->FindMapGroup(Point.x, Point.y))==NULL) return MapCreate(Point);

	LPCSTR szTitle = pMapGroupI->GetMapGroupID();

	// Searching for an open child with the same file:
//	if(Select(szTitle, 0)) return 1;

	if(!pMapGroupI->Load()) return 0;

	CMapEditorFrame *pChild = new CMapEditorFrame(this);
	HWND hChildWnd = pChild->CreateEx(m_hWndClient);
	ATLASSERT(hChildWnd);

	// get the child's 'view' (actually child's client control)
	CMapEditorView *pView = pChild->GetView();
	if(!pView->DoMapOpen(pMapGroupI, szTitle)) {
		// kill the failed window (will delete itself)
		::SendMessage(pChild->m_hWnd,WM_CLOSE,0,0);
		return 0;
	}
	return 1;
}

int CMainFrame::FileOpen(LPCTSTR szFilename, LPARAM lParam, BOOL bReadOnly)
{
	CVFile vfFile(szFilename);
	CProjectFactory::Interface()->LoadWorld(vfFile);
	return 1;
}

int CMainFrame::ScriptFileOpen(LPCTSTR szFilename, LPARAM lParam, BOOL bReadOnly)
{
	// Searching for an open child with the same file:
	if(Select(szFilename, lParam)) return 1;

	// make the title for the child window
	TCHAR szTitle [ _MAX_FNAME ], szExt [ _MAX_FNAME ];
	_splitpath(szFilename, NULL, NULL, szTitle, szExt);
	lstrcat(szTitle, szExt);

	CScriptEditorFrame *pChild = new CScriptEditorFrame(this);
	HWND hChildWnd = pChild->CreateEx(m_hWndClient);
	ATLASSERT(hChildWnd);

	// get the child's 'view' (actually child's client control)
	CScriptEditorView *pView = pChild->GetView();

	// open the requested file
	if(!pView->DoFileOpen(szFilename, szTitle)) {
		// kill the failed window (will delete itself)
		::SendMessage(pChild->m_hWnd,WM_CLOSE,0,0);
		return 0;
	} else {
		pView->SetReadOnly(bReadOnly);
		if(lParam!=0) SendMessage(pChild->m_hWnd, WMQD_SELECT, 0, lParam);
	}
	return 1;
}

void CMainFrame::UIUpdateMenuItems()
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

		UIEnable(ID_UNDO, FALSE);
		UIEnable(ID_REDO, FALSE);	
		UIEnable(ID_CUT, FALSE);
		UIEnable(ID_COPY, FALSE);
		UIEnable(ID_PASTE, FALSE);
		UIEnable(ID_ERASE, FALSE);

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
void CMainFrame::UIEnableToolbar(BOOL bEnable)
{
	const _AtlUpdateUIMap* pMap = GetUpdateUIMap();
	ATLASSERT(pMap != NULL);
	for(int nCount = 1; pMap->m_nID != (WORD)-1; nCount++) {
		UIEnable(pMap->m_nID, bEnable);
		pMap++;
	}
}
