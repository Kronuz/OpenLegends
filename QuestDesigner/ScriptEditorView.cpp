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
// ScriptEditorView.cpp : implementation of the CScriptEditorView class
//

#include "stdafx.h"

#include "MainFrm.h"
#include "ScriptEditorView.h"
#include "ScriptEditorFrm.h"
#include <ssfiledialog.h>

CScriptEditorView::CScriptEditorView(CScriptEditorFrame *pParentFrame) :
	CChildView(pParentFrame)
{
}

// Called to translate window messages before they are dispatched 
BOOL CScriptEditorView::PreTranslateMessage(MSG *pMsg)
{
	return FALSE;
}
// Called to do idle processing
BOOL CScriptEditorView::OnIdle ()
{
	// Update all the menu items
	UIUpdateMenuItems();
	
	// Update position display in the status bar
	UIUpdateStatusBar();

	bool bModified = (IsModified()?1:0);
	if(bModified != m_bModified) {
		m_bModified = bModified;
		if(m_bModified) {
			m_pParentFrame->SetTabText(m_sTitle+"*");
		} else {
			m_pParentFrame->SetTabText(m_sTitle);
		}
	}

	return FALSE;
}
// Called to clean up after window is destroyed (called when WM_NCDESTROY is sent)
void CScriptEditorView::OnFinalMessage(HWND /*hWnd*/)
{
/*
	// Update position display in the status bar (remove it)
	CMainFrame * pMainFrm = m_pParentFrame->GetMainFrame ();
	CMultiPaneStatusBarCtrl * pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl ();
	pStatusBar->SetPaneText (ID_POSITION_PANE, _T(""));
*/
	delete this;
}

LRESULT CScriptEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// Let the CodeMax control be created
	LONG lRet = DefWindowProc();
	// Set the current language and enable syntax highlighting
	CME_VERIFY(SetLanguage(CMLANG_ZES));
	CME_VERIFY(EnableColorSyntax());
	CME_VERIFY(SetTabSize(3));

	return lRet;
}
LRESULT CScriptEditorView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CScriptEditorView::OnFileReload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString sMessage;
	sMessage.LoadString(IDS_WARNING_RELOAD);

	if(IDYES==MessageBox(sMessage, "Quest Designer - Script Editor", MB_YESNO)) {
		DoReload();
	}
	return 0;
}

// Save handlers
LRESULT CScriptEditorView::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!m_sFilePath.IsEmpty()) {
		// save the file
		DoFileSave(m_sFilePath);
	} else {
		DoFileSaveAs();
	}
	return 0;
}
LRESULT CScriptEditorView::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoFileSaveAs();
	return 0;
}

// Indent/Unindent the current selection
LRESULT CScriptEditorView::OnEditTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	IndentSelection();
	return 0;
}
LRESULT CScriptEditorView::OnEditUntab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	UnIndentSelection();
	return 0;
}

// Find next/previous search match in the buffer
LRESULT CScriptEditorView::OnEditFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	FindNext();
	return 0;
}
LRESULT CScriptEditorView::OnEditFindPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	FindPrev();
	return 0;
}

// Searches the buffer using the current selection as the search parameter
LRESULT CScriptEditorView::OnEditFindSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	FindNextWord();
	return 0;
}
// Jump to the desired line
LRESULT CScriptEditorView::OnEditGotoLine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// The zero-based line number to jump to.  If -1, the user is prompted.
	GotoLine(-1);
	return 0;
}

// Find the current brace's 'sibling'
LRESULT CScriptEditorView::OnEditMatchBrace(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GotoMatchBrace();
	return 0;
}

// Set buffer to read-only
LRESULT CScriptEditorView::OnEditReadOnly(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// toggle read only
	BOOL bReadOnly = IsReadOnly();
	SetReadOnly(!bReadOnly);
	return 0;
}

// Advanced edit commands
LRESULT CScriptEditorView::OnEditUppercase(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	UpperCaseSelection();	
	return 0;
}
LRESULT CScriptEditorView::OnEditLowercase(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LowerCaseSelection();
	return 0;
}
LRESULT CScriptEditorView::OnEditTabify(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TabifySelection();
	return 0;
}
LRESULT CScriptEditorView::OnEditUntabify(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	UnTabifySelection();
	return 0;
}
LRESULT CScriptEditorView::OnShowWhiteSpace(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// toggle white space
	ToggleWhiteSpaceDisplay();
	return 0;
}

// Bookmark functions
LRESULT CScriptEditorView::OnEditToggleBookmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BookmarkToggle();
	return 0;
}
LRESULT CScriptEditorView::OnEditGotoNextBookmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BookmarkPrev();
	return 0;
}
LRESULT CScriptEditorView::OnUpdateEditGotoPrevBookmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BookmarkNext();
	return 0;
}
LRESULT CScriptEditorView::OnEditClearAllBookmarks(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BookmarkClearAll();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Open a file
BOOL CScriptEditorView::DoFileOpen(LPCTSTR lpszPathName, LPCTSTR lpszTitle /*=_T("Untitled")*/ ) 
{
	ATLTRACE(_T("Entering : CCodeMaxWTLSampleView::DoFileOpen(%s )\n" ), lpszPathName );
	
	// open the requested file
	CME_CODE lRet = OpenFile(lpszPathName);

	if(CME_FAILED(lRet)) {
		ATLTRACE(_T("Error: Failed to load file: %s\n" ), lpszPathName );

		// show the error message
		CString sMessage;
		sMessage.Format(_T("Failed to load: %s\n\n" ), lpszPathName );
		//ShowLastError ( sMessage, ::GetLastError());
		
		return FALSE;
	}

	// Save file name for later
	m_sFilePath = lpszPathName;

	// Save the tittle for later
	m_sTitle = lpszTitle;

	m_pParentFrame->m_sChildName = m_sFilePath;
	m_pParentFrame->SetTitle(m_sFilePath);
	m_pParentFrame->SetTabText(m_sTitle);

	return TRUE;
}
// Save a file
BOOL CScriptEditorView::DoFileSave ( const CString & sPathName ) 
{
	// open the requested file
	CME_CODE lRet = SaveFile(sPathName, FALSE /*bClearUndo*/);

	if(CME_FAILED(lRet)) {
		ATLTRACE(_T("Error: Failed to save: %s\n"), sPathName);

		// show the error message
		CString sMessage;
		sMessage.Format(_T("Error: Failed to save: %s\n\n"), sPathName);
//		MessageBox(sMessage, ::GetLastError());

		return FALSE;
	}

	// Save file name for later
	m_sFilePath = sPathName;	

	return TRUE;
}
BOOL CScriptEditorView::DoFileSaveAs()
{
	static TCHAR szFilter[] = "OZ Script files (*.zes;*.inc)|*.zes; *.inc|All Files (*.*)|*.*||";
	CSSFileDialog wndFileDialog(FALSE, NULL, NULL, OFN_HIDEREADONLY, szFilter, m_hWnd);
	
	if(IDOK == wndFileDialog.DoModal()) {
		if(!DoFileSave (wndFileDialog.m_ofn.lpstrFile)) {
			return FALSE;
		}
		// save the title
		m_sTitle = wndFileDialog.m_szFileTitle;
		// save the title
		m_sFilePath = wndFileDialog.m_szFileName;

		m_pParentFrame->m_sChildName = m_sFilePath;
		m_pParentFrame->SetTitle(m_sFilePath);
		m_pParentFrame->SetTabText(m_sTitle);
		
		return TRUE;
	}
	return FALSE;
}

// Reload a file	
BOOL CScriptEditorView::DoReload ()
{
	// simply re-open the file we opend
	CME_CODE lRet = OpenFile(m_sFilePath);

	if(CME_FAILED(lRet)) {		
		ATLTRACE(_T ( "Error: Failed to reload file: %s\n" ), m_sFilePath);
		return FALSE;
	}
	return TRUE;
}
void CScriptEditorView::UIUpdateStatusBar()
{
	CMainFrame * pMainFrm = m_pParentFrame->GetMainFrame();
	CMultiPaneStatusBarCtrl * pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl();

	// display position in buffer on the status bar
	CM_RANGE cmRange;
	if(CME_SUCCEEDED(GetSel(&cmRange, FALSE))) {
		CString sText;
		sText.Format(_T("Ln %d, Col %d"), cmRange.posEnd.nLine+1, cmRange.posEnd.nCol);
		pStatusBar->SetPaneText(ID_POSITION_PANE, sText);
	} else {
		pStatusBar->SetPaneText(ID_POSITION_PANE, "");
	}

	// display overtype indicator in the status bar
	if(IsOvertypeEnabled()) {
		CString sIndicator;
		sIndicator.LoadString(ID_OVERTYPE_PANE);
		pStatusBar->SetPaneText(ID_OVERTYPE_PANE, sIndicator);
	} else {
		pStatusBar->SetPaneText(ID_OVERTYPE_PANE, "");
	}
	
	// display overtype indicator in the status bar
	if(IsReadOnly()) {
		CString sIndicator;
		sIndicator.LoadString(ID_READONLY_PANE);
		pStatusBar->SetPaneText(ID_READONLY_PANE, sIndicator);
	} else {
		pStatusBar->SetPaneText(ID_READONLY_PANE, "");
	}
}
void CScriptEditorView::UIUpdateMenuItems()
{
	// Get the main window's UI updater
	CMainFrame *pMainFrm = m_pParentFrame->GetMainFrame();
	CUpdateUIBase *pUpdateUI = pMainFrm->GetUpdateUI();

	pUpdateUI->UIEnable(ID_FILE_RELOAD, !m_sFilePath.IsEmpty());
	pUpdateUI->UIEnable(ID_FILE_SAVE, IsModified());	
	pUpdateUI->UIEnable(ID_FILE_SAVE_AS, TRUE );
		
	pUpdateUI->UIEnable(ID_EDIT_UNDO, CanUndo());
	pUpdateUI->UIEnable(ID_EDIT_REDO, CanRedo());

	pUpdateUI->UIEnable(ID_EDIT_CUT, CanCut());
	pUpdateUI->UIEnable(ID_EDIT_COPY, CanCopy());
	pUpdateUI->UIEnable(ID_EDIT_PASTE, CanPaste());
	pUpdateUI->UIEnable(ID_EDIT_CLEAR, IsSelection());

	pUpdateUI->UIEnable(ID_EDIT_READ_ONLY, TRUE);
	pUpdateUI->UISetCheck(ID_EDIT_READ_ONLY, (IsReadOnly()?1:0));

	pUpdateUI->UIEnable(ID_EDIT_SHOW_WHITE_SPACE, TRUE);
	pUpdateUI->UISetCheck(ID_EDIT_SHOW_WHITE_SPACE, IsWhitespaceDisplayEnabled());

	// get the current selection
	BOOL bSelection, bColumnSelection;
	bSelection = IsSelection(bColumnSelection);

	pUpdateUI->UIEnable(ID_EDIT_TAB, TRUE ); // can always indent
	pUpdateUI->UIEnable(ID_EDIT_UNTAB, (bSelection && !bColumnSelection));

	pUpdateUI->UIEnable(ID_EDIT_FIND_SELECTION, (bSelection && !bColumnSelection));
		
	pUpdateUI->UIEnable(ID_EDIT_UPPERCASE, (bSelection && !bColumnSelection));
	pUpdateUI->UIEnable(ID_EDIT_LOWERCASE, (bSelection && !bColumnSelection));
	pUpdateUI->UIEnable(ID_EDIT_TABIFY, bSelection);
	pUpdateUI->UIEnable(ID_EDIT_UNTABIFY, bSelection);

	// retrive the number of bookmarks
	int nCount = GetAllBookmarks(NULL);
	
	pUpdateUI->UIEnable(ID_EDIT_GOTO_NEXT_BOOKMARK, nCount>0);
	pUpdateUI->UIEnable(ID_EDIT_GOTO_PREV_BOOKMARK, nCount>0);
	pUpdateUI->UIEnable(ID_EDIT_CLEAR_ALL_BOOKMARKS, nCount>0);
}
