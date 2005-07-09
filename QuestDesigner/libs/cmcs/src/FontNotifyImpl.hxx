/////////////////////////////////////////////////////////////////////////////
//
// FontNotifyImpl.hxx : Declaration of CFontNotifyImpl
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _FONTNOTIFYIMPL_HXX__INCLUDED_
#define _FONTNOTIFYIMPL_HXX__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CFontNotifyImpl

template<class _Parent>
class ATL_NO_VTABLE CFontNotifyImpl
:	public CComObjectRootEx<CComSingleThreadModel>
,	public IPropertyNotifySink
{
protected:
	DWORD m_dwCookie;
	_Parent* m_pParent;


public:
	CFontNotifyImpl()
	: m_dwCookie( 0 )
	, m_pParent( NULL )
	{
	}

	virtual ~CFontNotifyImpl()
	{
	}

	STDMETHOD(OnChanged)( DISPID )
	{
		m_pParent->FireOnFontChanged();
		return S_OK;
	}

	STDMETHOD(OnRequestEdit)( DISPID ) {
		return S_OK;
	}

	HRESULT Init( _Parent* pParent, IFontDisp* pFont )
	{
		m_pParent = pParent;

		HRESULT hResult = E_FAIL;
		CComQIPtr<IConnectionPointContainer> pCPC( pFont );

		if( pCPC )
		{
			CComPtr<IConnectionPoint> pCP;
	
			hResult = pCPC->FindConnectionPoint( IID_IPropertyNotifySink,
				&pCP );

			if( SUCCEEDED( hResult ) )
				hResult = pCP->Advise( this, &m_dwCookie );
		}

		return hResult;
	}


BEGIN_COM_MAP( CFontNotifyImpl<_Parent> )
	COM_INTERFACE_ENTRY(IPropertyNotifySink)
END_COM_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CFontNotifySinkImpl

template<class _Base, class _Ctrl>
class ATL_NO_VTABLE CFontNotifySinkImpl
:	public CComObjectRoot
{
	typedef CFontNotifySinkImpl<_Base, _Ctrl>		FONTNOTIFYSINKIMPL;

public:
	DECLARE_PROTECT_FINAL_CONSTRUCT();


protected:
	_Ctrl* m_pCtrl;

	typedef CComObject< CFontNotifyImpl<_Base> >	CFontNotifyObj;
	CFontNotifyObj* m_pSink;


public:
	CFontNotifySinkImpl()
	: m_pCtrl( NULL )
	, m_pSink( NULL )
	{
	}

	virtual ~CFontNotifySinkImpl()
	{
	}

	virtual HRESULT FinalConstruct()
	{
		HRESULT hResult = CFontNotifyObj::CreateInstance( &m_pSink );

		if( SUCCEEDED( hResult ) )
			m_pSink->AddRef();

		return hResult;
	}

	virtual void FinalRelease()
	{
		if( m_pSink )
			m_pSink->Release();

		m_pSink = NULL;
	}

	HRESULT SetControl( _Ctrl* pCtrl )
	{
		m_pCtrl = pCtrl;
		return m_pSink->Init( (_Base*)this, pCtrl->m_pFont );
	}

	virtual void FireOnFontChanged()
	{
		if( m_pCtrl && ::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			m_pCtrl->OnFontChanged();
	}

	STDMETHOD(get_Font)( IFontDisp** ppFont )
	{
		CHECK_PTR( ppFont );

		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		*ppFont = m_pCtrl->m_pFont;

		if( *ppFont )
			(*ppFont)->AddRef();

		return S_OK;
	}

	STDMETHOD(put_Font)( IFontDisp* pFont )
	{
		CHECK_PTR( pFont );

		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		HRESULT hResult = E_POINTER;
		CComQIPtr<IFont> pF( pFont );

		if( pF )
		{
			CComPtr<IFont> pFont;

			hResult = pF->Clone( &pFont );

			if( SUCCEEDED( hResult ) )
			{
				CComPtr<IFontDisp> pDisp = NULL;

				hResult = pFont->QueryInterface( IID_IFontDisp,
					(LPVOID*)&pDisp );

				if( SUCCEEDED( hResult ) )
					m_pCtrl->m_pFont = pDisp;
			}
		}

		if( SUCCEEDED( hResult ) )
		{
			m_pSink->Init( (_Base*)this, m_pCtrl->m_pFont );
			FireOnFontChanged();
		}

		return hResult;
	}

	STDMETHOD(putref_Font)( IFontDisp* pFont )
	{
		CHECK_PTR( pFont );

		ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
		if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
			return E_POINTER;

		m_pCtrl->m_pFont = pFont;
		m_pSink->Init( (_Base*)this, m_pCtrl->m_pFont );
		FireOnFontChanged();
		return S_OK;
	}
};


/////////////////////////////////////////////////////////////////////////////

#endif//#ifndef _FONTNOTIFYIMPL_HXX__INCLUDED_
