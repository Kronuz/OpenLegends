/////////////////////////////////////////////////////////////////////////////
//
// ICodeList.h : Declaration of CICodeList
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _ICODELIST_H__INCLUDED_
#define _ICODELIST_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Include Files ------------------------------------------------------------
#include "FontNotifyImpl.hxx"


/////////////////////////////////////////////////////////////////////////////
// CICodeList

class ATL_NO_VTABLE CICodeList
:	public CComCoClass<CICodeList, &CLSID_CodeList>
,	public IDispatchImpl<ICodeList, &IID_ICodeList, &LIBID_CodeSense, CM_X_VERSION>
,	public CFontNotifySinkImpl<CICodeList, CCodeListCtrl>
{

// CICodeList Protected Member Variables ------------------------------------
protected:

	cmCodeListSortStyle m_SortStyle;

	HIMAGELIST m_hImageList;
	CComPtr<IDispatch> m_spImageList;


// ICodeList Public Member Functions ----------------------------------------
public:

	STDMETHOD(get_hWnd)( long* phwnd );
	STDMETHOD(put_hImageList)( long hImageList );
	STDMETHOD(get_hImageList)( long* phImageList );
	STDMETHOD(get_SelectedItem)( long* plIndex );
	STDMETHOD(put_SelectedItem)( long ItemIndex );
	STDMETHOD(AddItem)( BSTR strAdd, VARIANT ImageIndex, VARIANT ItemData, long* plIndex );
	STDMETHOD(GetItemText)( long ItemIndex, BSTR* pstrVal );
	STDMETHOD(GetItemData)( long ItemIndex, long* plParam );
	STDMETHOD(FindString)( BSTR strFind, VARIANT bAcceptPartial, long* plItemIndex );
	STDMETHOD(Destroy)();
	STDMETHOD(DeleteItem)( long ItemIndex );
	STDMETHOD(get_ItemCount)( long* lItems );
	STDMETHOD(get_BackColor)( OLE_COLOR* pcrColor );
	STDMETHOD(put_BackColor)( OLE_COLOR crColor );
	STDMETHOD(get_TextColor)( OLE_COLOR* pcrColor );
	STDMETHOD(put_TextColor)( OLE_COLOR crColor );
	STDMETHOD(EnableHotTracking)( VARIANT bEnable );
	STDMETHOD(get_SortStyle)( cmCodeListSortStyle* SortStyle );
	STDMETHOD(put_SortStyle)( cmCodeListSortStyle SortStyle );
	STDMETHOD(InsertItem)( long lIndex, BSTR strAdd, VARIANT ImageIndex, VARIANT ItemData, long* plIndex );

	STDMETHOD(get_Font)( IFontDisp** ppFont ) {
		return FONTNOTIFYSINKIMPL::get_Font( ppFont );
	}

	STDMETHOD(put_Font)( IFontDisp* pFont ) {
		return FONTNOTIFYSINKIMPL::put_Font( pFont );
	}

	STDMETHOD(putref_Font)( IFontDisp* pFont ) {
		return FONTNOTIFYSINKIMPL::putref_Font( pFont );
	}

	STDMETHOD(get_ImageList)( IDispatch** ppImageList );
	STDMETHOD(put_ImageList)( IDispatch* pImageList );

	STDMETHOD(putref_ImageList)( IDispatch* pImageList ) {
		return put_ImageList( pImageList );
	}


// CICodeList Protected Member Functions ------------------------------------
protected:

	// Construction
	CICodeList();

	// Destruction
	virtual ~CICodeList();


BEGIN_COM_MAP(CICodeList)
	COM_INTERFACE_ENTRY(ICodeList)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

};


/////////////////////////////////////////////////////////////////////////////

#endif//#ifndef _ICODELIST_H__INCLUDED_
