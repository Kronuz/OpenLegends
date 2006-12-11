/////////////////////////////////////////////////////////////////////////////
//
// CodeListCtrl.cpp : Implementation of CCodeListCtrl
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include <commctrl.h>
#include "CodeListCtrl.h"

#ifdef _ACTIVEX
#include "editx.h"
#include "ICodeList.h"
#endif//#ifdef _ACTIVEX


#define	WM_HANDLE_KEYDOWN	WM_USER + 1


//***************************************************************************
// Public Member Functions
//***************************************************************************

// Destruction --------------------------------------------------------------
CCodeListCtrl::~CCodeListCtrl()
{
}

// Create -------------------------------------------------------------------
CCodeListCtrl* CCodeListCtrl::Create( POINT& pt, CEdit* pEdit )
{
	ASSERT( pEdit );

	CCodeListCtrl* pCtrl = new CCodeListCtrl( pEdit );

	if( pCtrl )
	{
		RECT rect = { pt.x, pt.y, pt.x + 250, pt.y + 150 };
		BOOL bCreated = pCtrl->Create( NULL, NULL, LVS_SORTASCENDING, &rect,
			pEdit->GetWindow() );

		ASSERT( bCreated );

		if( bCreated )
			pCtrl->InitControl();
		else
		{
			delete pCtrl;
			pCtrl = NULL;
		}
	}

	return pCtrl;
}

// AutoSizeControl ----------------------------------------------------------
void CCodeListCtrl::AutoSizeControl()
{
//	HDC hdc = ::GetDC( m_hWnd );
	int iItems = GetItemCount();
	CRect rWnd, rSize, rClient, rScreen;
	DWORD dwStyle = GetWindowLong( GWL_STYLE );
	DWORD dwExStyle = GetWindowLong( GWL_EXSTYLE );

	// Get the height of a single line of text
	VERIFY( ListView_GetItemRect( m_hWnd, 0, &rSize, LVIR_BOUNDS ) );

	// Client area should be big enough for about 10 lines, unless there are
	// fewer than 10 lines to begin with...
	//
	rClient.bottom = rSize.Height() * min( 10, iItems );

	// Get the width required to display entire column + scroll bar
	SendMessage( LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE );

	rClient.right = SendMessage( LVM_GETCOLUMNWIDTH, 0, 0 );

	// Add space for scroll bar only if necessary.
	if( iItems > 10 )
		rClient.right += ::GetSystemMetrics( SM_CXVSCROLL );

	// Keep control on visible part of screen, if possible
	::SystemParametersInfo( SPI_GETWORKAREA, 0, (PVOID)&rScreen, 0 );

	if( rClient.Width() > rScreen.Width() )
	{
		rClient.right = rScreen.Width();
		rClient.bottom += ::GetSystemMetrics( SM_CYHSCROLL );
	}

	::AdjustWindowRectEx( &rClient, dwStyle, FALSE, dwExStyle );
	GetWindowRect( &rWnd );

	if( rWnd.left + rClient.Width() > rScreen.Width() )
		rWnd.left = rScreen.Width() - rClient.Width();

	if( rWnd.left < rScreen.left )
		rWnd.left = rScreen.left;

	if( rWnd.top + rClient.Height() > rScreen.Height() )
		rWnd.top = rScreen.Height() - rClient.Height();

	if( rWnd.top < rScreen.top )
		rWnd.top = rScreen.top;

	// Reposition and/or resize the window
	::MoveWindow( m_hWnd, rWnd.left, rWnd.top, rClient.Width(),
		rClient.Height(), TRUE );
}

// SelectItem ---------------------------------------------------------------
void CCodeListCtrl::SelectItem( int iItem )
{
	if( -1 != iItem )
	{
		// Select the specified item
		ListView_SetItemState( m_hWnd, iItem, LVIS_SELECTED | LVIS_FOCUSED,
			LVIS_SELECTED | LVIS_FOCUSED );

		// Make sure the item is visible
		ListView_EnsureVisible( m_hWnd, iItem, FALSE );
	}
	else
	{
		// Unselect current item, if any
		int iSel = GetCurSel();

		if( -1 != iSel )
		{
			ListView_SetItemState( m_hWnd, iSel, 0,
				LVIS_SELECTED | LVIS_FOCUSED );
		}
	}
}

// DestroyWindow ------------------------------------------------------------
BOOL CCodeListCtrl::DestroyWindow()
{
	// Make sure parent doesn't want to prevent window being destroyed.
	if( m_bAcceptSel || NotifyParent( CMN_CODELISTCANCEL ) )
		return FALSE;

	// Destroy the tool tip if it exists
	SetTipText( NULL );

	return CDispatchWnd::DestroyWindow();
}

// InsertItem ---------------------------------------------------------------
int CCodeListCtrl::InsertItem( int iIndex, LPCTSTR lpszItem, int iImage,
							   LPARAM lParam )
{
	ASSERT( ::IsWindow( m_hWnd ) );

	LVITEM lvi = {0};

	if( -1 == iIndex )
		iIndex = GetItemCount() + 1;

	lvi.iItem = iIndex;
	lvi.iImage = iImage;
	lvi.lParam = lParam;
	lvi.pszText = (LPTSTR)lpszItem;
	lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;

	return ListView_InsertItem( m_hWnd, &lvi );
}

// GetItemText --------------------------------------------------------------
int CCodeListCtrl::GetItemText( int iItem, LPTSTR pszText, int cchTextMax )
{
	ASSERT( ::IsWindow( m_hWnd ) );

	LVITEM lvi = {0};

	if( NULL == pszText )
	{
		// Just calculate size of buffer required
		int nLen, nRes;
		for( nLen = 128, nRes = 127; nRes == nLen - 1; nLen *= 2 )
		{
			if( pszText )
				delete [] pszText;

			pszText = new TCHAR[ nLen + 1 ];

			if( NULL == pszText )
				return 0;

			lvi.pszText = pszText;
			lvi.cchTextMax = nLen;

			nRes = SendMessage( LVM_GETITEMTEXT, iItem, (LPARAM)&lvi );
		}

		delete [] pszText;
		return nRes + 1;
	}

	lvi.pszText = pszText;
	lvi.cchTextMax = cchTextMax;

	return SendMessage( LVM_GETITEMTEXT, iItem, (LPARAM)&lvi );
}

// GetItemData --------------------------------------------------------------
int CCodeListCtrl::GetItemData( int iItem )
{
	ASSERT( IsWindow( m_hWnd ) );

	LVITEM lvi = {0};

	lvi.mask = LVIF_PARAM;
	lvi.iItem = iItem;
	ListView_GetItem( m_hWnd, &lvi );

	return lvi.lParam;
}

// FindString ---------------------------------------------------------------
int CCodeListCtrl::FindString( LPCTSTR lpszString, BOOL bAcceptPartial )
{
	LVFINDINFO lvfi = {0};

	lvfi.psz = lpszString;
	lvfi.flags = LVFI_STRING;

	if( bAcceptPartial )
		lvfi.flags |= LVFI_PARTIAL;

	return ListView_FindItem( m_hWnd, -1, &lvfi );
}

// GetDispatch --------------------------------------------------------------
LPDISPATCH CCodeListCtrl::GetDispatch()
{
#ifdef _ACTIVEX

	if( NULL == m_lpDispatch )
	{
		CComObject<CICodeList>* pNew = NULL;
		HRESULT hR = pNew->CreateInstance( &pNew );

		if( SUCCEEDED( hR ) )
		{
			pNew->AddRef();
			pNew->SetControl( this );
			m_lpDispatch = pNew;
		}
	}

#endif//#ifdef _ACTIVEX

	return m_lpDispatch;
}

// SetTipText ---------------------------------------------------------------
BOOL CCodeListCtrl::SetTipText( LPCTSTR pszText )
{
	// Destroy the tooltip window if an empty string was passed in.
	if( NULL == pszText || 0 == lstrlen( pszText ) )
	{
		if( ::IsWindow( m_hwndToolTip ) )
			::DestroyWindow( m_hwndToolTip );

		m_hwndToolTip = NULL;
		return TRUE;
	}

	// Can't set tip if the window isn't visible yet!
	if( !::IsWindowVisible( m_hWnd ) )
		return FALSE;

	// Set up the TOOLINFO structure
	TOOLINFO ti = { sizeof(TOOLINFO) };

	ti.uFlags = TTF_TRACK | TTF_IDISHWND;
	ti.hwnd = m_hWnd;
	ti.uId = (UINT)m_hWnd;
	ti.lpszText = (LPTSTR)pszText;

	// Make sure the tooltip window exists...
	if( NULL == m_hwndToolTip )
	{
		extern HINSTANCE hInstance;

		m_hwndToolTip = CreateWindow( TOOLTIPS_CLASS, NULL,
			WS_POPUP | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, m_hWnd, NULL, hInstance, NULL );

		ASSERT( NULL != m_hwndToolTip );

		if( m_hwndToolTip )
		{
			::SetWindowPos( m_hwndToolTip, HWND_TOPMOST,0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

			// Attach the tooltip control to the listview control window
			::SendMessage( m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti );
		}
	}

	if( m_hwndToolTip )
	{
		// Initially position the tooltip to the right of the item selected
		// in the listview control or, if nothing is selected, at the top-
		// right corner of the control.
		//
		RECT rect = {0};
		int iSel = GetCurSel();

		if( -1 == iSel )
			GetWindowRect( &rect );
		else
		{
			ListView_GetItemRect( m_hWnd, iSel, &rect, LVIR_BOUNDS );
			ClientToScreen( &rect );
		}

		::SendMessage( m_hwndToolTip, TTM_TRACKPOSITION, 0,
			MAKELONG( rect.right, rect.top ) );

		// Set the tooltip text...
		::SendMessage( m_hwndToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti );

		// ...and display it!
		::SendMessage( m_hwndToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti );

		return TRUE;
	}

	return FALSE;
}

// OnSelChange --------------------------------------------------------------
void CCodeListCtrl::OnSelChange( int iNewItem )
{
	CM_CODELISTSELCHANGEDATA hdr = {0};

	hdr.iItem = iNewItem;

#ifdef _ACTIVEX

	LPTSTR pszText = (LPTSTR)NotifyParent( CMN_CODELISTSELCHANGE,
		(LPNMHDR)&hdr );

	SetTipText( pszText );

	if( pszText )
		delete [] pszText;

#else

	// Notify the parent of selection change, and see if a tooltip is
	// needed...
	//
	if( !NotifyParent( CMN_CODELISTSELCHANGE, (LPNMHDR)&hdr ) )
	{
		// Notification not handled by parent, so destroy the tooltip if it
		// exists.
		SetTipText( NULL );
	}
	else
	{
		// If no instance handle is specified, we'll just use the text
		// provided in either szText or pszText - whichever has valid data.
		// If no data is provided and the tooltip exists, it will be
		// destroyed.
		//
		if( NULL == hdr.hInstance )
			SetTipText( ( NULL == hdr.pszText ? hdr.szText : hdr.pszText ) );
		else
		{
			// An instance handle was specified, so the pszText member is the
			// identifier of a string resource to be loaded...
			//
			LPTSTR pszText = NULL;
			DWORD dwIn = 0, dwSize = CM_MAX_CODELIST_TIP;

			do
			{
				pszText = new TCHAR[ dwSize ];
				ASSERT( NULL != pszText );

				if( NULL == pszText )
				{
					// Out of memory, so destroy the tooltip.
					SetTipText( NULL );
					return;
				}

				dwIn = LoadString( hdr.hInstance, (UINT)hdr.pszText,
					pszText, dwSize );

				if( 0 == dwIn )
				{
					// String resource not found!  Make sure you didn't use
					// the MAKEINTRESOURCE macro - just pass in the numeric
					// ID of the string.
					//
					ASSERT( FALSE );
					SetTipText( NULL );// Destroy the tooltip
					return;
				}

				if( dwIn + 1 == dwSize )
				{
					// Need to try a bigger buffer...
					dwSize *= 2;
					delete [] pszText;
					pszText = NULL;
				}

			}while( NULL == pszText );

			SetTipText( pszText );
			delete [] pszText;
		}
	}

#endif//#ifdef _ACTIVEX

}

// OnHotTrack ---------------------------------------------------------------
void CCodeListCtrl::OnHotTrack( int iItem )
{
	CM_CODELISTHOTTRACKDATA hdr = {0};
	hdr.iItem = iItem;
	NotifyParent( CMN_CODELISTHOTTRACK, (LPNMHDR)&hdr );
}


//***************************************************************************
// Protected Member Functions
//***************************************************************************

// Message map --------------------------------------------------------------
BEGIN_HANDLE_MSG( CCodeListCtrl )
	HANDLE_MSG( WM_CHAR, OnChar )
	HANDLE_MSG( WM_LBUTTONDBLCLK, OnLButtonDblClk )
	HANDLE_MSG( CMN_KEYDOWN, OnParentKeyDown )
	HANDLE_MSG( WM_HANDLE_KEYDOWN, PostParentKeyDown )
	HANDLE_MSG( LVM_DELETEITEM, OnDeleteItem )
END_HANDLE_MSG( CDispatchWnd )


// Construction -------------------------------------------------------------
CCodeListCtrl::CCodeListCtrl( CEdit* pEdit )
: CDispatchWnd( pEdit )
, m_bAcceptSel( false )
, m_hwndToolTip( NULL )
{
}

// OnChar -------------------------------------------------------------------
BOOL CCodeListCtrl::OnChar( WPARAM wp, LPARAM lp, LRESULT* )
{
	// Give the parent the first crack at the message
	CM_CODELISTKEYDATA hdr = {0};

	hdr.wChar = wp;
	hdr.lKeyData = lp;

	if( NotifyParent( CMN_CODELISTCHAR, (LPNMHDR)&hdr ) )
		return TRUE;// handled by parent

	// See if it's a character we need to handle
	switch( wp )
	{
	case VK_RETURN:
	case VK_TAB:
	case L' ':
		{
			AcceptSelection();
		}
		return TRUE;// handled

	case VK_ESCAPE:
		{
			DestroyWindow();
		}
		return TRUE;// handled
	}

	return FALSE;// not handled
}

// OnLButtonDblClk ----------------------------------------------------------
BOOL CCodeListCtrl::OnLButtonDblClk( WPARAM, LPARAM, LRESULT* pResult )
{
	// Process the message first to allow the item to be selected
	*pResult = Default();

	// Now notify app of item selection
	AcceptSelection();

	return TRUE;// handled
}

// NotifyParent -------------------------------------------------------------
LRESULT CCodeListCtrl::NotifyParent( UINT unNotification, LPNMHDR lpNMHDR )
{
	CM_CODELISTDATA hdr = {0};

	if( NULL == lpNMHDR )
		lpNMHDR = (LPNMHDR)&hdr;

	((CM_CODELISTDATA*)lpNMHDR)->hListCtrl = m_hWnd;

	return CDispatchWnd::NotifyParent( unNotification, lpNMHDR );
}

// InitControl --------------------------------------------------------------
void CCodeListCtrl::InitControl()
{
	// Insert the one and only column
	LVCOLUMN lvc = {0};
	SendMessage( LVM_INSERTCOLUMN, 0, (LPARAM)&lvc );

	// Enable full-row selection
	SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT,
		LVS_EX_FULLROWSELECT );
}

// AcceptSelection ----------------------------------------------------------
void CCodeListCtrl::AcceptSelection()
{
	// If nothing is selected yet, we'll select the first item in the list
	// and we won't close the list until next time.
	//
	if( -1 == GetCurSel() )
		SelectItem( 0 );
	else if( !m_bAcceptSel )
	{
		m_bAcceptSel = true;

		// Notify the parent that something has been selected, and destroy the
		// list control if the parent has no objections.
		//
		if( !NotifyParent( CMN_CODELISTSELMADE ) )
			CDispatchWnd::DestroyWindow();
		else
			m_bAcceptSel = false;
	}
}

// Create -------------------------------------------------------------------
BOOL CCodeListCtrl::Create( LPCTSTR, LPCTSTR lpszWindowName, DWORD dwStyle,
						   LPCRECT lpRect, HWND hwndParent, UINT nID )
{
	// This version of Create() is declared as protected to keep it from
	// being called directly - you should call the public version of Create()
	// instead.
	//
	return CDispatchWnd::Create( WC_LISTVIEW, lpszWindowName, WS_POPUP |
		WS_DLGFRAME | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL |
		LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS | dwStyle, lpRect,
		hwndParent, nID );
}

// OnDestroy ----------------------------------------------------------------
void CCodeListCtrl::OnDestroy()
{
	// We don't want to receive any more messages from the CodeMax control...
	m_pEdit->m_pCodeList = NULL;

	// Destroy the window
	CDispatchWnd::OnDestroy();

	// Destroy the class instance
	delete this;
}

// OnParentKeyDown ----------------------------------------------------------
BOOL CCodeListCtrl::OnParentKeyDown( WPARAM wp, LPARAM lp, LRESULT* pResult )
{
	// See if it's a character we need to handle
	switch( wp )
	{
	case VK_RETURN:
	case VK_TAB:
		{
			AcceptSelection();
			*pResult = 1;
		}
		return TRUE;// handled

	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
		{
			// Pass message back to ourselves to perform default processing
			SendMessage( WM_KEYDOWN, wp, lp );
			*pResult = 1;
		}
		return TRUE;// handled
	}

	// Perform default processing to allow message to go to edit control
	*pResult = Default();

	// Post a message back to ourselves to allow parent to be updated with
	// new text before we try to locate the currently selected word.
	//
	PostMessage( WM_HANDLE_KEYDOWN );

	return TRUE;// handled
}

// PostParentKeyDown --------------------------------------------------------
BOOL CCodeListCtrl::PostParentKeyDown( WPARAM, LPARAM, LRESULT* )
{
	// Get currently selected word, if any
	int iItem = -1;
	LPTSTR pszWord = NULL;
	HWND hEdit = m_pEdit->GetWindow();
	int iLen = CM_GetWordLength( hEdit, NULL, FALSE );

	if( iLen )
	{
		pszWord = new TCHAR[ iLen + 1 ];

		if( pszWord )
		{
			*pszWord = L'\0';

			if( CME_SUCCESS == CM_GetWord( hEdit, pszWord, NULL ) )
			{
				// Select closest match in list
				iItem = FindString( pszWord, TRUE );
			}
		}
	}

	if( -1 != iItem )
	{
		// We found an item in the list that matches the current word.  See
		// if the parent wants us to automatically select the word in the
		// list.
		//
		CM_CODELISTSELWORDDATA hdr = {0};

		hdr.iItem = iItem;

		if( NotifyParent( CMN_CODELISTSELWORD, (LPNMHDR)&hdr ) )
			SelectItem( iItem );
	}

	if( pszWord )
		delete [] pszWord;

	return TRUE;// handled
}

// OnDeleteItem -------------------------------------------------------------
BOOL CCodeListCtrl::OnDeleteItem( WPARAM wp, LPARAM lp, LRESULT* pResult )
{
	*pResult = ::CallWindowProc( m_OldWndProc, m_hWnd, LVM_DELETEITEM, wp,
		lp );

	if( 0 == GetItemCount() )
		DestroyWindow();
	else
		AutoSizeControl();

	return TRUE;// handled
}

// OnSetFont ----------------------------------------------------------------
void CCodeListCtrl::OnSetFont( HFONT hFont, BOOL bRedraw )
{
	CWnd::OnSetFont( hFont, bRedraw );

	if( GetItemCount() )
		AutoSizeControl();
}
