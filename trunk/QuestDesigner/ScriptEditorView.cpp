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
// ScriptEditorView.cpp : implementation of the CScriptEditorView class
//

#include "stdafx.h"

#include "MainFrm.h"
#include "ScriptEditorView.h"
#include "ScriptEditorFrm.h"
#include <ssfiledialog.h>

CScriptEditorView::CScriptEditorView(CScriptEditorFrame *pParentFrame) :
	CChildView(pParentFrame),
	m_bModified(false),
	m_hFont(NULL)
{
	m_szTipFunc[0] = _T('\0');
}
CScriptEditorView::~CScriptEditorView()
{
	if(m_hFont) {
		DeleteObject(m_hFont);
	}
}

// Called to translate window messages before they are dispatched 
BOOL CScriptEditorView::PreTranslateMessage(MSG *pMsg)
{
	return FALSE;
}
// Called to do idle processing
BOOL CScriptEditorView::OnIdle()
{
	// Update all the menu items
	UIUpdateMenuItems();
	
	// Update position display in the status bar
	UIUpdateStatusBar();

	bool bModified = (IsModified()?true:false);
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
	// Let the CodeSense control be created
	LRESULT nResult = DefWindowProc();
	// Set the current language and load the profile
	CME_VERIFY(SetLanguage(CMLANG_ZES));

	::CMRegisterCommand( CMD_HELP, _T("Help"), _T("Help") );
	CM_HOTKEY cmHotKey = { 0, VK_F1, 0, 0 };
	CMRegisterHotKey( &cmHotKey, CMD_HELP );

	LoadProfile();

	return nResult;
}
LRESULT CScriptEditorView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	SaveProfile();
	return 0;
}

LRESULT CScriptEditorView::OnMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
	UpdateControlPositions();
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

LRESULT CScriptEditorView::OnCodeList1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CodeList();
	return 0;
}
LRESULT CScriptEditorView::OnCodeTip1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CodeTip();
	return 0;
}
LRESULT CScriptEditorView::OnCodeTip2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
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
	CMainFrame *pMainFrm = GetMainFrame();
	CUpdateUIBase *pUpdateUI = pMainFrm->GetUpdateUI();

	pUpdateUI->UIEnable(ID_APP_RELOAD, !m_sFilePath.IsEmpty());
	pUpdateUI->UIEnable(ID_APP_SAVE, IsModified());	
	pUpdateUI->UIEnable(ID_APP_SAVE_AS, TRUE );
		
	pUpdateUI->UIEnable(ID_UNDO, CanUndo());
	pUpdateUI->UIEnable(ID_REDO, CanRedo());

	pUpdateUI->UIEnable(ID_CUT, CanCut());
	pUpdateUI->UIEnable(ID_COPY, CanCopy());
	pUpdateUI->UIEnable(ID_PASTE, CanPaste());
	pUpdateUI->UIEnable(ID_ERASE, IsSelection());

	pUpdateUI->UIEnable(ID_SCRIPTED_READ_ONLY, TRUE);
	pUpdateUI->UISetCheck(ID_SCRIPTED_READ_ONLY, (IsReadOnly()?1:0));

	pUpdateUI->UIEnable(ID_SCRIPTED_SHOW_WHITE_SPACE, TRUE);
	pUpdateUI->UISetCheck(ID_SCRIPTED_SHOW_WHITE_SPACE, IsWhitespaceDisplayEnabled());

	// get the current selection
	BOOL bSelection, bColumnSelection;
	bSelection = IsSelection(bColumnSelection);

	pUpdateUI->UIEnable(ID_SCRIPTED_CODELIST, TRUE );
	pUpdateUI->UIEnable(ID_SCRIPTED_CODETIP1, TRUE );
	pUpdateUI->UIEnable(ID_SCRIPTED_CODETIP2, TRUE );

	pUpdateUI->UIEnable(ID_SCRIPTED_TAB, TRUE ); // can always indent
	pUpdateUI->UIEnable(ID_SCRIPTED_UNTAB, (bSelection && !bColumnSelection));

	pUpdateUI->UIEnable(ID_SCRIPTED_FIND_SELECTION, (bSelection && !bColumnSelection));
		
	pUpdateUI->UIEnable(ID_SCRIPTED_UPPERCASE, (bSelection && !bColumnSelection));
	pUpdateUI->UIEnable(ID_SCRIPTED_LOWERCASE, (bSelection && !bColumnSelection));
	pUpdateUI->UIEnable(ID_SCRIPTED_TABIFY, bSelection);
	pUpdateUI->UIEnable(ID_SCRIPTED_UNTABIFY, bSelection);

	// retrive the number of bookmarks
	int nCount = GetAllBookmarks(NULL);
	
	pUpdateUI->UIEnable(ID_SCRIPTED_GOTO_NEXT_BOOKMARK, nCount>0);
	pUpdateUI->UIEnable(ID_SCRIPTED_GOTO_PREV_BOOKMARK, nCount>0);
	pUpdateUI->UIEnable(ID_SCRIPTED_CLEAR_ALL_BOOKMARKS, nCount>0);
}

bool CScriptEditorView::hasChanged()
{
	return (IsModified()?true:false);
}

void CScriptEditorView::OnRegisteredCommand ( CM_REGISTEREDCMDDATA *prcd )
{
	switch ( prcd->wCmd )
	{
		case CMD_HELP: {
			char word[101];
			int len = GetWordLength(NULL);
			if(len>0 && len<100) {
				GetWord(word, NULL);
				ShowHelp(m_hWnd, word);
			}
		} break;
	}
}

BOOL CScriptEditorView::OnKeyPress ( CM_KEYDATA *pkd )
{
	// Invoke the CodeTip feature when the user presses the '(' key. We'll
	// determine whether or not we are actually ready to handle the message
	// in the OnCodeTip() handler.
	//
	if( '(' == pkd->nKeyCode ) {
		CodeTip();
	}

	return FALSE;
}

LRESULT CScriptEditorView::OnCodeTip ( CM_CODETIPDATA * )
{
	// We don't want to display a tip inside quoted or commented-out lines...
	DWORD dwToken = GetCurrentToken();

	if( CM_TOKENTYPE_TEXT == dwToken || CM_TOKENTYPE_KEYWORD == dwToken || CM_TOKENTYPE_UNKNOWN == dwToken ) {
		// See if there is a valid function on the current line
		if( GetCurrentFunction( NULL ) ) {
			// We want to use the tooltip control that automatically
			// highlights the arguments in the function prototypes for us.
			// This type also provides support for overloaded function
			// prototypes.
			//
			return CM_TIPSTYLE_MULTIFUNC;
		}
	}

	// Don't display a tooltip
	return CM_TIPSTYLE_NONE;
}

BOOL CScriptEditorView::OnCodeTipInitialize( CM_CODETIPDATA *pctd )
{
	ASSERT( CM_TIPSTYLE_MULTIFUNC == pctd->nTipType );

	HWND hToolTip = pctd->hToolTip;
	ASSERT( NULL != hToolTip );

	LPCM_CODETIPMULTIFUNCDATA pmfData = (LPCM_CODETIPMULTIFUNCDATA)pctd;

	// Save name of current function
	GetCurrentFunction( m_szTipFunc );

	// Default to first function prototype
	pmfData->nCurrFunc = 0;

	// Get first prototype for function
	char szProto[MAX_FUNC_PROTO] = {0};
	GetPrototype( m_szTipFunc, szProto, pmfData );

	// Set tooltip text
	::SetWindowText( hToolTip, szProto );

	// Default to first argument
	pmfData->ctfData.nArgument = 1;

	// Apply changes to pmfData members
	return TRUE;
}
BOOL CScriptEditorView::OnCodeTipUpdate( CM_CODETIPDATA *pctd )
{
	ASSERT( CM_TIPSTYLE_MULTIFUNC == pctd->nTipType );

	HWND hToolTip = pctd->hToolTip;
	ASSERT( NULL != hToolTip );

	LPCM_CODETIPMULTIFUNCDATA pmfData = (LPCM_CODETIPMULTIFUNCDATA)pctd;

	// Destroy the tooltip window if the caret is moved above or before the
	// starting point.
	//
	CM_RANGE range = {0};

	CME_VERIFY( GetSel( &range, TRUE ) );

	if( range.posEnd.nLine < m_posSel.nLine ||
		( range.posEnd.nCol < m_posSel.nCol &&
		range.posEnd.nLine == m_posSel.nLine ) )
	{
		// Caret moved too far up / back
		::DestroyWindow( hToolTip );
		return FALSE;
	}
	else 
	{
		// Determine which argument to highlight
		pmfData->ctfData.nArgument = GetCurrentArgument();

		if( -1 == pmfData->ctfData.nArgument ) {
			::DestroyWindow( hToolTip );// Right-paren found
			return FALSE;
		} else if( pmfData->nFuncCount )
		{
			// Function is overloaded, so get current prototype
			char szProto[MAX_FUNC_PROTO] = {0};
			GetPrototype( m_szTipFunc, szProto, pmfData );

			// Set tooltip text
			::SetWindowText( hToolTip, szProto );
		}
	}

	// Apply changes to pmfData members
	return TRUE;
}

int CScriptEditorView::GetCurrentArgument()
{
	// Parse the buffer to determine which argument to highlight...
	//
	int iArg = 1;
	int nSubParen = 0;
	CM_RANGE range = {0};
	char szLine[10000] = {0};

	CME_VERIFY( GetSel( &range, TRUE ) );

	enum {
		sNone,
		sQuote,
		sComment,
		sMultiComment,
		sEscape,
		sSubParen,
	
	} state = sNone;

	for( int nLine = m_posSel.nLine; nLine <= range.posEnd.nLine; nLine++ ) {
		// Get next line from buffer
		CME_VERIFY( GetLine( nLine, szLine ) );

		if( nLine == range.posEnd.nLine ) {
			// Trim off any excess beyond cursor pos so the argument with the
			// cursor in it will be highlighted.
			int iTrim = min( range.posEnd.nCol, lstrlen( szLine ) );
			szLine[iTrim] = '\0';
		}

		if( nLine == m_posSel.nLine ) {
			// Strip off function name & open paren.
			LPSTR psz = strchr( szLine+m_posSel.nCol, '(' );
			if(psz) strcpy( szLine, ++psz );
			else return 0;
		}

		// Parse arguments from current line
		for( int iPos = 0, len = strlen( szLine ); iPos < len; iPos++ ) {
			switch( szLine[iPos] ) {
			case '(':// Sub-parenthesis
				{
					switch( state ) {
					case sSubParen:
					case sNone:
						state = sSubParen;
						nSubParen++;
						break;
					case sEscape:
						state = sQuote;
						break;
					}
				}
				break;

			case '"':// String begin/end
				{
					switch( state ) {
					case sQuote:
						state = sNone;
						break;
					case sComment:
					case sMultiComment:
					case sSubParen:
						break;
					default:
						state = sQuote;
						break;
					}
				}
				break;

			case ',':// Argument separator
				{
					switch( state ) {
					case sNone:
						iArg++;
						break;
					case sEscape:
						state = sQuote;
						break;
					}
				}
				break;

			case ')':// End of function statement
				{
					switch( state ) {
					case sNone:
						return -1;// Destroy tooltip on return
					case sEscape:
						state = sQuote;
						break;
					case sSubParen:
						if(--nSubParen==0)
							state = sNone;
						break;
					}
				}
				break;

			case '\\':// Escape sequence
				{
					switch( state ) {
					case sQuote:
						state = sEscape;
						break;
					case sEscape:
						state = sQuote;
						break;
					}
				}
				break;

			case '/':// Possible comment begin/end
				{
					switch( state ) {
					case sNone:
						{
							if( iPos + 1 < len ) {
								char c = szLine[iPos + 1];

								if( '/' == c ) {
									state = sComment;
								} else if( '*' == c ) {
									state = sMultiComment;
									iPos++;
								}
							}
						}
						break;

					case sMultiComment:
						{
							if( iPos && '*' == szLine[iPos - 1] )
								state = sNone;
						}
						break;

					case sEscape:
						state = sQuote;
						break;
					}
				}
				break;

			default:
				{
					if( sEscape == state )
						state = sQuote;
				}
				break;
			}

			// No point in scanning each and every character in comment line!
			if( sComment == state )
				break;
		}

		// It's safe to clear this now that we're moving on to the next line
		if( sComment == state )
			state = sNone;
	}

	return iArg;
}

BOOL CScriptEditorView::GetPrototype( LPCSTR pszFunc, LPSTR pszProto, CM_CODETIPMULTIFUNCDATA *pmfData )
{
	strcpy(pszProto, pszFunc);
	strcat(pszProto, "(a, b, c)");
	pmfData->nFuncCount = 2;
	return TRUE;
}
BOOL CScriptEditorView::GetCurrentFunction( LPSTR pszName, bool bMustExist )
{
	CM_RANGE range = {0};
	char szLine[1000] = {0};
	char szFunc[MAX_FUNC_NAME] = {0};

	// Note:  We can't use CM_GetWord() here, because the user could have
	// typed a function name followed by a space, and *then* the left
	// parenthesis, i.e. "MessageBox ("

	// Get the current line
	GetSel( &range, TRUE );
	GetLine( range.posEnd.nLine, szLine );

	// There's nothing for us to do if the line is empty
	if( '\0' == *szLine )
		return FALSE;

	int nEnd = range.posEnd.nCol - 1;

	// Trim off trailing '(', if found
	if( nEnd > 0 && nEnd < lstrlen( szLine ) )
	{
		if( '(' == szLine[ nEnd ] )
			nEnd --;

		// Trim off trailing whitespace
		while( nEnd > 0 && ' ' == szLine[nEnd] )
			nEnd --;
	}

	if( nEnd < 0 )
		return FALSE;

	// The function name begins at the first alphanumeric character on line
	int nStart = min( nEnd, lstrlen( szLine ) );

	while( nStart > 0 && ( _istalnum( szLine[nStart - 1] ) ||
		'_' == szLine[nStart - 1] ) )
	{
		--nStart;
	}

	// Save the starting position for use with the CodeTip.  This is so we
	// can destroy the tip window if the user moves the cursor back before or
	// above the starting point.
	//
	m_posSel.nCol = nStart;
	m_posSel.nLine = range.posEnd.nLine;

	// Extract the function name
	lstrcpyn( szFunc, szLine + nStart, nEnd - nStart + 2 );

	if( '\0' == *szFunc )
		return FALSE;

	// If we don't care whether or not the function actually exists in the
	// list, just return the string.
	if( !bMustExist )
	{
		if( pszName )
			lstrcpy( pszName, szFunc );

		return TRUE;
	}

	// TODO: Look for the function name in the DB...

	return FALSE;
/*/
	return TRUE;
/**/
}

void CScriptEditorView::OnPropertiesChange()
{
	SaveProfile();
}
void CScriptEditorView::SaveProfile()
{
	// misc props
	WriteProfileInt( SCRIPT_SECTION, KEY_COLORSYNTAX, IsColorSyntaxEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_HSPLITTER, IsSplitterEnabled( TRUE ) );
	WriteProfileInt( SCRIPT_SECTION, KEY_VSPLITTER, IsSplitterEnabled( FALSE ) );
	WriteProfileInt( SCRIPT_SECTION, KEY_HSCROLLBAR, HasScrollBar( TRUE ) );
	WriteProfileInt( SCRIPT_SECTION, KEY_VSCROLLBAR, HasScrollBar( FALSE ) );
	WriteProfileInt( SCRIPT_SECTION, KEY_WHOLEWORD, IsWholeWordEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_AUTOINDENTMODE, GetAutoIndentMode() );
	WriteProfileInt( SCRIPT_SECTION, KEY_SMOOTHSCROLLING, IsSmoothScrollingEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_LINETOOLTIPS, IsLineToolTipsEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_LEFTMARGIN, IsLeftMarginEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_COLUMNSEL, IsColumnSelEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_DRAGDROP, IsDragDropEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_CASESENSITIVE, IsCaseSensitiveEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_PRESERVECASE, IsPreserveCaseEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_TABEXPAND, IsTabExpandEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_TABSIZE, GetTabSize() );
	WriteProfileInt( SCRIPT_SECTION, KEY_NORMALIZECASE, IsNormalizeCaseEnabled() );
	WriteProfileInt( SCRIPT_SECTION, KEY_SELBOUNDS, IsSelBoundsEnabled() );

	// color info
	CM_COLORS colors;
	GetColors( &colors );
	WriteProfileBinary( SCRIPT_SECTION, KEY_COLORS, ( LPBYTE ) &colors, sizeof( colors ) );

	// font info
	CM_FONTSTYLES fs;
	GetFontStyles( &fs );
	WriteProfileBinary( SCRIPT_SECTION, KEY_FONTSTYLES, ( LPBYTE ) &fs, sizeof( fs ) );

	// font
	LOGFONT lf;
	HFONT hFont = ( HFONT ) SendMessage( WM_GETFONT, 0, 0 );
	int cbSize = GetObject( hFont, sizeof( lf ), NULL );
	GetObject( hFont, cbSize, &lf );
	WriteProfileBinary( SCRIPT_SECTION, KEY_FONT, ( LPBYTE ) &lf, sizeof( lf ) );

	CM_LINENUMBERING cmLineNum;
	GetLineNumbering( &cmLineNum );
	WriteProfileBinary( SCRIPT_SECTION, KEY_LINENUMBERING, ( LPBYTE ) &cmLineNum, sizeof( cmLineNum ) );
}

void CScriptEditorView::LoadProfile()
{
	// load window settings
	EnableColorSyntax( GetProfileInt( SCRIPT_SECTION, KEY_COLORSYNTAX, TRUE ) );
	ShowScrollBar( TRUE, GetProfileInt( SCRIPT_SECTION, KEY_HSCROLLBAR, TRUE ) );
	ShowScrollBar( FALSE, GetProfileInt( SCRIPT_SECTION, KEY_VSCROLLBAR, TRUE ) );
	EnableSplitter( TRUE, GetProfileInt( SCRIPT_SECTION, KEY_HSPLITTER, FALSE ) );
	EnableSplitter( FALSE, GetProfileInt( SCRIPT_SECTION, KEY_VSPLITTER, TRUE ) );
	EnableWholeWord( GetProfileInt( SCRIPT_SECTION, KEY_WHOLEWORD, FALSE ) );
	SetAutoIndentMode( GetProfileInt( SCRIPT_SECTION, KEY_AUTOINDENTMODE, CM_INDENT_PREVLINE ) );
	EnableSmoothScrolling( GetProfileInt( SCRIPT_SECTION, KEY_SMOOTHSCROLLING, FALSE ) );
	EnableLineToolTips( GetProfileInt( SCRIPT_SECTION, KEY_LINETOOLTIPS, TRUE ) );
	EnableLeftMargin( GetProfileInt( SCRIPT_SECTION, KEY_LEFTMARGIN, TRUE ) );
	EnableColumnSel( GetProfileInt( SCRIPT_SECTION, KEY_COLUMNSEL, TRUE ) );
	EnableDragDrop( GetProfileInt( SCRIPT_SECTION, KEY_DRAGDROP, TRUE ) );
	EnableCaseSensitive( GetProfileInt( SCRIPT_SECTION, KEY_CASESENSITIVE, FALSE ) );
	EnablePreserveCase( GetProfileInt( SCRIPT_SECTION, KEY_PRESERVECASE, FALSE ) );
	EnableTabExpand( GetProfileInt( SCRIPT_SECTION, KEY_TABEXPAND, FALSE ) );
	SetTabSize( GetProfileInt( SCRIPT_SECTION, KEY_TABSIZE, 3 ) );
	EnableNormalizeCase( GetProfileInt( SCRIPT_SECTION, KEY_NORMALIZECASE, FALSE ) );
	EnableSelBounds( GetProfileInt( SCRIPT_SECTION, KEY_SELBOUNDS, TRUE ) );

	// colors
	LPBYTE pColors;
	UINT unSize;
	if ( GetProfileBinary( SCRIPT_SECTION, KEY_COLORS, &pColors, &unSize ) )
	{
		if ( unSize == sizeof( CM_COLORS ) )
		{
			SetColors( (CM_COLORS *)pColors );
		}
		delete [] pColors;
	}

	// font info
	LPBYTE pFontStyles;
	if ( GetProfileBinary( SCRIPT_SECTION, KEY_FONTSTYLES, &pFontStyles, &unSize ) )
	{
		if ( unSize == sizeof( CM_FONTSTYLES ) )
		{
			SetFontStyles( (CM_FONTSTYLES *)pFontStyles );
		}
		delete [] pFontStyles;
	}

	// font
	if ( !m_hFont )
	{
		LOGFONT *plf;
		if ( GetProfileBinary( SCRIPT_SECTION, KEY_FONT, ( LPBYTE * ) &plf, &unSize ) )
		{
			m_hFont = CreateFontIndirect( plf );
			delete [] plf;
			SendMessage( WM_SETFONT, ( WPARAM ) m_hFont, 0 );
		}
	}

	// colors
	LPBYTE pLineNum;
	if ( GetProfileBinary( SCRIPT_SECTION, KEY_LINENUMBERING, &pLineNum, &unSize ) )
	{
		if ( unSize == sizeof( CM_LINENUMBERING ) )
		{
			SetLineNumbering( (CM_LINENUMBERING *)pLineNum );
		}
		delete [] pLineNum;
	}
}