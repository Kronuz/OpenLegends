/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germ�n M�ndez Bravo (Kronuz)
   Copyright (C) 2001-2003. Open Legends's Project
 
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

CString g_sQuestFile;

static int nStatusBarPanes [] =
{
	ID_DEFAULT_PANE,
	ID_ICON_PANE,
	ID_POSITION_PANE,
	ID_OVERTYPE_PANE,
	ID_READONLY_PANE
};

static LPCTSTR lpcszQuestDesignerRegKey = _T("SOFTWARE\\OpenLegends\\QuestDesigner");

bool PathToParam(LPCSTR szPath, LPSTR szBuffer, int nSize)
{
	if(nSize < (int)strlen(szPath)+4) return false;
	*szBuffer = '\"';
	strcpy(szBuffer + 1, szPath);
	char *aux = strchr(szBuffer, '\0') - 1;
	bool dir = (*aux == '\\'); 
	if(!dir) aux++;
	*aux++ = '\"';
	if(dir) *aux++ = '\\';
	*aux = '\0';
	return true;
}

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

	m_ThumbnailsBox.OnIdle();

	if(!Connected()) CScriptEditorView::CleanStep();

	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_pProjectFactory = CProjectFactory::Instance(m_hWnd);
	m_pOLKernel = m_pProjectFactory->Interface();

	// updating the window handler of the project's console
	CConsole::ms_hWnd = m_hWnd;

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach the menu to the command bar
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	m_CmdBar.LoadImages(IDR_TB_MAIN);
	m_CmdBar.LoadImages(IDR_TB_DEBUG);
	// remove the old style menu
	SetMenu(NULL);

	// create a toolbar
	HWND hProjectToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | CCS_ADJUSTABLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hProjectToolBar);

	// create a toolbar
	HWND hMainToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_MAIN, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | CCS_ADJUSTABLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hMainToolBar);

	// create a toolbar
	HWND hDebugToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_DEBUG, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE | CCS_ADJUSTABLE);
	// add the toolbar to the UI update map
	UIAddToolBar(hDebugToolBar);

	// Initialize the Projects toolbar:
	m_ctrlProjectToolBar.SubclassWindow( hProjectToolBar );
	m_ctrlProjectToolBar.LoadTrueColorToolBar(IDR_TB1_PROJECT);

	// Initialize the Main toolbar:
	m_ctrlMainToolBar.SubclassWindow( hMainToolBar );
	m_ctrlMainToolBar.LoadTrueColorToolBar(IDR_TB1_MAIN);

	// Initialize the Debugging toolbar:
	m_ctrlDebugToolBar.SubclassWindow( hDebugToolBar );
	m_ctrlDebugToolBar.LoadTrueColorToolBar(IDR_TB1_DEBUG);

	TBBUTTONINFO tbbi;
	tbbi.cbSize = sizeof( TBBUTTONINFO );
	tbbi.dwMask = TBIF_SIZE;
	m_ctrlMainToolBar.GetButtonInfo( ID_MAPED_LAYER, &tbbi );

	tbbi.dwMask = TBIF_SIZE | TBIF_STYLE;
	tbbi.fsStyle = BTNS_SEP;
	tbbi.cx = 150;
	m_ctrlMainToolBar.SetButtonInfo( ID_MAPED_LAYER, &tbbi );

	RECT rc;
	m_ctrlMainToolBar.GetRect( ID_MAPED_LAYER, &rc );
	
	rc.top += (rc.bottom-rc.top)/2 - 11;
	rc.left += ::GetSystemMetrics( SM_CXEDGE );
	rc.right -= ::GetSystemMetrics( SM_CXEDGE );
	rc.bottom += 200;

	m_bLayers = TRUE;
	m_ctrlLayers.Create( hMainToolBar, &rc, NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_OWNERDRAWVARIABLE,
		0, ID_MAPED_LAYER );

	m_ctrlLayers.SetFont(AtlGetDefaultGuiFont());

	m_ctrlLayers.LoadStatesBitmap(IDB_COMBO_ICONS, IDB_COMBO_ICONS2);
	m_ctrlLayers.AddStateIcon("1_Visible", 1, 2);
	m_ctrlLayers.AddStateIcon("2_Locked", 3, 2);

	for(int i=0; i<MAX_LAYERS; i++) {
		if(*g_szLayerNames[i] == '\0') break;
		m_ctrlLayers.AddString(g_szLayerNames[i]);
	}
	m_ctrlLayers.SetCurSel(DEFAULT_LAYER);

	// create a rebar to hold both: the command bar and the toolbar
	if(!CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE | CCS_ADJUSTABLE)) {
		ATLTRACE("Failed to create applications rebar\n");
		return -1;      // fail to create
	}	
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hProjectToolBar, NULL, TRUE, 0, FALSE);
	AddSimpleReBarBand(hMainToolBar, NULL, TRUE, 0, FALSE);
	AddSimpleReBarBand(hDebugToolBar, NULL, FALSE, 0, FALSE);

	// create a status bar
	if(!CreateSimpleStatusBar(_T("")) ||
		 !m_wndStatusBar.SubclassWindow(m_hWndStatusBar) ||
		 !m_wndStatusBar.SetPanes(nStatusBarPanes, 
		 sizeof(nStatusBarPanes)/sizeof(int), false) ) {
		
		ATLTRACE("Failed to create status bar\n");
		return -1;      // fail to create
	}

    StatusBar("Loading...", IDI_ICO_WAIT);

	// finally we create the MDI client
	if(!CreateMDIClient()) {
	    StatusBar("Failed to create MDI client", IDI_ICO_ERROR);
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

	UIUpdateMenuItems();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	InitializeDockingFrame();
	InitializeDefaultPanes();

	m_OutputBox.SetSel(0, 0);
	m_OutputBox.SetSel(-1, -1);

	PostMessage(CWM_INITIALIZE);

    StatusBar("Ready", IDI_ICO_OK);

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

	CRect rcDockV(0, 0, 285, 300);
	CRect rcDockH(0, 0, 200, 250);

	CImageList ilIcons;
	ilIcons.Create(IDB_TAB_ICONS, 16, 15, RGB(0,255,0));

	////////////////////////////////////////////////////////////////////
	// Create the content for the docking windows. 
	// If the order is changed, it's needed to also change the resource.h ID

	//////////////////////// First Pane:

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL;
	m_TaskListView.Create(m_hWnd,		rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);
	
	m_DescriptionView.Create(m_hWnd,	rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);

	m_OutputBox.m_pMainFrame = this;
	m_OutputBox.Create(m_hWnd,			rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);

	m_ThumbnailsBox.m_pMainFrame = this;
	m_ThumbnailsBox.Create(m_hWnd,		rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);

	m_LocalVariablesBox.Create(m_hWnd,	rcDefault,	NULL,	dwStyle,	WS_EX_CLIENTEDGE);

	// All CreatePane calls must keep the current order, or otherwise, changes are needed to the
	// definitions between ID_VIEW_PANEFIRST and ID_VIEW_PANELAST in resource.h
	HWND hPane = 
	CreatePane(m_TaskListView,      _T("Things To Do"),			ilIcons.ExtractIcon(6),  rcDockH, NULL,  dockwins::CDockingSide(dockwins::CDockingSide::sRight));
	CreatePane(m_DescriptionView,   _T("Project Description"),	ilIcons.ExtractIcon(3),  rcDockH, hPane, dockwins::CDockingSide(dockwins::CDockingSide::sRight));
	CreatePane(m_OutputBox,         _T("Output Window"),		ilIcons.ExtractIcon(10), rcDockH, hPane, dockwins::CDockingSide(dockwins::CDockingSide::sRight));
	CreatePane(m_ThumbnailsBox,		_T("Thumbnails"),			ilIcons.ExtractIcon(14), rcDockH, hPane, dockwins::CDockingSide(dockwins::CDockingSide::sRight));
	CreatePane(m_LocalVariablesBox,	_T("Local variables"),		ilIcons.ExtractIcon(10), rcDockH, hPane, dockwins::CDockingSide(dockwins::CDockingSide::sRight));

	//////////////////////// Second Pane:

	m_PropertiesView.Create(m_hWnd);

	hPane = 
	CreatePane(m_PropertiesView,	_T("Properties"),			ilIcons.ExtractIcon(1), rcDockV, NULL, dockwins::CDockingSide(dockwins::CDockingSide::sBottom));

	//////////////////////// Third Pane:

	m_GameProject.m_pMainFrame = this;
	m_GameProject.Create(m_hWnd);

	m_Quest.Create(m_hWnd);
	m_SpriteSets.Create(m_hWnd);
	m_SpriteSets.InitDragDrop();
	m_SpriteSets.InsertRootItem("Sprite Sets", "", TVI_ROOT, ICO_PROJECT);
	
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
	//m_stateMgr.Restore();
	UpdateLayout();

	ShowWindow(SW_SHOW);

	Invalidate();
	UpdateWindow();

	return 0;
}

LRESULT CMainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled=FALSE;
	return OnClose();
}

int CMainFrame::OnSaveAll()
{
	int nSaved = 0;
	for(int nChild=0; nChild<m_ChildList.GetSize(); nChild++) {
		if(m_ChildList[nChild]->hasChanged()) {
			if(::SendMessage(m_ChildList[nChild]->m_hWnd, WM_COMMAND, ID_APP_SAVE, 0)) return -1;
			nSaved++;
		}
	}
	return nSaved;
}

void CMainFrame::OnDbgBreak()
{
	Send("break\r\n");
}
void CMainFrame::OnDbgStop()
{
	Send("stop\r\n");
}
void CMainFrame::OnShowNext()
{
	if(CScriptEditorView::ms_pCurrentScript) {
		ScriptFileOpen(CScriptEditorView::ms_pCurrentScript->GetFilePath(), -(CScriptEditorView::ms_nCurrentLine+1));
	} else if(CScriptEditorView::ms_sCurrentFile != "") {
		ScriptFileOpen(CScriptEditorView::ms_sCurrentFile, -(CScriptEditorView::ms_nCurrentLine+1));
	}
}
void CMainFrame::OnContinue()
{
	if(!Connected()) {
		OnRunProject();
		return;
	}

	CScriptEditorView::CleanStep();

	Send("go\r\n");
}
void CMainFrame::OnStepInto()
{
	Send("step\r\n");

}
void CMainFrame::OnStepOut()
{
	Send("out\r\n");
}
void CMainFrame::OnStepOver()
{
	Send("next\r\n");
}

LRESULT CMainFrame::OnDebugCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	static bool bComment = false; // multiline comment
	bool bValid = false;

	LPCSTR szCommand = (LPCSTR)lParam;
	if(*szCommand == '2' || bComment) { // comment (just ignore)
		if(!strncmp(szCommand, "201 ", 4)) { // Step using this ID
			bComment = true;
		} else if(!strcmp(szCommand, ".")) bComment = false; // end of multi line comment.
		bValid = true;
	} else if(*szCommand == '1') { // other commands (just ignore)
		if(!strncmp(szCommand, "101 ", 4)) { // Debugger bye
			Sleep(100); // wait for the connection to stabilize
			OnIdle(); // Foce OnIdle() to update toolbar and stuff
		}
		bValid = true;
	} else if(*szCommand == '4') { // error
		CONSOLE_PRINTF("Debugger error: %s\n", szCommand);
		bValid = true;
	} else if(*szCommand == '5') { // command
		if(!strncmp(szCommand, "500 ", 4)) { // Debugger stop
			bValid = true;
		} else if(!strncmp(szCommand, "501 ", 4)) { // Step using this ID
			bValid = true;
		} else if(!strncmp(szCommand, "502 ", 4)) { // Step using this file
			LPCSTR szLine = strchr(szCommand, ':');
			if(szLine) {
				szLine++;
				while(*szLine == ' ' || *szLine == '\t') szLine++;
				LPCSTR szFile = strchr(szLine, ' ');
				if(szFile) {
					while(*szFile == ' ' || *szFile == '\t') szFile++;
					if(*szLine && *szFile) {
						ScriptFileOpen(szFile, (LPARAM)(-(int)atol(szLine)));
						bValid = true;
					}
				}
			}
		}
	}

	if(!bValid) CONSOLE_PRINTF("Debugger warning: Buggy debug server or wrong version!\n");
	CONSOLE_DEBUG("%s\n", szCommand);

	delete []szCommand;
	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

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

void CMainFrame::OnFileExit()
{
	PostMessage(WM_CLOSE);
}
void CMainFrame::OnScriptFileNew()
{
	// create a new MDI child window
//	CreateNewChildWnd();

	// TODO: add code to initialize document

}

void CMainFrame::OnScriptFileOpen()
{
	static TCHAR szFilter[] = "OL Script files (*.zes;*.inc)|*.zes; *.inc|All Files (*.*)|*.*||";
	CSSFileDialog wndFileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter, m_hWnd);
	
	if(IDOK == wndFileDialog.DoModal()) {
		ScriptFileOpen(wndFileDialog.m_ofn.lpstrFile, 0, (wndFileDialog.m_ofn.Flags&OFN_READONLY)?TRUE:FALSE);
	}
}

void CMainFrame::OnQuestOpen()
{
	static TCHAR szFilter[] = "OL Quest files (*.qss;*.qsz)|*.qss; *.qsz|All Files (*.*)|*.*||";
	CSSFileDialog wndFileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter, m_hWnd);
	
	if(IDOK == wndFileDialog.DoModal()) {
		if(!OnQuestClose()) return;
		FileOpen(wndFileDialog.m_ofn.lpstrFile, 0, (wndFileDialog.m_ofn.Flags&OFN_READONLY)?TRUE:FALSE);
	}
}

/****************************************************************
 * This is just temporary solution to the directory selection   *
 ****************************************************************/
LPSTR strdup_new(LPCSTR str)
{
	if(!str) return NULL;
	if(!*str) return NULL;
	LPSTR ret = new char[strlen(str)+1];
	strcpy(ret, str);
	return ret;
}
int CALLBACK BrowseCallbackProc(HWND hwnd,
        UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch (uMsg) {
		case BFFM_INITIALIZED: {
            /* change the selected folder. */
			LPSTR base, aux;
			base = aux = strdup_new((LPSTR)lpData);
			while(*aux) {
				while(*aux!='\\' && *aux) aux++;
				if(*aux) aux++;
				CHAR tmp = *aux;
				*aux = '\0';
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)base);
				if(tmp) *aux++ = tmp;
			}
			delete []base;
            break;
							   }
        case BFFM_SELCHANGED:
            break;
        default:
            break;
    }
    return(0);
}
BOOL BrowseForFolder(HWND hwnd, // parent window.
        LPCSTR lpszRoot,        // root folder.
        LPCTSTR lpszDefault,    // default selection.
        LPTSTR lpszBuffer,      // return buffer.
        LPCTSTR lpszTitle,      // Title.
        UINT ulFlags)		    // Flags.
{
    LPMALLOC pMalloc;
    LPSHELLFOLDER pDesktop;
    LPITEMIDLIST pidlRoot, pidl;
    BROWSEINFO bi;
    BOOL bRet = FALSE;

    if (SHGetMalloc(&pMalloc) != NOERROR) return(FALSE);

    /* gets the ITEMIDLIST for the desktop folder. */
    if (SHGetDesktopFolder(&pDesktop) == NOERROR) {
          WCHAR wszPath[_MAX_PATH];
          ULONG cchEaten;
          ULONG dwAttr;

          /* converts the root path into unicode. */
          MultiByteToWideChar(CP_OEMCP, 0,
              lpszRoot, -1, wszPath, _MAX_PATH);

          /* translates the root path into ITEMIDLIST. */
          if(pDesktop->ParseDisplayName(NULL, NULL, wszPath, &cchEaten, &pidlRoot, &dwAttr) != NOERROR)
              pidlRoot = NULL;

          pDesktop->Release();
    }

    /* fills the structure. */
    bi.hwndOwner = hwnd;
    bi.pidlRoot = pidlRoot;
    bi.pszDisplayName = lpszBuffer;
    bi.lpszTitle = lpszTitle;
    bi.ulFlags = ulFlags;
	bi.lpfn = (BFFCALLBACK)BrowseCallbackProc;
    bi.lParam = (LPARAM)lpszDefault;
	bi.iImage = 0;

    /* invokes the dialog. */
    if ((pidl = SHBrowseForFolder(&bi)) != NULL) {
        bRet = SHGetPathFromIDList(pidl, lpszBuffer);
		strcat(lpszBuffer, "\\");
        pMalloc->Free(pidl);
    }

    /* clean up. */
    if (pidlRoot)
        pMalloc->Free(pidlRoot);
    pMalloc->Release();

    return(bRet);
}
/***************************************************************/

void CMainFrame::OnProjectOpen()
{
	UIEnableToolbar(FALSE);
	// Update all the toolbar items
	UIUpdateToolBar();
	UpdateWindow();

	if(g_sHomeDir.IsEmpty()) {
		g_sHomeDir = "C:\\qd\\Quest Designer 2.0.4\\";
	}

	TCHAR taux[_MAX_PATH];
	if(!BrowseForFolder(m_hWnd, 0, g_sHomeDir, taux, "Please select the folder where the game files are:", 0)) {
		return;
	}

	if(!OnProjectClose()) return; // closes the project (and the world if it's open)

    StatusBar("Loading...", IDI_ICO_WAIT);

	::SendMessage(m_GameProject, WM_SETREDRAW, FALSE, 0);
	g_sHomeDir = taux;

	SetCursor(LoadCursor(NULL, IDC_WAIT));
	if(!m_pOLKernel->LoadProject(g_sHomeDir)) {
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return;
	}
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	::SendMessage(m_GameProject, WM_SETREDRAW, TRUE, 0);
	::RedrawWindow(m_GameProject, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
	m_SpriteSets.PopulateTree(g_sHomeDir + "Sprite Sets");

	if(m_ThumbnailsBox.IsWindow()) {
		DWORD dwInitTicks = GetTickCount();
		CONSOLE_DEBUG("Building thumbnails...\n");
		m_ThumbnailsBox.DoFileOpen("");
		CONSOLE_DEBUG("Done! (%d milliseconds)\n", GetTickCount()-dwInitTicks);
	}

	m_bProjectLoaded = true;

	UIEnableToolbar(TRUE);

    StatusBar("Ready", IDI_ICO_OK);
}

void CMainFrame::OnViewToolBar()
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_APP_TOOLBAR, bVisible);
	UpdateLayout();
}

void CMainFrame::OnViewStatusBar()
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_APP_STATUS_BAR, bVisible);
	UpdateLayout();
}

HWND CMainFrame::GetOldFocus(_child_type ChildType)
{
	ASSERT(ChildType<10); // there are 10 elements in the m_ahWnd array.
	return m_ahWnd[ChildType];
}
void CMainFrame::SetOldFocus(_child_type ChildType, HWND hWnd)
{
	ASSERT(ChildType<10);
	m_ahWnd[0] = hWnd;
	m_ahWnd[ChildType] = hWnd;
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
CChildFrame* CMainFrame::FindChild(_child_type ChildType)
{
	return FindNextChild(ChildType, NULL);
}

CChildFrame* CMainFrame::FindNextChild(_child_type ChildType, CChildFrame* pLastChildFrame)
{
	int cnt = 0;
	CChildFrame *pChildFrame;
	bool bLooped = (pLastChildFrame == NULL);
	for(int l=0; l<2; l++) {
		for(int i=0; i<m_ChildList.GetSize(); i++) {
			pChildFrame = m_ChildList[i];
			if(pLastChildFrame == pChildFrame) {
				pLastChildFrame = NULL;
				bLooped = true;
			} else if(pChildFrame->m_ChildType == ChildType && bLooped) {
				return pChildFrame;
			}
		}
		bLooped = true;
	}
	return NULL;
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

void CMainFrame::OnSound()
{
	m_bAllowSounds = !m_bAllowSounds;
	OnIdle(); // Force idle processing to update the toolbar.

	if(!m_bAllowSounds) {
		for(int i=0; i<10; i++) m_ahWnd[i] = 0; // reset focuses

		ISoundManager *pSoundManager = CProjectFactory::Interface()->GetSoundManager();
		if(pSoundManager) pSoundManager->SwitchMusic(NULL, 0, false);
	}
}
void CMainFrame::OnAnim()
{
	m_bAllowAnimations = !m_bAllowAnimations;
	OnIdle(); // Force idle processing to update the toolbar.
}
void CMainFrame::OnParallax()
{
	m_bAllowParallax = !m_bAllowParallax;
	OnIdle(); // Force idle processing to update the toolbar.
}

void CMainFrame::OnViewWorldEditor()
{
	static CChildFrame *pChildFrame = NULL;
	if(CountChilds(tWorldEditor)) {
		pChildFrame = FindNextChild(tWorldEditor, pChildFrame);
		ATLASSERT(pChildFrame);
		pChildFrame->SetFocus();
	}
	OnIdle(); // Force idle processing to update the toolbar.
}
void CMainFrame::OnViewMapEditor()
{
	static CChildFrame *pChildFrame = NULL;
	if(CountChilds(tMapEditor)) {
		pChildFrame = FindNextChild(tMapEditor, pChildFrame);
		ATLASSERT(pChildFrame);
		pChildFrame->SetFocus();
	}
	OnIdle(); // Force idle processing to update the toolbar.
}
void CMainFrame::OnViewSpriteEditor()
{
	static CChildFrame *pChildFrame = NULL;
	if(CountChilds(tSpriteEditor)) {
		pChildFrame = FindNextChild(tSpriteEditor, pChildFrame);
		ATLASSERT(pChildFrame);
		pChildFrame->SetFocus();
	}
	OnIdle(); // Force idle processing to update the toolbar.
}
void CMainFrame::OnViewScriptEditor()
{
	static CChildFrame *pChildFrame = NULL;
	if(CountChilds(tScriptEditor)) {
		pChildFrame = FindNextChild(tScriptEditor, pChildFrame);
		ATLASSERT(pChildFrame);
		pChildFrame->SetFocus();
	}
	OnIdle(); // Force idle processing to update the toolbar.
}

void CMainFrame::OnSaveAs()
{
}
void CMainFrame::OnSave()
{
}
int CMainFrame::OnClose()
{
	//m_stateMgr.Store();
	return OnProjectClose();
}
int CMainFrame::OnQuestClose()
{
	if(!CountChilds(tWorldEditor)) return 1;

	CChildFrame *pChildFrame;
	bool bModified = false;
	int nChild = 0;

	// Find out if there are any files modified:
	for(nChild=0; nChild<m_ChildList.GetSize(); nChild++) {
		pChildFrame = m_ChildList[nChild];
		if(pChildFrame->m_ChildType == tWorldEditor || pChildFrame->m_ChildType == tMapEditor) {
			if(pChildFrame->hasChanged()) { 
				bModified = true; 
				break; 
			}
		}
	}

	// Check if anything has been modified. The world must keep the whole status of the Quest (world and maps)
	// if a map is modified, it must know about it. (scripts and sprite sheets are part of the game set, not the quest)
	if(!bModified) {
		int nChoice = MessageBox("Are you sure you want to close the Quest?", QD_MSG_TITLE, MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION);
		if(nChoice != IDYES) return 0;
	}

	bool bModifiedAux = false;
	for(nChild = 0; m_ChildList.GetSize(); nChild++) {
		if(nChild >= m_ChildList.GetSize()) {
			bModified = false;
			bModifiedAux = false;
			nChild = 0;
		}
		pChildFrame = m_ChildList[nChild];

		if(pChildFrame->m_ChildType == tWorldEditor && CountChilds(tMapEditor)) {
			bModifiedAux = bModified;
			continue; // first close all maps
		}

		if(pChildFrame->hasChanged()) {
			nChild = 0;
			bModifiedAux = true;
			pChildFrame->SetFocus();
			if(::SendMessage(pChildFrame->m_hWnd, WM_CLOSE, 0, 0)) return 1;
			if(pChildFrame->m_ChildType == tWorldEditor) break; // only close up to the Quest closing
		} else if(!bModified && !bModifiedAux) {
			nChild = 0;
			if(::SendMessage(pChildFrame->m_hWnd, WM_CLOSE, 0, 0)) return 1;
			if(pChildFrame->m_ChildType == tWorldEditor) break; // only close up to the Quest closing
		}
	}

	if(!m_pOLKernel->CloseWorld()) return 0;
	m_bQuestLoaded = false;
	OnIdle(); // Force idle processing to update the toolbar.
    StatusBar("Quest closed.", IDI_ICO_OK);
	return 1;
}
void CMainFrame::OnQuestNew()
{
}
int CMainFrame::OnProjectClose()
{
	if(!OnQuestClose()) return 0;

	bool bModified = true;
	int nChild = 0;
	CChildFrame *pChildFrame;
	
	while(m_ChildList.GetSize()) {
		if(nChild >= m_ChildList.GetSize()) {
			bModified = false;
			nChild = 0;
		}
		pChildFrame = m_ChildList[nChild++];
		if(pChildFrame->hasChanged()) {
			nChild = 0;
			bModified = true;
			pChildFrame->SetFocus();
			if(::SendMessage(pChildFrame->m_hWnd, WM_CLOSE, 0, 0)) return 1; //FIXME: only close files opened for the project
		} else if(bModified == false) {
			nChild = 0;
			if(::SendMessage(pChildFrame->m_hWnd, WM_CLOSE, 0, 0)) return 1; //FIXME: only close files opened for the project
		}
	}

	return Close();
}
void CMainFrame::OnProjectNew()
{
}

void CMainFrame::OnAppHelp()
{
	ShowHelp(m_hWnd);
}
void CMainFrame::OnAppAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}
void CMainFrame::OnAppConfig()
{
	CPropertiesDlg dlg(0);
	int result = dlg.DoModal();
}
void CMainFrame::OnWindowCascade()
{
	MDICascade();
}
void CMainFrame::OnWindowTile()
{
	MDITile();
}
void CMainFrame::OnWindowArrangeIcons()
{
	MDIIconArrange();
}
HRESULT CMainFrame::OnStopBuild()
{
	m_pProjectFactory->CancelBuild();
	OnIdle(); // Force idle processing to update the toolbar.
	return S_OK;
}

HRESULT CMainFrame::OnRunProject()
{
	return E_FAIL;
}
HRESULT CMainFrame::OnDebugProject()
{
	UIEnableToolbar(FALSE);
	// Update all the toolbar items
	UIUpdateToolBar();
	UpdateWindow();

	if(Connected()) return E_FAIL;

    StatusBar("Connecting debugger...", IDI_ICO_WAIT);
	OnIdle(); // Force idle processing to update the toolbar.

	// Try connecting to an already open "Open Legends" with an active debugger
	SOCKET s = Connect("localhost", 7683, Dispatch, Request);
	if(s == INVALID_SOCKET) {
		if(OnBuildProject(false) == 1) {
			CONSOLE_PRINTF("Debugger error: Couldn't build the quest!\n");
		}

		// Get the Open Legends's exe path and filename:
		char szFilename[MAX_PATH];
		GetModuleFileName(_Module.GetModuleInstance(), szFilename, MAX_PATH);

		LPSTR aux = strchr(szFilename, '\0');
		while(aux != szFilename && *aux != '\\') aux--;
		if(*aux == '\\') aux++;
		*aux = '\0';
		strcat(szFilename, "OpenLegends.exe");

		if(g_sHomeDir=="" || g_sQuestFile=="") {
			CONSOLE_PRINTF("Debugger error: No quest, try openning a quest first!\n");
		} else {
			// Convert the paths to valid parameters:
			char szGame[MAX_PATH];
			char szQuest[MAX_PATH];
			PathToParam(g_sHomeDir, szGame, sizeof(szGame));
			PathToParam(g_sQuestFile, szQuest, sizeof(szQuest));

			// Try to load Open Legends:
			if(_spawnl( _P_NOWAIT, szFilename, "1", "-d", szGame, szQuest, NULL) == -1) {
				MessageBox("Couldn't launch "OL_NAME, QD_MSG_TITLE);
			} else {
				// wait for connection with the debugger.
				for(int i=0; i<3; i++) {
					s = Connect("localhost", 7683, Dispatch, Request);
					if(s != INVALID_SOCKET) break;
					Sleep(100); // wait for the connection to stabilize
				}
				if(s == INVALID_SOCKET) {
					CONSOLE_PRINTF("Debugger error: Couldn't connect with "OL_NAME"'s debugger!");
					StatusBar("Couldn't connect with "OL_NAME"'s debugger!", IDI_ICO_ERROR);
				}
			}
		}
	}
    StatusBar("Ready", IDI_ICO_OK);

	return S_OK;
}
HRESULT CMainFrame::OnBuildProject(bool bForce)
{
//	m_InfoFrame.DisplayTab(m_OutputBox.m_hWnd);
	int nSaved = (int)OnSaveAll();
	if(nSaved<0) return E_FAIL;
	if(nSaved > 0 || bForce) m_pProjectFactory->StartBuild();
	OnIdle(); // Force idle processing to update the toolbar.
	return S_OK;
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
	if((pMapGroupI = m_pOLKernel->FindMapGroup(Point.x, Point.y))==NULL) {
		if((pMapGroupI = m_pOLKernel->BuildMapGroup(Point.x, Point.y, 1, 1))==NULL) return 0;
		else pMapGroupI->Load();
	}

	// Searching for an open child with the same file:
	LPCSTR szTitle = pMapGroupI->GetMapGroupID();

	CMapEditorFrame *pChild = new CMapEditorFrame(this);
	DWORD dwStyle = CountChilds()?0:WS_MAXIMIZE;
	HWND hChildWnd = pChild->CreateEx(m_hWndClient, NULL, NULL, dwStyle);
	//pChild->m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDIMAPED));
	ATLASSERT(::IsWindow(hChildWnd));

	// get the child's 'view' (actually child's client control)
	CMapEditorView *pView = pChild->GetView();
	if(!pView->DoFileOpen(NULL, szTitle, (WPARAM)pMapGroupI)) {
		// kill the failed window (will delete itself)
		::PostMessage(pChild->m_hWnd, WM_CLOSE, 0, 0);
		return 0;
	}
	return 1;
}

int CMainFrame::MapFileOpen(CPoint &Point)
{
	UIEnableToolbar(FALSE);
	// Update all the toolbar items
	UIUpdateToolBar();
	UpdateWindow();

	CMapGroup *pMapGroupI;
	if((pMapGroupI = m_pOLKernel->FindMapGroup(Point.x, Point.y))==NULL) return MapCreate(Point);

	LPCSTR szTitle = pMapGroupI->GetMapGroupID();

	// Searching for an open child with the same file:
	if(Select(szTitle, 0)) return 1;

	CBString strTmp;
	strTmp.Format("Loading (%d, %d)...", Point.x, Point.y);
    StatusBar(strTmp, IDI_ICO_WAIT);
	SetCursor(LoadCursor(NULL, IDC_WAIT));
	if(!pMapGroupI->Load()) {
	    StatusBar("Couldn't load the map group! (1)", IDI_ICO_ERROR);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return 0;
	}
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	CMapEditorFrame *pChild = new CMapEditorFrame(this);
	DWORD dwStyle = CountChilds()?0:WS_MAXIMIZE;
	HWND hChildWnd = pChild->CreateEx(m_hWndClient, NULL, NULL, dwStyle);
	//pChild->m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDIMAPED));
	ATLASSERT(::IsWindow(hChildWnd));

	// get the child's 'view' (actually child's client control)
	CMapEditorView *pView = pChild->GetView();
	if(!pView->DoFileOpen(NULL, szTitle, (WPARAM)pMapGroupI)) {
		// kill the failed window (will delete itself)
		::PostMessage(pChild->m_hWnd,WM_CLOSE,0,0);
	    StatusBar("Couldn't load the map group! (2)", IDI_ICO_ERROR);
		return 0;
	}
    StatusBar("Ready", IDI_ICO_OK);

	return 1;
}

int CMainFrame::SptShtFileOpen(CSpriteSheet *pSpriteSheet, LPCSTR lpszSprite)
{
	char szTitle[MAX_PATH];
	pSpriteSheet->GetName(szTitle, sizeof(szTitle));

	char szFilePath[MAX_PATH];
	pSpriteSheet->GetFilePath(szFilePath, sizeof(szFilePath));

	// Searching for an open child with the same file:
	if(Select(szFilePath, (LPARAM)lpszSprite)) return 1;

	CSptShtEditorFrame *pChild = new CSptShtEditorFrame(this);
	DWORD dwStyle = CountChilds()?0:WS_MAXIMIZE;
	HWND hChildWnd = pChild->CreateEx(m_hWndClient, NULL, NULL, dwStyle);
	//pChild->m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDISPTSHTED));
	ATLASSERT(::IsWindow(hChildWnd));

	// get the child's 'view' (actually child's client control)
	CSptShtEditorView *pView = pChild->GetView();
	if(!pView->DoFileOpen(szFilePath, szTitle, (WPARAM)pSpriteSheet, (LPARAM)lpszSprite)) {
		// kill the failed window (will delete itself)
		::PostMessage(pChild->m_hWnd, WM_CLOSE, 0, 0);
		return 0;
	}
	return 1;
}

void CMainFrame::StatusBar(LPCSTR szMessage, UINT Icon)
{
	m_wndStatusBar.SetPaneText(0, szMessage);
	HICON hIcon = AtlLoadIconImage(Icon, LR_DEFAULTCOLOR);
	m_wndStatusBar.SetPaneIcon(ID_ICON_PANE, hIcon);
	DeleteObject(hIcon);
}

int CMainFrame::Close()
{
	if(!m_pOLKernel->CloseProject()) {
		int nChoice = MessageBox("Save changes to the Game Set?", QD_MSG_TITLE, MB_YESNOCANCEL|MB_ICONWARNING);
		if(nChoice == IDYES) {
			if(!m_pOLKernel->SaveProject()) {
				MessageBox("Couldn't save!", QD_MSG_TITLE, MB_OK|MB_ICONERROR);
				return 0;
			}
		} else if(nChoice == IDCANCEL) {
			return 0;
		}
		m_pOLKernel->CloseProject(true);
	} 
	m_bProjectLoaded = false;
	OnIdle(); // Force idle processing to update the toolbar.
    StatusBar("Quest closed.", IDI_ICO_OK);
	return 1;
}

int CMainFrame::FileOpen(LPCTSTR szFilename, LPARAM lParam, BOOL bReadOnly)
{
	UIEnableToolbar(FALSE);
	// Update all the toolbar items
	UIUpdateToolBar();
	UpdateWindow();

	StatusBar("Loading...", IDI_ICO_WAIT);
	SetCursor(LoadCursor(NULL, IDC_WAIT));
	g_sQuestFile = szFilename;
	if(!m_pOLKernel->LoadWorld(g_sQuestFile)) {
	    StatusBar("Couldn't load quest!", IDI_ICO_ERROR);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return 0;
	}
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	m_bQuestLoaded = true;

	if(Select(_T("World Editor"), 0)) {
	    StatusBar("Couldn't open the World editor window!", IDI_ICO_ERROR);
		return 0;
	}

	CWorldEditorFrame *pChild = new CWorldEditorFrame(this);
	DWORD dwStyle = CountChilds()?0:WS_MAXIMIZE;
	HWND hChildWnd = pChild->CreateEx(m_hWndClient, NULL, NULL, dwStyle);
	//pChild->m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDIWORLDED));
	ATLASSERT(::IsWindow(hChildWnd));

	OnIdle(); // Force idle processing to update the toolbar.

    StatusBar("Ready", IDI_ICO_OK);
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
	//pChild->m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MDISCRIPTED));
	ATLASSERT(::IsWindow(hChildWnd));

	// get the child's 'view' (actually child's client control)
	CScriptEditorView *pView = pChild->GetView();

	// open the requested file
	if(!pView->DoFileOpen(szFilename, szTitle)) {
		// kill the failed window (will delete itself)
		::PostMessage(pChild->m_hWnd, WM_CLOSE, 0, 0);
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

	if( ActiveChildType!=tMapEditor ) {
		if(m_PaneWindows.size() >= (ID_VIEW_PANELAST-ID_VIEW_PANEFIRST)) {
			CTabbedDockingWindow *pThumbnails = m_PaneWindows[ID_PANE_THUMBNAILS-ID_VIEW_PANEFIRST];
			if(pThumbnails->IsWindowVisible() == TRUE) {
				pThumbnails->Toggle();
			}
		}
		m_ctrlLayers.EnableWindow(FALSE);
		m_bLayers = FALSE;
	}

	if( ActiveChildType!=tScriptEditor ) {
		if(m_PaneWindows.size() >= (ID_VIEW_PANELAST-ID_VIEW_PANEFIRST)) {
			CTabbedDockingWindow *pDebugger = m_PaneWindows[ID_PANE_DEBUGGER-ID_VIEW_PANEFIRST];
			if(pDebugger->IsWindowVisible() == TRUE) {
				pDebugger->Toggle();
			}
		}
	}

	ASSERT(!(m_bQuestLoaded && !m_bProjectLoaded));

	if(m_pProjectFactory->isBuilding()) {
		ASSERT(m_bQuestLoaded);
		UISetCheck(ID_APP_SOUND, FALSE);
		UISetCheck(ID_APP_ANIM, FALSE);
		UISetCheck(ID_APP_PARALLAX, FALSE);

		UIEnable(ID_PROJECT_NEW, FALSE);
		UIEnable(ID_PROJECT_OPEN, FALSE);
		UIEnable(ID_PROJECT_CLOSE, FALSE);

		UIEnable(ID_QUEST_NEW, FALSE);
		UIEnable(ID_QUEST_OPEN, FALSE);
		UIEnable(ID_QUEST_CLOSE, FALSE);

		UIEnable(ID_APP_RELOAD, FALSE);
		UIEnable(ID_APP_CLOSE, FALSE);
		UIEnable(ID_APP_SAVE, FALSE);
		UIEnable(ID_APP_SAVE_ALL, FALSE);
		UIEnable(ID_APP_SAVE_AS, FALSE);

		UIEnable(ID_APP_ADDNEWITEM, FALSE);
		UIEnable(ID_APP_ADDEXISTINGITEM, FALSE);

		UIEnable(ID_APP_PRINT, FALSE);
		UIEnable(ID_APP_BUILD, FALSE);
		UIEnable(ID_APP_REBUILD, FALSE);
		UIEnable(ID_APP_CLEANBUILD, FALSE);
		UIEnable(ID_APP_STOPBUILD, TRUE);

		UIEnable(ID_APP_PREFERENCES, FALSE);

		UIEnable(ID_APP_WORLDED, FALSE);
		UIEnable(ID_APP_MAPED, FALSE);
		UIEnable(ID_APP_SPTSHTED, FALSE);
		UIEnable(ID_APP_SCRIPTED, FALSE);

		UIEnable(ID_APP_SOUND, FALSE);
		UIEnable(ID_APP_ANIM, FALSE);
		UIEnable(ID_APP_PARALLAX, FALSE);

		// Debugging toolbar:
		UIEnable(ID_DBG_DEBUG, FALSE);
		UIEnable(ID_DBG_RUN, FALSE);
		UIEnable(ID_DBG_PAUSE, FALSE);
		UIEnable(ID_DBG_STOP, FALSE);
		UIEnable(ID_DBG_NEXT, FALSE);
		UIEnable(ID_DBG_STEPIN, FALSE);
		UIEnable(ID_DBG_STEPOVER, FALSE);
		UIEnable(ID_DBG_STEPOUT, FALSE);
		UIEnable(ID_DBG_PROFILER, FALSE);
		UIEnable(ID_DBG_BREAKPOINT, FALSE);
		UIEnable(ID_DBG_CONTINUE, FALSE);
	} else {
		CChildFrame *pChild = FindChild(tWorldEditor);
		UISetCheck(ID_APP_SOUND, m_bQuestLoaded && m_bAllowSounds);
		UISetCheck(ID_APP_ANIM, m_bQuestLoaded && m_bAllowAnimations);
		UISetCheck(ID_APP_PARALLAX, m_bQuestLoaded && m_bAllowParallax);

		UIEnable(ID_PROJECT_NEW, TRUE);
		UIEnable(ID_PROJECT_OPEN, TRUE);
		UIEnable(ID_PROJECT_CLOSE, m_bProjectLoaded);

		UIEnable(ID_QUEST_NEW, m_bProjectLoaded);
		UIEnable(ID_QUEST_OPEN, m_bProjectLoaded);
		UIEnable(ID_QUEST_CLOSE, m_bQuestLoaded);

		UIEnable(ID_APP_RELOAD, FALSE);
		UIEnable(ID_APP_CLOSE, CountChilds());
		UIEnable(ID_APP_SAVE, FALSE);
		UIEnable(ID_APP_SAVE_ALL, (nChanges>1));
		UIEnable(ID_APP_SAVE_AS, FALSE);

		UIEnable(ID_APP_ADDNEWITEM, m_bQuestLoaded);
		UIEnable(ID_APP_ADDEXISTINGITEM, m_bQuestLoaded);

		UIEnable(ID_APP_BUILD, m_bQuestLoaded);
		UIEnable(ID_APP_REBUILD, m_bQuestLoaded);
		UIEnable(ID_APP_CLEANBUILD, m_bQuestLoaded);
		UIEnable(ID_APP_STOPBUILD, FALSE);

		UIEnable(ID_APP_PREFERENCES, TRUE);

		UIEnable(ID_APP_WORLDED, m_bQuestLoaded);
		UIEnable(ID_APP_MAPED, m_bQuestLoaded);
		UIEnable(ID_APP_SPTSHTED, m_bQuestLoaded);
		UIEnable(ID_APP_SCRIPTED, m_bQuestLoaded);

		UIEnable(ID_APP_SOUND, m_bQuestLoaded);
		UIEnable(ID_APP_ANIM, m_bQuestLoaded);
		UIEnable(ID_APP_PARALLAX, m_bQuestLoaded);

		// Debugging toolbar:
		UIEnable(ID_DBG_DEBUG, !Connected());
		UIEnable(ID_DBG_RUN, !Connected() && m_bQuestLoaded);
		UIEnable(ID_DBG_PAUSE, Connected());
		UIEnable(ID_DBG_STOP, Connected());
		UIEnable(ID_DBG_NEXT, Connected());
		UIEnable(ID_DBG_STEPIN, Connected());
		UIEnable(ID_DBG_STEPOVER, Connected());
		UIEnable(ID_DBG_STEPOUT, Connected());
		UIEnable(ID_DBG_PROFILER, Connected());
		UIEnable(ID_DBG_BREAKPOINT, Connected() && (ActiveChildType!=tScriptEditor));
		UIEnable(ID_DBG_CONTINUE, Connected());
	}
	if( CScriptEditorView::ms_pCurrentScript == NULL &&
		CScriptEditorView::ms_sCurrentFile == "" ) {
		UIEnable(ID_DBG_NEXT, FALSE);
		UIEnable(ID_DBG_STEPIN, FALSE);
		UIEnable(ID_DBG_STEPOVER, FALSE);
		UIEnable(ID_DBG_BREAKPOINT, FALSE);
		UIEnable(ID_DBG_STEPOUT, FALSE);
		UIEnable(ID_DBG_CONTINUE, FALSE);
	} else {
		UIEnable(ID_DBG_PAUSE, FALSE);
	}

	if(ActiveChildType!=tScriptEditor) {
		UIEnable(ID_APP_PRINT, FALSE);
		UIEnable(ID_APP_PRINT_SETUP, FALSE);

		UIEnable(ID_SCRIPTED_CODELIST, FALSE );
		UIEnable(ID_SCRIPTED_CODETIP1, FALSE );
		UIEnable(ID_SCRIPTED_CODETIP2, FALSE );

		UIEnable(ID_SCRIPTED_TAB, FALSE);
		UIEnable(ID_SCRIPTED_UNTAB, FALSE);

		UIEnable(ID_SCRIPTED_BEAUTIFY, FALSE);
		
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
	if(ActiveChildType!=tMapEditor) {
		UIEnable(ID_MAPED_MERGE, FALSE);
	}

	if(ActiveChildType==tAny) {
		UIEnable(ID_UNDO, FALSE);
		UIEnable(ID_REDO, FALSE);	
		UIEnable(ID_CUT, FALSE);
		UIEnable(ID_COPY, FALSE);
		UIEnable(ID_PASTE, FALSE);
		UIEnable(ID_ERASE, FALSE);
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
	m_ctrlLayers.EnableWindow(bEnable);
	m_bLayers = bEnable;
}
