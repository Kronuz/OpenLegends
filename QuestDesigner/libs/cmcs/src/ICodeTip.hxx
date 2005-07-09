/////////////////////////////////////////////////////////////////////////////
//
// ICodeTip.hxx : Declaration of CICodeTipBase template class
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _ICODETIP_HXX__INCLUDED_
#define _ICODETIP_HXX__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Include Files ------------------------------------------------------------
#include "CodeTipCtrl.h"
#include "FontNotifyImpl.hxx"


/////////////////////////////////////////////////////////////////////////////
// CICodeTipBase

template<class Class, class TipCtrl, class Iface, const CLSID* pclsid,
	const GUID* plibid = &LIBID_CodeSense, WORD wMajor = CM_X_VERSION,
	WORD wMinor = 0, const IID* piidIface = &__uuidof(Iface)>
class ATL_NO_VTABLE CICodeTipBase
:	public CComCoClass<Class, pclsid>
,	public IDispatchImpl<Iface, piidIface, plibid, wMajor>
,	public CFontNotifySinkImpl< CICodeTipBase< Class, TipCtrl, Iface, pclsid,
		plibid, wMajor, wMinor, piidIface >, TipCtrl >
{

// CICodeTipBase Protected Member Functions ------------------------------
protected:

	// Construction
	CICodeTipBase()
	{
	}

	// Destruction
	virtual ~CICodeTipBase()
	{
	}

BEGIN_COM_MAP(CICodeTipBase)
	COM_INTERFACE_ENTRY_IID(*piidIface, Iface)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


// ICodeTip Public Member Functions --------------------------------------
public:

	STDMETHOD(get_hWnd)( long* phwnd )
	{
		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		CHECK_PTR( phwnd );
		*phwnd = reinterpret_cast<long>( m_pCtrl->m_hWnd );
		return S_OK;
	}

	STDMETHOD(get_TipText)( BSTR* Text )
	{
		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		CHECK_PTR( Text );

		LPCTSTR pszText = m_pCtrl->GetTipText();

		if( NULL == pszText )
			return E_OUTOFMEMORY;

		USES_CONVERSION;
		*Text = T2BSTR( pszText );

		return S_OK;
	}

	STDMETHOD(put_TipText)( BSTR Text )
	{
		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		LPTSTR psz = BSTR2TSTR( Text );

		if( NULL == psz )
			return E_OUTOFMEMORY;

		m_pCtrl->SetWindowText( psz );

		delete [] psz;

		return S_OK;
	}

	STDMETHOD(get_TipTextLength)( long* Length )
	{
		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		CHECK_PTR( Length );

		*Length = m_pCtrl->GetWindowTextLength();

		return S_OK;
	}

	STDMETHOD(Destroy)()
	{
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return S_OK;// Already destroyed

		return m_pCtrl->DestroyCodeTip() ? S_OK : ERROR_CANCELLED;
	}

	STDMETHOD(get_BackColor)( OLE_COLOR* pcrColor )
	{
		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		CHECK_PTR( pcrColor );
		*pcrColor = (OLE_COLOR)m_pCtrl->SendMessage( TTM_GETTIPBKCOLOR );
		return S_OK;
	}

	STDMETHOD(put_BackColor)( OLE_COLOR crColor )
	{
		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		COLORREF cr = {0};
		HRESULT hResult = OleTranslateColor( crColor, NULL, &cr );

		if( SUCCEEDED( hResult ) )
			m_pCtrl->SendMessage( TTM_SETTIPBKCOLOR, (WPARAM)cr );

		return hResult;
	}

	STDMETHOD(get_TextColor)( OLE_COLOR* pcrColor )
	{
		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		CHECK_PTR( pcrColor );
		*pcrColor = (OLE_COLOR)m_pCtrl->SendMessage( TTM_GETTIPTEXTCOLOR );
		return S_OK;
	}

	STDMETHOD(put_TextColor)( OLE_COLOR crColor )
	{
		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		COLORREF cr = {0};
		HRESULT hResult = OleTranslateColor( crColor, NULL, &cr );

		if( SUCCEEDED( hResult ) )
			m_pCtrl->SendMessage( TTM_SETTIPTEXTCOLOR, (WPARAM)cr );

		return hResult;
	}

	STDMETHOD(get_Font)( IFontDisp** ppFont ) {
		return FONTNOTIFYSINKIMPL::get_Font( ppFont );
	}

	STDMETHOD(put_Font)( IFontDisp* pFont ) {
		return FONTNOTIFYSINKIMPL::put_Font( pFont );
	}

	STDMETHOD(putref_Font)( IFontDisp* pFont ) {
		return FONTNOTIFYSINKIMPL::putref_Font( pFont );
	}
};


/////////////////////////////////////////////////////////////////////////////

#endif//#ifndef _ICODETIP_HXX__INCLUDED_
