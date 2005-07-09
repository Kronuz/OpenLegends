/////////////////////////////////////////////////////////////////////////////
//
// CodeTipCtrl.cpp : Implementation of CCodeTipCtrl
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include <commctrl.h>
#include "CodeTipCtrl.h"
#include "CodeTipHighlightCtrl.h"
#include "CodeTipFuncHighlightCtrl.h"
#include "CodeTipMultiFuncCtrl.h"

#ifdef _ACTIVEX
#include "editx.h"
#include "ICodeTip.h"
#endif//#ifdef _ACTIVEX


//***************************************************************************
// Public Member Functions
//***************************************************************************

// Destruction --------------------------------------------------------------
CCodeTipCtrl::~CCodeTipCtrl()
{
	if( m_lpszText )
		delete [] m_lpszText;
}

// Create -------------------------------------------------------------------
CCodeTipCtrl* CCodeTipCtrl::Create( CEdit* pEdit, UINT nType, POINT& pt )
{
	ASSERT( pEdit );

	// Determine which tooltip control class to create
	CCodeTipCtrl* pCtrl = NULL;

	switch( nType )
	{
	case CM_TIPSTYLE_NORMAL:
		pCtrl = new CCodeTipCtrl( pEdit );
		break;

	case CM_TIPSTYLE_HIGHLIGHT:
		pCtrl = new CCodeTipHighlightCtrl( pEdit );
		break;

	case CM_TIPSTYLE_FUNCHIGHLIGHT:
		pCtrl = new CCodeTipFuncHighlightCtrl( pEdit );
		break;

	case CM_TIPSTYLE_MULTIFUNC:
		pCtrl = new CCodeTipMultiFuncCtrl( pEdit );
		break;

	default:
		ASSERT( FALSE );// Unknown type!
		break;
	}

	if( pCtrl )
	{
		// Create the tooltip window
		RECT rect = { pt.x, pt.y, pt.x +10, pt.y + 10 };
		BOOL bCreated = pCtrl->Create( NULL, NULL, 0, &rect,
			pEdit->GetWindow() );

		ASSERT( bCreated );

		if( bCreated )
		{
			// Initialize the tooltip window (set fonts, etc.)
			pCtrl->InitControl();

			// Watch out - the application could have destroyed the tooltip
			// window inside its CodeTipUpdate handler.
			if( !::IsWindow( pCtrl->GetSafeHwnd() ) )
				pCtrl = NULL;
			else
			{
				// It's safe to display the control now
				pCtrl->ShowWindow( SW_SHOW );
				pCtrl->UpdateWindow();
			}
		}
		else
		{
			// Failed!
			delete pCtrl;
			pCtrl = NULL;
		}

		// Give the input focus back to the CodeMax control
		::SetFocus( pEdit->GetWindow() );
	}

	return pCtrl;
}

// AutoSizeControl ----------------------------------------------------------
void CCodeTipCtrl::AutoSizeControl()
{
	// Notify the parent window that the tooltip is about to be updated so it
	// can make any changes necessary...
	NotifyUpdate();

	// Watch out - the application could have destroyed the tooltip window
	// inside its CodeTipUpdate handler.
	if( !::IsWindow( GetSafeHwnd() ) )
		return;

	// Save original window area so we can tell if it changes...
	CRect rOld;
	GetWindowRect( &rOld );

	// Figure out how big a line of text is based on the current font
	int y = 17;
	HWND hEdit = m_pEdit->GetWindow();
	HFONT hFont = (HFONT)::SendMessage( hEdit, WM_GETFONT, 0, 0 );

	if( hFont )
	{
		LOGFONT lf = {0};
		HDC hdc = ::GetDC( hEdit );

		GetObject( hFont, sizeof(LOGFONT), &lf );
		y = -MulDiv( lf.lfHeight, GetDeviceCaps( hdc, LOGPIXELSY ), 72 );
		::ReleaseDC( hEdit, hdc );
	}

	// Get the caret position
	POINT ptCaret = {0};

	::GetCaretPos( &ptCaret );
	::ClientToScreen( hEdit, &ptCaret );

	// Initially position tooltip below caret position.
	CRect rClient;
	rClient.top = ptCaret.y + y;
	rClient.bottom = rClient.top + 10;

	// If the caret position has moved toward the left edge of the screen,
	// we'll move the tooltip to the left as well.
	CRect rTmp;
	GetClientRect( &rTmp );
	ClientToScreen( &rTmp );
	rClient.left = ( ptCaret.x < rTmp.left ) ? ptCaret.x : rTmp.left;

	// Set the right side so that it does not expand past the visible area on
	// the screen.  When we calculate the client area, it should automatically
	// wrap at this point & expand the bottom of the rectangle as needed.
	//
	CRect rScreen;
	::SystemParametersInfo( SPI_GETWORKAREA, 0, (LPVOID)&rScreen, 0 );
	rScreen.DeflateRect( 4, 4 );
	rClient.right = rScreen.right;

	// Calculate the client area needed for the text
	HDC hdc = GetDC();

	DrawTipText( hdc, rClient, TRUE );

	// Make sure we don't overflow the right edge of the screen.
	if( rClient.right > rScreen.right )
	{
		rClient.left = rScreen.right - rClient.Width();
		rClient.right = rScreen.right;
	}

	// If we overflow the bottom of the screen, move the tip up a line above
	// the caret instead.
	//
	if( rClient.bottom > rScreen.bottom )
	{
		int h = rClient.Height();
		rClient.bottom = ptCaret.y - y;
		rClient.top = rClient.bottom - h;

		// But watch out - we may we go off the top of the screen now.
		if( rClient.top < rScreen.top )
		{
			// Recalculate the window size using the full screen width, and
			// starting below the caret.
			rClient.top = ptCaret.y + y;
			rClient.left = rScreen.left;
			rClient.right = rScreen.right;

			DrawTipText( hdc, rClient, TRUE );
		}
	}

	// If the tip still goes off the bottom of the screen, we'll have one
	// last stab at placing it above the caret.  If that still doesn't help,
	// then tough noogies buddy - shorten your tooltip text!
	if( rClient.bottom > rScreen.bottom )
	{
		int h = rClient.Height();
		rClient.bottom = ptCaret.y - y;
		rClient.top = rClient.bottom - h;
	}

	ReleaseDC( hdc );

	// Convert client calculated area to screen area
	::AdjustWindowRectEx( &rClient, GetWindowLong( GWL_STYLE ), FALSE,
		GetWindowLong( GWL_EXSTYLE ) );

	// Okay - one last thing to do before we update the tooltip position: If
	// the caret position is to the right of the tooltip's left edge, and the
	// tooltip doesn't expand all the way across the screen, we'll bump the
	// tip to the right as close to the caret position as possible without
	// going off the screen...
	//
	if( ptCaret.x > rClient.left && rClient.right < rScreen.right )
	{
		// How far do we need to move the tooltip window to align it with the
		// caret position?
		int deltaX = ptCaret.x - rClient.left;

		// Now, how far can we actually move the tooltip without overflowing
		// the right edge of the screen?
		if( rClient.right + deltaX > rScreen.right )
			deltaX = rScreen.right - rClient.right;

		// Bump the tooltip to the right!
		rClient.left += deltaX;
		rClient.right += deltaX;
	}

	// Update the window position / size - but only if it has changed
	if( rOld.left != rClient.left || rOld.right != rClient.right || 
		rOld.top != rClient.top || rOld.bottom != rClient.bottom )
	{
		if( IsWindowVisible( m_hWnd ) )
			MoveWindow( &rClient );
		else
		{
			SetWindowPos( m_hWnd, NULL, rClient.left, rClient.top,
				rClient.Width(), rClient.Height(), SWP_NOACTIVATE |
				SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOREDRAW );
		}
	}

	// Always refresh the window, in case the tip has changed but the window
	// size has not.
	InvalidateRect( NULL );
	UpdateWindow();
}

// GetTipText ---------------------------------------------------------------
LPCTSTR CCodeTipCtrl::GetTipText()
{
	int iLen = GetWindowTextLength() + 1;

	if( m_lpszText )
		delete [] m_lpszText;

	VERIFY( m_lpszText = new TCHAR[ iLen ] );
	ZeroMemory( m_lpszText, sizeof( TCHAR ) * iLen );

	VERIFY( iLen - 1 == GetWindowText( m_lpszText, iLen ) );

	return m_lpszText;
}

// NotifyCancel -------------------------------------------------------------
BOOL CCodeTipCtrl::NotifyCancel()
{
	return ( 0 == NotifyParent( CMN_CODETIPCANCEL ) );
}

// GetDispatch --------------------------------------------------------------
LPDISPATCH CCodeTipCtrl::GetDispatch()
{
#ifdef _ACTIVEX

	if( NULL == m_lpDispatch )
	{
		CComObject<CICodeTip>* pNew = NULL;
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


//***************************************************************************
// Protected Member Functions
//***************************************************************************

// Message map --------------------------------------------------------------
BEGIN_HANDLE_MSG( CCodeTipCtrl )
	HANDLE_MSG( WM_CHAR, OnChar )
	HANDLE_MSG( WM_ERASEBKGND, OnEraseBkgnd )
	HANDLE_MSG( WM_LBUTTONDOWN, OnLButtonDown )
	HANDLE_MSG( WM_RBUTTONDOWN, OnRButtonDown )
	HANDLE_MSG( WM_MBUTTONDOWN, OnMButtonDown )
	HANDLE_MSG( WM_GETFONT, OnGetFont )
	HANDLE_MSG( TTM_SETTIPBKCOLOR, OnSetTipBkColor )
	HANDLE_MSG( TTM_SETTIPTEXTCOLOR, OnSetTipTextColor )
	HANDLE_MSG( TTM_GETTIPBKCOLOR, OnGetTipBkColor )
	HANDLE_MSG( TTM_GETTIPTEXTCOLOR, OnGetTipTextColor )
END_HANDLE_MSG( CDispatchWnd )


// Construction -------------------------------------------------------------
CCodeTipCtrl::CCodeTipCtrl( CEdit* pEdit )
: CDispatchWnd( pEdit )
, m_nTipType( CM_TIPSTYLE_NORMAL )
, m_lpszText( NULL )
, m_crText( ::GetSysColor( COLOR_INFOTEXT ) )
, m_crBk( ::GetSysColor( COLOR_INFOBK ) )
{
#ifndef _ACTIVEX
	m_hfText = NULL;
#endif
}

// Create -------------------------------------------------------------------
BOOL CCodeTipCtrl::Create( LPCTSTR, LPCTSTR lpszWindowName, DWORD dwStyle,
						  LPCRECT lpRect, HWND hwndParent, UINT nID )
{
	// This version of Create() is declared as protected to keep it from
	// being called directly - you should call the public version of Create()
	// instead.
	//
	LPCTSTR lpszClassName = CWnd::RegisterClass( 0,
		::LoadCursor( NULL, IDC_ARROW ) );

	return CDispatchWnd::Create( lpszClassName, lpszWindowName, WS_BORDER |
		WS_POPUP | dwStyle, lpRect, hwndParent, nID );
}

// InitControl --------------------------------------------------------------
void CCodeTipCtrl::InitControl()
{
	// Ask the parent window to initialize the tooltip contents before it is
	// displayed.
	NotifyUpdate( CMN_CODETIPINITIALIZE );

	// Watch out - the application could have destroyed the tooltip window
	// inside its CodeTipInitialize handler.
	if( !::IsWindow( GetSafeHwnd() ) )
		return;

	// Initialize tooltip font using system font.  Note that this will also
	// set the size and position of the tooltip window to fit on the screen
	// in the WM_SETFONT handler.
	//
	SendMessage( WM_SETFONT, (WPARAM)__GetFont() );
}

// OnDestroy ----------------------------------------------------------------
void CCodeTipCtrl::OnDestroy()
{
	// We don't want to receive any more messages from the CodeMax control...
	m_pEdit->m_pCodeTip = NULL;

	// Destroy the window
	CDispatchWnd::OnDestroy();

	// Destroy the class instance
	delete this;
}

// OnChar -------------------------------------------------------------------
BOOL CCodeTipCtrl::OnChar( WPARAM wp, LPARAM, LRESULT* )
{
	if( VK_ESCAPE == wp )
	{
		DestroyCodeTip();
		return TRUE;// handled
	}

	return FALSE;// not handled
}

// OnPaint ------------------------------------------------------------------
void CCodeTipCtrl::OnPaint()
{
	CRect rect;
	PAINTSTRUCT ps = {0};
	HDC hdc = BeginPaint( &ps );
	COLORREF crText = ::SetTextColor( hdc, m_crText );
	COLORREF crBk = ::SetBkColor( hdc, m_crBk );

	GetClientRect( &rect );
	DrawTipText( hdc, rect, FALSE );

	::SetBkColor( hdc, crBk );
	::SetTextColor( hdc, crText );

	EndPaint( &ps );
}

// OnEraseBkgnd -------------------------------------------------------------
BOOL CCodeTipCtrl::OnEraseBkgnd( WPARAM wp, LPARAM, LRESULT* pResult )
{
	CRect rc;
	HDC hdc = (HDC)wp;
	HBRUSH hbr = ::CreateSolidBrush( m_crBk );

	GetClientRect( &rc );
	FillRect( hdc, &rc, hbr );
	DeleteObject( hbr );

	*pResult = TRUE;
	return TRUE;// handled
}

// OnLButtonDown ------------------------------------------------------------
BOOL CCodeTipCtrl::OnLButtonDown( WPARAM, LPARAM, LRESULT* )
{
	// Return focus to CodeMax control to allow user to keep typing
	::SetFocus( m_pEdit->GetWindow() );

	return FALSE;// not handled
}

// OnRButtonDown ------------------------------------------------------------
BOOL CCodeTipCtrl::OnRButtonDown( WPARAM, LPARAM, LRESULT* )
{
	// Return focus to CodeMax control to allow user to keep typing
	::SetFocus( m_pEdit->GetWindow() );

	return FALSE;// not handled
}

// OnMButtonDown ------------------------------------------------------------
BOOL CCodeTipCtrl::OnMButtonDown( WPARAM, LPARAM, LRESULT* )
{
	// Return focus to CodeMax control to allow user to keep typing
	::SetFocus( m_pEdit->GetWindow() );

	return FALSE;// not handled
}

// NotifyParent -------------------------------------------------------------
LRESULT CCodeTipCtrl::NotifyParent( UINT unNotification, LPNMHDR lpNMHDR )
{
	CM_CODETIPDATA hdr = {0};

	if( NULL == lpNMHDR )
		lpNMHDR = (LPNMHDR)&hdr;

	CM_CODETIPDATA* pitData = (CM_CODETIPDATA*)lpNMHDR;

	pitData->hToolTip = m_hWnd;
	pitData->nTipType = m_nTipType;

	return CDispatchWnd::NotifyParent( unNotification, lpNMHDR );
}

// DrawTipText --------------------------------------------------------------
void CCodeTipCtrl::DrawTipText( HDC hdc, RECT& rect, BOOL bCalcRect )
{
	LPCTSTR lpszTip = GetTipText();
	UINT nFormat = DT_LEFT | DT_NOPREFIX | DT_WORDBREAK;
	HFONT hfOld = (HFONT)::SelectObject( hdc, __GetFont() );

	if( bCalcRect )
		nFormat |= DT_CALCRECT;

	::DrawText( hdc, lpszTip, lstrlen( lpszTip ), &rect, nFormat );

	if( hfOld )
		::SelectObject( hdc, hfOld );
}

// OnSetFont ----------------------------------------------------------------
void CCodeTipCtrl::OnSetFont( HFONT hFont, BOOL bRedraw )
{
	if( NULL == hFont )
		hFont = GetDefaultFont();

#ifdef _ACTIVEX
	SetFontDisp( hFont );
#else
	m_hfText = hFont;
#endif

	AutoSizeControl();

	// Redraw the control if redraw flag is set
	if( bRedraw )
	{
		InvalidateRect( NULL );
		UpdateWindow();
	}
}

// OnGetFont ----------------------------------------------------------------
BOOL CCodeTipCtrl::OnGetFont( WPARAM, LPARAM, LRESULT* pResult )
{
	*pResult = (LRESULT)__GetFont();
	return TRUE;// handled
}

// OnSetTipBkColor ----------------------------------------------------------
BOOL CCodeTipCtrl::OnSetTipBkColor( WPARAM wp, LPARAM, LRESULT* )
{
	m_crBk = (COLORREF)wp;
	InvalidateRect( NULL );
	UpdateWindow();
	return TRUE;// handled
}

// OnSetTipTextColor --------------------------------------------------------
BOOL CCodeTipCtrl::OnSetTipTextColor( WPARAM wp, LPARAM, LRESULT* )
{
	m_crText = (COLORREF)wp;
	InvalidateRect( NULL );
	UpdateWindow();
	return TRUE;// handled
}

// OnGetTipBkColor ----------------------------------------------------------
BOOL CCodeTipCtrl::OnGetTipBkColor( WPARAM, LPARAM, LRESULT* pResult )
{
	*pResult = m_crBk;
	return TRUE;// handled
}

// OnGetTipTextColor --------------------------------------------------------
BOOL CCodeTipCtrl::OnGetTipTextColor( WPARAM, LPARAM, LRESULT* pResult )
{
	*pResult = m_crText;
	return TRUE;// handled
}
