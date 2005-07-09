/////////////////////////////////////////////////////////////////////////////
//
// CodeTipHighlightCtrl.cpp : Implementation of CCodeTipHighlightCtrl
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include <commctrl.h>
#include "CodeTipHighlightCtrl.h"

#ifdef _ACTIVEX
#include "editx.h"
#include "ICodeTipHighlight.h"
#endif//#ifdef _ACTIVEX


//***************************************************************************
// Public Member Functions
//***************************************************************************

// Destruction --------------------------------------------------------------
CCodeTipHighlightCtrl::~CCodeTipHighlightCtrl()
{
	ReleaseHighlightFont();
}

// GetDispatch --------------------------------------------------------------
LPDISPATCH CCodeTipHighlightCtrl::GetDispatch()
{
#ifdef _ACTIVEX

	if( NULL == m_lpDispatch )
	{
		CComObject<CICodeTipHighlight>* pNew = NULL;
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

// SetStartPos --------------------------------------------------------------
void CCodeTipHighlightCtrl::SetStartPos( int iPos )
{
	m_iHighlightBegin = iPos;
}

// GetStartPos --------------------------------------------------------------
int CCodeTipHighlightCtrl::GetStartPos()
{
	return m_iHighlightBegin;
}

// SetEndPos ----------------------------------------------------------------
void CCodeTipHighlightCtrl::SetEndPos( int iPos )
{
	m_iHighlightEnd = iPos;
}

// GetEndPos ----------------------------------------------------------------
int CCodeTipHighlightCtrl::GetEndPos()
{
	return m_iHighlightEnd;
}


//***************************************************************************
// Protected Member Functions
//***************************************************************************

// Construction -------------------------------------------------------------
CCodeTipHighlightCtrl::CCodeTipHighlightCtrl( CEdit* pEdit )
: CCodeTipCtrl( pEdit )
, m_hfHighlight( NULL )
, m_iHighlightBegin( -1 )
, m_iHighlightEnd( -1 )
{
	m_nTipType = CM_TIPSTYLE_HIGHLIGHT;
}

// NotifyUpdate -------------------------------------------------------------
void CCodeTipHighlightCtrl::NotifyUpdate( UINT unNotify )
{
	CM_CODETIPHIGHLIGHTDATA hdr = {0};

	if( NotifyParent( unNotify, (LPNMHDR)&hdr ) )
	{
		m_iHighlightBegin = hdr.nHighlightStartPos;
		m_iHighlightEnd = hdr.nHighlightEndPos;
	}
}

// NotifyParent -------------------------------------------------------------
LRESULT CCodeTipHighlightCtrl::NotifyParent( UINT unNotification,
											LPNMHDR lpNMHDR )
{
	CM_CODETIPHIGHLIGHTDATA hdr = {0};

	if( NULL == lpNMHDR )
		lpNMHDR = (LPNMHDR)&hdr;

	CM_CODETIPHIGHLIGHTDATA* pithData =
		(CM_CODETIPHIGHLIGHTDATA*)lpNMHDR;

	pithData->nHighlightStartPos = m_iHighlightBegin;
	pithData->nHighlightEndPos = m_iHighlightEnd;

	return CCodeTipCtrl::NotifyParent( unNotification, lpNMHDR );
}

// DrawTipText --------------------------------------------------------------
void CCodeTipHighlightCtrl::DrawTipText( HDC hdc, RECT& rect, BOOL bCalcRect )
{
	GetHighlightFont();

	// Get normalized selection
	GetSelection();

	// Draw the tooltip text
	DrawTipTextEx( hdc, rect, bCalcRect, 0 );

	ReleaseHighlightFont();
}

// DrawTipTextEx ------------------------------------------------------------
void CCodeTipHighlightCtrl::DrawTipTextEx( HDC hdc, RECT& rect,
										  BOOL bCalcRect, int xIndent )
{
	LPCTSTR lpszTip = GetTipText();
	int iTipLen = lstrlen( lpszTip );
	HFONT hfOld = (HFONT)::SelectObject( hdc, __GetFont() );

	int iMaxLineWidth = 0;
	int x = rect.left + xIndent, y = rect.top + 1;
	int wBegin = 0, wEnd = 0, iLast = 0, yLine = 0;

	// Parse buffer for word wrapping
	while( GetNextWord( lpszTip, iTipLen, wBegin, wEnd ) )
	{
		// Determine how big the current word will be to see if we need to
		// wrap it to the next line.
		SIZE sWord = {0};
		int len = wEnd - iLast;

		CalcWordSize( hdc, iLast, len, sWord );

		// Keep track of maximum line height
		if( sWord.cy > yLine )
			yLine = sWord.cy;

		// See if we need to wrap to next line
		if( x != rect.left && x + sWord.cx > rect.right )
		{
			// Wrap line
			x = rect.left;
			y += yLine;
		}

		// Either draw the text or just update the width
		if( !bCalcRect )
			DrawWord( hdc, iLast, len, x, y );
		else
			x += sWord.cx;

		// Keep track of maximum line width
		if( x > iMaxLineWidth )
			iMaxLineWidth = x;

		// Move on to next word
		iLast = wEnd;
		wBegin = wEnd + 1;
	}

	if( bCalcRect )
	{
		rect.right = iMaxLineWidth + 2;
		rect.bottom = y + yLine + 2;
	}

	if( hfOld )
		::SelectObject( hdc, hfOld );
}

// GetSelection -------------------------------------------------------------
void CCodeTipHighlightCtrl::GetSelection()
{
	// Normalize selection
	if( m_iHighlightBegin > m_iHighlightEnd )
	{
		int iHighlightBegin = m_iHighlightBegin;

		m_iHighlightBegin = m_iHighlightEnd;
		m_iHighlightEnd = iHighlightBegin;
	}
}

// GetNextWord --------------------------------------------------------------
bool CCodeTipHighlightCtrl::GetNextWord( LPCTSTR lpszBuffer, int iBufferLen,
										int& iBegin, int& iEnd )
{
	// Skip any leading spaces
	while( iBegin < iBufferLen && L' ' == lpszBuffer[iBegin] )
		iBegin++;

	if( iBegin >= iBufferLen )
		return false;

	// Break word at next space
	for( iEnd = iBegin; iEnd < iBufferLen && L' ' != lpszBuffer[iEnd]; )
		iEnd++;

	iEnd = min( iEnd, iBufferLen );

	return true;
}

// CalcWordSize -------------------------------------------------------------
void CCodeTipHighlightCtrl::CalcWordSize( HDC hdc, int iWordBegin,
										 int iWordLen, SIZE& sWord )
{
	int iBegin = iWordBegin;
	int iWordEnd = iWordBegin + iWordLen - 1;

	while( iBegin <= iWordEnd )
	{
		SIZE size = {0};
		int iEnd = iWordEnd;
		LPCTSTR lpszWord = m_lpszText + iBegin;

		::SelectObject( hdc, __GetFont() );

		if( iBegin < m_iHighlightBegin )
			iEnd = min( iWordEnd, m_iHighlightBegin - 1 );
		else if( iBegin <= m_iHighlightEnd )
		{
			::SelectObject( hdc, GetHighlightFont() );
			iEnd = min( iWordEnd, m_iHighlightEnd );
		}

		int iLen = iEnd - iBegin + 1;

		VERIFY( GetTextExtentPoint32( hdc, lpszWord, iLen, &size ) );

		sWord.cx += size.cx;

		if( size.cy > sWord.cy )
			sWord.cy = size.cy;

		iBegin = iEnd + 1;
	}
}

// DrawWord -----------------------------------------------------------------
void CCodeTipHighlightCtrl::DrawWord( HDC hdc, int iWordBegin, int iWordLen,
									 int& x, int y )
{
	int iBegin = iWordBegin;
	int iWordEnd = iWordBegin + iWordLen - 1;

	while( iBegin <= iWordEnd )
	{
		int iEnd = iWordEnd;
		LPCTSTR lpszWord = m_lpszText + iBegin;

		::SelectObject( hdc, __GetFont() );

		if( iBegin < m_iHighlightBegin )
			iEnd = min( iWordEnd, m_iHighlightBegin - 1 );
		else if( iBegin <= m_iHighlightEnd )
		{
			::SelectObject( hdc, GetHighlightFont() );
			iEnd = min( iWordEnd, m_iHighlightEnd );
		}

		int iLen = iEnd - iBegin + 1;
		TextOut( hdc, x, y, lpszWord, iLen );

		SIZE size = {0};
		VERIFY( GetTextExtentPoint32( hdc, lpszWord, iLen, &size ) );
		x += size.cx;

		iBegin = iEnd + 1;
	}
}

// GetHighlightFont ---------------------------------------------------------
HFONT CCodeTipHighlightCtrl::GetHighlightFont()
{
	if( NULL == m_hfHighlight )
	{
		// Create a new bold font based on text font
		LOGFONT lf = {0};

		GetObject( __GetFont(), sizeof(LOGFONT), &lf );

		lf.lfWeight = FW_BOLD;
		m_hfHighlight = ::CreateFontIndirect( &lf );
	}

	ASSERT( NULL != m_hfHighlight );
	return m_hfHighlight;
}
