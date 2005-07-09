/////////////////////////////////////////////////////////////////////////////
//
// Wnd.h : Declaration of CWnd
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _WND_H__INCLUDED_
#define _WND_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Rect.h"
#include "RegClassList.h"

#define MSG_HANDLER(fnc)			virtual BOOL fnc( WPARAM, LPARAM, LRESULT* );

#define BEGIN_HANDLE_MSG(Class)		LRESULT Class::WindowProc( UINT msg, WPARAM wp, LPARAM lp ) \
									{ \
										LRESULT lResult = 0; \
										switch( msg ) \
										{

#define HANDLE_MSG(msg,fnc)				case msg: \
											if( fnc( wp, lp, &lResult ) ) \
												return lResult; \
											break;

#define END_HANDLE_MSG(Base)			} \
										return Base::WindowProc( msg, wp, lp ); \
									}

#define DECLARE_MSG_HANDLER()		virtual LRESULT WindowProc( UINT, WPARAM, LPARAM );


/////////////////////////////////////////////////////////////////////////////
// CWnd

class CWnd
{

// Public Member Variables --------------------------------------------------
public:
	HWND m_hWnd;

#ifdef _ACTIVEX
	CComPtr<IFontDisp> m_pFont;
#endif


// Protected Member Variables -----------------------------------------------
protected:
	WNDPROC m_OldWndProc;
	LPDISPATCH m_lpDispatch;
	static CRegClassList m_lClasses;
	MSG m_msgLast;


// Public Member Functions --------------------------------------------------
public:

	// Construction
	CWnd();

	// Destruction
	virtual ~CWnd();

	// Creation
	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
		DWORD dwStyle, LPCRECT lpRect, HWND hwndParent, UINT nID = 0 );

	// Inlines
	inline HWND GetSafeHwnd();
	inline LRESULT SendMessage( UINT msg, WPARAM wp = 0, LPARAM lp = 0 );
	inline LRESULT PostMessage( UINT msg, WPARAM wp = 0, LPARAM lp = 0 );
	inline BOOL ShowWindow( int nCmdShow );
	inline void UpdateWindow();
	inline void GetWindowRect( LPRECT lpRect ) const;
	inline void GetClientRect( LPRECT lpRect ) const;
	inline HDC GetDC() const;
	inline int ReleaseDC( HDC hdc );
	inline HDC BeginPaint( LPPAINTSTRUCT lpPaint );
	inline void EndPaint( LPPAINTSTRUCT lpPaint );
	inline void MoveWindow( LPCRECT lpRect, BOOL bRepaint = TRUE );

	inline void MoveWindow( int x, int y, int nWidth, int nHeight,
		BOOL bRepaint = TRUE );

	inline int GetWindowTextLength() const;
	inline int GetWindowText( LPTSTR lpszStringBuf, int nMaxCount ) const;
	inline void InvalidateRect( LPCRECT lpRect, BOOL bErase = TRUE );
	inline void SetWindowText( LPCTSTR lpszText );
	inline LONG GetWindowLong( int nIndex ) const;
	inline LONG SetWindowLong( int nIndex, LONG lNewVal ) const;
	inline void ClientToScreen( LPPOINT lpPoint ) const;
	inline void ScreenToClient( LPPOINT lpPoint ) const;

	void ScreenToClient( LPRECT lpRect ) const;
	void ClientToScreen(LPRECT lpRect) const;

	// Overrideables
	virtual BOOL DestroyWindow();

	static CWnd* FromHandle( HWND hWnd );

	static LPCTSTR RegisterClass( UINT nClassStyle, HCURSOR hCursor = NULL,
		HBRUSH hbrBackground = NULL, HICON hIcon = NULL );

	static BOOL RegisterClassEx( const WNDCLASS* lpWndClass );


	static HFONT GetDefaultFont()
	{
		HFONT hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

		if( NULL == hFont )
			hFont = (HFONT)GetStockObject( SYSTEM_FONT );

		ASSERT( NULL != hFont );
		return hFont;
	}


#ifdef _ACTIVEX

	virtual void OnFontChanged()
	{
		::CallWindowProc( m_OldWndProc, m_hWnd, WM_SETFONT,
			(WPARAM)__GetFont(), TRUE );

		InvalidateRect( NULL );
		UpdateWindow();
	}

#endif//#ifdef _ACTIVEX


// Protected Member Functions -----------------------------------------------
protected:

	// Overrideables
	virtual BOOL SubclassWindow( HWND hWnd );
	virtual HWND UnsubclassWindow();
	virtual LRESULT WindowProc( UINT msg, WPARAM wp, LPARAM lp );
	virtual LRESULT Default();
	virtual void OnDestroy();

	// Main window procedure
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wp,
		LPARAM lp );

	// Window procedure used for window classes registered with
	// CWnd::RegisterClass()
	//
	static LRESULT CALLBACK GlobalWndProc( HWND hwnd, UINT msg, WPARAM wp,
		LPARAM lp );

	virtual void OnPaint() {
		Default();
	}


#ifdef _ACTIVEX

	virtual HFONT __GetFont()
	{
		HFONT hFont = NULL;
		CComQIPtr<IFont> pFont( m_pFont );

		if( pFont )
			pFont->get_hFont( &hFont );

		return hFont;
	}

	virtual void OnSetFont( HFONT hFont, BOOL bRedraw )
	{
		UNREFERENCED_PARAMETER( bRedraw );

		Default();
		SetFontDisp( hFont );
	}

	virtual HRESULT SetFontDisp( HFONT hFont )
	{
		USES_CONVERSION;
		LOGFONT lf = {0};

		if( NULL == hFont )
			hFont = CWnd::GetDefaultFont();

		GetObject( hFont, sizeof(LOGFONT), &lf );

		HDC hdc = GetDC();
		double cyFont = ( ( (double)lf.lfHeight * 72.0 ) / 
			(double)GetDeviceCaps( hdc, LOGPIXELSY ) ) * 10000.0;

		ReleaseDC( hdc );

		FONTDESC fd =
		{
			sizeof(FONTDESC),
			T2OLE( lf.lfFaceName ),
			{ abs((int)cyFont), 0 },
			(SHORT)lf.lfWeight,
			(SHORT)lf.lfCharSet,
			(BOOL)lf.lfItalic,
			(BOOL)lf.lfUnderline,
			(BOOL)lf.lfStrikeOut
		};

		m_pFont = NULL;

		return OleCreateFontIndirect( &fd, IID_IFontDisp,
			reinterpret_cast<LPVOID*>( &m_pFont ) );
	}

#else

	virtual HFONT __GetFont() {
		return (HFONT)SendMessage( WM_GETFONT );
	}

	virtual void OnSetFont( HFONT hFont, BOOL bRedraw )
	{
		UNREFERENCED_PARAMETER( hFont );
		UNREFERENCED_PARAMETER( bRedraw );

		Default();
	}

#endif//#ifdef _ACTIVEX

};


// Implementation of inline functions
#include "Wnd.inl"

/////////////////////////////////////////////////////////////////////////////

#endif//#ifndef _WND_H__INCLUDED_
