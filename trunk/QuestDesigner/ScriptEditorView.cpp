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

#include "ScriptEditorView.h"
#include "ScriptEditorFrm.h"
#include <ssfiledialog.h>

CScriptEditorView::CScriptEditorView(CScriptEditorFrame *pParentFrame) :
m_pParentFrame(pParentFrame)
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
//	UIUpdateMenuItems ();
	
	// Update position display in the status bar
//	UIUpdateStatusBar ();	

	return FALSE;
}
// Called to clean up after window is destroyed (called when WM_NCDESTROY is sent)
void CScriptEditorView::OnFinalMessage(HWND /*hWnd*/)
{
/*
	// Update position display in the status bar (remove it)
	CMainFrame * pMainFrm = m_pParentFrame->GetMainFrame ();
	CMultiPaneStatusBarCtrl * pStatusBar = pMainFrm->GetMultiPaneStatusBarCtrl ();
	pStatusBar->SetPaneText ( ID_POSITION_PANE, _T ( "" ) );
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
	return 0;
}

// Save handlers
LRESULT CScriptEditorView::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

// Indent/Unindent the current selection
LRESULT CScriptEditorView::OnEditTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnEditUntab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

// Find next/previous search match in the buffer
LRESULT CScriptEditorView::OnEditFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnEditFindPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

// Searches the buffer using the current selection as the search parameter
LRESULT CScriptEditorView::OnEditFindSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
// Jump to the desired line
LRESULT CScriptEditorView::OnEditGotoLine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

// Find the current brace's 'sibling'
LRESULT CScriptEditorView::OnEditMatchBrace(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

// Set buffer to read-only
LRESULT CScriptEditorView::OnEditReadOnly(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

// Advanced edit commands
LRESULT CScriptEditorView::OnEditUppercase(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnEditLowercase(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnEditTabify(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnEditUntabify(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnShowWhiteSpace(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

// Bookmark functions
LRESULT CScriptEditorView::OnEditToggleBookmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnEditGotoNextBookmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnUpdateEditGotoPrevBookmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CScriptEditorView::OnEditClearAllBookmarks(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Open a file
BOOL CScriptEditorView::DoFileOpen ( LPCTSTR lpszPathName, LPCTSTR lpszTitle /*=_T("Untitled")*/ ) 
{
	ATLTRACE ( _T ( "Entering : CCodeMaxWTLSampleView::DoFileOpen ( %s )\n" ), lpszPathName );
	
	// open the requested file
	CME_CODE lRet = OpenFile(lpszPathName);

	if(CME_FAILED(lRet)) {
		ATLTRACE ( _T ( "Error: Failed to load file: %s\n" ), lpszPathName );

		// show the error message
		CString sMessage;
		sMessage.Format ( _T( "Failed to load: %s\n\n" ), lpszPathName );
		//ShowLastError ( sMessage, ::GetLastError () );
		
		return FALSE;
	}

	// Save file name for later
	m_sFileName = lpszPathName;

	// Save the tittle for later
	m_sTitle = lpszTitle;

	m_pParentFrame->SetTabText(m_sTitle);

	// If the CodeMax control has normalized case enabled, forcing
	//	a reload of the file will makes CodeMax normalize the entire file
	if(IsNormalizeCaseEnabled()) {
		// force a reload...
		//DoReload();
	}

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
	m_sFileName = sPathName;	

	return TRUE;
}
BOOL CScriptEditorView::DoFileSaveAs () 
{
	static TCHAR szFilter[] = "OZ Script files (*.zes;*.inc)|*.zes; *.inc|All Files (*.*)|*.*||";
	CSSFileDialog wndFileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter, m_hWnd);
	
	if(IDOK == wndFileDialog.DoModal()) {
		if(!DoFileSave (wndFileDialog.m_ofn.lpstrFile)) {
			return FALSE;
		}
		// save the title
		//m_sTitle = wndFileDialog.GetFileTitle ();
		// save the title
		//m_sFilename = wndFileDialog.GetFileTitle();
		return TRUE;
	}
	return FALSE;
}

// Reload a file	
BOOL CScriptEditorView::DoReload ()
{
	// simply re-open the file we opend
	CME_CODE lRet = OpenFile(m_sFileName);

	if(CME_FAILED(lRet)) {		
		ATLTRACE(_T ( "Error: Failed to reload file: %s\n" ), m_sFileName);
		return FALSE;
	}
	return TRUE;
}
