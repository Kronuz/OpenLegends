#ifndef __PROPERTYLIST__H
#define __PROPERTYLIST__H

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CPropertyList - A Property List control
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001-2003 Bjarke Viksoe.
//   Thanks to Pascal Binggeli for fixing the disabled items.
//   Column resize supplied by Remco Verhoef, thanks.
//   Also thanks to Daniel Bowen, Alex Kamenev and others for fixes.
//
// Add the following macro to the parent's message map:
//   REFLECT_NOTIFICATIONS()
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
// Modified by Kronuz on September 8th, 2003

#ifndef __cplusplus
#error WTL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
#error PropertyList.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
#error PropertyList.h requires atlctrls.h to be included first
#endif


// Extended List styles
#define PLS_EX_CATEGORIZED     0x00000001
#define PLS_EX_SORTED          0x00000002
#define PLS_EX_XPLOOK          0x00000004
#define PLS_EX_SHOWSELALWAYS   0x00000008
#define PLS_EX_SINGLECLICKEDIT 0x00000010
#define PLS_EX_NOCOLUMNRESIZE  0x00000020

// Include property base class
#include "PropertyItem.h"

// Include property implementations
#include "PropertyItemEditors.h"
#include "PropertyItemImpl.h"



/////////////////////////////////////////////////////////////////////////////
// Category property

class CCategoryProperty : public CProperty
{
public:
	CSimpleArray<IProperty*> m_arrItems;
	bool m_fExpanded;

	CCategoryProperty(LPCTSTR pstrName, LPARAM lParam) : CProperty(pstrName, lParam), m_fExpanded(true)
	{
	}
	virtual ~CCategoryProperty()
	{
		// Need to delete hidden items too
		for( int i = 0; i < m_arrItems.GetSize(); i++ ) delete m_arrItems[i];
	}

	BYTE GetKind() const 
	{ 
		return PROPKIND_CATEGORY; 
	}
	void DrawName(PROPERTYDRAWINFO& di)
	{
		CDCHandle dc(di.hDC);
		HFONT hOldFont = dc.SelectFont(di.CategoryFont);
		if( di.dwExtStyle & PLS_EX_XPLOOK ) {
			di.clrSelText = di.clrText;
			
			di.clrBack = ::GetSysColor(COLOR_3DFACE);
			di.clrSelBack = ::GetSysColor(COLOR_3DFACE);
			CProperty::DrawName(di);
			if( di.state & ODS_SELECTED ) {
				RECT rcFocus = di.rcItem;
				::InflateRect(&rcFocus, -1, -1);
				SIZE size;
				dc.GetTextExtent(m_pszName, ::lstrlen(m_pszName), &size);
				rcFocus.right = rcFocus.left + size.cx + 3;
				rcFocus.bottom = rcFocus.top + size.cy;
				dc.DrawFocusRect(&rcFocus);
			}
		}
		else {
			CProperty::DrawName(di);
		}
		dc.SelectFont(hOldFont);
	}
	void DrawValue(PROPERTYDRAWINFO& di) 
	{ 
		if( di.dwExtStyle & PLS_EX_XPLOOK ) {
			::FillRect(di.hDC,  &di.rcItem, ::GetSysColorBrush(COLOR_3DFACE));
		}
	}
	BOOL Activate(UINT action, LPARAM /*lParam*/)
	{
		switch( action ) {
			case PACT_SPACE:
			case PACT_EXPAND:
			case PACT_DBLCLICK:
				::SendMessage(m_hWndOwner, IsExpanded() ? WM_USER_PROP_COLLAPSE : WM_USER_PROP_EXPAND, 0, (LPARAM)this );
				break;
		}
		return TRUE;
	}

	BOOL IsExpanded() const
	{
		return m_fExpanded == true;
	}
	BOOL Expand(int idx)
	{
		ATLASSERT(::IsWindow(m_hWndOwner));
		CListBox ctrl(m_hWndOwner);
		ctrl.SetRedraw(FALSE);
		idx++;
		while( m_arrItems.GetSize() > 0 ) {
			IProperty* prop = m_arrItems[0];
			ATLASSERT(prop);
			m_arrItems.RemoveAt(0);
			int item = ctrl.InsertString(idx++, prop->GetName());
			ctrl.SetItemData(item, (DWORD) prop);
		}
		m_fExpanded = true;
		ctrl.SetRedraw(TRUE);
		ctrl.Invalidate();
		return TRUE;
	}
	BOOL Collapse(int idx)
	{
		ATLASSERT(::IsWindow(m_hWndOwner));
		CListBox ctrl(m_hWndOwner);
		ctrl.SetRedraw(FALSE);
		idx++;
		while( idx < ctrl.GetCount() ) {
			IProperty* prop = reinterpret_cast<IProperty*>(ctrl.GetItemData(idx));
			ATLASSERT(prop);
			if( prop->GetKind() == PROPKIND_CATEGORY ) break;
			ctrl.SetItemData(idx, 0); // Clear data now, so WM_DELETEITEM doesn't delete
			// the IProperty in the DeleteString() call below
			ctrl.DeleteString(idx);
			m_arrItems.Add(prop);
		}
		m_fExpanded = false;
		ctrl.SetRedraw(TRUE);
		ctrl.Invalidate();
		return TRUE;
	}
	IProperty* GetProperty(int iIndex) const
	{
		if( iIndex < 0 || iIndex >= m_arrItems.GetSize() ) return NULL;
		return m_arrItems[iIndex];
	}
};

inline HPROPERTY PropCreateCategory(LPCTSTR pstrName, LPARAM lParam=0)
{
	return new CCategoryProperty(pstrName, lParam);
}


/////////////////////////////////////////////////////////////////////////////
// CPropertyList control

template< class T, class TBase = CListBox, class TWinTraits = CWinTraitsOR<LBS_OWNERDRAWVARIABLE|LBS_NOTIFY> >
class ATL_NO_VTABLE CPropertyListImpl : 
	public CWindowImpl< T, TBase, TWinTraits >,
	public COwnerDraw< CPropertyListImpl >
{
public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	enum { CATEGORY_INDENT = 16 };

	PROPERTYDRAWINFO m_di;
	HWND m_hwndInplace;
	int  m_iInplaceIndex;
	DWORD m_dwExtStyle;
	CFont m_TextFont;
	CFont m_CategoryFont;
	CPen m_BorderPen;
	int m_iPrevious;
	int m_iPrevXGhostBar;
	int m_iMiddle;
	bool m_bColumnFixed;
	bool m_bLockInplaceEditor; // Added by Kronuz

	CPropertyListImpl() : 
		m_hwndInplace(NULL), 
		m_iInplaceIndex(-1), 
		m_dwExtStyle(0UL),
		m_iMiddle(0),
		m_bColumnFixed(false),
		m_iPrevious(0),
		m_iPrevXGhostBar(0),
		m_bLockInplaceEditor(false) // Added by Kronuz
	{
	}

	// Operations

	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd==NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow(hWnd);
		if( bRet ) _Init();
		return bRet;
	}

	void SetExtendedListStyle(DWORD dwExtStyle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		// Assign styles
		if( dwExtStyle & PLS_EX_SORTED ) {
			ATLASSERT((dwExtStyle & PLS_EX_CATEGORIZED)==0); // We don't support sorted categories!
			ATLASSERT(GetStyle() & LBS_SORT);
			ATLASSERT(GetStyle() & LBS_HASSTRINGS);
		}
		m_dwExtStyle = dwExtStyle;
		// Recalc colours and fonts
		SendMessage(WM_SETTINGCHANGE);
	}
	DWORD GetExtendedListStyle() const
	{
		return m_dwExtStyle;
	}

	void ResetContent()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		_DestroyInplaceWindow();
		TBase::ResetContent();
	}
	HPROPERTY AddItem(HPROPERTY prop)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return NULL;
		prop->SetOwner(m_hWnd, NULL);
		int nItem = TBase::AddString(prop->GetName());
		if( nItem == LB_ERR ) return NULL;
		TBase::SetItemData(nItem, (DWORD_PTR) prop);
		return prop;
	}
	BOOL DeleteItem(HPROPERTY prop)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		ATLASSERT(prop->GetKind()!=PROPKIND_CATEGORY);
		// Delete *visible* property!
		int iIndex = FindProperty(prop);
		if( iIndex == -1 ) return FALSE;
		return TBase::DeleteString((UINT) iIndex) != LB_ERR;
	}
	HPROPERTY GetProperty(int index) const 
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(index!=-1);
		IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(index));
		if( prop == (IProperty*) -1 ) prop = NULL;
		return prop;
	}
	// Modified by Kronuz to add search by category
	HPROPERTY FindProperty(LPCTSTR pstrName, LPCTSTR pstrCategory = _T("")) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(pstrName);
		if( pstrName == NULL ) return NULL;
		// Find property from title
		LPCTSTR pstrLastCategory = _T("");
		for( int i = 0; i < GetCount(); i++ ) {
			IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(i));
			ATLASSERT(prop);
			if( prop->GetKind() == PROPKIND_CATEGORY ) {
				pstrLastCategory = prop->GetName();
			}
			if(::lstrcmp(pstrLastCategory, pstrCategory) == 0 || *pstrCategory == _T('\0')) {
				if( ::lstrcmp(prop->GetName(), pstrName) == 0 ) return prop;
				// Search properties in collapsed category items
				if( prop->GetKind() == PROPKIND_CATEGORY ) {
					const CCategoryProperty* category = static_cast<CCategoryProperty*>(prop);            
					int j = 0;
					for( IProperty* prop = NULL; (prop = category->GetProperty(j)) != NULL; j++ ) {
						if( ::lstrcmp(prop->GetName(), pstrName) == 0 ) return prop;
					}
				}
			}
		}
		return NULL;
	}
	HPROPERTY FindProperty(LPARAM lParam) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		// Find property from item-data
		// Method mostly supplied by DJ (thanks to this masked site reader)
		for( int i = 0; i < GetCount(); i++ ) {
			IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(i));
			ATLASSERT(prop);
			if( prop->GetItemData() == lParam ) return prop;
			// Search properties in collapsed category items
			if( prop->GetKind() == PROPKIND_CATEGORY ) {
				CCategoryProperty* pCategory = static_cast<CCategoryProperty*>(prop);
				IProperty* prop = NULL;
				int j = 0;
				while( (prop = pCategory->GetProperty(j++)) != NULL ) {
					if( prop->GetItemData() == lParam ) return prop;
				}
			}
		}
		return NULL;
	}
	int FindProperty(HPROPERTY prop) const
	{
		// Find *visible* property!
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return 0;
		for( int i = 0; i < GetCount(); i++ ) {
			if( TBase::GetItemData(i) == (DWORD_PTR) prop ) return i;
		}
		return -1;
	}
	void GetItemName(HPROPERTY prop, LPTSTR pstr, UINT cchMax) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return;
		::lstrcpyn(pstr, prop->GetName(), cchMax);
	}
	BOOL GetItemValue(HPROPERTY prop, VARIANT* pValue) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		ATLASSERT(pValue);
		if( prop == NULL || pValue == NULL ) return FALSE;
		return prop->GetValue(pValue);
	}
	// Added by Kronuz
	void SetItemMultivalue(HPROPERTY prop, bool bMultivalue)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return;
		// Set as multivalue and repaint
		prop->SetMultivalue(bMultivalue);
		// Thanks to Daniel Bowen for fixing the recreation of inplace editor here.
		int idx = FindProperty(prop);
		if( idx >= 0 ) {
			InvalidateItem(idx);
			if( idx == m_iInplaceIndex ) _SpawnInplaceWindow(prop, m_iInplaceIndex);
		}
	}
	BOOL SetItemValue(HPROPERTY prop, VARIANT* pValue)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		ATLASSERT(pValue);
		if( prop == NULL || pValue == NULL ) return FALSE;
		// Assign value and repaint
		BOOL bRes = prop->SetValue(*pValue);
		// Thanks to Daniel Bowen for fixing the recreation of inplace editor here.
		int idx = FindProperty(prop);
		if( idx >= 0 ) {
			InvalidateItem(idx);
			if( idx == m_iInplaceIndex ) _SpawnInplaceWindow(prop, m_iInplaceIndex);
		}
		return bRes;
	}
	LPARAM GetItemData(HPROPERTY prop) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return 0;
		return prop->GetItemData();
	}
	void SetItemData(HPROPERTY prop, LPARAM dwData)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return;
		prop->SetItemData(dwData);
	}
	BOOL GetItemEnabled(HPROPERTY prop) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return FALSE;
		return prop->IsEnabled();
	}
	void SetItemEnabled(HPROPERTY prop, BOOL bEnable)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return;
		prop->SetEnabled(bEnable);
		InvalidateItem(FindProperty(prop));
	}
	BOOL ExpandItem(HPROPERTY prop)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return FALSE;
		if( prop->GetKind() != PROPKIND_CATEGORY ) return FALSE;
		CCategoryProperty* pCategory = static_cast<CCategoryProperty*>(prop);
		if( pCategory->Expand(FindProperty(prop)) ) {
			// Let owner know
			NMPROPERTYITEM nmh = { m_hWnd, GetDlgCtrlID(), PIN_EXPANDING, prop };
			::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh);
		}
		return TRUE;
	}
	BOOL CollapseItem(HPROPERTY prop)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(prop);
		if( prop == NULL ) return FALSE;
		if( prop->GetKind() != PROPKIND_CATEGORY ) return FALSE;
		// Destroy in-place window
		_DestroyInplaceWindow(); 
		// Let property do actual collapse
		CCategoryProperty* pCategory = static_cast<CCategoryProperty*>(prop);
		if( pCategory->Collapse(FindProperty(prop)) ) {
			// Let owner know
			NMPROPERTYITEM nmh = { m_hWnd, GetDlgCtrlID(), PIN_COLLAPSING, prop };
			::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh);
		}
		return TRUE;
	}
	int GetColumnWidth() const
	{
		return m_iMiddle;
	}
	void SetColumnWidth(int iWidth)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		m_iMiddle = iWidth;
		m_bColumnFixed = true;
		Invalidate();
	}
	void InvalidateItem(int idx)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		if( idx == -1 ) return;
		RECT rc;
		GetItemRect(idx, &rc);
		InvalidateRect(&rc);
	}

	// Unsupported methods

	int AddString(LPCTSTR /*lpszItem*/)
	{
		ATLASSERT(false);
		return LB_ERR;
	}
	int InsertString(int /*nIndex*/, LPCTSTR /*lpszItem*/)
	{
		ATLASSERT(false);
		return LB_ERR;
	}
	int DeleteString(UINT /*nIndex*/)
	{
		ATLASSERT(false);
		return LB_ERR;
	}

	// Implementation

	void _Init()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		// Needs LBS_OWNERDRAWVARIABLE and LBS_NOTIFY flags,
		// but don't want multiselect or multicolumn flags.
		ATLASSERT(GetStyle() & LBS_OWNERDRAWVARIABLE);
		ATLASSERT(GetStyle() & LBS_NOTIFY);
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL|LBS_NODATA|LBS_MULTICOLUMN))==0);
		SendMessage(WM_SIZE);
		SendMessage(WM_SETTINGCHANGE);
	}
	void _GetInPlaceRect(int idx, RECT& rc) const
	{
		GetItemRect(idx, &rc);
		if( m_dwExtStyle & PLS_EX_CATEGORIZED ) rc.left += CATEGORY_INDENT;
		rc.left += m_iMiddle + 1;
	}
	BOOL _SpawnInplaceWindow(IProperty* prop, int idx)
	{
		if(m_bLockInplaceEditor) return FALSE;
		ATLASSERT(prop);
		// Destroy old editor
		_DestroyInplaceWindow();
		// Do we need an editor here?
		if( idx == -1 || idx != GetCurSel() ) return FALSE;
		if( !prop->IsEnabled() ) return FALSE;
		// Create a new editor window
		RECT rcValue;
		_GetInPlaceRect(idx, rcValue);
		::InflateRect(&rcValue, 0, -1);

		m_hwndInplace = prop->CreateInplaceControl(m_hWnd, rcValue);
		if( m_hwndInplace != NULL ) {
			// Activate the new editor window
			ATLASSERT(::IsWindow(m_hwndInplace));
			// The similar VB6 IDE control doesn't immediately set focus to the new editor window.
			// One must click on the item again. So we added the SWP_NOACTIVATE flag to 
			// simulate this behaviour.
			::SetWindowPos(m_hwndInplace, HWND_TOP, 0,0,0,0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

			m_iInplaceIndex = idx;
		}
		return m_hwndInplace != NULL;
	}
	void _DestroyInplaceWindow()
	{
		if(m_bLockInplaceEditor) return;
		if( m_hwndInplace && ::IsWindow(m_hwndInplace) ) {
			IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(m_iInplaceIndex));
			ATLASSERT(prop);
			BYTE bKind = prop->GetKind();
			// Set focus back to our control
			if( ::GetFocus() != m_hWnd && IsOwned(::GetFocus()) ) SetFocus();
			// Destroy control
			switch( bKind ) {
				case PROPKIND_CONTROL:
					::DestroyWindow(m_hwndInplace);
					break;
				default:
					::PostMessage(m_hwndInplace, WM_CLOSE, 0,0);
			}
		}
		m_hwndInplace = NULL;
		m_iInplaceIndex = -1;
	}
	void _DrawGhostBar(int x)
	{
		if( m_iPrevious == 0 ) return;
		// PatBlt without clip children on
		CWindow wndParent = GetParent();
		DWORD dwOldStyle = wndParent.GetWindowLong(GWL_STYLE);
		wndParent.SetWindowLong(GWL_STYLE, dwOldStyle & ~WS_CLIPCHILDREN);

		RECT rcClient = { 0 };
		GetClientRect(&rcClient);
		rcClient.left = x;
		rcClient.right = x + 1;     
		MapWindowPoints(wndParent, &rcClient);

		// Invert the brush pattern (looks just like frame window sizing)
		CClientDC dc = wndParent;
		CBrush hbrHalfTone = CDCHandle::GetHalftoneBrush();
		HBRUSH hOldBrush = NULL;
		if( hbrHalfTone != NULL ) hOldBrush = dc.SelectBrush(hbrHalfTone);

		dc.PatBlt(rcClient.left, rcClient.top, 2, rcClient.bottom - rcClient.top, PATINVERT);

		dc.SelectBrush(hOldBrush);
		wndParent.SetWindowLong(GWL_STYLE, dwOldStyle);
	}
	long _GetDragPos(int x) const
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		::InflateRect(&rcClient, -10, 0);
		if( x > rcClient.right ) x = rcClient.right;
		if( x < rcClient.left ) x = rcClient.left;
		return x;
	}
	// Added by Kronuz:
	int _GetIdxAtPoint(POINT pt) 
	{
		int nItemHeight = GetItemHeight(0);
		int nTopIndex   = GetTopIndex();
		int nItems      = GetCount();

		int nItem = (pt.y / nItemHeight) + nTopIndex;
		if(nItem > nItems) return -1;
		if(nItem < 0) return -1;
		return nItem;
	}


	// Message map and handlers

	BEGIN_MSG_MAP(CPropertyListImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnScroll)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnDblClick)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor);
		MESSAGE_HANDLER(WM_USER_PROP_NAVIGATE, OnNavigate);
		MESSAGE_HANDLER(WM_USER_PROP_EDITPROPERTY, OnEditProperty);
		MESSAGE_HANDLER(WM_USER_PROP_UPDATEPROPERTY, OnUpdateProperty);
		MESSAGE_HANDLER(WM_USER_PROP_CANCELPROPERTY, OnCancelProperty);
		MESSAGE_HANDLER(WM_USER_PROP_CHANGEDPROPERTY, OnChangedProperty);
		MESSAGE_HANDLER(WM_USER_PROP_EXPAND, OnExpand);
		MESSAGE_HANDLER(WM_USER_PROP_COLLAPSE, OnCollapse);
		REFLECTED_COMMAND_CODE_HANDLER(LBN_SELCHANGE, OnSelChange)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CPropertyListImpl>, 1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	// Added by Kronuz:
	bool IsOwned(HWND hWnd)
	{
		if(hWnd == m_hWnd) return true;
		if(m_hwndInplace == NULL) return false;
		if(m_hwndInplace == hWnd) return true;
		return (::SendMessage(m_hwndInplace, WM_USER_PROP_OWNED, 0, (LPARAM)hWnd) == TRUE);
	}

	// Added by Kronuz:
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
	{
		HWND hWndFocus = ::GetFocus();
		HWND hWndFocusParent = ::GetParent(hWndFocus);
		if(	!IsOwned(::GetFocus()) ) {
			_DestroyInplaceWindow();
			Invalidate();
		}
		if(m_hWnd != ::GetFocus()) bHandled = FALSE;
		return 0;
	}
	// Added by Kronuz:
	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) 
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

		if( (m_dwExtStyle & PLS_EX_XPLOOK) == PLS_EX_XPLOOK ) {
			CDC dc(GetWindowDC());
			if(dc) {
				RECT rcWindow;
				GetWindowRect(&rcWindow);
				::OffsetRect(&rcWindow, -rcWindow.left, -rcWindow.top);
				dc.DrawEdge(&rcWindow, EDGE_ETCHED, BF_FLAT|BF_RECT);
			}
		}
		return 0;
	}
	// Added by Kronuz:
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
	{
		// Hide ghost bar:
		if( m_iPrevXGhostBar > 0 ) _DrawGhostBar(m_iPrevXGhostBar);

		int nItemHeight = GetItemHeight(0);
		int nTopIndex   = GetTopIndex();
		int nItems      = GetCount();

		// Get the client area rect:
		RECT rcClient;
		GetClientRect(&rcClient);

		// Resize the rect to cover only the section after the last item:
		rcClient.top = (nItems - nTopIndex) * nItemHeight;

		// If there is a section to draw:
		if(rcClient.top < rcClient.bottom) {
			CDC pDC(GetDC());
			pDC.FillSolidRect(&rcClient, m_di.clrBack);
		}

		// Show ghost bar:
		if( m_iPrevXGhostBar > 0 ) _DrawGhostBar(m_iPrevXGhostBar);

		return 1;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
		_Init();
		return lRes;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ResetContent(); // Make sure to delete editor and item-data memory
		return 0;
	}
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{      
		// Size handler supplied by DJ (thank to this masked site reader)
		_DestroyInplaceWindow();
		// Calculate drag
		RECT rc;
		GetClientRect(&rc);
		if( !m_bColumnFixed ) m_iMiddle = (rc.right - rc.left) / 2;
		m_iPrevious = 0;
		BOOL bDummy;
		OnSelChange(0, 0, 0, bDummy);
		//
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		_DestroyInplaceWindow();
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		switch( LOWORD(wParam) ) {
			case VK_TAB:
				{
					int idx = GetCurSel();
					if( idx != -1 ) {
						IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(idx));
						ATLASSERT(prop);
						prop->Activate(PACT_TAB, 0);
					}
				}
				break;
			case VK_F2:
			case VK_SPACE:
				{
					int idx = GetCurSel();
					if( idx != -1 ) {
						IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(idx));
						ATLASSERT(prop);
						if( prop->IsEnabled() ) prop->Activate(PACT_SPACE, 0);
					}
				}
				break;
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{      
		// If the user is typing stuff, we should spawn an editor right away
		// and simulate the keypress in the editor-window...
		if( wParam > _T(' ') ) {
			int idx = GetCurSel();
			if( idx != -1 ) {
				IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(idx));
				ATLASSERT(prop);
				if( prop->IsEnabled() ) {
					if( _SpawnInplaceWindow(prop, idx) ) {
						prop->Activate(PACT_SPACE, 0);
						// Simulate typing in the inplace editor...
						::SendMessage(m_hwndInplace, WM_CHAR, wParam, 1L);
					}
				}
			}
			return 0;
		}
		// Kill the nasty BEEP sound!
		if( wParam == _T(' ') ) return 0;
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		// Should we do some column resize?
		// NOTE: ListBox control will internally to SetCapture() to
		//       capture all mouse-movements.
		m_iPrevious = 0;
		int iIndent = m_dwExtStyle & PLS_EX_CATEGORIZED ? CATEGORY_INDENT : 0;
		if( (m_dwExtStyle & PLS_EX_NOCOLUMNRESIZE) == 0 &&
			ptCursor.x > m_iMiddle + iIndent - 4 && // Modified by Kronuz
			ptCursor.x < m_iMiddle + iIndent + 4 ) // Modified by Kronuz
		{
			_DestroyInplaceWindow(); // Added by Kronuz
			Invalidate();
			UpdateWindow();

			SetCapture(); // Added by Kronuz (capture the mouse ourselves, since DefWindowProc will never be called)
			// Draw the ghost bar:
			m_iPrevious = ptCursor.x; // Modified by Kronuz
			PostMessage(WM_MOUSEMOVE, wParam, lParam);
			return 0;	// Added by Kronuz (completely take over while resizing the column)
		}

		// Call the default procedure:
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam); // Moved by Kronuz

		int idx = GetCurSel();
		if( idx != -1 ) {
			IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(idx));
			ATLASSERT(prop);
			// Ask owner first
			NMPROPERTYITEM nmh = { m_hWnd, GetDlgCtrlID(), PIN_CLICK, prop };
			if( ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh) == 0 ) {
				// Translate into action
				if( (m_dwExtStyle & PLS_EX_CATEGORIZED) && ptCursor.x < CATEGORY_INDENT ) {
					prop->Activate(PACT_EXPAND, 0);
				}
				else {
					if( prop->IsEnabled() ) prop->Activate(PACT_CLICK, lParam);
				}
			}
			// Added by Kronuz (in case we click over where the inplace control should be):
			RECT rcValue;
			_GetInPlaceRect(idx, rcValue);
			if(::PtInRect(&rcValue, ptCursor)) { 
				// Simulate a button up (to change the selection, thus creating the inplace control)
				SendMessage(WM_LBUTTONUP,wParam,lParam);
				// if the inplace window was indeed created, we send it this very same message:
				if(::IsWindow(m_hwndInplace)) {
					::MapWindowPoints(m_hWnd, m_hwndInplace, &ptCursor, 1);
					::PostMessage(m_hwndInplace, WM_LBUTTONDOWN, wParam, MAKELPARAM(ptCursor.x, ptCursor.y));
				}
			}
		}
		return lRes;
	}
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		int x = _GetDragPos(GET_X_LPARAM(lParam));
		if( m_iPrevious > 0 ) {
			m_iMiddle += x - m_iPrevious;
			Invalidate();
			if( ::GetCapture() == m_hWnd ) ReleaseCapture();
		} else bHandled = FALSE;
		m_iPrevious = 0;
		m_iPrevXGhostBar = 0;
		return 0;
	}
	LRESULT OnDblClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
		int idx = GetCurSel();
		if( idx != -1 ) {
			IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(idx));
			ATLASSERT(prop);
			// Ask owner first
			NMPROPERTYITEM nmh = { m_hWnd, GetDlgCtrlID(), PIN_DBLCLICK, prop };
			if( ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh) == 0 ) {
				// Send DblClick action
				if( prop->IsEnabled() ) prop->Activate(PACT_DBLCLICK, lParam);
			}
		}
		return lRes;
	}
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		// Added by Kronuz:
		if((wParam&MK_LBUTTON) != MK_LBUTTON) {
			if( ::GetCapture() == m_hWnd ) ReleaseCapture();
			m_iPrevious = 0;
			m_iPrevXGhostBar = 0;
		}
		// Column resize code added by Remco Verhoef, thanks.
		if( m_iPrevious > 0 ) {
			int x = _GetDragPos(GET_X_LPARAM(lParam));
			if( m_iPrevXGhostBar != x ) {
				if( m_iPrevXGhostBar ) _DrawGhostBar(m_iPrevXGhostBar);
				m_iPrevXGhostBar = x;
				_DrawGhostBar(m_iPrevXGhostBar);
			}
			return 0;
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		POINT pt = { 0 };
		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		int iIndent = m_dwExtStyle & PLS_EX_CATEGORIZED ? CATEGORY_INDENT : 0;
		if(  m_iPrevious > 0 ||
			pt.x > m_iMiddle + iIndent - 4 && pt.x < m_iMiddle + iIndent + 4 ) { // Modified by Kronuz
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
				return FALSE;
		} 
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// Custom styles
		m_di.dwExtStyle = m_dwExtStyle;
		// Standard colors
		m_di.clrText = ::GetSysColor(COLOR_WINDOWTEXT);
		m_di.clrBack = ::GetSysColor(COLOR_WINDOW);
		m_di.clrSelText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		m_di.clrSelBack = ::GetSysColor(COLOR_HIGHLIGHT);
		m_di.clrDisabled = ::GetSysColor(COLOR_GRAYTEXT);
		// Border
		m_di.clrBorder = ::GetSysColor(COLOR_BTNFACE);
		if( !m_BorderPen.IsNull() ) m_BorderPen.DeleteObject();
		m_di.Border = m_BorderPen.CreatePen(PS_SOLID, 1, m_di.clrBorder);
		// Fonts
		if( !m_TextFont.IsNull() ) m_TextFont.DeleteObject();
		if( !m_CategoryFont.IsNull() ) m_CategoryFont.DeleteObject();
		LOGFONT lf;
		HFONT hFont = (HFONT)::SendMessage(GetParent(), WM_GETFONT, 0, 0);
		if( hFont == NULL ) hFont = AtlGetDefaultGuiFont();
		::GetObject(hFont, sizeof(lf), &lf);
		m_di.TextFont = m_TextFont.CreateFontIndirect(&lf);
		SetFont(m_di.TextFont);
		if( (m_dwExtStyle & PLS_EX_XPLOOK) == 0 ) lf.lfWeight += FW_BOLD;
		m_di.CategoryFont = m_CategoryFont.CreateFontIndirect(&lf);
		// Text metrics
		CClientDC dc(m_hWnd);
		HFONT hOldFont = dc.SelectFont(m_di.TextFont);
		dc.GetTextMetrics(&m_di.tmText);
		dc.SelectFont(hOldFont);
		// Repaint
		Invalidate();
		return 0;
	}

	LRESULT OnNavigate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		switch( wParam ) {
			case VK_TAB:	// Added by Kronuz
			case VK_UP:
			case VK_DOWN:
				{
					_DestroyInplaceWindow();
					SetCurSel(GetCurSel() + (wParam == VK_UP ? -1 : 1));
					BOOL bDummy;
					OnSelChange(0, 0, NULL, bDummy);
				}
				break;
		}
		return 0;
	}
	HPROPERTY GetCategory(int index)
	{
		index = min(index, GetCount());
		HPROPERTY hRetProp = NULL;
		for( int i = 0; i < index; i++ ) {
			IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(i));
			ATLASSERT(prop);
			if( prop->GetKind() == PROPKIND_CATEGORY ) {
				hRetProp = prop;
			}
		}
		return hRetProp;
	}
	LRESULT OnEditProperty(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Tell the user a property is about to be edited.
		HWND hWnd = reinterpret_cast<HWND>(lParam);
		ATLASSERT(hWnd);
		if( !::IsWindow(hWnd) || m_iInplaceIndex == -1 ) return 0;
		ATLASSERT(hWnd==m_hwndInplace);
		IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(m_iInplaceIndex));
		ATLASSERT(prop);
		if( prop == NULL ) return 0;

		prop->SetCategory(GetCategory(m_iInplaceIndex));
		NMPROPERTYITEM nmh = { m_hWnd, GetDlgCtrlID(), PIN_ITEMEDIT, prop };
		::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh);

		return 0;
	}
	LRESULT OnUpdateProperty(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Updates a property value using an active editor window.
		// The editor window uses this message to update the attached property class.
		HWND hWnd = reinterpret_cast<HWND>(lParam);
		ATLASSERT(hWnd);
		if( !::IsWindow(hWnd) || m_iInplaceIndex == -1 ) return 0;
		ATLASSERT(hWnd==m_hwndInplace);
		IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(m_iInplaceIndex));
		ATLASSERT(prop);
		if( prop == NULL ) return 0;
		// Ask owner about change
		m_bLockInplaceEditor = true; // Added by Kronuz
		prop->SetCategory(GetCategory(m_iInplaceIndex));
		NMPROPERTYITEM nmh = { m_hWnd, GetDlgCtrlID(), PIN_ITEMCHANGING, prop };
		if( ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh) == 0 ) {
			// Set new value
			if( !prop->SetValue(hWnd) ) ::MessageBeep((UINT)-1);
			// Let owner know
			nmh.hdr.code = (wParam==TRUE)?PIN_ITEMUPDATED:PIN_ITEMCHANGED;
			::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh);
			// Stuff commented by Kronuz:
			// Repaint item 
//			InvalidateItem(m_iInplaceIndex);
			// Recycle in-place control so it displays the new value
			// NOTE: Traps condition where the SetValue() fails!
//			int idx = FindProperty(prop);
//			if( idx >= 0 && idx == m_iInplaceIndex ) _SpawnInplaceWindow(prop, idx);
		}
		m_bLockInplaceEditor = false; // Added by Kronuz
		return 0;
	}

	LRESULT OnCancelProperty(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Updates a property value using an active editor window.
		// The editor window uses this message to update the attached property class.
		HWND hWnd = reinterpret_cast<HWND>(lParam);
		ATLASSERT(hWnd);
		if( !::IsWindow(hWnd) || m_iInplaceIndex == -1 ) return 0;
		ATLASSERT(hWnd==m_hwndInplace);
		IProperty* prop = reinterpret_cast<IProperty*>(TBase::GetItemData(m_iInplaceIndex));
		ATLASSERT(prop);
		if( prop == NULL ) return 0;
		prop->SetCategory(GetCategory(m_iInplaceIndex));
		// Stuff commented by Kronuz:
		// Repaint item
//		InvalidateItem(m_iInplaceIndex);
		// Recycle in-place control so it displays the (old) value
//		int idx = FindProperty(prop);
//		if( idx >= 0 && idx == m_iInplaceIndex ) _SpawnInplaceWindow(prop, idx);
		return 0;
	}
	LRESULT OnChangedProperty(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Updates a property value.
		// A property class uses this message to make sure the corresponding editor window
		// is updated as well.
		IProperty* prop = reinterpret_cast<IProperty*>(lParam);
		VARIANT* pVariant = reinterpret_cast<VARIANT*>(wParam);
		ATLASSERT(prop && pVariant);
		if( prop == NULL || pVariant == NULL ) return 0;
		// Ask owner about change
		NMPROPERTYITEM nmh = { m_hWnd, GetDlgCtrlID(), PIN_ITEMCHANGING, prop };
		if( ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh) == 0 ) {
			// Set new value
			// NOTE: Do not call this from IProperty::SetValue(VARIANT*) = endless loop
			if( !prop->SetValue(*pVariant) ) ::MessageBeep((UINT)-1);
			// Let owner know
			nmh.hdr.code = PIN_ITEMCHANGED;
			::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh);
		}
		// Locate the updated property index
		int idx = FindProperty(prop);
		// Repaint item
		InvalidateItem(idx);
		// Recycle in-place control so it displays the new value
		if( idx >= 0 && idx == m_iInplaceIndex ) _SpawnInplaceWindow(prop, idx);
		return 0;
	}
	LRESULT OnExpand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		ATLASSERT(lParam);
		ExpandItem(reinterpret_cast<IProperty*>(lParam));
		return 0;
	}
	LRESULT OnCollapse(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		ATLASSERT(lParam);
		CollapseItem(reinterpret_cast<IProperty*>(lParam));
		return 0;
	}

	LRESULT OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if( m_iPrevXGhostBar > 0 ) _DrawGhostBar(m_iPrevXGhostBar);

		IProperty* prop = NULL;
		int idx = GetCurSel();
		if( idx != -1 ) {
			prop = reinterpret_cast<IProperty*>(TBase::GetItemData(idx));
			ATLASSERT(prop);
			if( prop->GetKind() != PROPKIND_CATEGORY && prop->IsEnabled() ) {
				_SpawnInplaceWindow(prop, idx);
				prop->Activate(PACT_ACTIVATE, 0);
				if( m_dwExtStyle & PLS_EX_SINGLECLICKEDIT ) {
					if( prop->GetKind() == PROPKIND_EDIT ) prop->Activate(PACT_DBLCLICK, 0);
				}
			}
			else {
				_DestroyInplaceWindow();
			}
		}
		// Let owner know
		NMPROPERTYITEM nmh = { m_hWnd, GetDlgCtrlID(), PIN_SELCHANGED, prop };
		::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh);

		if( m_iPrevXGhostBar > 0 ) _DrawGhostBar(m_iPrevXGhostBar);
		return 0;
	}

	// Owner draw methods

	void DeleteItem(LPDELETEITEMSTRUCT lpDIS)
	{
		_DestroyInplaceWindow();
		if( lpDIS->itemData ) delete reinterpret_cast<IProperty*>(lpDIS->itemData);
	}
	void MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
	{
		lpMIS->itemHeight = m_di.tmText.tmHeight + 3;
	}
	void DrawItem(LPDRAWITEMSTRUCT lpDIS)
	{
		if( lpDIS->itemID == -1 ) return; // If there are no list box items, skip this message. 

		CDCHandle dc(lpDIS->hDC);
		RECT rc = lpDIS->rcItem;

		if( m_iPrevXGhostBar > 0 ) _DrawGhostBar(m_iPrevXGhostBar);

		IProperty* prop = reinterpret_cast<IProperty*>(lpDIS->itemData);
		ATLASSERT(prop);
		BYTE kind = prop->GetKind();

		// Customize item
		PROPERTYDRAWINFO di = m_di;
		di.hDC = dc;
		di.state = lpDIS->itemState & ~ODS_DISABLED;
		if( lpDIS->itemID == (UINT) m_iInplaceIndex ) di.state |= ODS_COMBOBOXEDIT;

		// Special style for removing selection when control hasn't focus
		if( (di.dwExtStyle & PLS_EX_SHOWSELALWAYS) == 0 && 
			(!IsOwned(::GetFocus())) ) { // Modified by Kronuz
				if(di.state & ODS_SELECTED) di.state |= ODS_GRAYED;	// Added by Kronuz
				di.state &= ~ODS_SELECTED;
			}

			// Prepare drawing
			HFONT hOldFont = dc.SelectFont(di.TextFont);

			// If this control is painted with categories
			if( m_dwExtStyle & PLS_EX_CATEGORIZED ) {
				RECT rcIndent = { rc.left, rc.top, rc.left + CATEGORY_INDENT - 1, rc.bottom };
				if( m_dwExtStyle & PLS_EX_XPLOOK ) {
					dc.FillRect(&rcIndent, ::GetSysColorBrush(COLOR_3DFACE));
				} else {
					dc.FillSolidRect(&rcIndent, di.clrBack);
				}

				// We paint a nice border in the gap with the plus/minus signs
				HPEN hOldPen = dc.SelectPen(di.Border);
				dc.MoveTo(rc.left + CATEGORY_INDENT - 1, rc.top);
				dc.LineTo(rc.left + CATEGORY_INDENT - 1, rc.bottom);

				// Paint plus/minus sign if it's actually a category item
				if( kind == PROPKIND_CATEGORY ) {
					dc.SelectFont(di.CategoryFont);
					CCategoryProperty* pCategory = static_cast<CCategoryProperty*>(prop);
					POINT ptMiddle = { rc.left + (CATEGORY_INDENT / 2), rc.top + ((rc.bottom - rc.top) / 2) };
					RECT rcSymbol = { ptMiddle.x - 4, ptMiddle.y - 4, ptMiddle.x + 5, ptMiddle.y + 5 };
					dc.SelectStockPen(BLACK_PEN);
					HBRUSH hOldBrush = dc.SelectStockBrush(NULL_BRUSH);
					dc.Rectangle(&rcSymbol);
					dc.SelectBrush(hOldBrush);
					if( !pCategory->IsExpanded() ) {
						dc.MoveTo(ptMiddle.x, ptMiddle.y - 2);
						dc.LineTo(ptMiddle.x, ptMiddle.y + 3);
					}
					dc.MoveTo(ptMiddle.x - 2, ptMiddle.y);
					dc.LineTo(ptMiddle.x + 3, ptMiddle.y);
				}
				dc.SelectPen(hOldPen);
				rc.left += CATEGORY_INDENT;
			}

			// Calculate rectangles for the two sides
			RECT rcName = rc;
			RECT rcValue = rc;
			rcName.right = rc.left + m_iMiddle;
			rcValue.left = rc.left + m_iMiddle + 1;

			// Special handling of XP-like categories
			if( kind == PROPKIND_CATEGORY && (m_dwExtStyle & PLS_EX_XPLOOK) != 0 ) {
				rcName.right = rcValue.left = rc.right;
			}

			// Draw name
			di.rcItem = rcName;
			if( !prop->IsEnabled() ) di.state |= ODS_DISABLED; // Added by Kronuz
			prop->DrawName(di);

			// Draw value
			// Thanks to Pascal Binggeli who suggested the Inplace-editor
			// check below...
			if( (int) lpDIS->itemID != m_iInplaceIndex ) {
				di.rcItem = rcValue;
				dc.FillSolidRect(&rcValue, di.clrBack);
				prop->DrawValue(di);
			} else {
				// Added by Kronuz: (paint the upper border of the inplace control)
				rcValue.bottom = rcValue.top + 1;
				dc.FillSolidRect(&rcValue, di.clrBack);
			}

			// Paint borders
			HPEN hOldPen = dc.SelectPen(di.Border);
			dc.MoveTo(rc.left, rc.bottom - 1);
			dc.LineTo(rc.right, rc.bottom - 1);     
			dc.MoveTo(rc.left + m_iMiddle, rc.top);
			dc.LineTo(rc.left + m_iMiddle, rc.bottom - 1);
			dc.SelectPen(hOldPen);

			dc.SelectFont(hOldFont);

			if( m_iPrevXGhostBar > 0 ) _DrawGhostBar(m_iPrevXGhostBar);
		}
};


class CPropertyListCtrl : public CPropertyListImpl<CPropertyListCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_PropertyList"), GetWndClassName())
};


#endif __PROPERTYLIST__H
