#ifndef __PROPERTYITEMEDITORS__H
#define __PROPERTYITEMEDITORS__H

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CPropertyItemEditors - Editors for Property controls
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001-2002 Bjarke Viksoe.
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
   bool m_fCancel;

   CPropertyEditWindow() : m_fCancel(false)
   {
   }

   virtual void OnFinalMessage(HWND /*hWnd*/)
   {
      delete this;
   }

   BEGIN_MSG_MAP(CPropertyEditWindow)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
      MESSAGE_HANDLER(WM_CHAR, OnChar)
      MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
      MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
      MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
   END_MSG_MAP()

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
      SetFont( (HFONT)::SendMessage(GetParent(), WM_GETFONT, 0, 0) );
      SetMargins(PROP_TEXT_INDENT, 0);   // Force EDIT margins so text doesn't jump
      return lRes;
   }
   LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {
      switch( wParam ) {
      case VK_ESCAPE:
         m_fCancel = true;
         // FALL THROUGH...
      case VK_RETURN:
         // Force focus to parent to update value (see OnKillFocus()...)
         ::SetFocus(GetParent());
         break;
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
      case VK_ESCAPE:
      case VK_RETURN:
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
      ::SendMessage(GetParent(), m_fCancel ? WM_USER_PROP_CANCELPROPERTY : WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
      return lRes;
   }
   LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      return DefWindowProc(uMsg, wParam, lParam) | DLGC_WANTALLKEYS | DLGC_WANTARROWS;
   }
};


/////////////////////////////////////////////////////////////////////////////
// General implementation of editor with button

template< class T >
class CPropertyDropWindowImpl : 
   public CWindowImpl< T, CEdit, CControlWinTraits >
{
public:
   CContainedWindowT<CButton> m_wndButton;

   virtual void OnFinalMessage(HWND /*hWnd*/)
   {
      delete (T*) this;
   };

   typedef CPropertyDropWindowImpl< T > thisClass;

   BEGIN_MSG_MAP(thisClass)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
      MESSAGE_HANDLER(WM_CHAR, OnChar)
      MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseButtonClick)
      MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouseButtonClick)
      MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
   ALT_MSG_MAP(1) // Button
      MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
      MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
   END_MSG_MAP()

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
      RECT rcClient;
      GetClientRect(&rcClient);
      int cy = (rcClient.bottom-rcClient.top);
      // Setup EDIT control
      SetFont( (HFONT)::SendMessage(GetParent(), WM_GETFONT, 0, 0) );
      ModifyStyle(WS_BORDER, ES_LEFT);
      SetMargins(PROP_TEXT_INDENT, cy);
      // Create button
      RECT rcButton = { rcClient.right-cy, rcClient.top-1, rcClient.right, rcClient.bottom };
      m_wndButton.Create(this, 1, m_hWnd, &rcButton, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | BS_PUSHBUTTON | BS_OWNERDRAW);
      ATLASSERT(m_wndButton.IsWindow());
      m_wndButton.SetFont( (HFONT)::SendMessage(GetParent(), WM_GETFONT, 0, 0) );
      return lRes;
   }
   LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc();
      m_wndButton.SetFocus(); // Set focus to button to prevent input
      return lRes;
   }
   LRESULT OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      // Don't allow any editing
      return 0;
   }
   LRESULT OnMouseButtonClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      return 0; // Don't allow selection or context menu for edit box
   }

   // Button

   LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {
      switch( wParam ) {
      case VK_ESCAPE:
         ::SendMessage(GetParent(), WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
         return 0;
      }
      bHandled = FALSE;
      return 0;
   }
   LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      return DefWindowProc(uMsg, wParam, lParam) | DLGC_WANTALLKEYS;
   }
};


/////////////////////////////////////////////////////////////////////////////
// Editor with dropdown list

class CPropertyComboWindow : 
   public CPropertyDropWindowImpl<CPropertyComboWindow>
{
public:
   CContainedWindowT<CListBox> m_wndList;
   int m_cyList;  // Used to resize the listbox when first shown

   typedef CPropertyDropWindowImpl<CPropertyComboWindow> baseClass;

   BEGIN_MSG_MAP(CPropertyComboWindow)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
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

   LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
   {
      // Create dropdown list (as hidden)
      RECT rc = CWindow::rcDefault;
      m_wndList.Create(this, 2, m_hWnd, &rc, NULL, WS_POPUP | WS_BORDER | WS_VSCROLL);
      m_wndList.SetFont( (HFONT)::SendMessage(GetParent(), WM_GETFONT, 0, 0) );
      ATLASSERT(m_wndList.IsWindow());
      // Go create the rest of the control...
      bHandled = FALSE;
      return 0;
   }
   LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
   {
      if( m_wndList.IsWindow() ) m_wndList.DestroyWindow();
      bHandled = FALSE;
      return 0;
   }
   LRESULT OnButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
   {
      if( m_cyList==0 ) {
         // Resize list to fit all items (but not more than 140 pixels)
         const int MAX_HEIGHT = 140;
         RECT rc;
         m_wndList.GetWindowRect(&rc);
         int cy = m_wndList.GetCount() * m_wndList.GetItemHeight(0);
         m_cyList = min( MAX_HEIGHT, rc.top + cy + (::GetSystemMetrics(SM_CYBORDER)*2) );
      }
      // Move the dropdown under the item
      RECT rcClient;
      GetWindowRect(&rcClient);
      RECT rc = { rcClient.left, rcClient.bottom, rcClient.right, rcClient.bottom + m_cyList };
      ::SetWindowPos(m_wndList, HWND_TOP, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_SHOWWINDOW);
      return 0;
   }
   LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {
      switch( wParam ) {
      case VK_RETURN:
         {
            int idx = m_wndList.GetCurSel();
            if( idx>=0 ) {
               // Copy text from list to item
               int len = m_wndList.GetTextLen(idx)+1;
               LPTSTR pstr = (LPTSTR) _alloca(len * sizeof(TCHAR));
               m_wndList.GetText(idx, pstr);
               SetWindowText(pstr);
               // Announce the new value
               ::SendMessage(GetParent(), WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM)m_hWnd);
            }
         }
         ::SetFocus(GetParent());
         break;
      case VK_ESCAPE:
         // Announce the cancellation
         ::SendMessage(GetParent(), WM_USER_PROP_CANCELPROPERTY, 0, (LPARAM)m_hWnd);
         ::SetFocus(GetParent());
         break;
      }
      bHandled = FALSE;
      return 0;
   }
   LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
   {
      // Selected an item? Fake RETURN key to copy new value...
      BOOL bDummy;
      OnKeyDown(WM_KEYDOWN, VK_RETURN, 0, bDummy);
      bHandled = FALSE;
      return 0;
   }
   LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc();
      m_wndList.ShowWindow(SW_HIDE);
      return lRes;
   }

   LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
      if( m_wndButton!=lpdis->hwndItem ) return 0;
      CDCHandle dc(lpdis->hDC);
      // Paint as dropdown button
      dc.DrawFrameControl(&lpdis->rcItem, DFC_SCROLL, (lpdis->itemState & ODS_SELECTED) ? DFCS_SCROLLDOWN|DFCS_PUSHED : DFCS_SCROLLDOWN);
      return 0;
   }
};


/////////////////////////////////////////////////////////////////////////////
// Editor with browse button

class CPropertyButtonWindow : 
   public CPropertyDropWindowImpl<CPropertyButtonWindow>
{
public:
   IProperty* m_prop; // BUG: Dangerous reference

   typedef CPropertyDropWindowImpl<CPropertyButtonWindow> baseClass;

   BEGIN_MSG_MAP(CPropertyButtonWindow)
      COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
      MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
      CHAIN_MSG_MAP(baseClass)
   ALT_MSG_MAP(1) // Button
      CHAIN_MSG_MAP_ALT(baseClass, 1)
   END_MSG_MAP()

   LRESULT OnButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
   {
      ATLASSERT(m_prop);
      // Call Property class' implementation of BROWSE action
      m_prop->Activate(PACT_BROWSE, 0);
      return 0;
   }

   LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
      if( m_wndButton!=lpdis->hwndItem ) return 0;
      CDCHandle dc(lpdis->hDC);
      // Paint as ellipsis button
      dc.DrawFrameControl(&lpdis->rcItem, DFC_BUTTON, (lpdis->itemState & ODS_SELECTED) ? DFCS_BUTTONPUSH|DFCS_PUSHED : DFCS_BUTTONPUSH);
      dc.SetBkMode(TRANSPARENT);
      LPCTSTR pstrEllipsis = _T("...");
      dc.DrawText(pstrEllipsis, ::lstrlen(pstrEllipsis), &lpdis->rcItem, DT_CENTER | DT_EDITCONTROL | DT_SINGLELINE | DT_VCENTER);
      return 0;
   }
};


#endif // __PROPERTYITEMEDITORS__H
