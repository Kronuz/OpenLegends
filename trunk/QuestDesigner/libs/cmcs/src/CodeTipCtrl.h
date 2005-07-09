/////////////////////////////////////////////////////////////////////////////
//
// CodeTipCtrl.h : Declaration of CCodeTipCtrl
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _CODETIPCTRL_H__INCLUDED_
#define _CODETIPCTRL_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "DispatchWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CCodeTipCtrl

class CCodeTipCtrl : public CDispatchWnd
{

// Protected Member Variables -----------------------------------------------
protected:
	UINT m_nTipType;
	LPTSTR m_lpszText;
	COLORREF m_crBk;
	COLORREF m_crText;

#ifndef _ACTIVEX
	HFONT m_hfText;
#endif


// Public Member Functions --------------------------------------------------
public:

	// Destruction
	virtual ~CCodeTipCtrl();

	// Creation
	static CCodeTipCtrl* Create( CEdit* pEdit, UINT nType, POINT& pt );

	virtual void AutoSizeControl();

	virtual LPCTSTR GetTipText();
	virtual BOOL NotifyCancel();

	virtual BOOL DestroyCodeTip() {
		return m_pEdit->DestroyCodeTip();
	}

	// Overrides
	virtual LPDISPATCH GetDispatch();

#ifdef _ACTIVEX

	virtual void OnFontChanged() {
		AutoSizeControl();
	}

#endif//#ifdef _ACTIVEX


// Protected Member Functions -----------------------------------------------
protected:

	// Construction
	CCodeTipCtrl( CEdit* pEdit );

	// Creation / Initialization
	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
		DWORD dwStyle, LPCRECT lpRect, HWND hwndParent, UINT nID = 0 );

	virtual void InitControl();

	virtual void NotifyUpdate( UINT unNotify = CMN_CODETIPUPDATE ) {
		NotifyParent( unNotify );
	}

	// Overrides
	virtual void OnDestroy();
	virtual LRESULT NotifyParent( UINT unNotification, LPNMHDR lpNMHDR = NULL );
	virtual void OnSetFont( HFONT hFont, BOOL bRedraw );

	// Drawing
	virtual void OnPaint();
	virtual void DrawTipText( HDC hdc, RECT& rect, BOOL bCalcRect );

#ifndef _ACTIVEX

	virtual HFONT __GetFont() {
		return m_hfText;
	}

#endif//#ifndef _ACTIVEX

	// Message handlers
	DECLARE_MSG_HANDLER()
		MSG_HANDLER( OnChar )
		MSG_HANDLER( OnEraseBkgnd )
		MSG_HANDLER( OnLButtonDown )
		MSG_HANDLER( OnRButtonDown )
		MSG_HANDLER( OnMButtonDown )
		MSG_HANDLER( OnGetFont )
		MSG_HANDLER( OnSetTipBkColor )
		MSG_HANDLER( OnSetTipTextColor )
		MSG_HANDLER( OnGetTipBkColor )
		MSG_HANDLER( OnGetTipTextColor )
};



/////////////////////////////////////////////////////////////////////////////

#endif//#ifndef _CODELISTCTRL_H__INCLUDED_
