/////////////////////////////////////////////////////////////////////////////
//
// ICodeList.cpp : Implementation of CICodeList
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
#include "ICodeList.h"
#include "CodeListCtrl.h"


//***************************************************************************
// ICodeList Public Member Functions
//***************************************************************************

// get_hWnd -----------------------------------------------------------------
STDMETHODIMP CICodeList::get_hWnd( long* phwnd )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );

	CHECK_PTR( phwnd );
	*phwnd = reinterpret_cast<long>( m_pCtrl->m_hWnd );
	return S_OK;
}

// put_hImageList -----------------------------------------------------------
STDMETHODIMP CICodeList::put_hImageList( long hNewImageList )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	m_pCtrl->SetImageList( reinterpret_cast<HIMAGELIST>( hNewImageList ) );
	return S_OK;
}

// get_hImageList -----------------------------------------------------------
STDMETHODIMP CICodeList::get_hImageList( long* phImageList )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( phImageList );
	*phImageList = reinterpret_cast<long>( m_pCtrl->GetImageList() );
	return S_OK;
}

// get_SelectedItem ---------------------------------------------------------
STDMETHODIMP CICodeList::get_SelectedItem( long* plIndex )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( plIndex );
	*plIndex = m_pCtrl->GetCurSel();
	return S_OK;
}

// put_SelectedItem ---------------------------------------------------------
STDMETHODIMP CICodeList::put_SelectedItem( long ItemIndex )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	m_pCtrl->SelectItem( ItemIndex );
	return S_OK;
}

// AddItem ------------------------------------------------------------------
STDMETHODIMP CICodeList::AddItem( BSTR strAdd, VARIANT ImageIndex,
								 VARIANT ItemData, long* plIndex )
{
	return InsertItem( -1, strAdd, ImageIndex, ItemData, plIndex );
}

// GetItemText --------------------------------------------------------------
STDMETHODIMP CICodeList::GetItemText( long ItemIndex, BSTR* pstrVal )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( pstrVal );
	int iLen = m_pCtrl->GetItemText( ItemIndex, NULL, 0 );
	LPTSTR pszText = new TCHAR[ iLen + 1 ];

	if( NULL == pszText )
		return E_OUTOFMEMORY;

	ZeroMemory( pszText, sizeof(TCHAR) * (iLen + 1) );
	m_pCtrl->GetItemText( ItemIndex, pszText, iLen );

	USES_CONVERSION;
	*pstrVal = T2BSTR( pszText );

	delete [] pszText;

	return S_OK;
}

// GetItemData --------------------------------------------------------------
STDMETHODIMP CICodeList::GetItemData( long ItemIndex, long* plParam )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( plParam );
	*plParam = m_pCtrl->GetItemData( ItemIndex );
	return S_OK;
}

// FindString ---------------------------------------------------------------
STDMETHODIMP CICodeList::FindString( BSTR strFind, VARIANT bAcceptPartial,
									long* plItemIndex )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( plItemIndex );

	BOOL bPartial = TRUE;

	// bAcceptPartial flag is optional...
	if( !IS_VT_EMPTY( bAcceptPartial ) )
	{
		HRESULT hR = ::VariantChangeType( &bAcceptPartial, &bAcceptPartial,
			0, VT_BOOL );

		if( SUCCEEDED( hR ) )
			bPartial = ( VARIANT_FALSE != bAcceptPartial.boolVal );
	}

	
	// Convert string
	USES_CONVERSION;
	*plItemIndex = m_pCtrl->FindString( OLE2T( strFind ), bPartial );

	return S_OK;
}

// Destroy ------------------------------------------------------------------
STDMETHODIMP CICodeList::Destroy()
{
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return S_OK;// Already destroyed

	return m_pCtrl->DestroyCodeList() ? S_OK : ERROR_CANCELLED;
}

// DeleteItem ---------------------------------------------------------------
STDMETHODIMP CICodeList::DeleteItem( long ItemIndex )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	m_pCtrl->SendMessage( LVM_DELETEITEM, ItemIndex );
	return S_OK;
}

// get_ItemCount ------------------------------------------------------------
STDMETHODIMP CICodeList::get_ItemCount( long* lItems )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( lItems );
	*lItems = m_pCtrl->SendMessage( LVM_GETITEMCOUNT );
	return S_OK;
}

// get_BackColor ------------------------------------------------------------
STDMETHODIMP CICodeList::get_BackColor( OLE_COLOR* pcrColor )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( pcrColor );
	*pcrColor = m_pCtrl->SendMessage( LVM_GETBKCOLOR );
	return S_OK;
}

// put_BackColor ------------------------------------------------------------
STDMETHODIMP CICodeList::put_BackColor( OLE_COLOR crColor )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	COLORREF cr = {0};
	HRESULT hResult = OleTranslateColor( crColor, NULL, &cr );

	if( SUCCEEDED( hResult ) )
	{
		m_pCtrl->SendMessage( LVM_SETBKCOLOR, 0, (LPARAM)cr );
		m_pCtrl->SendMessage( LVM_SETTEXTBKCOLOR, 0, (LPARAM)cr );
	}

	return hResult;
}

// get_TextColor ------------------------------------------------------------
STDMETHODIMP CICodeList::get_TextColor( OLE_COLOR* pcrColor )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( pcrColor );
	*pcrColor = (OLE_COLOR)m_pCtrl->SendMessage( LVM_GETTEXTCOLOR );
	return S_OK;
}

// put_TextColor ------------------------------------------------------------
STDMETHODIMP CICodeList::put_TextColor( OLE_COLOR crColor )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	COLORREF cr = {0};
	HRESULT hResult = OleTranslateColor( crColor, NULL, &cr );

	if( SUCCEEDED( hResult ) )
		m_pCtrl->SendMessage( LVM_SETTEXTCOLOR, 0, (LPARAM)cr );

	return hResult;
}

// EnableHotTracking --------------------------------------------------------
STDMETHODIMP CICodeList::EnableHotTracking( VARIANT bEnable )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	BOOL bEnableTracking = TRUE;
	DWORD dwExStyle = LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE;

	if( !IS_VT_EMPTY( bEnable ) )
	{
		HRESULT hR = ::VariantChangeType( &bEnable, &bEnable, 0, VT_BOOL );

		if( SUCCEEDED( hR ) )
			bEnableTracking = ( VARIANT_FALSE != bEnable.boolVal );
	}

	m_pCtrl->SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE, dwExStyle,
		( bEnableTracking ? dwExStyle : 0 ) );

	return S_OK;
}

// get_SortStyle ------------------------------------------------------------
STDMETHODIMP CICodeList::get_SortStyle( cmCodeListSortStyle* SortStyle )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( SortStyle );
	*SortStyle = m_SortStyle;
	return S_OK;
}

// put_SortStyle ------------------------------------------------------------
STDMETHODIMP CICodeList::put_SortStyle( cmCodeListSortStyle SortStyle )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	DWORD dwStyle = m_pCtrl->GetWindowLong( GWL_STYLE );
	DWORD dwNewStyle = dwStyle & ~( LVS_SORTDESCENDING | LVS_SORTASCENDING );

	switch( SortStyle )
	{
	case cmCodeListSortNone:
		break;

	case cmCodeListSortAscending:
		dwNewStyle |= LVS_SORTASCENDING;
		break;

	case cmCodeListSortDescending:
		dwNewStyle |= LVS_SORTDESCENDING;
		break;

	default:
		return E_INVALIDARG;
	}

	if( dwStyle != dwNewStyle )
	{
		m_pCtrl->SetWindowLong( GWL_STYLE, dwNewStyle );

		SetWindowPos( m_pCtrl->m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE |
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
	}

	m_SortStyle = SortStyle;
	return S_OK;
}

// InsertItem ---------------------------------------------------------------
STDMETHODIMP CICodeList::InsertItem( long lIndex, BSTR strAdd,
									 VARIANT ImageIndex, VARIANT ItemData,
									 long* plIndex )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	long lIcon = 0, lParam = 0;

	// Image index is optional...
	if( !IS_VT_EMPTY( ImageIndex ) )
	{
		HRESULT hR = ::VariantChangeType( &ImageIndex, &ImageIndex, 0, VT_I4 );

		if( SUCCEEDED( hR ) )
			lIcon = ImageIndex.lVal;
	}

	// Item data is optional...
	if( !IS_VT_EMPTY( ItemData ) )
	{
		HRESULT hR = ::VariantChangeType( &ItemData, &ItemData, 0, VT_I4 );

		if( SUCCEEDED( hR ) )
			lParam = ItemData.lVal;
	}

	// Convert string
	LPTSTR psz = BSTR2TSTR( strAdd );

	if( NULL == psz )
		return E_OUTOFMEMORY;

	// Add to list view control
	int iItem = m_pCtrl->InsertItem( lIndex, psz, lIcon, lParam );

	if( plIndex )
		*plIndex = iItem;

	delete [] psz;

	return S_OK;
}

// get_ImageList ------------------------------------------------------------
STDMETHODIMP CICodeList::get_ImageList( IDispatch** ppImageList )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	CHECK_PTR( ppImageList );

	*ppImageList = m_spImageList;

	if( m_spImageList )
		m_spImageList.p->AddRef();

	return S_OK;
}

// put_ImageList ------------------------------------------------------------
STDMETHODIMP CICodeList::put_ImageList( IDispatch* pImageList )
{
	ASSERT( m_pCtrl != NULL && ::IsWindow( m_pCtrl->m_hWnd ) );
	if( !::IsWindow( m_pCtrl->GetSafeHwnd() ) )
		return E_POINTER;

	if( NULL == pImageList )
	{
		if( m_hImageList )
			ImageList_Destroy( m_hImageList );

		m_hImageList = NULL;
		m_spImageList = NULL;

		return put_hImageList( (long)(HIMAGELIST)NULL );
	}

	CComDispatchDriver pDisp( pImageList );

	VARIANT var = {0};
	VariantInit( &var );

	HRESULT hResult = pDisp.GetPropertyByName( L"hImageList", &var );

	if( SUCCEEDED( hResult ) )
	{
		if( m_hImageList )
		{
			ImageList_Destroy( m_hImageList );
			m_hImageList = NULL;
			m_spImageList = NULL;
		}

		if( VT_I4 != V_VT( &var ) )
			hResult = E_INVALIDARG;
		else
		{
			m_hImageList = ImageList_Duplicate( (HIMAGELIST)var.lVal );
			m_spImageList = pImageList;
			put_hImageList( var.lVal );
		}
	}

	VariantClear( &var );

	return hResult;
}


//***************************************************************************
// CICodeList Protected Member Functions
//***************************************************************************

// Construction -------------------------------------------------------------
CICodeList::CICodeList()
: m_SortStyle( cmCodeListSortAscending )
, m_hImageList( NULL )
{
}

// Destruction --------------------------------------------------------------
CICodeList::~CICodeList()
{
	if( m_hImageList )
		ImageList_Destroy( m_hImageList );
}



/////////////////////////////////////////////////////////////////////////////

#endif//#ifdef _ACTIVEX
