/////////////////////////////////////////////////////////////////////////////
//
//	cmcswtl.h - Simple WTL wraper classes for CodeSense
//
/////////////////////////////////////////////////////////////////////////////
//
//	 Version:	2.0
//	 Created:	2001/1/29 - 8:00 pm
//  Modified:   2003/9/20 - 6:32 pm
//	Filename: 	cmcswtl.h
//	 Authors:	Ben Burnett
//				German Mendez Bravo (a.k.a. Kronuz)
//	   Email:	ben.burnett@quaggait.com
//				kronuz@users.sourceforge.net
//
//	 Purpose:	Simple WTL wraper classes for CodeSense
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2000 - 2003, Quagga IT - All rights reserved.
//
// The code and information is provided "as-is" without warranty of 
//	any kind, either expressed or implied.
//
// Send any bug fixes and/or suggestions to the e-mail listed above.
//
/////////////////////////////////////////////////////////////////////////////
#if !defined ( _cmcswtl_h_included_ )
#define _cmcswtl_h_included_

#if _MSC_VER >= 1000
#	pragma once
#endif // _MSC_VER >= 1000

#ifndef __cplusplus
#	error CodeSense WTL wrapers require C++ compilation (use a .cpp suffix)
#endif

#ifndef _CODESENSE_H__INCLUDED_
#	pragma message ( "'cmcswtl.h'' requires CodeSense.h to be included first. Once you include it" )
#	pragma message ( " you will not get this message." )
#	include <CodeSense.h>
#endif // _CODESENSE_H__INCLUDED_

/////////////////////////////////////////////////////////////////////////////

namespace cmcs
{

/////////////////////////////////////////////////////////////////////////////
// CodeSense library
#ifdef _DEBUG
#pragma comment ( lib, "cmcs21d.lib" )
#else
#pragma comment ( lib, "cmcs21.lib" )
#endif
////////////////////////////////////////////////////////////////////////////////////
// Forward declarations

// CodeSense library interface
class CodeSenseLibrary;

// Simple blocked undo helper ( Undo block start when the object is created and ends 
//	when it is detroyed )
//	
//	Blocked undo - A method by which series of undoable commands 'blocked' together 
//				   so that they can be undone all at once
//
template < class TControl > class UndoBlockT;
	// class UndoBlock

// Template for the CodeSense window
template < class TBase > class CodeSenseControlT;
	// class CodeSenseControl

// Template used to handle the CodeSense window's notifications (reflected or not)
template < class T > class CodeSenseControlNotifications;

// Template used to handle the CodeSense window's standard commands
template < class T > class CodeSenseControlCommands;

////////////////////////////////////////////////////////////////////////////////////
// Simple Macros & Defines

// Simple verifiers
#define CME_SUCCEEDED(Status)	( CME_SUCCESS == Status )
#define CME_FAILED(Status)		( CME_SUCCESS != Status )

#ifdef _DEBUG
#	define CME_VERIFY(Status)	ATLASSERT ( CME_SUCCEEDED ( Status ) )
#else
#	define CME_VERIFY(Status)	Status
#endif

/////////////////////////////////////////////////////////////////////////////
// CodeSenseLibrary : a wraper for the general CodeSense library function calls

class CodeSenseLibrary  
{

// Data
private:
	
	// Set as true when the CodeSense library has been initialized
	static bool m_bInitialized;

// Constructors / Destructors
public:
	
	CodeSenseLibrary ()
	{ /**/ }
	
	virtual ~CodeSenseLibrary ()
	{
		if ( m_bInitialized ) {
			
			// Unregister the CodeSense control
			::CMUnregisterControl ();
		}		
	}	
		
	BOOL Initialize ( DWORD dwVersion = CM_VERSION )
	{
		ATLASSERT( !m_bInitialized );
		
		// Check if we are alredy initialized
		if ( m_bInitialized ) {
			
			return FALSE;
		}		
		
		// Register the CodeSense control	
		CME_CODE lRet = ::CMRegisterControl ( dwVersion );
		
		if ( CME_FAILED ( lRet ) ) {
			
			return FALSE;
		}
		
		// Were ready to be used
		m_bInitialized = true;
		
		return TRUE;
	}
	
// Operations
public:
	
	//
	// Functions to retrieve the Most Recently Used (MRU) list displayed in 
	//	the Find and Find/Replace dialog.
	//

	static void SetFindReplaceMRUList ( LPCTSTR lpszMRUList, BOOL bFind = TRUE )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( NULL != lpszMRUList );
		
		::CMSetFindReplaceMRUList ( lpszMRUList, bFind );
	}

	static void GetFindReplaceMRUList ( LPTSTR lpszMRUList, BOOL bFind = TRUE )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( NULL != lpszMRUList );
		
		::CMGetFindReplaceMRUList ( lpszMRUList, bFind );
	}

#ifdef __ATLSTR_H__
	static void GetFindReplaceMRUList ( CString & sMRUList, BOOL bFind = TRUE )
	{
		ATLASSERT ( m_bInitialized );
		
		::CMGetFindReplaceMRUList ( sMRUList.GetBuffer ( CM_FIND_REPLACE_MRU_BUFF_SIZE ), bFind );
		sMRUList.ReleaseBuffer ();
	}
#endif // __ATLSTR_H__
	
	//
	// Functions to save/restores the CodeSense keystroke assignments 
	//

	static int GetHotKeys ( LPBYTE pBuff )
	{
		ATLASSERT ( m_bInitialized );
		// ATLASSERT ( NULL != pBuff );		
		// No check because if pBuff = NULL, the required buffer size (in BYTES) 
		// is returned so that a buffer can be sufficiently allocated. 
		
		// Retrieve the entire set of global hotkeys currently used.
		return ::CMGetHotKeys ( pBuff );
	}
	
	static BOOL SetHotKeys ( const LPBYTE pBuff )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( NULL != pBuff );
		
		// Set hotkeys
		CME_CODE lRet = ::CMSetHotKeys ( pBuff );
		
		if ( CME_FAILED ( lRet ) ) {
			
			return FALSE;
		}		
		
		return TRUE;
	}
	
	static BOOL RegisterHotKey ( CM_HOTKEY * pHotKey, WORD wCmd )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( NULL != pHotKey );
		
		// Register new hotkey.
		CME_CODE lRet = ::CMRegisterHotKey ( pHotKey, wCmd );
		
		if ( CME_FAILED ( lRet ) ) {
			
			return FALSE;
		}		
		
		return TRUE;
	}
	
	static BOOL UnregisterHotKey ( CM_HOTKEY * pHotKey )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( NULL != pHotKey );
		
		// Register new hotkey.
		CME_CODE lRet = ::CMUnregisterHotKey ( pHotKey );
		
		if ( CME_FAILED ( lRet ) ) {
			
			return FALSE;	
		}		
		
		return TRUE;
	}
	
	static void ResetDefaultHotKeys()
	{
		ATLASSERT ( m_bInitialized );
		
		::CMResetDefaultHotKeys (); // do reset
	}
	
	static int GetHotKeysForCmd ( WORD wCmd, CM_HOTKEY * pHotKeys )
	{
		ATLASSERT ( m_bInitialized );
		// ATLASSERT ( NULL != pHotKey );
		// No check because if pHotKeys = NULL, the total number of hotkeys is 
		// returned, and can be used to allocate a sufficiently sized array.
		
		// Retrieve a list of hotkeys assigned to a specific CodeSense Command.
		return ::CMGetHotKeysForCmd ( wCmd, pHotKeys );
	}
	
	//
	// Return the name or description of a specific CodeSense editing command.  
	//

	static void GetCommandString ( WORD wCmd, BOOL bDescription, LPTSTR pszBuff, int nBuffLen )
	{	
		ATLASSERT ( m_bInitialized );
		
		::CMGetCommandString ( wCmd, bDescription, pszBuff, nBuffLen );
	}

#ifdef __ATLSTR_H__
	static void GetCommandString ( WORD wCmd, BOOL bDescription, CString & sBuff )
	{	
		ATLASSERT ( m_bInitialized );
		
		int nBuffLen = bDescription ? CM_MAX_CMD_DESCRIPTION : CM_MAX_CMD_STRING;
		::CMGetCommandString ( wCmd, bDescription, sBuff.GetBuffer ( nBuffLen ), nBuffLen );
		sBuff.ReleaseBuffer ();
	}
#endif // __ATLSTR_H__
	
	//
	// Functions to get a description of the requested command
	//

	static int GetMacro ( int nMacro, LPBYTE pMacroBuff )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( nMacro <= CM_MAX_MACROS );
		// ATLASSERT ( NULL != pMacroBuff );
		// No check because if pMacroBuff = NULL, the required buffer size
		// is returned so that a buffer can be sufficiently allocated. 
		
		return ::CMGetMacro ( nMacro, pMacroBuff );
	}
	
	static int SetMacro ( int nMacro, const LPBYTE pMacroBuff )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( nMacro <= CM_MAX_MACROS );
		ATLASSERT ( NULL != pMacroBuff );
		
		return ::CMSetMacro ( nMacro, pMacroBuff );
	}
	
	//
	// Language registration functions
	//

	static BOOL RegisterLanguage ( LPCTSTR lpszName, CM_LANGUAGE * pLang )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( NULL != lpszName );
		ATLASSERT ( _tcslen ( lpszName ) <= CM_MAX_LANGUAGE_NAME ); 
		ATLASSERT ( NULL != pLang );
		
		// Register the new language
		CME_CODE lRet = ::CMRegisterLanguage ( lpszName, pLang );	
		
		if ( CME_FAILED ( lRet ) ) {
			
			return FALSE;
		}		
		
		return TRUE;	
	}

	static BOOL UnregisterLanguage ( LPCTSTR lpszName )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( NULL != lpszName );
		
		// Unregister requested language
		CME_CODE lRet = ::CMUnregisterLanguage ( lpszName );
		
		if ( CME_FAILED ( lRet ) ) {
			
			return FALSE;
		}		
		
		return TRUE;
	}

	static BOOL UnregisterAllLanguages ()
	{
		ATLASSERT ( m_bInitialized );
		
		// Unregister all languages
		CME_CODE lRet = ::CMUnregisterAllLanguages ();
		
		if ( CME_FAILED ( lRet ) ) {
			
			return FALSE;
		}		
		
		return TRUE;
	}
	
	static int GetLanguageDef ( LPCTSTR lpszName, CM_LANGUAGE * pLang )
	{
		ATLASSERT ( m_bInitialized );
		ATLASSERT ( NULL != lpszName );
		ATLASSERT ( NULL != pLang );
		
		return ::CMGetLanguageDef ( lpszName, pLang );
	}
	
};

// static data init.
__declspec ( selectany ) bool CodeSenseLibrary::m_bInitialized = false;

/////////////////////////////////////////////////////////////////////////////
// CodeSenseControlT : a wraper for the CodeSense window control

template < class TBase >
class CodeSenseControlT : public TBase 
{
	
// Constructors / Destructors
public:
	
	CodeSenseControlT ( HWND hWnd = NULL ) : TBase ( hWnd ) 
	{ /**/ };

	virtual ~CodeSenseControlT () 
	{ /**/ };
	
	CodeSenseControlT < TBase > & operator = ( HWND hWnd )
	{
		m_hWnd = hWnd;
		return (*this);
	}
	
	HWND Create( HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0, ATL::_U_MENUorID MenuOrID = 0U, 
		LPVOID lpCreateParam = NULL)
	{
		return CWindow::Create ( GetWndClassName(), hWndParent, rect.m_lpRect, 
			szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}		
	
// Attributes
public:
	
	static LPCTSTR GetWndClassName ()
	{
		return CODESENSEWNDCLASS;
	}
	
// Operations 
public:
	
	// Check for regular selection
	virtual BOOL IsSelection () const
	{
		CM_RANGE cmRange;
		::ZeroMemory ( &cmRange, sizeof ( cmRange ) );
		
		// Get the current selection	
		if ( CME_FAILED ( GetSel ( &cmRange ) ) ) {
			
			ATLTRACE ( _T ( "Failed to get current selection\n" ) );

			return FALSE;
		}
		
		// Check if there is a selection
		BOOL bSelection = !(cmRange.posStart.nLine == cmRange.posEnd.nLine && 
							cmRange.posStart.nCol == cmRange.posEnd.nCol);
		
		// Done...
		return bSelection;
	}
	
	// Check for regular selection and column selection
	virtual BOOL IsSelection ( BOOL & bColumnSelection ) const
	{
		CM_RANGE cmRange;
		::ZeroMemory ( &cmRange, sizeof ( cmRange ) );
		
		// Get the current selection	
		if ( CME_FAILED ( GetSel ( &cmRange ) ) ) {
			
			ATLTRACE ( _T ( "Failed to get current selection\n" ) );

			return FALSE;
		}

		// Is column selection
		bColumnSelection = cmRange.bColumnSel;
		
		// Check if there is a selection
		BOOL bSelection = !(cmRange.posStart.nLine == cmRange.posEnd.nLine && 
							cmRange.posStart.nCol == cmRange.posEnd.nCol);
		
		// Done...
		return bSelection;
	}

	virtual BOOL InsertTextBlock ( LPTSTR lpszText )
	{
		TCHAR szDelims [] = _T ( "\r\n\0" );
		TCHAR * pToken = NULL;
		
		// Extract first line
		pToken = _tcstok ( lpszText, szDelims );

		// Begin an undo block
		UndoBlock block ( this );

		while ( NULL != pToken ) {

			// Insert extacted line
			if ( ReplaceText ( pToken ) ) {

				return FALSE;
			}

			// Insert a new-line character in the buffer
			NewLine ();

			// Get next line
			pToken = _tcstok ( NULL, szDelims );
		}

		return TRUE;
		
	}

// Messages
public:
	
	CME_CODE SetLanguage( LPCTSTR pszName )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetLanguage( m_hWnd, pszName ); }
	CME_CODE GetLanguage( LPTSTR pszName ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetLanguage( m_hWnd, pszName ); }
	CME_CODE EnableColorSyntax( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableColorSyntax( m_hWnd, bEnable ); }
	BOOL IsColorSyntaxEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsColorSyntaxEnabled( m_hWnd ); }
	CME_CODE OpenFile( LPCTSTR pszFileName )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_OpenFile( m_hWnd, pszFileName ); }
	CME_CODE InsertFile( LPCTSTR pszFileName, const CM_POSITION * pPos = NULL )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_InsertFile( m_hWnd, pszFileName, pPos ); }
	CME_CODE InsertText( LPCTSTR pszText, const CM_POSITION * pPos = NULL )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_InsertText( m_hWnd, pszText, pPos ); }
	CME_CODE SetText( LPCTSTR pszText )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetText( m_hWnd, pszText ); }
	CME_CODE SetColors( const CM_COLORS * pColors )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetColors( m_hWnd, pColors ); }
	CME_CODE GetColors( CM_COLORS * pColors ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetColors( m_hWnd, pColors ); }
	CME_CODE EnableWhitespaceDisplay( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableWhitespaceDisplay( m_hWnd, bEnable ); }
	BOOL IsWhitespaceDisplayEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsWhitespaceDisplayEnabled( m_hWnd ); }
	CME_CODE EnableTabExpand( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableTabExpand( m_hWnd, bEnable ); }
	BOOL IsTabExpandEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsTabExpandEnabled( m_hWnd ); }
	CME_CODE EnableSmoothScrolling( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableSmoothScrolling( m_hWnd, bEnable ); }
	BOOL IsSmoothScrollingEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsSmoothScrollingEnabled( m_hWnd ); }
	CME_CODE SetTabSize( int nTabSize )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetTabSize( m_hWnd, nTabSize ); }
	int GetTabSize() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetTabSize( m_hWnd ); }
	CME_CODE SetReadOnly( BOOL bReadOnly = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetReadOnly( m_hWnd, bReadOnly ); }
	BOOL IsReadOnly() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsReadOnly( m_hWnd ); }
	CME_CODE EnableLineToolTips( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableLineToolTips( m_hWnd, bEnable ); }
	BOOL IsLineToolTipsEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsLineToolTipsEnabled( m_hWnd ); }
	CME_CODE EnableLeftMargin( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableLeftMargin( m_hWnd, bEnable ); }
	BOOL IsLeftMarginEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsLeftMarginEnabled( m_hWnd ); }
	CME_CODE SaveFile( LPCTSTR pszFileName, BOOL bClearUndo = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SaveFile( m_hWnd, pszFileName, bClearUndo ); }
	CME_CODE ReplaceText( LPCTSTR pszText, const CM_RANGE * pRange = NULL )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_ReplaceText( m_hWnd, pszText, pRange ); }
	int GetTextLength( const CM_RANGE * pRange = NULL, BOOL bLogical = FALSE ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetTextLength( m_hWnd, pRange, bLogical ); }
	CME_CODE GetText( LPTSTR pszBuff, const CM_RANGE * pRange = NULL ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetText( m_hWnd, pszBuff, pRange ); }
	
#ifdef __ATLSTR_H__
	CME_CODE GetText( CString & strText, const CM_RANGE * pRange = NULL ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) );
		CME_CODE nCode = CM_GetText( m_hWnd, strText.GetBufferSetLength ( GetTextLength( pRange ) + 1 ), pRange );
		strText.ReleaseBuffer();
		return nCode; }
#endif // __ATLSTR_H__
	
	CME_CODE GetLine( int nLine, LPTSTR pszBuff ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetLine ( m_hWnd, nLine, pszBuff ); }
	
#ifdef __ATLSTR_H__
	CME_CODE GetLine( int nLine, CString & strLine ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) );
		CME_CODE nCode = CM_GetLine ( m_hWnd, nLine, strLine.GetBufferSetLength ( GetLineLength ( nLine ) + 1 ) );
		strLine.ReleaseBuffer ();
		return nCode; }
#endif // __ATLSTR_H__
	
	int GetLineCount() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetLineCount ( m_hWnd ); }
	int GetLineLength( int nLine, BOOL bLogical = FALSE ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetLineLength ( m_hWnd, nLine, bLogical ); }
	CME_CODE GetWord( LPTSTR pszBuff, CM_POSITION * pPos ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetWord ( m_hWnd, pszBuff, pPos ); }
	
#ifdef __ATLSTR_H__
	CME_CODE GetWord ( CString & strWord, CM_POSITION * pPos ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) );
		CME_CODE nCode = CM_GetWord( m_hWnd, strWord.GetBufferSetLength ( GetWordLength ( pPos ) + 1 ), pPos );
		strWord.ReleaseBuffer ();
		return nCode; }
#endif // __ATLSTR_H__
	
	int GetWordLength( CM_POSITION * pPos, BOOL bLogical = FALSE ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetWordLength ( m_hWnd, pPos, bLogical ); }
	CME_CODE AddText( LPCTSTR pszText )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_AddText ( m_hWnd, pszText ); }
	CME_CODE DeleteLine( int nLine )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_DeleteLine ( m_hWnd, nLine ); }
	CME_CODE InsertLine( int nLine, LPCTSTR pszText )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_InsertLine ( m_hWnd, nLine, pszText ); }
	CME_CODE GetSel( CM_RANGE * pRange, BOOL bNormalized = TRUE ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetSel ( m_hWnd, pRange, bNormalized ); }
	CME_CODE SetSel( const CM_RANGE * pRange, BOOL bMakeVisible = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetSel ( m_hWnd, pRange, bMakeVisible ); }
	CME_CODE DeleteSel()
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_DeleteSel ( m_hWnd ); }
	CME_CODE ReplaceSel( LPCTSTR pszText )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_ReplaceSel ( m_hWnd, pszText ); }
	CME_CODE ExecuteCmd( WORD wCmd, DWORD dwCmdData = 0 )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) );
		ATLASSERT ( wCmd >= CMD_FIRST && wCmd <= CMD_LAST );
		return CM_ExecuteCmd( m_hWnd, wCmd, dwCmdData ); }
	CME_CODE SetSplitterPos( BOOL bHorz, int nPos )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetSplitterPos ( m_hWnd, bHorz, nPos ); }
	int GetSplitterPos( BOOL bHorz ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetSplitterPos ( m_hWnd, bHorz ); }
	CME_CODE SetAutoIndentMode( int nMode )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetAutoIndentMode ( m_hWnd, nMode ); }
	int GetAutoIndentMode() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetAutoIndentMode ( m_hWnd ); }
	BOOL CanCopy() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_CanCopy ( m_hWnd ); }
	BOOL CanCut() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_CanCut ( m_hWnd ); }
	BOOL CanPaste() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_CanPaste ( m_hWnd ); }
	CME_CODE Copy() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_Copy ( m_hWnd ); }
	CME_CODE Cut()
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_Cut ( m_hWnd ); }
	CME_CODE Paste()
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_Paste ( m_hWnd ); }
	BOOL CanUndo() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_CanUndo ( m_hWnd ); }
	BOOL CanRedo() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_CanRedo ( m_hWnd ); }
	CME_CODE Redo()
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_Redo ( m_hWnd ); }
	CME_CODE Undo()
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_Undo ( m_hWnd ); }
	CME_CODE ClearUndoBuffer()
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_ClearUndoBuffer ( m_hWnd ); }
	CME_CODE SetUndoLimit( int nLimit )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetUndoLimit ( m_hWnd, nLimit ); }
	int GetUndoLimit() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetUndoLimit ( m_hWnd ); }
	BOOL IsModified() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsModified ( m_hWnd ); }
	CME_CODE SetModified( BOOL bModified = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetModified ( m_hWnd, bModified ); }
	CME_CODE EnableOvertype( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableOvertype ( m_hWnd, bEnable ); }
	BOOL IsOvertypeEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsOvertypeEnabled ( m_hWnd ); }
	CME_CODE EnableCaseSensitive( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableCaseSensitive ( m_hWnd, bEnable ); }
	BOOL IsCaseSensitiveEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsCaseSensitiveEnabled ( m_hWnd ); }
	CME_CODE EnablePreserveCase( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnablePreserveCase ( m_hWnd, bEnable ); }
	BOOL IsPreserveCaseEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsPreserveCaseEnabled ( m_hWnd ); }
	CME_CODE EnableWholeWord( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableWholeWord ( m_hWnd, bEnable ); }
	BOOL IsWholeWordEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsWholeWordEnabled ( m_hWnd ); }
	CME_CODE EnableRegExp( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableRegExp ( m_hWnd, bEnable ); }
	BOOL IsRegExpEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsRegExpEnabled ( m_hWnd ); }
	CME_CODE SetTopIndex( int nView, int nLine )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetTopIndex ( m_hWnd, nView, nLine ); }
	int GetTopIndex( int nView ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetTopIndex ( m_hWnd, nView ); }
	int GetVisibleLineCount( int nView, BOOL bFullyVisible = TRUE ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetVisibleLineCount ( m_hWnd, nView, bFullyVisible ); }
	CME_CODE EnableCRLF( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableCRLF ( m_hWnd, bEnable ); }
	BOOL IsCRLFEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsCRLFEnabled ( m_hWnd ); }
	CME_CODE SetFontOwnership( BOOL bEnable )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetFontOwnership ( m_hWnd, bEnable ); }
	BOOL GetFontOwnership() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetFontOwnership ( m_hWnd ); }
	int GetCurrentView() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetCurrentView ( m_hWnd ); }
	CME_CODE SetCurrentView(int nLine)
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetCurrentView ( m_hWnd, nLine ); }
	int GetViewCount() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetViewCount ( m_hWnd ); }
	CME_CODE ShowScrollBar( BOOL bHorz, BOOL bShow = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_ShowScrollBar ( m_hWnd, bHorz, bShow ); }
	BOOL HasScrollBar( BOOL bHorz ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_HasScrollBar ( m_hWnd, bHorz ); }
	CME_CODE GetSelFromPoint( int xClient, int yClient, CM_POSITION * pPos ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetSelFromPoint ( m_hWnd, xClient, yClient, pPos ); }
	CME_CODE SelectLine( int nLine, BOOL bMakeVisible = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SelectLine ( m_hWnd, nLine, bMakeVisible ); }
	int HitTest( int xClient, int yClient ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_HitTest ( m_hWnd, xClient, yClient ); }
	CME_CODE EnableDragDrop( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableDragDrop ( m_hWnd, bEnable ); }
	BOOL IsDragDropEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsDragDropEnabled ( m_hWnd ); }
	CME_CODE EnableSplitter( BOOL bHorz, BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableSplitter ( m_hWnd, bHorz, bEnable ); }
	BOOL IsSplitterEnabled( BOOL bHorz ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsSplitterEnabled ( m_hWnd, bHorz ); }
	CME_CODE EnableColumnSel( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableColumnSel ( m_hWnd, bEnable ); }
	BOOL IsColumnSelEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsColumnSelEnabled ( m_hWnd ); }
	CME_CODE EnableGlobalProps( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableGlobalProps ( m_hWnd, bEnable ); }
	BOOL IsGlobalPropsEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsGlobalPropsEnabled ( m_hWnd ); }
	BOOL IsRecordingMacro() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsRecordingMacro ( m_hWnd ); }
	BOOL IsPlayingMacro() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsPlayingMacro ( m_hWnd ); }
	CME_CODE SetDlgParent( CWindow * pWnd ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetDlgParent ( m_hWnd, pWnd->m_hWnd ); }
	CME_CODE EnableSelBounds( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableSelBounds ( m_hWnd, bEnable ); }
	BOOL IsSelBoundsEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsSelBoundsEnabled ( m_hWnd ); }
	CME_CODE SetFontStyles( CM_FONTSTYLES * pFontStyles )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetFontStyles ( m_hWnd, pFontStyles ); }
	CME_CODE GetFontStyles( CM_FONTSTYLES * pFontStyles ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetFontStyles ( m_hWnd, pFontStyles ); }
	CME_CODE SetItemData( int nLine, LPARAM lParam )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetItemData ( m_hWnd, nLine, lParam ); }
	LPARAM GetItemData( int nLine ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetItemData ( m_hWnd, nLine ); }
	CME_CODE SetLineStyle( int nLine, DWORD dwStyle )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetLineStyle ( m_hWnd, nLine, dwStyle ); }
	DWORD GetLineStyle( int nLine ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetLineStyle ( m_hWnd, nLine ); }
	CME_CODE SetBookmark( int nLine, BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetBookmark ( m_hWnd, nLine, bEnable ); }
	BOOL GetBookmark( int nLine ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetBookmark ( m_hWnd, nLine ); }
	CME_CODE SetAllBookmarks( int nCount, LPDWORD pdwLines )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetAllBookmarks ( m_hWnd, nCount, pdwLines ); }
	int GetAllBookmarks( LPDWORD pdwLines ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetAllBookmarks ( m_hWnd, pdwLines ); }
	CME_CODE SetLineNumbering( CM_LINENUMBERING * pNumbering )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetLineNumbering ( m_hWnd, pNumbering ); }
	CME_CODE GetLineNumbering( CM_LINENUMBERING * pNumbering ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetLineNumbering ( m_hWnd, pNumbering ); }
	CME_CODE PosFromChar ( CM_POSITION * pPos, LPRECT pRect ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_PosFromChar ( m_hWnd, pPos, pRect ); }
	CME_CODE EnableHideSel( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableHideSel ( m_hWnd, bEnable ); }
	BOOL IsHideSelEnabled() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsHideSelEnabled ( m_hWnd ); }
	CME_CODE SetHighlightedLine( int nLine )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetHighlightedLine ( m_hWnd, nLine ); }
	int GetHighlightedLine() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetHighlightedLine ( m_hWnd ); }
	CME_CODE EnableNormalizeCase ( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableNormalizeCase ( m_hWnd, bEnable ); }
	BOOL IsNormalizeCaseEnabled () const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsNormalizeCaseEnabled( m_hWnd ); }
	CME_CODE SetDivider ( int nLine, BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetDivider ( m_hWnd, nLine, bEnable ); }
	BOOL GetDivider( int nLine ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetDivider ( m_hWnd, nLine ); }
	CME_CODE EnableOvertypeCaret ( BOOL bEnable = TRUE )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_EnableOvertypeCaret ( m_hWnd, bEnable ); }
	BOOL IsOvertypeCaretEnabled () const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_IsOvertypeCaretEnabled ( m_hWnd ); }
	CME_CODE SetFindText ( LPCTSTR pszText )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetFindText ( m_hWnd, pszText ); }
	CME_CODE GetFindText ( LPTSTR pszText ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetFindText ( m_hWnd, pszText ); }
	
#ifdef __ATLSTR_H__
	CME_CODE GetFindText( CString & strText ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) );
		CME_CODE nCode = CM_GetFindText( m_hWnd, strText.GetBufferSetLength ( CM_MAX_FINDREPL_TEXT ) );
		strText.ReleaseBuffer ();
		return nCode; }
#endif // __ATLSTR_H__
	
	CME_CODE SetReplaceText( LPCTSTR pszText )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetReplaceText ( m_hWnd, pszText ); }
	CME_CODE GetReplaceText( LPTSTR pszText ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetReplaceText ( m_hWnd, pszText ); }
	
#ifdef __ATLSTR_H__
	CME_CODE GetReplaceText( CString & strText ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) );
		CME_CODE nCode = CM_GetReplaceText ( m_hWnd, strText.GetBufferSetLength ( CM_MAX_FINDREPL_TEXT ) );
		strText.ReleaseBuffer ();
		return nCode; }
#endif // __ATLSTR_H__
	
	CME_CODE SetImageList( HIMAGELIST hImageList )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetImageList ( m_hWnd, hImageList ); }
	HIMAGELIST GetImageList() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetImageList ( m_hWnd ); }
	CME_CODE SetMarginImages( int nLine, BYTE byImages )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetMarginImages ( m_hWnd, nLine, byImages ); }
	BYTE GetMarginImages( int nLine ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetMarginImages ( m_hWnd, nLine ); }
	CME_CODE AboutBox() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_AboutBox ( m_hWnd ); }
	CME_CODE Print( HDC hDC, DWORD dwFlags ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_Print ( m_hWnd, hDC, dwFlags ); }
	CME_CODE SetCaretPos( int nLine, int nCol )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetCaretPos ( m_hWnd, nLine, nCol ); }
	int ViewColToBufferCol( int nLine, int nViewCol ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_ViewColToBufferCol ( m_hWnd, nLine, nViewCol ); }
	int BufferColToViewCol( int nLine, int nBufferCol ) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_BufferColToViewCol ( m_hWnd, nLine, nBufferCol ); }
	CME_CODE SetBorderStyle( DWORD dwStyle )
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_SetBorderStyle ( m_hWnd, dwStyle ); }
	DWORD GetBorderStyle() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetBorderStyle ( m_hWnd ); }

	//
	// CodeSense 2.1.0.22
	//
	DWORD GetCurrentToken() const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetCurrentToken ( m_hWnd ); }
	void UpdateControlPositions()
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); CM_UpdateControlPositions ( m_hWnd ); }

	//
	// CodeSense 2.1.0.22 (Kronuz)
	//
	DWORD GetTokenAt(int col, int row) const
	{ ATLASSERT ( ::IsWindow ( m_hWnd ) ); return CM_GetTokenAt ( m_hWnd, col, row); }

// Commands
public:
	LRESULT WordUpperCase ()
	{ return ExecuteCmd ( CMD_WORDUPPERCASE ); }
	LRESULT WordTranspose ()
	{ return ExecuteCmd ( CMD_WORDTRANSPOSE ); }
	LRESULT WordRightExtend ()
	{ return ExecuteCmd ( CMD_WORDRIGHTEXTEND ); }
	LRESULT WordRight ()
	{ return ExecuteCmd ( CMD_WORDRIGHT ); }
	LRESULT WordEndRight ()
	{ return ExecuteCmd ( CMD_WORDENDRIGHT ); }
	LRESULT WordEndRightExtend ()
	{ return ExecuteCmd ( CMD_WORDENDRIGHTEXTEND ); }	
	LRESULT WordLowerCase ()
	{ return ExecuteCmd ( CMD_WORDLOWERCASE ); }	
	LRESULT WordLeftExtend ()
	{ return ExecuteCmd ( CMD_WORDLEFTEXTEND ); }	
	LRESULT WordLeft ()
	{ return ExecuteCmd ( CMD_WORDLEFT ); }
	LRESULT WordEndLeft ()
	{ return ExecuteCmd ( CMD_WORDENDLEFT ); }	
	LRESULT WordEndLeftExtend ()
	{ return ExecuteCmd ( CMD_WORDENDLEFTEXTEND ); }
	LRESULT WordDeleteToStart ()
	{ return ExecuteCmd ( CMD_WORDDELETETOSTART ); }
	LRESULT WordDeleteToEnd ()
	{ return ExecuteCmd ( CMD_WORDDELETETOEND ); }	
	LRESULT WordCapitalize ()
	{ return ExecuteCmd ( CMD_WORDCAPITALIZE ); }	
	LRESULT WindowStart ()
	{ return ExecuteCmd ( CMD_WINDOWSTART ); }
	LRESULT WindowScrollUp ()
	{ return ExecuteCmd ( CMD_WINDOWSCROLLUP ); }
	LRESULT WindowScrollToTop ()
	{ return ExecuteCmd ( CMD_WINDOWSCROLLTOTOP ); }
	LRESULT WindowScrollToCenter ()
	{ return ExecuteCmd ( CMD_WINDOWSCROLLTOCENTER ); }
	LRESULT WindowScrollToBottom ()
	{ return ExecuteCmd ( CMD_WINDOWSCROLLTOBOTTOM ); }
	LRESULT WindowScrollRight ()
	{ return ExecuteCmd ( CMD_WINDOWSCROLLRIGHT ); }
	LRESULT WindowScrollLeft ()
	{ return ExecuteCmd ( CMD_WINDOWSCROLLLEFT ); }
	LRESULT WindowScrollDown ()
	{ return ExecuteCmd ( CMD_WINDOWSCROLLDOWN ); }
	LRESULT WindowRightEdge ()
	{ return ExecuteCmd ( CMD_WINDOWRIGHTEDGE ); }
	LRESULT WindowLeftEdge ()
	{ return ExecuteCmd ( CMD_WINDOWLEFTEDGE ); }
	LRESULT WindowEnd ()
	{ return ExecuteCmd ( CMD_WINDOWEND ); }	
	LRESULT UpperCaseSelection ()
	{ return ExecuteCmd ( CMD_UPPERCASESELECTION ); }
	LRESULT UnTabifySelection ()
	{ return ExecuteCmd ( CMD_UNTABIFYSELECTION ); }	
	LRESULT UnIndentSelection ()
	{ return ExecuteCmd ( CMD_UNINDENTSELECTION ); }	
	LRESULT UndoChanges ()
	{ return ExecuteCmd ( CMD_UNDOCHANGES ); }
	// Message is favoured because it will post a notification on an error
	// #define CMD_UNDO                        ( CMD_FIRST + 29 )
	LRESULT TabifySelection ()
	{ return ExecuteCmd ( CMD_TABIFYSELECTION ); }	
	LRESULT SentenceRight ()
	{ return ExecuteCmd ( CMD_SENTENCERIGHT ); }	
	LRESULT SentenceLeft ()
	{ return ExecuteCmd ( CMD_SENTENCELEFT ); }
	LRESULT SentenceCut ()
	{ return ExecuteCmd ( CMD_SENTENCECUT ); }	
	LRESULT SelectSwapAnchor ()
	{ return ExecuteCmd ( CMD_SELECTSWAPANCHOR ); }	
	LRESULT SelectPara ()
	{ return ExecuteCmd ( CMD_SELECTPARA ); }
	LRESULT SelectLine ()
	{ return ExecuteCmd ( CMD_SELECTLINE ); }
	LRESULT SelectAll ()
	{ return ExecuteCmd ( CMD_SELECTALL ); }	
	LRESULT RedoChanges ()
	{ return ExecuteCmd ( CMD_REDOCHANGES ); }
	// Messages are favoured because they will post a notification on an error
	// #define CMD_REDO                        ( CMD_FIRST + 39 )
	// #define CMD_PASTE                       ( CMD_FIRST + 40 )	
	LRESULT ParaUp ()
	{ return ExecuteCmd ( CMD_PARAUP ); }
	LRESULT ParaDown ()
	{ return ExecuteCmd ( CMD_PARADOWN ); }
	LRESULT PageUpExtend ()
	{ return ExecuteCmd ( CMD_PAGEUPEXTEND ); }
	LRESULT PageUp ()
	{ return ExecuteCmd ( CMD_PAGEUP ); }
	LRESULT PageDownExtend ()
	{ return ExecuteCmd ( CMD_PAGEDOWNEXTEND ); }
	LRESULT PageDown ()
	{ return ExecuteCmd ( CMD_PAGEDOWN ); }
	LRESULT LowerCaseSelection ()
	{ return ExecuteCmd ( CMD_LOWERCASESELECTION ); }
	LRESULT LineUpExtend ()
	{ return ExecuteCmd ( CMD_LINEUPEXTEND ); }
	LRESULT LineUp ()
	{ return ExecuteCmd ( CMD_LINEUP ); }
	LRESULT LineTranspose ()
	{ return ExecuteCmd ( CMD_LINETRANSPOSE ); }
	LRESULT LineStart ()
	{ return ExecuteCmd ( CMD_LINESTART ); }
	LRESULT LineOpenBelow ()
	{ return ExecuteCmd ( CMD_LINEOPENBELOW ); }
	LRESULT LineOpenAbove ()
	{ return ExecuteCmd ( CMD_LINEOPENABOVE ); }
	LRESULT LineEndExtend ()
	{ return ExecuteCmd ( CMD_LINEENDEXTEND ); }
	LRESULT LineEnd ()
	{ return ExecuteCmd ( CMD_LINEEND ); }
	LRESULT LineDownExtend ()
	{ return ExecuteCmd ( CMD_LINEDOWNEXTEND ); }
	LRESULT LineDown ()
	{ return ExecuteCmd ( CMD_LINEDOWN ); }
	LRESULT LineDeleteToStart ()
	{ return ExecuteCmd ( CMD_LINEDELETETOSTART ); }
	LRESULT LineDeleteToEnd ()
	{ return ExecuteCmd ( CMD_LINEDELETETOEND ); }
	LRESULT LineDelete ()
	{ return ExecuteCmd ( CMD_LINEDELETE ); }
	LRESULT LineCut ()
	{ return ExecuteCmd ( CMD_LINECUT ); }
	LRESULT IndentToPrev ()
	{ return ExecuteCmd ( CMD_INDENTTOPREV ); }
	LRESULT IndentSelection ()
	{ return ExecuteCmd ( CMD_INDENTSELECTION ); }
	LRESULT HomeExtend ()
	{ return ExecuteCmd ( CMD_HOMEEXTEND ); }
	LRESULT Home ()
	{ return ExecuteCmd ( CMD_HOME ); }
	LRESULT GotoMatchBrace ()
	{ return ExecuteCmd ( CMD_GOTOMATCHBRACE ); }
	LRESULT GotoIndentation ()
	{ return ExecuteCmd ( CMD_GOTOINDENTATION ); }
	LRESULT GotoLine ( const int nLine = -1 )
	{ return ExecuteCmd ( CMD_GOTOLINE, nLine ); }	
	LRESULT FindReplace ()
	{ return ExecuteCmd ( CMD_FINDREPLACE ); }
	LRESULT Replace ()
	{ return ExecuteCmd ( CMD_REPLACE ); }
	LRESULT ReplaceAllInbuffer ()
	{ return ExecuteCmd ( CMD_REPLACEALLINBUFFER ); }
	LRESULT ReplaceAllInselection ()
	{ return ExecuteCmd ( CMD_REPLACEALLINSELECTION ); }
	LRESULT FindPrevWord ()
	{ return ExecuteCmd ( CMD_FINDPREVWORD ); }
	LRESULT FindPrev ()
	{ return ExecuteCmd ( CMD_FINDPREV ); }
	LRESULT FindNextWord ()
	{ return ExecuteCmd ( CMD_FINDNEXTWORD ); }
	LRESULT FindNext ()
	{ return ExecuteCmd ( CMD_FINDNEXT ); }
	LRESULT FindMarkAll ()
	{ return ExecuteCmd ( CMD_FINDMARKALL ); }
	LRESULT Find ()
	{ return ExecuteCmd ( CMD_FIND ); }
	// Messages are favoured because they will post a notification on an error
	// #define CMD_SETFINDTEXT                 ( CMD_FIRST + 79 )
	// #define CMD_SETREPLACETEXT              ( CMD_FIRST + 80 )	
	LRESULT TogglePreserveCase ()
	{ return ExecuteCmd ( CMD_TOGGLEPRESERVECASE ); }
	LRESULT ToggleWholeWord ()
	{ return ExecuteCmd ( CMD_TOGGLEWHOLEWORD ); }
	LRESULT ToggleCaseSensitive ()
	{ return ExecuteCmd ( CMD_TOGGLECASESENSITIVE ); }
	LRESULT End ()
	{ return ExecuteCmd ( CMD_END ); }
	LRESULT ToggleWhiteSpaceDisplay ()
	{ return ExecuteCmd ( CMD_TOGGLEWHITESPACEDISPLAY ); }
	LRESULT ToggleOvertype ()
	{ return ExecuteCmd ( CMD_TOGGLEOVERTYPE ); }
	LRESULT SetRepeatCount ()
	{ return ExecuteCmd ( CMD_SETREPEATCOUNT ); }	
	LRESULT DocumentStartExtend ()
	{ return ExecuteCmd ( CMD_DOCUMENTSTARTEXTEND ); }
	LRESULT DocumentStart ()
	{ return ExecuteCmd ( CMD_DOCUMENTSTART ); }
	LRESULT DocumentEndExtend ()
	{ return ExecuteCmd ( CMD_DOCUMENTENDEXTEND ); }
	LRESULT DocumentEnd ()
	{ return ExecuteCmd ( CMD_DOCUMENTEND ); }
	LRESULT DeleteHorizontalSpace ()
	{ return ExecuteCmd ( CMD_DELETEHORIZONTALSPACE ); }
	LRESULT DeleteBlankLine ()
	{ return ExecuteCmd ( CMD_DELETEBLANKLINES ); }
	LRESULT DeleteBack ()
	{ return ExecuteCmd ( CMD_DELETEBACK ); }
	LRESULT Delete ()
	{ return ExecuteCmd ( CMD_DELETE ); }
	LRESULT CutSelection ()
	{ return ExecuteCmd ( CMD_CUTSELECTION ); }
	// Messages are favoured because they will post a notification on an error
	// #define CMD_CUT                         ( CMD_FIRST + 97 )
	// #define CMD_COPY                        ( CMD_FIRST + 98 )
	LRESULT CharTranspose ()
	{ return ExecuteCmd ( CMD_CHARTRANSPOSE ); }
	LRESULT CharrightExtend ()
	{ return ExecuteCmd ( CMD_CHARRIGHTEXTEND ); }
	LRESULT CharRight ()
	{ return ExecuteCmd ( CMD_CHARRIGHT ); }
	LRESULT CharLeftExtend ()
	{ return ExecuteCmd ( CMD_CHARLEFTEXTEND ); }
	LRESULT CharLeft ()
	{ return ExecuteCmd ( CMD_CHARLEFT ); }
	LRESULT BookmarkToggle ()
	{ return ExecuteCmd ( CMD_BOOKMARKTOGGLE ); }
	LRESULT BookmarkPrev ()
	{ return ExecuteCmd ( CMD_BOOKMARKPREV ); }
	LRESULT BookmarkNext ()
	{ return ExecuteCmd ( CMD_BOOKMARKNEXT ); }
	LRESULT BookmarkClearAll ()
	{ return ExecuteCmd ( CMD_BOOKMARKCLEARALL ); }
	LRESULT BookMarkJumpToFirst ()
	{ return ExecuteCmd ( CMD_BOOKMARKJUMPTOFIRST ); }
	LRESULT BookMarkJumpToLast ()
	{ return ExecuteCmd ( CMD_BOOKMARKJUMPTOLAST ); }	
	LRESULT AppendNextCut ()
	{ return ExecuteCmd ( CMD_APPENDNEXTCUT ); }
	LRESULT InsertChar ( const TCHAR cChar )
	{ return ExecuteCmd ( CMD_INSERTCHAR,  static_cast < DWORD > ( cChar ) ); }
	LRESULT NewLine ()
	{ return ExecuteCmd ( CMD_NEWLINE ); }
	LRESULT RecordMacro ()
	{ return ExecuteCmd ( CMD_RECORDMACRO ); }
	LRESULT PlayMacro1 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO1 ); }
	LRESULT PlayMacro2 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO2 ); }
	LRESULT PlayMacro3 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO3 ); }
	LRESULT PlayMacro4 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO4 ); }
	LRESULT PlayMacro5 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO5 ); }
	LRESULT PlayMacro6 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO6 ); }
	LRESULT PlayMacro7 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO7 ); }
	LRESULT PlayMacro8 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO8 ); }
	LRESULT PlayMacro9 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO9 ); }
	LRESULT PlayMacro10 ()
	{ return ExecuteCmd ( CMD_PLAYMACRO10 ); }	
	// Play macro
	//  param1 - the macro to be played (zero based index)
	LRESULT PlayMacro ( int nMacro ) // zero based ( 0, 1, ..., 9 )
	{ ATLASSERT ( ( CMD_PLAYMACRO1 + nMacro ) >= CMD_PLAYMACRO1 && 
		( CMD_PLAYMACRO1 + nMacro ) <= CMD_PLAYMACRO10 ); 
		return ExecuteCmd ( CMD_PLAYMACRO1 + nMacro ); }	
	LRESULT Properties ()
	{ return ExecuteCmd ( CMD_PROPERTIES ); }		
	LRESULT BeginUndo ()
	{ return ExecuteCmd ( CMD_BEGINUNDO ); }
	LRESULT EndUndo ()
	{ return ExecuteCmd ( CMD_ENDUNDO ); }	
	
	//
	// CodeMax Version 2.0
	//

	LRESULT ToggleRegExp ()
	{ return ExecuteCmd ( CMD_TOGGLEREGEXP ); }	
	LRESULT ClearSelection ()
	{ return ExecuteCmd ( CMD_CLEARSELECTION ); }
	LRESULT RegExpOn ()
	{ return ExecuteCmd ( CMD_REGEXPON ); }
	LRESULT RegExpOff ()
	{ return ExecuteCmd ( CMD_REGEXPOFF ); }	
	LRESULT WholeWordOn ()
	{ return ExecuteCmd ( CMD_WHOLEWORDON ); }
	LRESULT WholeWordOff ()
	{ return ExecuteCmd ( CMD_WHOLEWORDOFF ); }	
	LRESULT PreserveCaseOn ()
	{ return ExecuteCmd ( CMD_PRESERVECASEON ); }
	LRESULT PreserveCaseOff ()
	{ return ExecuteCmd ( CMD_PRESERVECASEOFF ); }	
	LRESULT WhiteSpaceDisplayOn ()
	{ return ExecuteCmd ( CMD_WHITESPACEDISPLAYON ); }
	LRESULT WhiteSpaceDisplayOff ()
	{ return ExecuteCmd ( CMD_WHITESPACEDISPLAYOFF ); }	
	LRESULT OvertypeOn ()
	{ return ExecuteCmd ( CMD_OVERTYPEON ); }
	LRESULT OverTypeOff ()
	{ return ExecuteCmd ( CMD_OVERTYPEOFF ); }	

	//
	// CodeSense 2.1.0.22
	//
	LRESULT CodeTip ()
	{ return ExecuteCmd ( CMD_CODETIP ); }
	LRESULT CodeList ()
	{ return ExecuteCmd ( CMD_CODELIST ); }
	LRESULT DestroyCodeTip ()
	{ return ExecuteCmd ( CMD_DESTROYCODETIP ); }

};

// simple typedef for standard use
typedef CodeSenseControlT < CWindow > CodeSenseControl;

////////////////////////////////////////////////////////////////////////////////////
// CodeSenseControlNotifications - notification handlers for CodeSense

// Chain to CodeSenseControlNotifications message map. Your class must also derive from 
//	CodeSenseControl.
// Example:
// class CMyCodeSenseControl : public CWindowImpl < CMyCodeSenseControl, CodeSenseControl >,
//							 public CodeSenseControlNotifications < CMyCodeSenseControl >
// {
// public:
//      BEGIN_MSG_MAP(CMyCodeSenseControl)
//              // your handlers...
//              CHAIN_MSG_MAP_ALT ( CodeSenseControlNotifications < CMyCodeSenseControl >, 1 )
//      END_MSG_MAP()
//      // other stuff...
//
//		// your notification overides
//		void OnNotifyDestroy ()
//		{
//			MessageBox ( _T ( "Bye!!" ) );
//		}
// };

#define CMAX_NOTIFY_CODE_HANDLERS			1
#define CMAX_REFLECTED_NOTIFY_CODE_HANDLERS	2

template < class T >
class CodeSenseControlNotifications
{

public:

	BEGIN_MSG_MAP ( CodeSenseControlNotifications < T > )

	ALT_MSG_MAP ( 1 ) // Start of alternate message map
	
		//
		// Notification handlers
		//

		NOTIFY_CODE_HANDLER ( CMN_CHANGE, _OnChange )
		NOTIFY_CODE_HANDLER ( CMN_HSCROLL, _OnHorizontalScroll )
		NOTIFY_CODE_HANDLER ( CMN_VSCROLL, _OnVerticalScroll )
		NOTIFY_CODE_HANDLER ( CMN_SELCHANGE, _OnSelectionChange )
		NOTIFY_CODE_HANDLER ( CMN_VIEWCHANGE, _OnViewChange )
		NOTIFY_CODE_HANDLER ( CMN_MODIFIEDCHANGE, _OnModifiedChange )
		NOTIFY_CODE_HANDLER ( CMN_SHOWPROPS, _OnShowProperties )
		NOTIFY_CODE_HANDLER ( CMN_PROPSCHANGE, _OnPropertiesChange )
		NOTIFY_CODE_HANDLER ( CMN_CREATE, _OnNotifyCreate )
		NOTIFY_CODE_HANDLER ( CMN_DESTROY, _OnNotifyDestroy )

		//
		// CodeMax Version 2.0
		//

		NOTIFY_CODE_HANDLER ( CMN_DRAWLINE, _OnDrawLine )
		NOTIFY_CODE_HANDLER ( CMN_DELETELINE, _OnDeleteLine )
		NOTIFY_CODE_HANDLER ( CMN_CMDFAILURE, _OnCommandFailure )
		NOTIFY_CODE_HANDLER ( CMN_REGISTEREDCMD, _OnRegisteredCommand )
		NOTIFY_CODE_HANDLER ( CMN_KEYDOWN, _OnKeyDown )
		NOTIFY_CODE_HANDLER ( CMN_KEYUP, _OnKeyUp )
		NOTIFY_CODE_HANDLER ( CMN_KEYPRESS, _OnKeyPress )
		NOTIFY_CODE_HANDLER ( CMN_MOUSEDOWN, _OnMouseDown )
		NOTIFY_CODE_HANDLER ( CMN_MOUSEUP, _OnMouseUp )
		NOTIFY_CODE_HANDLER ( CMN_MOUSEMOVE, _OnMouseMove )
		NOTIFY_CODE_HANDLER ( CMN_OVERTYPECHANGE, _OnOvertypeChange )
		
		//
		// CodeMax Version 2.1
		//		

		NOTIFY_CODE_HANDLER ( CMN_FINDWRAPPED, _OnFindWrapped )

		//
		// CodeSense Version 2.1.0.22
		//		

		NOTIFY_CODE_HANDLER ( CMN_CODELIST, _OnCodeList )
		NOTIFY_CODE_HANDLER ( CMN_CODELISTSELMADE, _OnCodeListSelMade )
		NOTIFY_CODE_HANDLER ( CMN_CODELISTCANCEL, _OnCodeListCancel )
		NOTIFY_CODE_HANDLER ( CMN_CODELISTCHAR, _OnCodeListChar ) // CM_CODELISTKEYDATA

		NOTIFY_CODE_HANDLER ( CMN_CODETIP, _OnCodeTip )
		NOTIFY_CODE_HANDLER ( CMN_CODETIPINITIALIZE, _OnCodeTipInitialize ) 
		NOTIFY_CODE_HANDLER ( CMN_CODETIPCANCEL, _OnCodeTipCancel ) // CM_CODETIPDATA 
		NOTIFY_CODE_HANDLER ( CMN_CODETIPUPDATE, _OnCodeTipUpdate )

		NOTIFY_CODE_HANDLER ( CMN_CODELISTSELWORD, _OnCodeListSelWord )
		NOTIFY_CODE_HANDLER ( CMN_CODELISTSELCHANGE, _OnCodeListSelChange )
		NOTIFY_CODE_HANDLER ( CMN_CODELISTHOTTRACK, _OnCodeListHotTrack )

	ALT_MSG_MAP ( 2 ) // Start of alternate message map ( for reflected messages )

		//
		// Reflected notification handlers
		//

		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CHANGE, _OnChange )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_HSCROLL, _OnHorizontalScroll )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_VSCROLL, _OnVerticalScroll )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_SELCHANGE, _OnSelectionChange )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_VIEWCHANGE, _OnViewChange )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_MODIFIEDCHANGE, _OnModifiedChange )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_SHOWPROPS, _OnShowProperties )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_PROPSCHANGE, _OnPropertiesChange )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CREATE, _OnNotifyCreate )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_DESTROY, _OnNotifyDestroy )

		//
		// CodeMax Version 2.0
		//

		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_DRAWLINE, _OnDrawLine )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_DELETELINE, _OnDeleteLine )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CMDFAILURE, _OnCommandFailure )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_REGISTEREDCMD, _OnRegisteredCommand )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_KEYDOWN, _OnKeyDown )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_KEYUP, _OnKeyUp )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_KEYPRESS, _OnKeyPress )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_MOUSEDOWN, _OnMouseDown )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_MOUSEUP, _OnMouseUp )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_MOUSEMOVE, _OnMouseMove )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_OVERTYPECHANGE, _OnOvertypeChange )
		
		//
		// CodeMax Version 2.1
		//		

		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_FINDWRAPPED, _OnFindWrapped )

		//
		// CodeSense Version 2.1.0.22
		//		

		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODELIST, _OnCodeList )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODELISTSELMADE, _OnCodeListSelMade )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODELISTCANCEL, _OnCodeListCancel )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODELISTCHAR, _OnCodeListChar )

		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODETIP, _OnCodeTip )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODETIPINITIALIZE, _OnCodeTipInitialize ) 
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODETIPCANCEL, _OnCodeTipCancel )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODETIPUPDATE, _OnCodeTipUpdate )

		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODELISTSELWORD, _OnCodeListSelWord )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODELISTSELCHANGE, _OnCodeListSelChange )
		REFLECTED_NOTIFY_CODE_HANDLER ( CMN_CODELISTHOTTRACK, _OnCodeListHotTrack )

	END_MSG_MAP ()

// Internal handlers
private:
	
	LRESULT _OnChange ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnChange ();
		return 0;
	}

	LRESULT _OnHorizontalScroll ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnHorizontalScroll ();
		return 0;
	}

	LRESULT _OnVerticalScroll ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnVerticalScroll ();
		return 0;
	}

	LRESULT _OnSelectionChange ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnSelectionChange ();
		return 0;
	}

	LRESULT _OnViewChange ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnViewChange ();
		return 0;
	}

	LRESULT _OnModifiedChange ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnModifiedChange ();
		return 0;
	}

	LRESULT _OnShowProperties ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnShowProperties ();		
	}
	
	LRESULT _OnPropertiesChange ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnPropertiesChange ();
		return 0;
	}

	LRESULT _OnNotifyCreate ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnNotifyCreate ();
		return 0;
	}

	LRESULT _OnNotifyDestroy ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnNotifyDestroy ();
		return 0;
	}

	LRESULT _OnDrawLine ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )		
	{
		T * pT = static_cast < T * > ( this );
		pT->OnDrawLine ( reinterpret_cast < CM_DRAWLINEDATA * > ( pnmh ) );
		return 0;
	}

	LRESULT _OnDeleteLine ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnDeleteLine ( reinterpret_cast < CM_DELETELINEDATA * > ( pnmh ) );
		return 0;
	}

	LRESULT _OnCommandFailure ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCommandFailure ( reinterpret_cast < CM_CMDFAILUREDATA * > ( pnmh ) );		
	}

	LRESULT _OnRegisteredCommand ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnRegisteredCommand ( reinterpret_cast < CM_REGISTEREDCMDDATA * > ( pnmh ) );
		return 0;
	}

	LRESULT _OnKeyDown ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnKeyDown ( reinterpret_cast < CM_KEYDATA * > ( pnmh ) );
	}

	LRESULT _OnKeyUp ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnKeyUp ( reinterpret_cast < CM_KEYDATA * > ( pnmh ) );
	}

	LRESULT _OnKeyPress ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnKeyPress ( reinterpret_cast < CM_KEYDATA * > ( pnmh ) );
	}

	LRESULT _OnMouseDown ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnMouseDown ( reinterpret_cast < CM_MOUSEDATA * > ( pnmh ) );
	}

	LRESULT _OnMouseUp ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnMouseUp ( reinterpret_cast < CM_MOUSEDATA * > ( pnmh ) );
	}

	LRESULT _OnMouseMove ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnMouseMove ( reinterpret_cast < CM_MOUSEDATA * > ( pnmh ) );
	}

	LRESULT _OnOvertypeChange ( int /* idCtrl */, LPNMHDR /* pnmh */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnOvertypeChange ();
		return 0;
	}

	LRESULT _OnFindWrapped ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnFindWrapped ( reinterpret_cast < CM_FINDWRAPPEDDATA * > ( pnmh ) );
	}

	// CodeList / CodeTip support
	LRESULT _OnCodeList ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeList ( reinterpret_cast < CM_CODELISTDATA * > ( pnmh ) );
	}

	LRESULT	_OnCodeListSelMade ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeListSelMade ( reinterpret_cast < CM_CODELISTDATA * > ( pnmh ) );
	}

	LRESULT _OnCodeListCancel ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeListCancel ( reinterpret_cast < CM_CODELISTDATA * > ( pnmh ) );
	}

	LRESULT _OnCodeListChar ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeListChar ( reinterpret_cast < CM_CODELISTKEYDATA * > ( pnmh ) );
	}

	LRESULT _OnCodeTipCancel ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeTipCancel ( reinterpret_cast < CM_CODETIPDATA * > ( pnmh ) );
	}

	LRESULT _OnCodeTip ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeTip ( reinterpret_cast < CM_CODETIPDATA * > ( pnmh ) );
	}

	LRESULT _OnCodeTipInitialize  ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeTipInitialize ( reinterpret_cast < CM_CODETIPDATA * > ( pnmh ) );
	}

	LRESULT _OnCodeTipUpdate ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeTipUpdate ( reinterpret_cast < CM_CODETIPDATA * > ( pnmh ) );
	}

	LRESULT _OnCodeListSelWord ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeListSelWord ( reinterpret_cast < CM_CODELISTSELWORDDATA * > ( pnmh ) );
	}

	LRESULT _OnCodeListSelChange ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		return pT->OnCodeListSelChange ( reinterpret_cast < CM_CODELISTSELCHANGEDATA * > ( pnmh ) );
	}

	LRESULT _OnCodeListHotTrack ( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->OnCodeListHotTrack ( reinterpret_cast < CM_CODELISTHOTTRACKDATA * > ( pnmh ) );
		return 0;
	}


// Overidables
public:

	virtual void OnChange () 
	{ /**/ }

	virtual void OnHorizontalScroll ()
	{ /**/ }
	
	virtual void OnVerticalScroll ()
	{ /**/ }

	virtual void OnSelectionChange ()
	{ /**/ }
	
	virtual void OnViewChange ()
	{ /**/ }

	virtual void OnModifiedChange ()
	{ /**/ }

	virtual BOOL OnShowProperties ()
	{ 
		return FALSE; // display the default Window Properties dialog
	}

	virtual void OnPropertiesChange ()
	{ /**/ }

	virtual void OnNotifyCreate ()
	{ /**/ }

	virtual void OnNotifyDestroy ()
	{ /**/ }

	virtual void OnDrawLine ( CM_DRAWLINEDATA * )
	{ /**/ }

	virtual void OnDeleteLine ( CM_DELETELINEDATA * )
	{ /**/ }

	virtual BOOL OnCommandFailure ( CM_CMDFAILUREDATA * pFailureData )
	{ 
#ifdef _DEBUG
		static TCHAR * szErrors [] = {
			_T ( "CMDERR_FAILURE" ),
			_T ( "CMDERR_INPUT" ),
			_T ( "CMDERR_SELECTION" ),
			_T ( "CMDERR_NOTFOUND" ),
			_T ( "CMDERR_EMPTYBUF" ),
			_T ( "CMDERR_READONLY" )
		};
		ATLTRACE ( _T ( "Command: 0x%x, Error code: %s\n" ), pFailureData->wCmd,
			szErrors [ pFailureData->dwErr ] );
#else // !_DEBUG
		UNREFERENCED_PARAMETER ( pFailureData );
#endif // _DEBUG

		return FALSE; // perform the default processing 
	}

	virtual void OnRegisteredCommand ( CM_REGISTEREDCMDDATA * )
	{ /**/ }

	virtual BOOL OnKeyDown ( CM_KEYDATA * )
	{ 
		return FALSE; // let CodeMax process the key.
	}

	virtual BOOL OnKeyUp ( CM_KEYDATA * )
	{ 
		return FALSE; // let CodeMax process the key.
	}

	virtual BOOL OnKeyPress ( CM_KEYDATA * )
	{ 
		return FALSE; // let CodeMax process the key.
	}

	virtual BOOL OnMouseDown ( CM_MOUSEDATA * ) 
	{ 
		return FALSE; // perform the default action 
	}

	virtual BOOL OnMouseUp ( CM_MOUSEDATA * )
	{ 
		return FALSE; // perform the default action 
	}

	virtual BOOL OnMouseMove ( CM_MOUSEDATA * )
	{ 
		return FALSE; // perform the default action 
	}

	virtual void OnOvertypeChange ()
	{ /**/ }

	virtual BOOL OnFindWrapped ( CM_FINDWRAPPEDDATA * )
	{ 
		return FALSE; // allow wrapping.
	}

	// CodeList / CodeTip support

	virtual BOOL OnCodeList ( CM_CODELISTDATA * )
	{ 
		return FALSE; // the list view control should not be displayed.
	}

	virtual BOOL OnCodeListSelMade ( CM_CODELISTDATA * )
	{ 
		return FALSE; // control should be hidden.
	}

	virtual BOOL OnCodeListCancel ( CM_CODELISTDATA * )
	{ 
		return FALSE; // the list view control should be hidden.
	}

	virtual BOOL OnCodeListChar ( CM_CODELISTKEYDATA * )
	{ 
		return FALSE; // the control should perform the default action.
	}

	virtual LRESULT OnCodeTip ( CM_CODETIPDATA * )
	{ 
		return CM_TIPSTYLE_NONE; // no tooltip control should be displayed. 
	}

	virtual BOOL OnCodeTipInitialize ( CM_CODETIPDATA * ) 
	{
		return FALSE; // no changes.
	}

	virtual BOOL OnCodeTipCancel ( CM_CODETIPDATA * ) 
	{
		return FALSE; // the tooltip control should be hidden
	}

	virtual BOOL OnCodeTipUpdate ( CM_CODETIPDATA * )
	{
		return FALSE; // no changes.
	}

	virtual BOOL OnCodeListSelWord ( CM_CODELISTSELWORDDATA * )
	{ 
		return TRUE; // automatic selection.
	}

	virtual BOOL OnCodeListSelChange ( CM_CODELISTSELCHANGEDATA * )
	{ 
		return FALSE; // no tooltip text provided.
	}

	virtual void OnCodeListHotTrack ( CM_CODELISTHOTTRACKDATA * )
	{ /**/ }

};

////////////////////////////////////////////////////////////////////////////////////
// CodeSenseControlCommands - message handlers for standard CodeSense commands

// Chain to CodeSenseControlCommands message map. Your class must also derive from CodeSenseControl.
// Example:
// class CMyCodeSenseControl : public CWindowImpl < CMyCodeSenseControl, CodeSenseControl >,
//							 public CodeSenseControlCommands < CMyCodeSenseControl >
// {
// public:
//      BEGIN_MSG_MAP(CMyCodeSenseControl)
//              // your handlers...
//              CHAIN_MSG_MAP_ALT ( CodeSenseControlCommands < CMyCodeSenseControl >, 1 )
//      END_MSG_MAP()
//      // other stuff...
// };

#define CMAX_BASIC_COMMAND_ID_HANDLERS 1

template < class T >
class CodeSenseControlCommands
{

public:

	// Message map for common CodeSense commands
	BEGIN_MSG_MAP ( CodeSenseControlCommands < T > )

	ALT_MSG_MAP ( 1 ) // Start of alternate message map

		COMMAND_ID_HANDLER ( ID_EDIT_CLEAR, OnEditClear )
		COMMAND_ID_HANDLER ( ID_EDIT_CLEAR_ALL, OnEditClearAll )
		COMMAND_ID_HANDLER ( ID_EDIT_COPY, OnEditCopy )
		COMMAND_ID_HANDLER ( ID_EDIT_CUT, OnEditCut )
		COMMAND_ID_HANDLER ( ID_EDIT_PASTE, OnEditPaste )
		COMMAND_ID_HANDLER ( ID_EDIT_SELECT_ALL, OnEditSelectAll )	
		COMMAND_ID_HANDLER ( ID_EDIT_FIND, OnEditFind )		
		COMMAND_ID_HANDLER ( ID_EDIT_REPLACE, OnEditReplace )
		COMMAND_ID_HANDLER ( ID_EDIT_UNDO, OnEditUndo )
		COMMAND_ID_HANDLER ( ID_EDIT_REDO, OnEditRedo )		

	END_MSG_MAP ()

	LRESULT OnEditClear ( WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->Delete ();
		return 0;
	}
	
	LRESULT OnEditClearAll (  WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->SelectAll ();
		pT->Delete ();
		return 0;
	}
	
	LRESULT OnEditCopy ( WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->Copy ();
		return 0;
	}
	
	LRESULT OnEditCut ( WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->Cut ();
		return 0;
	}
	
	LRESULT OnEditPaste ( WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL & /* bHandled */ )
	{ 
		T * pT = static_cast < T * > ( this );
		pT->Paste ();
		return 0;
	}

	LRESULT OnEditSelectAll (  WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->SelectAll ();		
		return 0;
	}	

	LRESULT OnEditFind (  WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL & /* bHandled */ )
	{
		T * pT = static_cast < T * > ( this );
		pT->Find ();
		return 0;
	}

	LRESULT OnEditReplace ( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
	{
		T * pT = static_cast < T * > ( this );
		pT->FindReplace ();		
		return 0;
	}
	
	LRESULT OnEditUndo ( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
	{
		T * pT = static_cast < T * > ( this );
		pT->Undo ();		
		return 0;
	}

	LRESULT OnEditRedo ( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
	{
		T * pT = static_cast < T * > ( this );
		pT->Redo ();		
		return 0;
	}

};

////////////////////////////////////////////////////////////////////////////////////
// UndoBlock : a block undo wraper for the CodeSense window
//
//	Blocked undo - A method by which series of undoable commands 'blocked' together 
//				   so that they can be undone all at once
//
template < class TControl >
class UndoBlockT
{

// Data
private:

	TControl * m_pCodeSenseControl;

// Constructors / Destructors
public:
	
	UndoBlockT ( TControl * pCtrl ) : m_pCodeSenseControl ( pCtrl ) 
	{ 
		ATLASSERT ( m_pCodeSenseControl->IsWindow () );
		m_pCodeSenseControl->BeginUndo (); 
	}

	~UndoBlockT ()
	{
		m_pCodeSenseControl->EndUndo (); 
	}

};

// simple typedef for standard use
typedef UndoBlockT < CodeSenseControl > UndoBlock;

/////////////////////////////////////////////////////////////////////////////

}; // namespace cmcs

#if !defined ( _CMAX_NO_AUTOMATIC_NAMESPACE )
using namespace cmcs;
#endif // !defined ( _CMAX_NO_AUTOMATIC_NAMESPACE )

/////////////////////////////////////////////////////////////////////////////

#endif // !defined ( _cmcswtl_h_included_ )

/////////////////////////////////////////////////////////////////////////////
// EOF //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////