#ifndef __SUPERCOMBO_H__
#define __SUPERCOMBO_H__

/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germán Méndez Bravo (Kronuz)
   Copyright (C) 2001-2003. Open Legends's Project

   Based on the drag and drop code by Leon Finker (Jan/2001)
 
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/*	
	Combo Box Control implementing multiselection, icons and icon's states.
	Author: Kronuz (Germán Méndez Bravo)
			Autocomplete based on the code by Chris Maunder
*/

#pragma once

#ifndef __cplusplus
   #error WTL requires C++ compilation (use a .cpp suffix)
#endif

#if (_WTL_VER < 0x0700)
   #error This file requires WTL version 7.0 or higher
#endif

#define ODS_MSELECTED		0x2000

#define CBN_STATECHANGE		20

#include <map>

template< class T, class TBase = CComboBox, class TWinTraits = CWinTraitsOR<CBS_OWNERDRAWVARIABLE|CBS_HASSTRINGS> >
class ATL_NO_VTABLE CSuperComboBoxImpl :
	public CWindowImpl< T, TBase, TWinTraits >,
	public COwnerDraw< CSuperComboBoxImpl< T, TBase, TWinTraits > >
{
	typedef CWindowImpl< T, TBase, TWinTraits > baseCtrlClass;

	typedef std::pair< CString, std::pair<int, int> > pairState;
	typedef std::map< CString, std::pair<int, int> > mapState;

	typedef struct tagSUPERITEMDATA {
		// Map contains a list of the current state-lists's states in the item:
		// < state_ID, < icon_state, reserved > >
		// Where:
		//   state_ID     ID of the state list
		//                (accessed as Iterator->first)
		//   icon_state   Current state of the item's state-lists
		//                (accessed as Iterator->second.first)
		//   reserved     this is reserved and should always be 0
		DWORD_PTR dwItemData;
		UINT uItemState;
		mapState States; 
	} SUPERITEMDATA, *LPSUPERITEMDATA;

	// Map contains a list of states as follows:
	// map< state_ID, < base_index, states > >
	// Where:
	//   state_ID     ID of the state list
	//                (accessed as Iterator->first)
	//   base_index   image index for the first icon in the state-list. 
	//                (accessed as Iterator->second.first)
	//   states       number of state icons in for the state-list or the type of the icon. 
    //                if states == 0, this state-list represents just an icon with no states.
	//                (accessed as Iterator->second.second)
	mapState m_States; 

	mutable int m_nLastGetSel;

protected:

	bool m_bAllowMultiSel;
	bool m_bShouldClose;
	bool m_bAutoComplete;
	int m_nImageWidth;
	int m_nItemHeight;
	int m_nItemWidth;
	int m_nCurSel;
	int m_nChangedState;
	int m_nAnchor;
	int m_nCaret;

	CImageList m_ImageList1;
	CImageList m_ImageList2;

	CContainedWindowT<CEdit> m_ctrlEdit;
	CContainedWindowT<CListBox> m_ctrlListBox;

public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	CSuperComboBoxImpl() : 
		m_ctrlEdit(_T("EDIT"), this, 1),
		m_ctrlListBox(_T("LISTBOX"), this, 2),
		m_bAllowMultiSel(false),
		m_bShouldClose(true),
		m_bAutoComplete(true),
		m_nCurSel(-1),
		m_nChangedState(-1),
		m_nAnchor(-1),
		m_nCaret(-1),
		m_nLastGetSel(-1),
		m_nImageWidth(0),
		m_nItemHeight(18),
		m_nItemWidth(150),
		m_ImageList1(NULL),
		m_ImageList2(NULL)
	{
	}

	// Operations
	void SetMultiSelection(bool bAllowMultiSel = true)
	{
		m_bAllowMultiSel = bAllowMultiSel;
	}

	int SetCurSel(int nSelect)
	{
		if(_SetCurSel(nSelect) == CB_ERR) return CB_ERR;
		m_nAnchor = nSelect;
		return baseCtrlClass::SetCurSel(nSelect);
	}

	int GetCurSel()
	{
		return GetFirstSel();
	}

	int GetChanged()
	{
		return m_nChangedState;
	}
	// for multiple-selection comboboxes
	int GetSel(int nIndex) const	// also works for single-selection
	{
		UINT uState = GetItemState(nIndex);
		if(uState == CB_ERR) return CB_ERR;

		return ((uState & ODS_MSELECTED) == ODS_MSELECTED);
	}
	int SetSel(int nIndex, BOOL bSelect = TRUE)
	{
		if(nIndex == -1) return SelItemRange(bSelect, 0, GetCount()-1);

		UINT uState = GetItemState(nIndex);
		if(uState == CB_ERR) return CB_ERR;
		if(bSelect == TRUE) {
			uState |= ODS_MSELECTED;
		} else {
			uState &= ~ODS_MSELECTED;
		}

		return SetItemState(nIndex, uState);
	}
	int GetSelCount() const
	{
		int nCount = 0;
		for(int nItem = 0; nItem<GetCount(); nItem++) {
			UINT uState = GetItemState(nItem);
			ATLASSERT(uState != CB_ERR);
			if(uState == CB_ERR) return CB_ERR;
			if((uState & ODS_MSELECTED) == ODS_MSELECTED) {
				nCount++;
			}
		}

		return nCount;
	}
	int GetSelItems(int nMaxItems, LPINT rgIndex) const
	{
		ATLASSERT(m_bAllowMultiSel == true);
		if(m_bAllowMultiSel == false) return CB_ERR;

		int nCount = 0;
		for(int nItem = 0; nItem<GetCount(); nItem++) {
			UINT uState = GetItemState(nItem);
			ATLASSERT(uState != CB_ERR);
			if(uState == CB_ERR) return CB_ERR;
			if((uState & ODS_MSELECTED) == ODS_MSELECTED) {
				if(nCount == nMaxItems) break;
				rgIndex[nCount++] = nItem;
			}
		}

		return nCount;
	}
	int GetAnchorIndex() const
	{
		ATLASSERT(m_bAllowMultiSel == true);
		if(m_bAllowMultiSel == false) return CB_ERR;

		return m_nAnchor;
	}
	void SetAnchorIndex(int nIndex)
	{
		ATLASSERT(m_bAllowMultiSel == true);

		if(nIndex < 0) m_nAnchor = 0;
		else if(nIndex >= GetCount()) m_nAnchor = GetCount();
		else m_nAnchor = nIndex;
	}
	int GetCaretIndex() const
	{
		return m_nCaret;
	}
	int SetCaretIndex(int nIndex, BOOL bScroll = TRUE)
	{
		if(bScroll) {
			if(m_ctrlListBox.SetTopIndex(nIndex) == LB_ERR) 
				return CB_ERR;
		}

		m_nCaret = nIndex;
		return 0;
	}
	int SelItemRange(BOOL bSelect, int nFirstItem, int nLastItem)
	{
		ATLASSERT(m_bAllowMultiSel == true);
		if(m_bAllowMultiSel == false) return CB_ERR;
		
		if(nFirstItem<0 || nFirstItem>=GetCount()) return CB_ERR;
		if(nLastItem<0 || nLastItem>=GetCount()) return CB_ERR;

		int nCount = 0;
		for(int nItem = min(nFirstItem, nLastItem); nItem<=max(nFirstItem, nLastItem); nItem++) {
			if(SetSel(nItem, bSelect) == CB_ERR) return CB_ERR;
		}
		return 0;
	}

	int GetFirstSel() const
	{
		m_nLastGetSel = 0;
		return GetNextSel();
	}
	int GetNextSel() const
	{
		while(m_nLastGetSel < GetCount()) {
			int nItemState = GetItemState(m_nLastGetSel);
			if(nItemState == CB_ERR) return CB_ERR;
			if((nItemState & ODS_MSELECTED)) {
				return m_nLastGetSel++;
			}
			m_nLastGetSel++;
		}
		
		return -1;
	}

	// Icons and State-Icons:
	BOOL LoadStatesBitmap(UINT uStates, UINT uStatesDisabled=0) 
	{
		ATLASSERT(IS_INTRESOURCE(uStates));
		SetStatesBitmap(m_ImageList1, uStates);

		if(uStatesDisabled) {
			ATLASSERT(IS_INTRESOURCE(uStatesDisabled));
			if(!SetStatesBitmap(m_ImageList2, uStatesDisabled))
				return FALSE;
		}

		return TRUE;
	}

	BOOL SetStatesBitmap(CImageList &cImageList, UINT uBitmap)
	{
		CImage Image;
		if(!LoadImage(&Image, _Module.GetModuleInstance(), uBitmap)) 
			return FALSE;

		int nWidth = Image.GetHeight();
		int	nImages	= Image.GetWidth() / nWidth;

		m_nImageWidth = nWidth;
		m_nItemHeight = nWidth;

		if(!cImageList.IsNull()) m_ImageList1.Destroy();
		if(!cImageList.Create(nWidth, Image.GetHeight(), ILC_COLOR32, nImages, 0))
			return FALSE;

		if(cImageList.Add(Image) == -1)
			return FALSE;

		return TRUE;
	}

	bool AddIcon(LPCSTR szID)
	{
		return AddStateIcon(szID, 0, 0);
	}
	void DeleteIcon(LPCSTR szID)
	{
		DeleteStateIcon(szID);
	}
	bool AddStateIcon(LPCSTR szID, int nBaseIndex, int nStates)
	{
		ASSERT(!m_ImageList1.IsNull());
		ASSERT(nBaseIndex+nStates-1 < m_ImageList1.GetImageCount());
		return (m_States.insert(pairState(szID, std::pair<int, int>(nBaseIndex, nStates)))).second;
	}
	void DeleteStateIcon(LPCSTR szID)
	{
		ASSERT(m_States.find(szID) != m_States.end());
		m_States.erase(szID);
	}
	void UseImageList(HIMAGELIST hImageList) 
	{ 
		if(!m_ImageList1.IsNull()) m_ImageList1.Destroy();
		if(!m_ImageList2.IsNull()) m_ImageList2.Destroy();

		m_ImageList1.Attach(hImageList);
	}

	int SetItemIcon(int nItemIndex, LPCSTR szID, int nIconIndex)
	{
		return SetItemState(nItemIndex, szID, nIconIndex);
	}

	// gets the item's internal state (not the user defined states by the icons)
	UINT GetItemState(int nItemIndex) const 
	{
		LPSUPERITEMDATA pSuperItemData = (LPSUPERITEMDATA)baseCtrlClass::GetItemData(nItemIndex);
		if((int)pSuperItemData == CB_ERR) return CB_ERR;

		if(!pSuperItemData) return 0;
		return pSuperItemData->uItemState;
	}
	int SetItemState(int nItemIndex, UINT uItemState) 
	{
		LPSUPERITEMDATA pSuperItemData = (LPSUPERITEMDATA)baseCtrlClass::GetItemData(nItemIndex);
		if((int)pSuperItemData == CB_ERR) return CB_ERR;

		if(!pSuperItemData) {
			pSuperItemData = new SUPERITEMDATA;
			pSuperItemData->dwItemData = NULL;
		}
		pSuperItemData->uItemState = uItemState;
		return baseCtrlClass::SetItemData(nItemIndex, (DWORD_PTR)pSuperItemData);
	}

	// gets the item's user-defined state (the state of the the icons)
	int GetItemState(int nItemIndex, LPCSTR szID) const
	{
		LPSUPERITEMDATA pSuperItemData = (LPSUPERITEMDATA)baseCtrlClass::GetItemData(nItemIndex);
		if((int)pSuperItemData == CB_ERR) return CB_ERR;

		// if the stat does not exist in the state list, error
		if(m_States.find(szID) == m_States.end()) return CB_ERR;

		if(!pSuperItemData) return 0;
		mapState::iterator Iterator = pSuperItemData->States.find(szID);
		if(Iterator == pSuperItemData->States.end()) return 0;
		return Iterator->second.first;
	}
	int SetItemState(int nItemIndex, LPCSTR szID, int nValue)
	{
		LPSUPERITEMDATA pSuperItemData = (LPSUPERITEMDATA)baseCtrlClass::GetItemData(nItemIndex);
		if((int)pSuperItemData == CB_ERR) return CB_ERR;

		if(!pSuperItemData) {
			pSuperItemData = new SUPERITEMDATA;
			pSuperItemData->dwItemData = NULL;
			pSuperItemData->uItemState = 0;
		}

		mapState::iterator Iterator = pSuperItemData->States.find(szID);
		if(Iterator == pSuperItemData->States.end()) {
			if(!pSuperItemData->States.insert(pairState(szID, std::pair<int, int>(nValue,0))).second)
				return CB_ERR;
		} else {
			Iterator->second = std::pair<int, int>(nValue, 0);
		}
		
		return baseCtrlClass::SetItemData(nItemIndex, (DWORD_PTR)pSuperItemData);
	}
	
	// gets the item's associated data
	DWORD_PTR GetItemData(int nItemIndex) const
	{
		LPSUPERITEMDATA pSuperItemData = (LPSUPERITEMDATA)baseCtrlClass::GetItemData(nItemIndex);
		if((int)pSuperItemData == CB_ERR) return CB_ERR;

		if(!pSuperItemData) return NULL;
		return pSuperItemData->dwItemData;
	}
	int SetItemData(int nItemIndex, DWORD_PTR dwItemData)
	{
		LPSUPERITEMDATA pSuperItemData = (LPSUPERITEMDATA)baseCtrlClass::GetItemData(nItemIndex);
		if((int)pSuperItemData == CB_ERR) return CB_ERR;

		if(!pSuperItemData) {
			pSuperItemData = new SUPERITEMDATA;
			pSuperItemData->uItemState = 0;
		}
		pSuperItemData->dwItemData = dwItemData;
		return baseCtrlClass::SetItemData(nItemIndex, (DWORD_PTR)pSuperItemData);
	}
	void* GetItemDataPtr(int nItemIndex)
	{
		return (void *)GetItemData(nItemIndex);
	}
	int SetItemDataPtr(int nItemIndex, void *pData)
	{	
		return SetItemData(nItemIndex, (DWORD_PTR)pData);
	}

	// Subclassing:
	BOOL SubclassWindow( HWND hWnd )
	{
		ATLASSERT(m_hWnd==NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = baseCtrlClass::SubclassWindow( hWnd );
		if( bRet ) _Init();
		return bRet;
	}

protected:
	// Implementation:
	int _SetCurSel(int nSelect)
	{
		if(m_bAllowMultiSel == true) {
			if(SetSel(-1, FALSE) == CB_ERR) return CB_ERR;
		} else {
			int nCurSel = GetCurSel();
			if(nCurSel != CB_ERR) SetSel(nCurSel, FALSE);
		}
		if(SetSel(nSelect, TRUE) == CB_ERR) return CB_ERR;
		return nSelect;
	}

	void _Init()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetStyle() & CBS_OWNERDRAWVARIABLE);
		ATLASSERT(GetStyle() & CBS_HASSTRINGS);

		// Subclass child (edit) control if present
		if( GetWindow(GW_CHILD) && m_ctrlEdit.m_hWnd == NULL ) {
			m_ctrlEdit.SubclassWindow(GetWindow(GW_CHILD));
		}
		SendMessage(WM_SETTINGCHANGE);
	}

	// Checks if the mouse was pressed in one of the status icons, and if so, it updates
	// the icon, sends a notification to the parent, sets m_bShouldClose to false, and returns true.
	bool _CheckMousePoint(int nItemIndex, UINT itemState, const CPoint &ptMouse, CRect *prcItem) 
	{
		int nImages = m_States.size();
		int nShiftRight = 2;
		int nHeight = prcItem->bottom - prcItem->top;

		// Just check if we are drawing the item in the combo box or the items
		// in the dropdown. If it´s in combobox, center the picture about 4 pixels
		if(!(itemState & ODS_COMBOBOXEDIT) && nImages) nShiftRight += 4;

		// Move the item rectangle to the left, so we can check state changes
		prcItem->left += nImages*m_nImageWidth + nShiftRight;

		if(!prcItem->PtInRect(ptMouse)) { // The mouse is somewhere in the icons:
			prcItem->left -= nImages*m_nImageWidth + nShiftRight;

			int nItem = 0;
			for(mapState::iterator Iterator = m_States.begin(); Iterator!=m_States.end(); Iterator++) {
				if( ptMouse.x-prcItem->left >= nShiftRight + nItem * m_nImageWidth &&
					ptMouse.x-prcItem->left < nShiftRight + (nItem+1) * m_nImageWidth ) {
						// if the state is a non-variable state image treat click normally
						if(Iterator->second.second != 0) {
							m_bShouldClose = false;
							int nState = GetItemState(nItemIndex, Iterator->first);
							if(nState == -1) break;
							if(++nState >= Iterator->second.second) nState = 0;
							SetItemState(nItemIndex, Iterator->first, nState);
						}
						break;
					}
					nItem++;
			}
			m_nChangedState = nItemIndex;
			// Notify that the state has changed
			::PostMessage(GetParent(), WM_COMMAND, MAKEWPARAM(GetWindowLong(GWL_ID), CBN_STATECHANGE), (LPARAM)m_hWnd);
			return true;
		}
		return false;
	}

public:
	BEGIN_MSG_MAP(CSuperComboBoxImpl)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnIgnore)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)

		MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, OnCtlColorListBox)

		REFLECTED_COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnSelChange)
		REFLECTED_COMMAND_CODE_HANDLER(CBN_EDITUPDATE, OnEditUpdate)

		CHAIN_MSG_MAP_ALT(COwnerDraw<CSuperComboBoxImpl>, 1)

		DEFAULT_REFLECTION_HANDLER()

	ALT_MSG_MAP(1)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown_Edit)

	ALT_MSG_MAP(2)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground_ListBox)

		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown_ListBox)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove_ListBox)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown_ListBox)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp_ListBox)

	END_MSG_MAP()

	LRESULT OnIgnore(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return 0;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
		_Init();
		return lRes;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ResetContent();
		if(!m_ImageList1.IsNull()) m_ImageList1.Destroy();
		if(!m_ImageList2.IsNull()) m_ImageList2.Destroy();
		return 0;
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);

		// Add here to change the aspect of the combo box (flat styles for example)

		return lRes;
	}

	LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
/*
		m_clrHiliteSelected		= ::GetSysColor(COLOR_BTNHILIGHT);
		m_clrHilite				= ::GetSysColor(COLOR_BTNHILIGHT);
		m_clrSelected			= ::GetSysColor(COLOR_BTNHILIGHT);

		m_clrBackground			= ::GetSysColor(COLOR_BTNSHADOW);
		m_clrText				= ::GetSysColor(COLOR_BTNFACE);
*/
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		return lRet;
	}

	LRESULT OnCtlColorListBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Here we need to get a reference to the listbox of the combobox
		// (the dropdown part). We can do it using 
		if(lParam && m_ctrlListBox.m_hWnd == NULL) {
			// Subclass ListBox
			m_ctrlListBox.SubclassWindow((HWND)lParam);
		}

		return DefWindowProc(uMsg, wParam, lParam);
	}

	LRESULT OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_nCurSel = GetCurSel();

		CString sItem;
		if(GetSelCount() > 1) sItem = "*VARIES*";
		else if(m_nCurSel != CB_ERR) {
			// get the old item selection text
			GetLBText(m_nCurSel, sItem);
		}
		SetWindowText(sItem);

		// select the whole text
		SetEditSel(0, -1);
		
		return 0;
	}

	// Autocomplete stuff for the edit box:
	LRESULT OnEditUpdate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// if we are not to auto update the text, get outta here
		if (!m_bAutoComplete) 
			return 0;

		// Get the text in the edit box
		CString str;
		GetWindowText(str);
		int nLength = str.GetLength();
  
		// Currently selected range
		DWORD dwCurSel = GetEditSel();
		WORD dStart = LOWORD(dwCurSel);
		WORD dEnd   = HIWORD(dwCurSel);

		// Search for, and select in, and string in the combo box that is prefixed
		// by the text in the edit box
		if (SelectString(-1, str) == CB_ERR) {
			SetWindowText(str);		// No text selected, so restore what was there before
			if(dwCurSel != CB_ERR)
				SetEditSel(dStart, dEnd);	//restore cursor postion
		}

		// Set the text selection as the additional text that we have added
		if (dEnd < nLength && dwCurSel != CB_ERR)
			SetEditSel(dStart, dEnd);
		else
			SetEditSel(nLength, -1);

		return 0;
	}

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		CRect rcItem;
		GetClientRect(&rcItem);

		CPoint ptMouse(lParam);
		int nItemIndex = GetCurSel();

		m_bShouldClose = true;
		// check if the mouse was pressed in a status-icon:
		if(_CheckMousePoint(nItemIndex, ODS_COMBOBOXEDIT, ptMouse, &rcItem)) {
			// Invalidate this window
			InvalidateRect(rcItem, FALSE);
		}

		if(m_bShouldClose == true) bHandled = FALSE;

		return 0;
	}

	// List box messages:
	LRESULT OnEraseBackground_ListBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		int nItemHeight = m_ctrlListBox.GetItemHeight(0);
		int nTopIndex   = m_ctrlListBox.GetTopIndex();
		int nItems      = m_ctrlListBox.GetCount();

		RECT rcClient;
		m_ctrlListBox.GetClientRect(&rcClient);
		rcClient.top = (nItems - nTopIndex) * nItemHeight;

		if(rcClient.top >= rcClient.bottom) return 0;

		CDC pDC(m_ctrlListBox.GetDC());
		pDC.FillSolidRect(&rcClient, ::GetSysColor(COLOR_WINDOW));

		return 1;
	}

	LRESULT OnMouseMove_ListBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnLButtonDown_ListBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		bHandled = FALSE;
		CRect rcClient;
		m_ctrlListBox.GetClientRect(rcClient);

		CPoint ptMouse(lParam);

		m_bShouldClose = true;
		if(rcClient.PtInRect(ptMouse)) {
			int nItemHeight = m_ctrlListBox.GetItemHeight(0);
			int nTopIndex   = m_ctrlListBox.GetTopIndex();

			// Check which item was selected (item)
			int nItemIndex = nTopIndex + ptMouse.y / nItemHeight;

			CRect rcItem;
			m_ctrlListBox.GetItemRect(nItemIndex, &rcItem);
			if(_CheckMousePoint(nItemIndex, 0, ptMouse, &rcItem)) {
				// Invalidate this window
				m_ctrlListBox.InvalidateRect(rcItem, FALSE);
				bHandled = TRUE;
			} else {
				if(m_bAllowMultiSel) {
					if((wParam&MK_CONTROL)==MK_CONTROL) {
						bHandled = TRUE;
						m_bShouldClose = false;
					} else {
						SetSel(-1, FALSE);
						m_ctrlListBox.Invalidate();
					}
					if((wParam&MK_SHIFT)==MK_SHIFT) {
						bHandled = TRUE;
						m_bShouldClose = false;
						if(m_nAnchor != -1 && m_nCaret != -1 && (wParam&MK_CONTROL)!=MK_CONTROL) {
							SelItemRange(FALSE, m_nAnchor, m_nCaret);
						}
						if(m_nAnchor == -1) m_nAnchor = GetFirstSel();
						if(m_nAnchor == -1) m_nAnchor = 0;
						m_nCaret = nItemIndex;
						SelItemRange(TRUE, m_nAnchor, m_nCaret);
						m_ctrlListBox.Invalidate();
					} else {
						m_nAnchor = nItemIndex;
						m_nCaret = nItemIndex;
						int nSel = GetSel(nItemIndex);
						ASSERT(nSel!=CB_ERR);
						SetSel(nItemIndex, nSel?FALSE:TRUE);
						m_ctrlListBox.InvalidateRect(rcItem, FALSE);
					}
					if((wParam&MK_SHIFT)==MK_SHIFT || (wParam&MK_CONTROL)==MK_CONTROL) {
						// Notify that the selection has changed
						::PostMessage(GetParent(), WM_COMMAND, MAKEWPARAM(GetWindowLong(GWL_ID), CBN_SELCHANGE), (LPARAM)m_hWnd);
					}
				} else {
					_SetCurSel(nItemIndex);
					m_ctrlListBox.InvalidateRect(rcItem, FALSE);
				}
			}
		}

		return 0;
	}

	LRESULT OnLButtonUp_ListBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		bHandled = FALSE;
		if(!m_bShouldClose) {
			m_bShouldClose = true;
			bHandled = TRUE;
		}
		return 0;
	}

	// Edit box messages:
	LRESULT OnKeyDown_Edit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		bHandled = FALSE;
		m_bAutoComplete = true;

		int nVirtKey = (int)wParam;
		if(nVirtKey == VK_DELETE || nVirtKey == VK_BACK) {
			// Need to check for backspace/delete. These will modify the text in
			// the edit box, causing the auto complete to just add back the text
			// the user has just tried to delete. 
			m_bAutoComplete = false;
		} else if(nVirtKey == VK_RETURN) {
			CString str;
			GetWindowText(str);
			if(str != "*VARIES*") {
				int nLength = str.GetLength();
				int nSel = SelectString(-1, str);
				if(CB_ERR != nSel) {
					SetCurSel(nSel);
					// notify message
					::PostMessage(GetParent(), WM_COMMAND, MAKEWPARAM(GetWindowLong(GWL_ID), CBN_SELCHANGE), (LPARAM)m_hWnd);
				}
			}
		} else if(nVirtKey == VK_ESCAPE) {
			CString sItem;
			if(GetSelCount() > 1) sItem = "*VARIES*";
			else if(m_nCurSel != CB_ERR) {
				// get the old item selection text
				GetLBText(m_nCurSel, sItem);
			}
			SetWindowText(sItem);

			// select the whole text
			SetEditSel(0, -1);
		}
		return 0;
	}

	// Owner draw override:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
	{
		lpMeasureItemStruct->itemHeight = m_nItemHeight;
		lpMeasureItemStruct->itemWidth  = m_nItemWidth;
	}

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		// This code is used to draw each one of the items in the combobox

		// First, check if we are drawing and item
		if(lpDrawItemStruct->itemID == -1)
			return;

		CString sItem;
		CString sItemTitle;

		CMemDC dcMem(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem);

		UINT itemState = lpDrawItemStruct->itemState;
		if(m_bAllowMultiSel) {
			itemState |= GetItemState(lpDrawItemStruct->itemID);
		}

		RECT rcImages = lpDrawItemStruct->rcItem;
		int nImages = m_States.size();
		int nShiftRight = 2;
		int nHeight = lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top;
		int nMiddle = lpDrawItemStruct->rcItem.top + nHeight/2 - m_nImageWidth/2;

		// Just check if we are drawing the item in the combo box or the items
		// in the dropdown. If it´s in combobox, center the picture about 4 pixels
		if(!(itemState & ODS_COMBOBOXEDIT) && nImages) nShiftRight += 4;

		// Move the drawing rectangle to the left, so we can draw the buttons
		lpDrawItemStruct->rcItem.left += nImages*m_nImageWidth + nShiftRight;

		rcImages.right = lpDrawItemStruct->rcItem.left;

		// If we have the focus, draw the item with a blue background
		// if we haven´t, just plain blank
		UINT uStyle = ILD_NORMAL;
		if( itemState & ODS_DISABLED || itemState & ODS_GRAYED ) {
			dcMem.FillSolidRect(&lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_BTNFACE));
			dcMem.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
			uStyle = ILD_FOCUS;
			dcMem.FillSolidRect(&rcImages, ::GetSysColor(COLOR_BTNFACE));
		} else if( !(itemState & ODS_COMBOBOXEDIT) && (itemState & ODS_MSELECTED || itemState & ODS_FOCUS || ((itemState & ODS_SELECTED) && (GetStyle() & CBS_DROPDOWN)) ) ) {
			if(itemState & ODS_MSELECTED || !m_bAllowMultiSel) {
				dcMem.FillSolidRect(&lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_HIGHLIGHT));
				dcMem.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			} else {
				dcMem.FillSolidRect(&lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_BTNFACE));
				dcMem.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
			}

			if( itemState & ODS_FOCUS || ((itemState & ODS_SELECTED) && (GetStyle() & CBS_DROPDOWN)) ) {
				dcMem.DrawFocusRect(&lpDrawItemStruct->rcItem);
			}

			uStyle = ILD_FOCUS;
			dcMem.FillSolidRect(&rcImages, ::GetSysColor(COLOR_WINDOW));
		} else {
			dcMem.FillSolidRect(&lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_WINDOW));
			dcMem.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
			dcMem.FillSolidRect(&rcImages, ::GetSysColor(COLOR_WINDOW));
		}

		lpDrawItemStruct->rcItem.left += 2;

		// Copy the text of the item to a string
		GetLBText(lpDrawItemStruct->itemID, sItem);

		dcMem.SetBkMode(TRANSPARENT);

		HFONT hOldFont = dcMem.SelectFont(GetFont());
		int idx = sItem.Find(":");
		// if there is a title (something before " - "), draw the text in bold:
		if(idx != -1) {
			sItemTitle = sItem.Mid(0, idx + 1);
			sItem = sItem.Mid(idx + 1);

			HFONT hOldFont = NULL;
			HFONT hTitleFont = NULL;

			LOGFONT lf = {0};
			GetObject( GetFont(), sizeof(LOGFONT), &lf );
			lf.lfWeight = FW_BOLD;
			hTitleFont = ::CreateFontIndirect( &lf );
			hOldFont = dcMem.SelectFont(hTitleFont);

			dcMem.DrawText(sItemTitle, -1, &lpDrawItemStruct->rcItem, DT_VCENTER | DT_SINGLELINE);

			// calculate the size of the drawn text:
			CSize Size;
			dcMem.GetTextExtent(sItemTitle, sItemTitle.GetLength(), &Size);
			lpDrawItemStruct->rcItem.left += Size.cx;
			
			dcMem.SelectFont(hOldFont);
			::DeleteObject(hTitleFont);
		}

		// Draw the text
		dcMem.DrawText(sItem, -1, &lpDrawItemStruct->rcItem, DT_VCENTER | DT_SINGLELINE);

		dcMem.SelectFont(hOldFont);

		int nDrawedImages = 0;
		int nImage = 0;
		for(mapState::iterator Iterator = m_States.begin(); Iterator!=m_States.end(); Iterator++) {
			nImage = GetItemState(lpDrawItemStruct->itemID, Iterator->first);
			ASSERT(nImage != -1);

			UINT uStyleThis = uStyle;
			// if the state is a variable state image, don't show focused
			if(Iterator->second.second != 0) uStyleThis = ILD_NORMAL;

			// if the image is a variable status image, it should 
			// be within the boundaries of the defined status options
			ASSERT(
				Iterator->second.second == 0 ||
				nImage >= 0 && nImage < Iterator->second.second
			);

			nImage += Iterator->second.first;

			// the index should be a valid index
			ASSERT( nImage >= 0 && nImage < m_ImageList1.GetImageCount() );
			if((itemState & ODS_DISABLED || itemState & ODS_GRAYED) && !m_ImageList2.IsNull() )
				m_ImageList2.Draw(dcMem, nImage, (nDrawedImages++)*m_nImageWidth + nShiftRight, nMiddle, ILD_NORMAL);
			else
				m_ImageList1.Draw(dcMem, nImage, (nDrawedImages++)*m_nImageWidth + nShiftRight, nMiddle, uStyleThis);
		}
	}
	void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
	{
		//ASSERT(lpDeleteItemStruct->CtlType == ODT_COMBOBOX);
		LPSUPERITEMDATA pData = reinterpret_cast<LPSUPERITEMDATA>(lpDeleteItemStruct->itemData);
		if(!pData) return;

		ASSERT((int)pData != CB_ERR);

		delete pData;
	}
};

class CSuperComboBoxCtrl : public CSuperComboBoxImpl<CSuperComboBoxCtrl>
{
public:
   DECLARE_WND_SUPERCLASS(_T("WTL_SuperComboBox"), GetWndClassName())  
};

#endif __SUPERCOMBO_H__
