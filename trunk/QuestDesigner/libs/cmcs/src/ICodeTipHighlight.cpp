/////////////////////////////////////////////////////////////////////////////
//
// ICodeTipHighlight.cpp : Implementation of CICodeTipHighlight
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#ifdef _ACTIVEX

/////////////////////////////////////////////////////////////////////////////

#include "editx.h"
#include "ICodeTipHighlight.h"
#include "CodeTipHighlightCtrl.h"


//***************************************************************************
// CICodeTipHighlight Protected Member Functions
//***************************************************************************

// Construction -------------------------------------------------------------
CICodeTipHighlight::CICodeTipHighlight()
{
}


// Destruction --------------------------------------------------------------
CICodeTipHighlight::~CICodeTipHighlight()
{
}


//***************************************************************************
// ICodeTipHighlight Public Member Functions
//***************************************************************************

// get_HighlightStartPos ----------------------------------------------------
STDMETHODIMP CICodeTipHighlight::get_HighlightStartPos( long* plPos )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( plPos );

	*plPos = m_pCtrl->GetStartPos();

	return S_OK;
}

// put_HighlightStartPos ----------------------------------------------------
STDMETHODIMP CICodeTipHighlight::put_HighlightStartPos( long lPos )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	m_pCtrl->SetStartPos( lPos );

	return S_OK;
}

// get_HighlightEndPos ------------------------------------------------------
STDMETHODIMP CICodeTipHighlight::get_HighlightEndPos( long* plPos )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( plPos );

	*plPos = m_pCtrl->GetEndPos();

	return S_OK;
}

// put_HighlightEndPos ------------------------------------------------------
STDMETHODIMP CICodeTipHighlight::put_HighlightEndPos( long lPos )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	m_pCtrl->SetEndPos( lPos );

	return S_OK;
}



/////////////////////////////////////////////////////////////////////////////

#endif//#ifdef _ACTIVEX
