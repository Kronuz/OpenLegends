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

CBString g_sHomeDir;

static LPCTSTR lpcszQuestDesignerRegKey = _T("SOFTWARE\\OpenZelda\\QuestDesigner");

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
	else {
		for(_PaneWindowIter iter=m_PaneWindows.begin(); iter!=m_PaneWindows.end(); iter++) {
			CTabbedDockingWindow* pPaneWindow = *iter;
			if(pPaneWindow->IsChild(hWndFocus)) {
				if(pPaneWindow->PreTranslateMessage(pMsg)) {
					return TRUE;
				}
			}
		}
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
	HWND hProjectToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | CCS_ADJUSTABLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hProjectToolBar);

	// create a toolbar
	HWND hMapEdToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_MAIN, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | CCS_ADJUSTABLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hMapEdToolBar);

	CToolBarCtrl wndToolBar;
	wndToolBar.Attach( hMapEdToolBar );

	TBBUTTONINFO tbbi;
	tbbi.cbSize = sizeof( TBBUTTONINFO );
	tbbi.dwMask = TBIF_SIZE;
	wndToolBar.GetButtonInfo( ID_MAPED_LAYER, &tbbi );

	tbbi.dwMask = TBIF_SIZE | TBIF_STYLE;
	tbbi.fsStyle = TBSTYLE_SEP;
	tbbi.cx = 150;
	wndToolBar.SetButtonInfo( ID_MAPED_LAYER, &tbbi );

	RECT rc;
	wndToolBar.GetRect( ID_MAPED_LAYER, &rc );
	wndToolBar.Detach();
	 
	rc.left += ::GetSystemMetrics( SM_CXEDGE );
	rc.right -= ::GetSystemMetrics( SM_CXEDGE );
	rc.top += ::GetSystemMetrics( SM_CYEDGE );
	rc.bottom += 200;
	m_Layers.Create( m_hWnd, &rc, NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		CBS_DROPDOWNLIST | CBS_HASSTRINGS,
		0, ID_MAPED_LAYER );
	m_bLayers = TRUE;
	 
	m_Layers.SetFont(AtlGetDefaultGuiFont());
	m_Layers.SetParent(hMapEdToolBar);

	m_Layers.AddString(_T("Layer 0"));
    m_Layers.AddString(_T("Layer 1"));
    m_Layers.AddString(_T("Layer 2"));
    m_Layers.AddString(_T("Layer 3"));
    m_Layers.AddString(_T("Layer 4"));
    m_Layers.AddString(_T("Layer 5"));
	m_Layers.SelectString(-1,_T("Layer 0"));

	// create a rebat to hold both: the command bar and the toolbar
	if(!CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE | CCS_ADJUSTABLE)) {
		ATLTRACE("Failed to create applications rebar\n");
		return -1;      // fail to create
	}	
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hProjectToolBar, NULL, TRUE, 0, TRUE);
	AddSimpleReBarBand(hMapEdToolBar, NULL, TRUE, 0, TRUE);

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

	UIUpdateMenuItems();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	InitializeDockingFrame();
	InitializeDefaultPanes();

	PostMessage(CWM_INITIALIZE);

	return 0;
}

HWND CMainFrame::CreatePane(HWND hWndClient, LPCTSTR sName, HICON hIcon, CRect& rcDock, HWND hDockTo, dockwins::CDockingSide side)
{
	HWND hWndPane = NULL;

	// Task List
	CTabbedDockingWindow* pPaneWindow = CTabbedDockingWindow::CreateInstance();
	if(pPaneWindow) {
		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		hWndPane = pPaneWindow->Create(m_hWnd, rcDefault, sName, dwStyle);
		DockWindow(
			*pPaneWindow,
			side,
			0 /*nBar*/,
			float(0.0)/*fPctPos*/,
			rcDock.Width() /* nWidth*/,
			rcDock.Height() /* nHeight*/);

		::SetParent(hWndClient, hWndPane);
		::SendMessage(hWndClient, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

		m_PaneWindowIcons.insert(m_PaneWindowIcons.end(), hIcon);
		m_PaneWindows.insert(m_PaneWindows.end(), pPaneWindow);

		pPaneWindow->SetReflectNotifications(false);
		pPaneWindow->SetClient(hWndClient);

		if(hDockTo) 	{
			pPaneWindow->DockTo(hDockTo, m_PaneWindows.size());
		}	

	}
	return hWndPane;
}

void CMainFrame::InitializeDefaultPanes()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	CRect rcDockV(0, 0, 250, 300);
	CRect rcDockH(0, 0, 200, 250);

	CImageList ilIcons;
	ilIcons.Create(IDB_TAB_ICONS, 16, 4, RGB(0,255,0));

	////////////////////////////////////////////////////////////////////
	// Create the content for the docking windows. 
	// If the order is changed, it's needed to also change the resource.h ID

	//////////////////////// First Pane:

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL;
	m_TaskListView.Create(m_hWnd,		rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);
	m_DescriptionView.Create(m_hWnd,	rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);
	m_OutputBox.Create(m_hWnd,			rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);
	m_OutputBox.m_pMainFrame = this;

	HWND hPane = 
	CreatePane(m_TaskListView,      _T("Things To Do"),			ilIcons.ExtractIcon(6),  rcDockH, NULL,dockwins::CDockingSide(dockwins::CDockingSide::sRight));
	CreatePane(m_DescriptionView,   _T("Project Description"),	ilIcons.ExtractIcon(3),  rcDockH, hPane,dockwins::CDockingSide(dockwins::CDockingSide::sRight));
	CreatePane(m_OutputBox,         _T("Output Window"),		ilIcons.ExtractIcon(10), rcDockH, hPane,dockwins::CDockingSide(dockwins::CDockingSide::sRight));

	//////////////////////// Second Pane:

	m_PropertiesView.Create(m_hWnd);

	hPane = 
	CreatePane(m_PropertiesView,	_T("Properties"),			ilIcons.ExtractIcon(1), rcDockV, NULL, dockwins::CDockingSide(dockwins::CDockingSide::sBottom));

	//////////////////////// Third Pane:

	m_GameProject.Create(m_hWnd);
	m_GameProject.m_pMainFrame = this;
	m_Quest.Create(m_hWnd);
	m_SpriteSets.Create(m_hWnd);
	m_SpriteSets.InitDragDrop();
	
	hPane = 
	CreatePane(m_GameProject,		_T("Project"),		ilIcons.ExtractIcon(2),  rcDockV, NULL, dockwins::CDockingSide(dockwins::CDockingSide::sBottom));
	CreatePane(m_Quest,				_T("Quest"),		ilIcons.ExtractIcon(14), rcDockV, hPane, dockwins::CDockingSide(dockwins::CDockingSide::sBottom));
	CreatePane(m_SpriteSets,		_T("Sprite Sets"),	ilIcons.ExtractIcon(4),	 rcDockV, hPane, dockwins::CDockingSide(dockwins::CDockingSide::sBottom));

}

LRESULT CMainFrame::OnInitialize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	sstate::CDockWndMgr mgrDockWnds;

	for(_PaneWindowIter iter=m_PaneWindows.begin(); iter!=m_PaneWindows.end(); iter++) {
		mgrDockWnds.Add(sstate::CDockingWindowStateAdapter<CTabbedDockingWindow>(*(*iter)));
	}

	m_stateMgr.Initialize(lpcszQuestDesignerRegKey, m_hWnd);
	m_stateMgr.Add(sstate::CRebarStateAdapter(m_hWndToolBar));
	m_stateMgr.Add(sstate::CToggleWindowAdapter(m_hWndStatusBar));
	m_stateMgr.Add(mgrDockWnds);
	m_stateMgr.Restore();
	UpdateLayout();

	ShowWindow(SW_SHOW);
	Invalidate();
	UpdateWindow();

	return 0;
}

LRESULT CMainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled=FALSE;
	//m_stateMgr.Store();
	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{

	// NOTE: the pane windows will delete themselves,
	//  so we just need to remove them from out list
	m_PaneWindows.clear();

	for(_PaneWindowIconsIter iter=m_PaneWindowIcons.begin(); iter!=m_PaneWindowIcons.end(); iter++) {
		::DestroyIcon(*iter);
		*iter = NULL;
	}
	m_PaneWindowIcons.clear();

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
	return 0;
}

LRESULT CMainFrame::OnProjectOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(!CProjectFactory::Interface(m_hWnd)->Close()) {
		int nChoice = MessageBox("Changes Done, save?", "Quest Designer", MB_YESNOCANCEL);
		if(nChoice == IDYES) {
			if(!CProjectFactory::Interface()->Save()) {
				MessageBox("Couldn't save!");
				return 0;
			}
		} else if(nChoice == IDCANCEL) {
			return 0;
		}
		CProjectFactory::Interface()->Close(true);
	} 

	UIEnableToolbar(FALSE);
	// Update all the toolbar items
	UIUpdateToolBar();
	UpdateWindow();

	::SendMessage(m_GameProject, WM_SETREDRAW, FALSE, 0);
	g_sHomeDir = "C:\\qd\\Quest Designer 2.1.4\\";
	CProjectFactory::Interface()->Load(g_sHomeDir);
	::SendMessage(m_GameProject, WM_SETREDRAW, TRUE, 0);
	::RedrawWindow(m_GameProject, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
	m_SpriteSets.PopulateTree(g_sHomeDir + "Sprite Sets");

	UIEnableToolbar(TRUE);

	return 0;
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
	if(tAny == ChildType) 
		return m_ChildList.GetSize();

	int cnt = 0;
	CChildFrame *pChildFrame;
	for(int i=0; i<m_ChildList.GetSize(); i++) {
		pChildFrame = m_ChildList[i];
		if(pChildFrame->m_ChildType == ChildType) cnt++;
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
		DWORD dwStyle = CountChilds()?0:WS_MAXIMIZE;
		HWND hChildWnd = pChild->CreateEx(m_hWndClient, NULL, NULL, dwStyle);
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
//	m_InfoFrame.DisplayTab(m_OutputBox.m_hWnd);
	CProjectFactory::Instance(m_hWnd)->StartBuild();
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
	DWORD dwStyle = CountChilds()?0:WS_MAXIMIZE;
	HWND hChildWnd = pChild->CreateEx(m_hWndClient, NULL, NULL, dwStyle);
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
	DWORD dwStyle = CountChilds()?0:WS_MAXIMIZE;
	HWND hChildWnd = pChild->CreateEx(m_hWndClient, NULL, NULL, dwStyle);
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
	if(!CProjectFactory::Interface()->CloseWorld()) {
		int nChoice = MessageBox("Changes Done, save?", "Quest Designer", MB_YESNOCANCEL);
		if(nChoice == IDYES) {
			if(!CProjectFactory::Interface()->SaveWorld()) {
				MessageBox("Couldn't save!");
				return 0;
			}
		} else if(nChoice == IDCANCEL) {
			return 0;
		}
		CProjectFactory::Interface()->CloseWorld(true);
	} 
	CProjectFactory::Interface()->LoadWorld(szFilename);

	if(Select(_T("World Editor"), 0)) return 1;

	CWorldEditorFrame *pChild = new CWorldEditorFrame(this);
	DWORD dwStyle = CountChilds()?0:WS_MAXIMIZE;
	HWND hChildWnd = pChild->CreateEx(m_hWndClient, NULL, NULL, dwStyle);
	ATLASSERT(hChildWnd);

	OnIdle(); // Force idle processing to update the toolbar.

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
	DWORD dwStyle = CountChilds()?0:WS_MAXIMIZE;
	HWND hChildWnd = pChild->CreateEx(m_hWndClient, NULL, NULL, dwStyle);
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
	int nChanges = 0;
	for(int i=0; i<m_ChildList.GetSize(); i++) {
		pChildFrame = m_ChildList[i];
		if(pChildFrame->hasChanged()) nChanges++;
		if(pChildFrame->m_hWnd==hWndMDIActive) ActiveChildType = pChildFrame->m_ChildType;
	}
	
	int nChildWindows = m_ChildList.GetSize();

	int id = ID_VIEW_PANEFIRST;
	for(_PaneWindowIter iter=m_PaneWindows.begin(); iter!=m_PaneWindows.end(); id++, iter++) {
		if((*iter)->IsWindow()) {
			UISetCheck(id, (*iter)->IsWindowVisible());
		}
	}

	UISetCheck(ID_APP_WORLDED, CountChilds(tWorldEditor));

	if( ActiveChildType!=tMapEditor ) {
		m_Layers.EnableWindow(FALSE);
		m_bLayers = FALSE;
	}

	if( ActiveChildType!=tScriptEditor ) {
		UIEnable(ID_APP_SAVE, FALSE);
		UIEnable(ID_APP_SAVE_ALL, FALSE);
		UIEnable(ID_APP_SAVE_AS, FALSE);

		UIEnable(ID_APP_RELOAD, FALSE);
		UIEnable(ID_APP_PRINT, FALSE);
		UIEnable(ID_APP_PRINT_SETUP, FALSE);

		UIEnable(ID_UNDO, FALSE);
		UIEnable(ID_REDO, FALSE);	
		UIEnable(ID_CUT, FALSE);
		UIEnable(ID_COPY, FALSE);
		UIEnable(ID_PASTE, FALSE);
		UIEnable(ID_ERASE, FALSE);

		UIEnable(ID_SCRIPTED_CODELIST, FALSE );
		UIEnable(ID_SCRIPTED_CODETIP1, FALSE );
		UIEnable(ID_SCRIPTED_CODETIP2, FALSE );

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

	if(nChanges > 1) {
		UIEnable(ID_APP_SAVE_ALL, TRUE);
	} else {
		UIEnable(ID_APP_SAVE_ALL, FALSE);
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
	m_Layers.EnableWindow(bEnable);
	m_bLayers = bEnable;
}
