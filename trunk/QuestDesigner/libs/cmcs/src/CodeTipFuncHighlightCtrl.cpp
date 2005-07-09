/////////////////////////////////////////////////////////////////////////////
//
// CodeTipFuncHighlightCtrl.cpp : Implementation of CCodeTipFuncHighlightCtrl
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include <commctrl.h>
#include "CodeTipFuncHighlightCtrl.h"

#ifdef _ACTIVEX
#include "editx.h"
#include "ICodeTipFuncHighlight.h"
#endif//#ifdef _ACTIVEX


//***************************************************************************
// Public Member Functions
//***************************************************************************

// Destruction --------------------------------------------------------------
CCodeTipFuncHighlightCtrl::~CCodeTipFuncHighlightCtrl()
{
}

// GetDispatch --------------------------------------------------------------
LPDISPATCH CCodeTipFuncHighlightCtrl::GetDispatch()
{
#ifdef _ACTIVEX

	if( NULL == m_lpDispatch )
	{
		CComObject<CICodeTipFuncHighlight>* pNew = NULL;
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

// Construction -------------------------------------------------------------
CCodeTipFuncHighlightCtrl::CCodeTipFuncHighlightCtrl( CEdit* pEdit )
: CCodeTipHighlightCtrl( pEdit )
, m_nArgument( (UINT)-1 )
{
	m_nTipType = CM_TIPSTYLE_FUNCHIGHLIGHT;
}

// NotifyUpdate -------------------------------------------------------------
void CCodeTipFuncHighlightCtrl::NotifyUpdate( UINT unNotify )
{
	CM_CODETIPFUNCHIGHLIGHTDATA hdr = {0};

	if( NotifyParent( unNotify, (LPNMHDR)&hdr ) )
		m_nArgument = hdr.nArgument;
}

// NotifyParent -------------------------------------------------------------
LRESULT CCodeTipFuncHighlightCtrl::NotifyParent( UINT unNotification,
												LPNMHDR lpNMHDR )
{
	CM_CODETIPFUNCHIGHLIGHTDATA hdr = {0};

	if( NULL == lpNMHDR )
		lpNMHDR = (LPNMHDR)&hdr;

	CM_CODETIPFUNCHIGHLIGHTDATA* pitfhData =
		(CM_CODETIPFUNCHIGHLIGHTDATA*)lpNMHDR;

	pitfhData->nArgument = m_nArgument;

	// Don't call CCodeTipHighlightCtrl::NotifyParent()!!!
	return CCodeTipCtrl::NotifyParent( unNotification, lpNMHDR );
}

// DrawTipText --------------------------------------------------------------
void CCodeTipFuncHighlightCtrl::DrawTipText( HDC hdc, RECT& rect,
											BOOL bCalcRect )
{
	GetHighlightFont();

	// Get the text selection range based on the current argument index
	GetSelection();

	// Now draw the tip with the appropriate selection range
	CCodeTipHighlightCtrl::DrawTipText( hdc, rect, bCalcRect );

	ReleaseHighlightFont();
}

// GetSelection -------------------------------------------------------------
void CCodeTipFuncHighlightCtrl::GetSelection()
{
	LPCTSTR lpszTip = GetTipText();
	int iTipLen = lstrlen( lpszTip ), iBegin = 0;

	m_iHighlightBegin = -1;
	m_iHighlightEnd = -1;

	for( int i = 0, iArg = -1; -1 == iArg && i < iTipLen; i++ )
	{
		switch( lpszTip[i] )
		{
		case L'(':
			iBegin = ++i;
		case L',':
			iArg = 1;
			break;
		}
	}

	if( 0 == (int)m_nArgument && iBegin > 1 )
	{
		m_iHighlightBegin = 0;
		m_iHighlightEnd = iBegin - 2;
		return;
	}

	if( 1 == iArg )
	{
		for( int i = iBegin; iArg != (int)m_nArgument && i < iTipLen; i++ )
		{
			if( L',' == lpszTip[i] )
			{
				iArg++;
				iBegin = ++i;
			}
		}
	}

	if( iArg == (int)m_nArgument )
	{
		for( int iEnd = iBegin; iEnd < iTipLen; iEnd++ )
		{
			if( L',' == lpszTip[iEnd] || L')' == lpszTip[iEnd] )
			{
				iEnd--;
				break;
			}
		}

		m_iHighlightBegin = iBegin;
		m_iHighlightEnd = iEnd;
	}
}
