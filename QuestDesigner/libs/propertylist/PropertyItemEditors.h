#ifndef __PROPERTYITEMEDITORS__H
#define __PROPERTYITEMEDITORS__H

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CPropertyItemEditors - Editors for Property controls
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001-2003 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef __PROPERTYITEM__H
#error PropertyItemEditors.h requires PropertyItem.h to be included first
#endif

#define PROP_TEXT_INDENT 2


/////////////////////////////////////////////////////////////////////////////
// Plain editor with a EDIT box

class CPropertyEditWindow : 
	public CWindowImpl< CPropertyEditWindow, CEdit, CControlWinTraits >
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_InplacePropertyEdit"), CEdit::GetWndClassName())

	bool m_fCancel;
	DWORD m_dwSlider;
	int m_iLowerLimit;
	int m_iHigherLimit;
	int m_nSlideInit;
	int m_nInitValue;
	int m_nOldValue;

	CPropertyEditWindow(int iLower, int iHigher, DWORD dwSlider) : 
		m_fCancel(false),
		m_iLowerLimit(iLower),
		m_iHigherLimit(iHigher),
		m_dwSlider(dwSlider),
		m_nSlideInit(-1),
		m_nInitValue(-1),
		m_nOldValue(-1)
	{
	}
	CPropertyEditWindow() : 
		m_fCancel(false),
		m_iLowerLimit(0),
		m_iHigherLimit(0),
		m_dwSlider(0),
		m_nSlideInit(-1),
		m_nInitValue(-1),
		m_nOldValue(-1)
	{
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	BEGIN_MSG_MAP(CPropertyEditWindow)
		MESSAGE_HANDLER(WM_USER_PROP_OWNED, OnIsOwned)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
	END_MSG_MAP()

	// Added by Kronuz
	LRESULT OnIsOwned(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if((HWND)lParam == m_hWnd) return TRUE;
		return FALSE;
	}
	// Added by Kronuz:
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if(m_dwSlider & 0xff000000) {
			POINT pt = { 0 };
			::GetCursorPos(&pt);
			ScreenToClient(&pt);

			RECT rcClient;
			GetClientRect(&rcClient);
			RECT rcSlider = rcClient;
			rcSlider.top = rcSlider.bottom - 8;

			// clicked on the slider:
			if(::PtInRect(&rcSlider, pt)==TRUE) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
				return 0;
			}
		}
		bHandled = FALSE;
		return 0;
	}
	// Added by Kronuz:
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if(m_nSlideInit != -1) {
			if((wParam&MK_LBUTTON) != MK_LBUTTON) {
				return OnLButtonUp(uMsg, wParam, lParam, bHandled);
			} else {
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

				RECT rcClient;
				GetClientRect(&rcClient);
				int len = (rcClient.right - rcClient.left);
				if(len>80) len = 80;
				int dif = ((m_iHigherLimit - m_iLowerLimit) * (m_nSlideInit - (pt.x - rcClient.left))) / len;
				long lVal = m_nInitValue - dif;

				if( lVal < m_iLowerLimit ) lVal = m_iLowerLimit;
				if( lVal > m_iHigherLimit  ) lVal = m_iHigherLimit;

				if((wParam&MK_CONTROL) != MK_CONTROL) {
					// snapping stuff:
					int snap = (int)((float)(m_iHigherLimit - m_iLowerLimit + 1) * 0.05);
					int octave = ((m_iHigherLimit - m_iLowerLimit + 1) / 8);

					for(int i=0; i<8; i++) {
						int pos = m_iLowerLimit + (i*octave);
						if(lVal > pos-snap && lVal < pos+snap) {
							lVal = pos;
							break;
						}
					}
					if( lVal < m_iLowerLimit ) lVal = m_iLowerLimit;
					if( lVal > m_iHigherLimit  ) lVal = m_iHigherLimit;
				}

				if(m_nOldValue != lVal) {
					SetFocus();
					m_nOldValue = lVal;
					TCHAR szVal[64] = { 0 };
					_stprintf(szVal, _T("%d"), lVal);
					SetWindowText(szVal);
					SetModify();
					SetSel(0, -1);
					::SendMessage(GetParent(), WM_USER_PROP_UPDATEPROPERTY, TRUE, (LPARAM) m_hWnd);
				}
				return 0;
			}
		} else {
			if((wParam&MK_LBUTTON) == MK_LBUTTON) {
				return OnLButtonDown(uMsg, wParam, lParam, bHandled);
			}
		}
		bHandled = FALSE;
		return 0;
	}
	// Added by Kronuz:
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if(m_nSlideInit != -1) {
			m_nSlideInit = -1;
			if( ::GetCapture() == m_hWnd ) ReleaseCapture();
			::SendMessage(GetParent(), WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
			return 0;
		}

		bHandled = FALSE;
		return 0;
	}
	// Added by Kronuz:
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if(m_dwSlider & 0xff000000) {
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			RECT rcClient;
			GetClientRect(&rcClient);
			RECT rcSlider = rcClient;
			rcSlider.top = rcSlider.bottom - 8;

			// clicked on the slider:
			if(::PtInRect(&rcSlider, pt)==TRUE) {
				m_nOldValue = -1;
				m_nSlideInit = (pt.x - rcSlider.left);

				int len = (rcClient.right - rcClient.left);
				if(len>80) len = 80;
				m_nInitValue = m_iLowerLimit + ((m_iHigherLimit - m_iLowerLimit) * m_nSlideInit) / len;
				if( m_nInitValue < m_iLowerLimit ) m_nInitValue = m_iLowerLimit;
				if( m_nInitValue > m_iHigherLimit  ) m_nInitValue = m_iHigherLimit;

				// tell the owner the property is about to change
				::SendMessage(GetParent(), WM_USER_PROP_EDITPROPERTY, 0, (LPARAM) m_hWnd);
				SetCapture();

				return OnMouseMove(uMsg, wParam, lParam, bHandled);
			}
		}

		bHandled = FALSE;
		return 0;
	}
	// Added by Kronuz:
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
		if(m_dwSlider & 0xff000000) {
			TCHAR szVal[64] = { 0 };
			GetWindowText(szVal, (sizeof(szVal) / sizeof(TCHAR)) - 1);
			long lVal = _ttol(szVal);

			CDC dc(GetDC());
			RECT rcClient;
			GetClientRect(&rcClient);

			RECT rcSlider = rcClient;
			rcSlider.top = rcSlider.bottom - 3;

			int len = (rcSlider.right - rcSlider.left);
			if(len>80) len = 80;

			int pos = (len * (lVal - m_iLowerLimit)) / (m_iHigherLimit - m_iLowerLimit);

			rcSlider.right = rcSlider.left + pos;
			dc.FillSolidRect(&rcSlider, m_dwSlider & 0x00ffffff);

			DWORD dwBkColor = m_dwSlider >> 24;
			dwBkColor |= dwBkColor << 8;
			dwBkColor |= dwBkColor << 8;
			rcSlider.left = rcSlider.right;
			rcSlider.right = rcClient.left + len;
			dc.FillSolidRect(&rcSlider, dwBkColor);
		}
		return lRes;
	}
	// Added by Kronuz:
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
	{
		if( ::GetCapture() == m_hWnd ) ReleaseCapture();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc();
		SetFont( CWindow(GetParent()).GetFont() );
		SetMargins(PROP_TEXT_INDENT, 0);   // Force EDIT margins so text doesn't jump

		return lRes;
	}
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		switch( wParam ) {
			case VK_ESCAPE:
				m_fCancel = true;
				// FALL THROUGH...
			case VK_RETURN: {
				// Force focus to parent to update value (see OnKillFocus()...)

				if(m_dwSlider & 0xff000000) {
					// Added by Kronuz:
					TCHAR szVal[64] = { 0 };
					GetWindowText(szVal, (sizeof(szVal) / sizeof(TCHAR)) - 1);
					long lVal = _ttol(szVal);

					if( lVal < m_iLowerLimit ) lVal = m_iLowerLimit;
					if( lVal > m_iHigherLimit  ) lVal = m_iHigherLimit;

					_stprintf(szVal, _T("%d"), lVal);
					SetWindowText(szVal);
					SetModify();
				}

				::SetFocus(GetParent());
				break;
			}
			case VK_TAB:
			case VK_UP:
			case VK_DOWN:
				return ::PostMessage(GetParent(), WM_USER_PROP_NAVIGATE, LOWORD(wParam), 0);
			case VK_LEFT:
				int lLow, lHigh;
				GetSel(lLow, lHigh);
				if( lLow != lHigh || lLow != 0 ) break;
				return ::PostMessage(GetParent(), WM_USER_PROP_NAVIGATE, LOWORD(wParam), 0);
			case VK_RIGHT:
				GetSel(lLow, lHigh);
				if( lLow != lHigh || lLow != GetWindowTextLength() ) break;
				return ::PostMessage(GetParent(), WM_USER_PROP_NAVIGATE, LOWORD(wParam), 0);
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		switch( LOWORD(wParam) ) {
			case VK_TAB:
			case VK_RETURN:
			case VK_ESCAPE:
				// Do not BEEP!!!!
				return 0;
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_fCancel = false;
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
		m_fCancel |= (GetModify() == FALSE);
		::SendMessage(GetParent(), m_fCancel ? WM_USER_PROP_CANCELPROPERTY : WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
		::PostMessage(GetParent(), WM_KILLFOCUS, 0, 0);
		return lRes;
	}
	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return DefWindowProc(uMsg, wParam, lParam) | DLGC_WANTALLKEYS | DLGC_WANTARROWS;
	}
};


/////////////////////////////////////////////////////////////////////////////
// General implementation of editor with button

template< class T, class TBase = CEdit >
class CPropertyDropWindowImpl : 
	public CWindowImpl< T, TBase, CControlWinTraits >
{
public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	CContainedWindowT<CButton> m_wndButton;

	bool m_fCancel;
	bool m_bWaitFocus;
	bool m_bReadOnly;
	int m_nButtonWidth;

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete (T*) this;
	}

	typedef CPropertyDropWindowImpl< T > thisClass;

	CPropertyDropWindowImpl() : m_nButtonWidth(0), m_bReadOnly(true), m_bWaitFocus(false), m_fCancel(false) {} // Added by Kronuz

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_USER_PROP_OWNED, OnIsOwned)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseButtonClick)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouseButtonClick)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		ALT_MSG_MAP(1) // Button
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_KEYDOWN, OnButtonKeyDown)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
	END_MSG_MAP()

	// Added by Kronuz
	LRESULT OnIsOwned(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if((HWND)lParam == m_hWnd) return TRUE;
		return ((HWND)lParam == m_wndButton.m_hWnd)?TRUE:FALSE;
	}
	
	// Added by Kronuz
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if(m_bReadOnly) {
			// show an arrow:
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
			return 0;
		}
		bHandled = FALSE;
		return 0;
	}
	// Modified by Kronuz:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
		RECT rcClient;
		GetClientRect(&rcClient);

		// Added by Kronuz:
		if(m_nButtonWidth == 0) m_nButtonWidth = (rcClient.bottom - rcClient.top); //(rcClient.bottom - rcClient.top);

		// Setup EDIT control
		SetFont( CWindow(GetParent()).GetFont() );
		ModifyStyle(WS_BORDER, ES_LEFT);
		SendMessage(EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG(PROP_TEXT_INDENT, m_nButtonWidth));

		// Create button
		RECT rcButton = { rcClient.right - m_nButtonWidth, rcClient.top, rcClient.right, rcClient.bottom };
		m_wndButton.Create(this, 1, m_hWnd, &rcButton, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_PUSHBUTTON | BS_OWNERDRAW);
		ATLASSERT(m_wndButton.IsWindow());
		m_wndButton.SetFont(GetFont());
		// HACK: Windows needs to repaint this guy again!
//		m_wndButton.SetFocus(); // Commented by Kronuz
		m_bReadOnly = true;
		return lRes;
	}
	// Added by Kronuz (The button was never explicitly destroyed):
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if( m_wndButton.IsWindow() ) m_wndButton.DestroyWindow();
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_fCancel = false;
		if( !m_bReadOnly ) {
			bHandled = FALSE;
		}
		else {
			// Set focus to button to prevent input
			m_wndButton.SetFocus();
			m_wndButton.Invalidate();
		}
		return 0;
	}
	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(m_bWaitFocus) return FALSE; // Added by Kronuz

		if( (HWND)wParam != m_wndButton && (HWND)wParam != m_hWnd) {
			m_fCancel |= ((BOOL)SendMessage(EM_GETMODIFY, 0, 0L) == FALSE);
			::SendMessage(GetParent(), m_fCancel ? WM_USER_PROP_CANCELPROPERTY : WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
			::PostMessage(GetParent(), WM_KILLFOCUS, 0, 0);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( m_bReadOnly ) { // Don't allow any editing
			return 0;
		}

		switch( wParam ) {
			case VK_F2:
			case VK_F4:
			case VK_SPACE:
				m_wndButton.Click();
				return 0;
			case VK_ESCAPE:
				m_fCancel = true;
				// FALL THROUGH...
			case VK_RETURN: {
				// Force focus to parent to update value (see OnKillFocus()...)
				::SetFocus(GetParent());
				break;
			}
			case VK_TAB:
			case VK_UP:
			case VK_DOWN:
				return ::PostMessage(GetParent(), WM_USER_PROP_NAVIGATE, LOWORD(wParam), 0);
			case VK_LEFT:
				int lLow, lHigh;
				SendMessage(EM_GETSEL, (WPARAM) &lLow, (LPARAM) &lHigh);
				if( lLow != lHigh || lLow != 0 ) break;
				return ::PostMessage(GetParent(), WM_USER_PROP_NAVIGATE, LOWORD(wParam), 0);
			case VK_RIGHT:
				SendMessage(EM_GETSEL, (WPARAM) &lLow, (LPARAM) &lHigh);
				if( lLow != lHigh || lLow != GetWindowTextLength() ) break;
				return ::PostMessage(GetParent(), WM_USER_PROP_NAVIGATE, LOWORD(wParam), 0);
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( m_bReadOnly ) { // Don't allow any editing
			return 0;
		}

		switch( LOWORD(wParam) ) {
			case VK_TAB:
			case VK_RETURN:
			case VK_ESCAPE:
				// Do not BEEP!!!!
				return 0;
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnMouseButtonClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// Don't allow selection or context menu for edit box
		if( !m_bReadOnly ) bHandled = FALSE;
		else m_wndButton.Click();
		return 0;
	}

	// Button
	LRESULT OnButtonKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		switch( wParam ) {
			case VK_TAB:
			case VK_UP:
			case VK_DOWN:
				return ::PostMessage(GetParent(), WM_USER_PROP_NAVIGATE, LOWORD(wParam), 0);
			case VK_F2:
			case VK_F4:
			case VK_SPACE:
				m_wndButton.Click();
				return 0;
			case VK_ESCAPE:
				::PostMessage(GetParent(), WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
				return 0;
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return DefWindowProc(uMsg, wParam, lParam) | DLGC_WANTALLKEYS | (m_bReadOnly?0:DLGC_WANTARROWS);
	}
};


/////////////////////////////////////////////////////////////////////////////
// Editor with calendar dropdown

class CPropertyDateWindow : 
	public CPropertyDropWindowImpl<CPropertyDateWindow>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_InplacePropertyDateEdit"), CEdit::GetWndClassName())

	CContainedWindowT<CMonthCalendarCtrl> m_wndCalendar;

	typedef CPropertyDropWindowImpl<CPropertyDateWindow> baseClass;

	BEGIN_MSG_MAP(CPropertyDateWindow)
		MESSAGE_HANDLER(WM_USER_PROP_OWNED, OnIsOwned)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
		NOTIFY_CODE_HANDLER(MCN_SELECT, OnDateSelect)
		CHAIN_MSG_MAP( baseClass )
		ALT_MSG_MAP(1) // Button
		CHAIN_MSG_MAP_ALT( baseClass, 1 )
		ALT_MSG_MAP(2) // Calendar
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	END_MSG_MAP()

	// Added by Kronuz
	LRESULT OnIsOwned(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if((HWND)lParam == m_hWnd) return TRUE;
		if((HWND)lParam == m_wndCalendar.m_hWnd) return TRUE;
		return baseClass::OnIsOwned(uMsg, wParam, lParam, bHandled);
	}
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRes = baseClass::OnCreate(uMsg, wParam, lParam, bHandled);
		PostMessage(WM_SETFOCUS); // Added by Kronuz

		// Create dropdown list (as hidden)
		m_wndCalendar.Create(this, 2, m_hWnd, &rcDefault, NULL, WS_POPUP | WS_BORDER);
		ATLASSERT(m_wndCalendar.IsWindow());
		m_wndCalendar.SetFont(GetFont());
		m_bReadOnly = false;

		return lRes;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( m_wndCalendar.IsWindow() ) m_wndCalendar.DestroyWindow();
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// Set selection
		TCHAR szDate[64] = { 0 };
		GetWindowText(szDate, (sizeof(szDate) / sizeof(TCHAR)) - 1);
		CComVariant v = szDate;
		v.ChangeType(VT_DATE);
		SYSTEMTIME st = { 0 };
		::VariantTimeToSystemTime(v.date, &st);
		m_wndCalendar.SetCurSel(&st);
		// Move the calendar under the item
		RECT rcCalendar;
		m_wndCalendar.GetMinReqRect(&rcCalendar);
		RECT rcWin;
		GetWindowRect(&rcWin);
		::OffsetRect(&rcCalendar, rcWin.left, rcWin.bottom);
		m_wndCalendar.SetWindowPos(HWND_TOPMOST, &rcCalendar, SWP_SHOWWINDOW);
		m_wndCalendar.SetFocus();
		return 0;
	}
	LRESULT OnDateSelect(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	{
		USES_CONVERSION;
		SYSTEMTIME st = { 0 };
		m_wndCalendar.GetCurSel(&st);
		st.wHour = 0; st.wMinute = 0; st.wSecond = 0; st.wMilliseconds = 0;
		CComVariant v;
		v.vt = VT_DATE;
		if( st.wYear > 0 ) ::SystemTimeToVariantTime(&st, &v.date);
		v.ChangeType(VT_BSTR);
		SetWindowText(OLE2CT(v.bstrVal));
		SetFocus();
		return 0;
	}

	// Calendar message handlers

	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		LRESULT lRes = m_wndCalendar.DefWindowProc();
		m_wndCalendar.ShowWindow(SW_HIDE);
		::PostMessage(GetParent(), WM_KILLFOCUS, 0, 0);
		return lRes;
	}

	// Ownerdrawn button message handler

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
		if( m_wndButton != lpdis->hwndItem ) return 0;
		CDCHandle dc(lpdis->hDC);
		// Paint as dropdown button
		dc.DrawFrameControl(&lpdis->rcItem, DFC_SCROLL, (lpdis->itemState & ODS_SELECTED) ? DFCS_SCROLLDOWN | DFCS_PUSHED : DFCS_SCROLLDOWN);
		return 0;
	}
};


/////////////////////////////////////////////////////////////////////////////
// Editor with dropdown list

class CPropertyListWindow : 
	public CPropertyDropWindowImpl<CPropertyListWindow>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_InplacePropertyList"), CEdit::GetWndClassName())

	CContainedWindowT<CListBox> m_wndList;
	int m_cyList;      // Used to resize the listbox when first shown

	typedef CPropertyDropWindowImpl<CPropertyListWindow> baseClass;

	BEGIN_MSG_MAP(CPropertyListWindow)
		MESSAGE_HANDLER(WM_USER_PROP_OWNED, OnIsOwned)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
		CHAIN_MSG_MAP(baseClass)
		ALT_MSG_MAP(1) // Button
		CHAIN_MSG_MAP_ALT(baseClass, 1)
		ALT_MSG_MAP(2) // List
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	END_MSG_MAP()

	// Added by Kronuz
	LRESULT OnIsOwned(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if((HWND)lParam == m_hWnd) return TRUE;
		if((HWND)lParam == m_wndList.m_hWnd) return TRUE;
		return baseClass::OnIsOwned(uMsg, wParam, lParam, bHandled);
	}
	void AddItem(LPCTSTR pstrItem)
	{
		ATLASSERT(m_wndList.IsWindow());
		ATLASSERT(!::IsBadStringPtr(pstrItem,-1));
		m_wndList.AddString(pstrItem);
		m_cyList = 0;
	}
	void SelectItem(int idx)
	{
		ATLASSERT(m_wndList.IsWindow());      
		m_wndList.SetCurSel(idx);
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// Create dropdown list (as hidden)
		RECT rc = CWindow::rcDefault;

		m_wndList.Create(this, 2, m_hWnd, &rc, NULL, WS_POPUP | WS_BORDER | WS_VSCROLL);
		ATLASSERT(m_wndList.IsWindow());
		m_wndList.SetFont( CWindow(GetParent()).GetFont() );

		// Go create the rest of the control...
		LRESULT lRes = baseClass::OnCreate(uMsg, wParam, lParam, bHandled);
		PostMessage(WM_SETFOCUS); // Added by Kronuz
		return lRes;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( m_wndList.IsWindow() ) m_wndList.DestroyWindow();
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Let the dropdown-box handle the keypress...
		if( m_wndList.GetStyle() & WS_VISIBLE ) {
			m_wndList.PostMessage(uMsg, wParam, lParam);
		}
		else {
			TCHAR szStr[] = { (TCHAR) wParam, _T('\0') };
			int idx = m_wndList.FindString(-1, szStr);
			if( idx == LB_ERR ) return 0;
			m_wndList.SetCurSel(idx);
			BOOL bDummy;
			OnKeyDown(WM_KEYDOWN, VK_RETURN, 0, bDummy);      
		}
		return 0; // Don't allow any editing
	}
	LRESULT OnButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if( m_cyList == 0 ) {
			// Resize list to fit all items (but not more than 140 pixels)
			const int MAX_HEIGHT = 140;
			int cy = m_wndList.GetCount() * m_wndList.GetItemHeight(0);
			m_cyList = min( MAX_HEIGHT, cy + (::GetSystemMetrics(SM_CYBORDER)*2) );
		}
		// Move the dropdown under the item
		RECT rcWin;
		GetWindowRect(&rcWin);
		RECT rc = { rcWin.left, rcWin.bottom, rcWin.right, rcWin.bottom + m_cyList };
		m_wndList.SetWindowPos(HWND_TOPMOST, &rc, SWP_SHOWWINDOW);
		return 0;
	}

	// List message handlers

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		switch( wParam ) {
			case VK_RETURN:
				{
					int idx = m_wndList.GetCurSel();
					if( idx >= 0 ) {
						// Copy text from list to item
						int len = m_wndList.GetTextLen(idx) + 1;
						LPTSTR pstr = (LPTSTR) _alloca(len * sizeof(TCHAR));
						m_wndList.GetText(idx, pstr);
						SetWindowText(pstr);
						// Announce the new value
						::SendMessage(GetParent(), WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
					}
				}
				SetFocus(); // Modified by Kronuz
				break;
			case VK_ESCAPE:
				// Announce the cancellation
				::SendMessage(GetParent(), WM_USER_PROP_CANCELPROPERTY, 0, (LPARAM) m_hWnd);
				SetFocus(); // Modified by Kronuz
				break;
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		LRESULT lRes = m_wndList.DefWindowProc();
		// Selected an item? Fake RETURN key to copy new value...
		BOOL bDummy;
		OnKeyDown(WM_KEYDOWN, VK_RETURN, 0, bDummy);
		return lRes;
	}
	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		LRESULT lRes = m_wndList.DefWindowProc();
		m_wndList.ShowWindow(SW_HIDE);
		::PostMessage(GetParent(), WM_KILLFOCUS, 0, 0);
		return lRes;
	}

	// Ownerdrawn button message handler

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
		if( m_wndButton != lpdis->hwndItem ) return 0;
		CDCHandle dc(lpdis->hDC);
		// Paint as dropdown button
		dc.DrawFrameControl(&lpdis->rcItem, DFC_SCROLL, (lpdis->itemState & ODS_SELECTED) ? DFCS_SCROLLDOWN | DFCS_PUSHED : DFCS_SCROLLDOWN);
		return 0;
	}
};


/////////////////////////////////////////////////////////////////////////////
// Editor with embedded ListBox control

class CPropertyComboWindow : 
	public CPropertyDropWindowImpl<CPropertyComboWindow, CStatic>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_InplacePropertyCombo"), CStatic::GetWndClassName())

	CContainedWindowT<CListBox> m_wndList;
	HWND m_hWndCombo;  // Listbox supplied by Property class
	int m_cyList;      // Used to resize the listbox when first shown

	typedef CPropertyDropWindowImpl<CPropertyComboWindow, CStatic> baseClass;

	CPropertyComboWindow() : 
		m_wndList(this, 2), 
		m_hWndCombo(NULL), 
		m_cyList(0)
	{
	}

	BEGIN_MSG_MAP(CPropertyComboWindow)
		MESSAGE_HANDLER(WM_USER_PROP_OWNED, OnIsOwned)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
		CHAIN_MSG_MAP( baseClass )
		ALT_MSG_MAP(1) // Button
		CHAIN_MSG_MAP_ALT( baseClass, 1 )
		ALT_MSG_MAP(2) // List
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	END_MSG_MAP()

	// Added by Kronuz
	LRESULT OnIsOwned(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if((HWND)lParam == m_hWnd) return TRUE;
		if((HWND)lParam == m_wndList.m_hWnd) return TRUE;
		return baseClass::OnIsOwned(uMsg, wParam, lParam, bHandled);
	}
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ATLASSERT(::IsWindow(m_hWndCombo));      
		m_wndList.SubclassWindow(m_hWndCombo);

		// Go create the rest of the control...
		LRESULT lRes = baseClass::OnCreate(uMsg, wParam, lParam, bHandled);
		PostMessage(WM_SETFOCUS); // Added by Kronuz

		return lRes;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( ::GetCapture() == m_wndList ) ::ReleaseCapture();
		if( m_wndList.IsWindow() ) m_wndList.UnsubclassWindow();
		if( ::IsWindowVisible(m_hWndCombo) ) ::ShowWindow(m_hWndCombo, SW_HIDE);
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc( m_hWnd );      
		RECT rcButton;
		m_wndButton.GetWindowRect(&rcButton);
		RECT rcClient;
		GetClientRect(&rcClient);
		rcClient.right -= rcButton.right - rcButton.left;
		DRAWITEMSTRUCT dis = { 0 };
		dis.hDC = dc;
		dis.hwndItem = m_wndList;
		dis.CtlID = m_wndList.GetDlgCtrlID();
		dis.CtlType = ODT_LISTBOX;
		dis.rcItem = rcClient;
		dis.itemState = ODS_DEFAULT | ODS_COMBOBOXEDIT;
		dis.itemID = m_wndList.GetCurSel();
		dis.itemData = (int) m_wndList.GetItemData(dis.itemID);
		m_wndList.SendMessage(OCM_DRAWITEM, dis.CtlID, (LPARAM) &dis);
		return 0;
	}
	LRESULT OnButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if( m_cyList == 0 ) {
			// Resize list to fit all items (but not more than 140 pixels)
			const int MAX_HEIGHT = 140;
			int cy = m_wndList.GetCount() * m_wndList.GetItemHeight(0);
			m_cyList = min( MAX_HEIGHT, cy + (::GetSystemMetrics(SM_CYBORDER)*2) );
		}
		// Move the dropdown under the item
		RECT rcWin;
		GetWindowRect(&rcWin);
		RECT rc = { rcWin.left, rcWin.bottom, rcWin.right, rcWin.bottom + m_cyList };
		m_wndList.SetWindowPos(HWND_TOPMOST, &rc, SWP_SHOWWINDOW);
		m_wndList.SetFocus();
		::SetCapture(m_wndList);
		return 0;
	}

	// List message handlers

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{      
		LRESULT lRes = m_wndList.DefWindowProc();
		::ReleaseCapture();
		m_wndList.ShowWindow(SW_HIDE);
		//
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		BOOL bOutside = TRUE;
		int iSel = m_wndList.ItemFromPoint(pt, bOutside);
		if( !bOutside ) {
			m_wndList.SetCurSel(iSel);
			Invalidate();
			// Announce the new value
			::PostMessage(GetParent(), WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
		}
		return lRes;
	}

	// Ownerdrawn button message handler

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
		if( m_wndButton != lpdis->hwndItem ) return 0;
		CDCHandle dc(lpdis->hDC);
		// Paint as dropdown button
		dc.DrawFrameControl(&lpdis->rcItem, DFC_SCROLL, (lpdis->itemState & ODS_SELECTED) ? DFCS_SCROLLDOWN | DFCS_PUSHED : DFCS_SCROLLDOWN);
		return 0;
	}
};


/////////////////////////////////////////////////////////////////////////////
// Editor with browse button

class CPropertyButtonWindow : 
	public CPropertyDropWindowImpl<CPropertyButtonWindow>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_InplacePropertyButton"), CEdit::GetWndClassName())

	IProperty* m_prop; // BUG: Dangerous reference

	typedef CPropertyDropWindowImpl<CPropertyButtonWindow> baseClass;

	BEGIN_MSG_MAP(CPropertyButtonWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_USER_PROP_OWNED, OnIsOwned)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		CHAIN_MSG_MAP( baseClass )
		ALT_MSG_MAP(1) // Button
		CHAIN_MSG_MAP_ALT( baseClass, 1 )
	END_MSG_MAP()

	// Added by Kronuz
	LRESULT OnIsOwned(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if((HWND)lParam == m_hWnd) return TRUE;
		return baseClass::OnIsOwned(uMsg, wParam, lParam, bHandled);
	}
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRes = baseClass::OnCreate(uMsg, wParam, lParam, bHandled);
		PostMessage(WM_SETFOCUS); // Added by Kronuz
		return lRes;
	}

	LRESULT OnButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ATLASSERT(m_prop);
		// Call Property class' implementation of BROWSE action
		m_prop->Activate(PACT_BROWSE, 0);
		::PostMessage(GetParent(), WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
		return 0;
	}

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
		if( m_wndButton != lpdis->hwndItem ) return 0;
		CDCHandle dc(lpdis->hDC);
		// Paint as ellipsis button
		dc.DrawFrameControl(&lpdis->rcItem, DFC_BUTTON, (lpdis->itemState & ODS_SELECTED) ? DFCS_BUTTONPUSH | DFCS_PUSHED : DFCS_BUTTONPUSH);
		dc.SetBkMode(TRANSPARENT);
		LPCTSTR pstrEllipsis = _T("...");
		dc.DrawText(pstrEllipsis, ::lstrlen(pstrEllipsis), &lpdis->rcItem, DT_CENTER | DT_EDITCONTROL | DT_SINGLELINE | DT_VCENTER);
		return 0;
	}
};

/////////////////////////////////////////////////////////////////////////////
// Editor with color peeker (Added by Kronuz)

#define WM_USER_COLOR			WM_USER + 100
#define WM_USER_COLORSET		WM_USER + 101
#define WM_USER_COLORCANCEL		WM_USER + 102

class CTransparentWindowImpl :
	public CWindowImpl<CTransparentWindowImpl>
{
	COLORREF m_Color;
public:
	HWND m_hOwner;

	BEGIN_MSG_MAP(CTransparentWindowImpl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()

	CTransparentWindowImpl() : m_hOwner(NULL) {}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		return 1;
	}

	LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		SetCursor(LoadCursor(NULL, IDC_CROSS));
		return 0;
	}

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		::SendMessage(m_hOwner, WM_USER_COLORCANCEL, (WPARAM)m_Color, 0);
		return 0;
	}

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		::SendMessage(m_hOwner, WM_USER_COLORSET, (WPARAM)m_Color, 0);
		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		CPoint Point(lParam);
		ClientToScreen(&Point);
		PeekColor(Point);

		return 0;
	}
	void PeekColor(const CPoint &Point) 
	{
		// create a DC for the screen and create
		// a memory DC compatible to screen DC
		HDC hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

		COLORREF Color = GetPixel(hScrDC, Point.x, Point.y);
		DeleteDC(hScrDC);

		if(m_Color != Color) {
			m_Color = Color;
			::SendMessage(m_hOwner, WM_USER_COLOR, (WPARAM)m_Color, 0);
		}
	}
};
class CCaptureColorDlg : public CDialogImpl<CCaptureColorDlg>
{
// Data
private:
	CTransparentWindowImpl m_Window;

	COLORREF m_CurrColor;
public:
	COLORREF m_OldColor;
	COLORREF m_Color;

	enum { IDD = IDD_COLORPEEK };

	BEGIN_MSG_MAP(CCaptureColorDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		MESSAGE_HANDLER(WM_PAINT, OnPaint)

		MESSAGE_HANDLER(WM_USER_COLOR, OnColor)
		MESSAGE_HANDLER(WM_USER_COLORSET, OnColorSet)
		MESSAGE_HANDLER(WM_USER_COLORCANCEL, OnColorCancel)

		MESSAGE_HANDLER(WM_KEYDOWN, OnColorCancel)

		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnColor(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_CurrColor = (COLORREF)wParam;
		char ret[10];
		sprintf(ret,"#%02X%02X%02X", GetRValue(m_CurrColor), GetGValue(m_CurrColor), GetBValue(m_CurrColor));
		SetDlgItemText(IDC_STATIC2, ret);
		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnColorCancel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_Color = m_OldColor;
		PostMessage(WM_COMMAND, IDCANCEL);
		return 0;
	}

	LRESULT OnColorSet(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_Color = (COLORREF)wParam;
		PostMessage(WM_COMMAND, IDOK);
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		char ret[10];
		m_OldColor = m_Color;
		sprintf(ret,"#%02X%02X%02X", GetRValue(m_OldColor), GetGValue(m_OldColor), GetBValue(m_OldColor));
		SetDlgItemText(IDC_STATIC1, ret);

		RECT rcWindow = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };

		m_Window.m_hOwner = m_hWnd;
		m_Window.Create(GetParent(), rcWindow, NULL, WS_POPUP, WS_EX_TRANSPARENT);
		::ShowWindow(m_Window.m_hWnd, SW_SHOWNOACTIVATE);

		return 0;
	}
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		CPaintDC dc(m_hWnd);

		HBRUSH hOldBrush = dc.SelectStockBrush(NULL_BRUSH);

		RECT rcBox = { 110, 11, 210, 30 };
		dc.FillSolidRect(&rcBox, m_OldColor);
		dc.Rectangle(&rcBox);

		::OffsetRect(&rcBox, 0, 26);
		dc.FillSolidRect(&rcBox, m_CurrColor);
		dc.Rectangle(&rcBox);

		dc.SelectBrush(hOldBrush);

		return TRUE;
	}
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_Window.DestroyWindow();

		EndDialog(wID);
		return 0;
	}
};

class CPropertyColorWindow : 
	public CPropertyDropWindowImpl<CPropertyColorWindow>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_InplacePropertyColor"), CEdit::GetWndClassName())

	//CCaptureColorWindowImpl m_Window;
	typedef CPropertyDropWindowImpl<CPropertyColorWindow> baseClass;

	BEGIN_MSG_MAP(CPropertyColorWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_USER_PROP_OWNED, OnIsOwned)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		CHAIN_MSG_MAP( baseClass )
		ALT_MSG_MAP(1) // Button
		CHAIN_MSG_MAP_ALT( baseClass, 1 )
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_nButtonWidth = 19; // Set up the button width.

		LRESULT lRes = baseClass::OnCreate(uMsg, wParam, lParam, bHandled);
		// Create background window (as hidden)
		//RECT rcWindow = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
		//m_Window.Create(this, 2, m_hWnd, &rcWindow, NULL, WS_OVERLAPPEDWINDOW|WS_VISIBLE );
		//m_Window.Create(NULL, rcWindow, NULL, WS_POPUP );
		//ATLASSERT(m_Window.IsWindow());
		//m_wndCalendar.SetFont(GetFont());
		m_bReadOnly = false;
		return lRes;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		//if( m_Window.IsWindow() ) m_Window.DestroyWindow();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnIsOwned(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if((HWND)lParam == m_hWnd) return TRUE;
		//if((HWND)lParam == m_Window.m_hWnd) return TRUE;
		return baseClass::OnIsOwned(uMsg, wParam, lParam, bHandled);
	}
	LRESULT OnButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		char szColor[10];
		GetWindowText(szColor, 10);
		int r,g,b;
		if(sscanf(szColor, "#%02X%02X%02X", &r, &g, &b) == 3) {
			CCaptureColorDlg dlg;
			dlg.m_Color = RGB(r, g, b);
			m_bWaitFocus = true; // Set to live without the focus (wait for something)
			dlg.DoModal();
			SetFocus();
			m_bWaitFocus = false;

			m_fCancel = (dlg.m_Color == dlg.m_OldColor);
			if(!m_fCancel) {
				sprintf(szColor, "#%02X%02X%02X", GetRValue(dlg.m_Color), GetGValue(dlg.m_Color), GetBValue(dlg.m_Color));
				SetWindowText(szColor);
				SetModify();
			}
			::SetFocus(GetParent());
		}

		return 0;
	}

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
		if( m_wndButton != lpdis->hwndItem ) return 0;

		CDCHandle dc(lpdis->hDC);
		CDC dcmem = ::CreateCompatibleDC(lpdis->hDC);

		HBITMAP hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_CAMARA));
		HBITMAP hOldBmp = dcmem.SelectBitmap(hBitmap);

		dc.DrawFrameControl(&lpdis->rcItem, DFC_BUTTON, (lpdis->itemState & ODS_SELECTED) ? DFCS_BUTTONPUSH | DFCS_PUSHED : DFCS_BUTTONPUSH);
		dc.BitBlt(lpdis->rcItem.left, lpdis->rcItem.top, 19, 14, dcmem, 0, 0, SRCCOPY);

		dcmem.SelectBitmap(hOldBmp);
		DeleteObject(hBitmap);
		return 0;
	}
};


#endif // __PROPERTYITEMEDITORS__H
