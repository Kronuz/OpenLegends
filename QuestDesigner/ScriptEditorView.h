/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germ�n M�ndez Bravo)
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
/*! \file		ScriptEditorView.h 
	\brief		Interface of the CScriptEditorView class.
	\date		April 15, 2003
*/

#pragma once

#include "ChildView.h"

#include <CodeSense.h>
#include "CMCSWTL.h"

#define CMLANG_ZES _T("OZ Script file")

#define CMD_HELP	CMD_USER_BASE + 0

#define MAX_FUNC_NAME	100
#define MAX_FUNC_PROTO	1000

// Registry keys
#define SCRIPT_SECTION "Script Editor"
#define KEY_WINDOWPOS "WINPOS"
#define KEY_COLORSYNTAX "COLORSYNTAX"
#define KEY_COLORS "COLORS"
#define KEY_WHITESPACEDISPLAY "WHITESPACEDISPLAY"
#define KEY_TABEXPAND "TABEXPAND"
#define KEY_SMOOTHSCROLLING "SMOOTHSCROLLING"
#define KEY_TABSIZE "TABSIZE"
#define KEY_LINETOOLTIPS "LINETOOLTIPS"
#define KEY_LEFTMARGIN "LEFTMARGIN"
#define KEY_COLUMNSEL "COLUMNSEL"
#define KEY_DRAGDROP "DRAGDROP"
#define KEY_CASESENSITIVE "CASESENSITIVE"
#define KEY_PRESERVECASE "PRESERVECASE"
#define KEY_WHOLEWORD "WHOLEWORD"
#define KEY_AUTOINDENTMODE "AUTOINDENTMODE"
#define KEY_HSCROLLBAR "HSCROLLBAR"
#define KEY_VSCROLLBAR "VSCROLLBAR"
#define KEY_HSPLITTER "HSPLITTER"
#define KEY_VSPLITTER "VSPLITTER"
#define KEY_HOTKEYS "HOTKEYS"
#define KEY_FINDMRULIST "FINDMRULIST"
#define KEY_REPLMRULIST "REPLMRULIST"
#define KEY_FONT "FONT"
#define KEY_LINENUMBERING "LINENUMBERING"
#define KEY_FONTSTYLES "FONTSTYLES"
#define KEY_NORMALIZECASE "NORMALIZECASE"
#define KEY_SELBOUNDS "SELBOUNDS"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CScriptEditorFrame;

class CScriptEditorView:
	public CChildView,
	public CWindowImpl<CScriptEditorView, CodeSenseControl>,
	public CodeSenseControlNotifications<CScriptEditorView>,
	public CodeSenseControlCommands<CScriptEditorView>
{
	HFONT m_hFont;
	CM_POSITION m_posSel;
	TCHAR m_szTipFunc[MAX_FUNC_NAME];

public:
	bool m_bModified;

	// Construction/Destruction
	CScriptEditorView(CScriptEditorFrame *pParentFrame);
	~CScriptEditorView();

	DECLARE_WND_SUPERCLASS(NULL, CodeSenseControl::GetWndClassName())

	// Called to translate window messages before they are dispatched 
	BOOL PreTranslateMessage(MSG *pMsg);
	// Called to do idle processing
	virtual BOOL OnIdle();
	// Called to clean up after window is destroyed
	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CScriptEditorView)
		
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_MOVE, OnMove)

		COMMAND_ID_HANDLER(ID_APP_RELOAD, OnFileReload)
		COMMAND_ID_HANDLER(ID_APP_SAVE, OnFileSave)
		COMMAND_ID_HANDLER(ID_APP_SAVE_AS, OnFileSaveAs)
		
		COMMAND_ID_HANDLER(ID_SCRIPTED_CODELIST, OnCodeList1)
		COMMAND_ID_HANDLER(ID_SCRIPTED_CODETIP1, OnCodeTip1)
		COMMAND_ID_HANDLER(ID_SCRIPTED_CODETIP2, OnCodeTip2)

		COMMAND_ID_HANDLER(ID_SCRIPTED_TAB, OnEditTab)
		COMMAND_ID_HANDLER(ID_SCRIPTED_UNTAB, OnEditUntab)
		
		COMMAND_ID_HANDLER(ID_SCRIPTED_FIND_NEXT, OnEditFindNext)
		COMMAND_ID_HANDLER(ID_SCRIPTED_FIND_PREV, OnEditFindPrev)
		COMMAND_ID_HANDLER(ID_SCRIPTED_FIND_SELECTION, OnEditFindSelection)

		COMMAND_ID_HANDLER(ID_SCRIPTED_GOTO_LINE, OnEditGotoLine)
		COMMAND_ID_HANDLER(ID_SCRIPTED_MATCH_BRACE, OnEditMatchBrace)
		
		COMMAND_ID_HANDLER(ID_SCRIPTED_READ_ONLY, OnEditReadOnly)

		COMMAND_ID_HANDLER(ID_SCRIPTED_UPPERCASE, OnEditUppercase)
		COMMAND_ID_HANDLER(ID_SCRIPTED_LOWERCASE, OnEditLowercase)
		COMMAND_ID_HANDLER(ID_SCRIPTED_TABIFY, OnEditTabify)
		COMMAND_ID_HANDLER(ID_SCRIPTED_UNTABIFY, OnEditUntabify)
		COMMAND_ID_HANDLER(ID_SCRIPTED_SHOW_WHITE_SPACE, OnShowWhiteSpace)
		
		COMMAND_ID_HANDLER(ID_SCRIPTED_TOGGLE_BOOKMARK, OnEditToggleBookmark)
		COMMAND_ID_HANDLER(ID_SCRIPTED_GOTO_NEXT_BOOKMARK, OnEditGotoNextBookmark)
		COMMAND_ID_HANDLER(ID_SCRIPTED_GOTO_PREV_BOOKMARK, OnUpdateEditGotoPrevBookmark)
		COMMAND_ID_HANDLER(ID_SCRIPTED_CLEAR_ALL_BOOKMARKS, OnEditClearAllBookmarks)


		// Make sure that the notification and default message handlers get a crack at the messages
		CHAIN_MSG_MAP_ALT(CodeSenseControlNotifications<CScriptEditorView>, CMAX_REFLECTED_NOTIFY_CODE_HANDLERS)
		CHAIN_MSG_MAP_ALT(CodeSenseControlCommands<CScriptEditorView>, CMAX_BASIC_COMMAND_ID_HANDLERS)

	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	LRESULT OnFileReload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Save handlers
	LRESULT OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnCodeList1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCodeTip1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCodeTip2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Indent/Unindent the current selection
	LRESULT OnEditTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditUntab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Find next/previous search match in the buffer
	LRESULT OnEditFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditFindPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Searches the buffer using the current selection as the search parameter
	LRESULT OnEditFindSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	// Jump to the desired line
	LRESULT OnEditGotoLine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Find the current brace's 'sibling'
	LRESULT OnEditMatchBrace(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	// Set buffer to read-only
	LRESULT OnEditReadOnly(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Advanced edit commands
	LRESULT OnEditUppercase(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditLowercase(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditTabify(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditUntabify(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowWhiteSpace(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	// Bookmark functions
	LRESULT OnEditToggleBookmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditGotoNextBookmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUpdateEditGotoPrevBookmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditClearAllBookmarks(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Open a file
	BOOL DoFileOpen(LPCTSTR , LPCTSTR=_T("Untitled"));

	// Save the file
	BOOL DoFileSave(const CString &);
	BOOL DoFileSaveAs();

	// Reload a file
	BOOL DoReload ();

	void UIUpdateStatusBar();
	void UIUpdateMenuItems();

	// has the content of the control changed?
	bool hasChanged();

	void SaveProfile();
	void LoadProfile();

	int GetCurrentArgument();
	BOOL GetCurrentFunction( LPSTR pszName, bool bMustExist = false );
	BOOL GetPrototype( LPCSTR pszFunc, LPSTR pszProto, CM_CODETIPMULTIFUNCDATA *pmfData );

	// Overloads:
	void OnPropertiesChange();
	void OnRegisteredCommand ( CM_REGISTEREDCMDDATA *prcd );
	BOOL OnKeyPress ( CM_KEYDATA *pkd );
	LRESULT OnCodeTip ( CM_CODETIPDATA * );
	BOOL OnCodeTipInitialize( CM_CODETIPDATA *pctd );
	BOOL OnCodeTipUpdate( CM_CODETIPDATA *pctd );

};
