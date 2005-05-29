// Windows Template Library - WTL version 7.5
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// This file is a part of the Windows Template Library.
// The use and distribution terms for this software are covered by the
// Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
// which can be found in the file CPL.TXT at the root of this distribution.
// By using this software in any fashion, you are agreeing to be bound by
// the terms of this license. You must not remove this notice, or
// any other, from this software.

#ifndef __ATLUSER_H__
#define __ATLUSER_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atluser.h requires atlapp.h to be included first
#endif


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CMenuItemInfo
// CMenuT<t_bManaged>
// CAcceleratorT<t_bManaged>
// CIconT<t_bManaged>
// CCursorT<t_bManaged>
// CResource
//
// Global functions:
//   AtlMessageBox()


namespace WTL
{

///////////////////////////////////////////////////////////////////////////////
// AtlMessageBox - accepts both memory and resource based strings

inline int AtlMessageBox(HWND hWndOwner, ATL::_U_STRINGorID message, ATL::_U_STRINGorID title = (LPCTSTR)NULL, UINT uType = MB_OK | MB_ICONINFORMATION)
{
	ATLASSERT(hWndOwner == NULL || ::IsWindow(hWndOwner));

	LPTSTR lpstrMessage = NULL;
	if(IS_INTRESOURCE(message.m_lpstr))
	{
		for(int nLen = 256; ; nLen *= 2)
		{
			ATLTRY(lpstrMessage = new TCHAR[nLen]);
			if(lpstrMessage == NULL)
			{
				ATLASSERT(FALSE);
				return 0;
			}
#if (_ATL_VER >= 0x0700)
			int nRes = ::LoadString(ATL::_AtlBaseModule.GetResourceInstance(), LOWORD(message.m_lpstr), lpstrMessage, nLen);
#else //!(_ATL_VER >= 0x0700)
			int nRes = ::LoadString(ATL::_pModule->GetResourceInstance(), LOWORD(message.m_lpstr), lpstrMessage, nLen);
#endif //!(_ATL_VER >= 0x0700)
			if(nRes < nLen - 1)
				break;
			delete [] lpstrMessage;
			lpstrMessage = NULL;
		}

		message.m_lpstr = lpstrMessage;
	}

	LPTSTR lpstrTitle = NULL;
	if(IS_INTRESOURCE(title.m_lpstr) && LOWORD(title.m_lpstr) != 0)
	{
		for(int nLen = 256; ; nLen *= 2)
		{
			ATLTRY(lpstrTitle = new TCHAR[nLen]);
			if(lpstrTitle == NULL)
			{
				ATLASSERT(FALSE);
				return 0;
			}
#if (_ATL_VER >= 0x0700)
			int nRes = ::LoadString(ATL::_AtlBaseModule.GetResourceInstance(), LOWORD(title.m_lpstr), lpstrTitle, nLen);
#else //!(_ATL_VER >= 0x0700)
			int nRes = ::LoadString(ATL::_pModule->GetResourceInstance(), LOWORD(title.m_lpstr), lpstrTitle, nLen);
#endif //!(_ATL_VER >= 0x0700)
			if(nRes < nLen - 1)
				break;
			delete [] lpstrTitle;
			lpstrTitle = NULL;
		}

		title.m_lpstr = lpstrTitle;
	}

	int nRet = ::MessageBox(hWndOwner, message.m_lpstr, title.m_lpstr, uType);

	delete [] lpstrMessage;
	delete [] lpstrTitle;

	return nRet;
}


///////////////////////////////////////////////////////////////////////////////
// CMenu

#if (WINVER >= 0x0500)
  #ifndef MII_SIZEOF_STRUCT
    #define MII_SIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
  #endif
  #define MENUITEMINFO_SIZE_VERSION_400A  MII_SIZEOF_STRUCT(MENUITEMINFOA, cch)
  #define MENUITEMINFO_SIZE_VERSION_400W  MII_SIZEOF_STRUCT(MENUITEMINFOW, cch)
  #ifdef UNICODE
    #define MENUITEMINFO_SIZE_VERSION_400  MENUITEMINFO_SIZE_VERSION_400W
  #else
    #define MENUITEMINFO_SIZE_VERSION_400  MENUITEMINFO_SIZE_VERSION_400A
  #endif // !UNICODE
#endif //(WINVER >= 0x0500)

class CMenuItemInfo : public MENUITEMINFO
{
public:
	CMenuItemInfo()
	{
		memset(this, 0, sizeof(MENUITEMINFO));
		cbSize = sizeof(MENUITEMINFO);
#if (WINVER >= 0x0500)
		// adjust struct size if running on older version of Windows
		if(AtlIsOldWindows())
		{
			ATLASSERT(cbSize > MENUITEMINFO_SIZE_VERSION_400);   // must be
			cbSize = MENUITEMINFO_SIZE_VERSION_400;
		}
#endif //(WINVER >= 0x0500)
	}
};


// forward declarations
template <bool t_bManaged> class CMenuT;
typedef CMenuT<false>   CMenuHandle;
typedef CMenuT<true>    CMenu;


template <bool t_bManaged>
class CMenuT
{
public:
// Data members
	HMENU m_hMenu;

// Constructor/destructor/operators
	CMenuT(HMENU hMenu = NULL) : m_hMenu(hMenu)
	{ }

	~CMenuT()
	{
		if(t_bManaged && m_hMenu != NULL)
			DestroyMenu();
	}

	CMenuT<t_bManaged>& operator =(HMENU hMenu)
	{
		Attach(hMenu);
		return *this;
	}

	void Attach(HMENU hMenuNew)
	{
		ATLASSERT(::IsMenu(hMenuNew));
		if(t_bManaged && m_hMenu != NULL && m_hMenu != hMenuNew)
			::DestroyMenu(m_hMenu);
		m_hMenu = hMenuNew;
	}

	HMENU Detach()
	{
		HMENU hMenu = m_hMenu;
		m_hMenu = NULL;
		return hMenu;
	}

	operator HMENU() const { return m_hMenu; }

	bool IsNull() const { return (m_hMenu == NULL); }

	BOOL IsMenu() const
	{
		return ::IsMenu(m_hMenu);
	}

// Create/destroy methods
	BOOL CreateMenu()
	{
		ATLASSERT(m_hMenu == NULL);
		m_hMenu = ::CreateMenu();
		return (m_hMenu != NULL) ? TRUE : FALSE;
	}

	BOOL CreatePopupMenu()
	{
		ATLASSERT(m_hMenu == NULL);
		m_hMenu = ::CreatePopupMenu();
		return (m_hMenu != NULL) ? TRUE : FALSE;
	}

	BOOL LoadMenu(ATL::_U_STRINGorID menu)
	{
		ATLASSERT(m_hMenu == NULL);
#if (_ATL_VER >= 0x0700)
		m_hMenu = ::LoadMenu(ATL::_AtlBaseModule.GetResourceInstance(), menu.m_lpstr);
#else //!(_ATL_VER >= 0x0700)
		m_hMenu = ::LoadMenu(_Module.GetResourceInstance(), menu.m_lpstr);
#endif //!(_ATL_VER >= 0x0700)
		return (m_hMenu != NULL) ? TRUE : FALSE;
	}

#ifndef _WIN32_WCE
	BOOL LoadMenuIndirect(const void* lpMenuTemplate)
	{
		ATLASSERT(m_hMenu == NULL);
		m_hMenu = ::LoadMenuIndirect(lpMenuTemplate);
		return (m_hMenu != NULL) ? TRUE : FALSE;
	}
#endif //!_WIN32_WCE

	BOOL DestroyMenu()
	{
		if (m_hMenu == NULL)
			return FALSE;
		BOOL bRet = ::DestroyMenu(m_hMenu);
		if(bRet)
			m_hMenu = NULL;
		return bRet;
	}

// Menu Operations
	BOOL DeleteMenu(UINT nPosition, UINT nFlags)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::DeleteMenu(m_hMenu, nPosition, nFlags);
	}

	BOOL TrackPopupMenu(UINT nFlags, int x, int y, HWND hWnd, LPCRECT lpRect = NULL)
	{
		ATLASSERT(::IsMenu(m_hMenu));
#ifndef _WIN32_WCE
#if (WINVER >= 0x0500)
		x = _FixTrackMenuPopupX(x, y);
#endif //!(WINVER >= 0x0500)
		return ::TrackPopupMenu(m_hMenu, nFlags, x, y, 0, hWnd, lpRect);
#else // CE specific
		lpRect;
		return ::TrackPopupMenuEx(m_hMenu, nFlags, x, y, hWnd, NULL);
#endif //_WIN32_WCE
	}

	BOOL TrackPopupMenuEx(UINT uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm = NULL)
	{
		ATLASSERT(::IsMenu(m_hMenu));
#if (WINVER >= 0x0500) && !defined(_WIN32_WCE)
		x = _FixTrackMenuPopupX(x, y);
#endif //(WINVER >= 0x0500) && !defined(_WIN32_WCE)
		return ::TrackPopupMenuEx(m_hMenu, uFlags, x, y, hWnd, lptpm);
	}

#if (WINVER >= 0x0500) && !defined(_WIN32_WCE)
	// helper that fixes popup menu X position when it's off-screen
	static int _FixTrackMenuPopupX(int x, int y)
	{
		POINT pt = { x, y };
		HMONITOR hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
		if(hMonitor == NULL)
		{
			HMONITOR hMonitorNear = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
			if(hMonitorNear != NULL)
			{
				MONITORINFO mi = { 0 };
				mi.cbSize = sizeof(MONITORINFO);
				if(::GetMonitorInfo(hMonitorNear, &mi) != FALSE)
				{
					if(x < mi.rcWork.left)
						x = mi.rcWork.left;
					else if(x > mi.rcWork.right)
						x = mi.rcWork.right;
				}
			}
		}

		return x;
	}

	BOOL GetMenuInfo(LPMENUINFO lpMenuInfo) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::GetMenuInfo(m_hMenu, lpMenuInfo);
	}

	BOOL SetMenuInfo(LPCMENUINFO lpMenuInfo)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::SetMenuInfo(m_hMenu, lpMenuInfo);
	}
#endif //(WINVER >= 0x0500) && !defined(_WIN32_WCE)

// Menu Item Operations
	BOOL AppendMenu(UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::AppendMenu(m_hMenu, nFlags, nIDNewItem, lpszNewItem);
	}

	BOOL AppendMenu(UINT nFlags, HMENU hSubMenu, LPCTSTR lpszNewItem)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		ATLASSERT(::IsMenu(hSubMenu));
		return ::AppendMenu(m_hMenu, nFlags | MF_POPUP, (UINT_PTR)hSubMenu, lpszNewItem);
	}

#ifndef _WIN32_WCE
	BOOL AppendMenu(UINT nFlags, UINT_PTR nIDNewItem, HBITMAP hBmp)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::AppendMenu(m_hMenu, nFlags | MF_BITMAP, nIDNewItem, (LPCTSTR)hBmp);
	}

	BOOL AppendMenu(UINT nFlags, HMENU hSubMenu, HBITMAP hBmp)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		ATLASSERT(::IsMenu(hSubMenu));
		return ::AppendMenu(m_hMenu, nFlags | (MF_BITMAP | MF_POPUP), (UINT_PTR)hSubMenu, (LPCTSTR)hBmp);
	}
#endif //!_WIN32_WCE

	UINT CheckMenuItem(UINT nIDCheckItem, UINT nCheck)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return (UINT)::CheckMenuItem(m_hMenu, nIDCheckItem, nCheck);
	}

	UINT EnableMenuItem(UINT nIDEnableItem, UINT nEnable)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::EnableMenuItem(m_hMenu, nIDEnableItem, nEnable);
	}

#ifndef _WIN32_WCE
	BOOL HiliteMenuItem(HWND hWnd, UINT uIDHiliteItem, UINT uHilite)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::HiliteMenuItem(hWnd, m_hMenu, uIDHiliteItem, uHilite);
	}

	int GetMenuItemCount() const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::GetMenuItemCount(m_hMenu);
	}

	UINT GetMenuItemID(int nPos) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::GetMenuItemID(m_hMenu, nPos);
	}

	UINT GetMenuState(UINT nID, UINT nFlags) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::GetMenuState(m_hMenu, nID, nFlags);
	}

	int GetMenuString(UINT nIDItem, LPTSTR lpString, int nMaxCount, UINT nFlags) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::GetMenuString(m_hMenu, nIDItem, lpString, nMaxCount, nFlags);
	}

	int GetMenuStringLen(UINT nIDItem, UINT nFlags) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::GetMenuString(m_hMenu, nIDItem, NULL, 0, nFlags);
	}

#ifndef _ATL_NO_COM
	BOOL GetMenuString(UINT nIDItem, BSTR& bstrText, UINT nFlags) const
	{
		USES_CONVERSION;
		ATLASSERT(::IsMenu(m_hMenu));
		ATLASSERT(bstrText == NULL);

		int nLen = GetMenuStringLen(nIDItem, nFlags);
		if(nLen == 0)
		{
			bstrText = ::SysAllocString(OLESTR(""));
			return (bstrText != NULL) ? TRUE : FALSE;
		}

		nLen++;   // increment to include terminating NULL char
		LPTSTR lpszText = (LPTSTR)_alloca((nLen) * sizeof(TCHAR));

		if(!GetMenuString(nIDItem, lpszText, nLen, nFlags))
			return FALSE;

		bstrText = ::SysAllocString(T2OLE(lpszText));
		return (bstrText != NULL) ? TRUE : FALSE;
	}
#endif //!_ATL_NO_COM
#endif //!_WIN32_WCE

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	int GetMenuString(UINT nIDItem, _CSTRING_NS::CString& strText, UINT nFlags) const
	{
		ATLASSERT(::IsMenu(m_hMenu));

		int nLen = GetMenuStringLen(nIDItem, nFlags);
		if(nLen == 0)
			return 0;

		nLen++;   // increment to include terminating NULL char
		LPTSTR lpstr = strText.GetBufferSetLength(nLen);
		if(lpstr == NULL)
			return 0;
		int nRet = GetMenuString(nIDItem, lpstr, nLen, nFlags);
		strText.ReleaseBuffer();
		return nRet;
	}
#endif //defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	CMenuHandle GetSubMenu(int nPos) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return CMenuHandle(::GetSubMenu(m_hMenu, nPos));
	}

	BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::InsertMenu(m_hMenu, nPosition, nFlags, nIDNewItem, lpszNewItem);
	}

	BOOL InsertMenu(UINT nPosition, UINT nFlags, HMENU hSubMenu, LPCTSTR lpszNewItem)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		ATLASSERT(::IsMenu(hSubMenu));
		return ::InsertMenu(m_hMenu, nPosition, nFlags | MF_POPUP, (UINT_PTR)hSubMenu, lpszNewItem);
	}

#ifndef _WIN32_WCE
	BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, HBITMAP hBmp)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::InsertMenu(m_hMenu, nPosition, nFlags | MF_BITMAP, nIDNewItem, (LPCTSTR)hBmp);
	}

	BOOL InsertMenu(UINT nPosition, UINT nFlags, HMENU hSubMenu, HBITMAP hBmp)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		ATLASSERT(::IsMenu(hSubMenu));
		return ::InsertMenu(m_hMenu, nPosition, nFlags | (MF_BITMAP | MF_POPUP), (UINT_PTR)hSubMenu, (LPCTSTR)hBmp);
	}

	BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::ModifyMenu(m_hMenu, nPosition, nFlags, nIDNewItem, lpszNewItem);
	}

	BOOL ModifyMenu(UINT nPosition, UINT nFlags, HMENU hSubMenu, LPCTSTR lpszNewItem)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		ATLASSERT(::IsMenu(hSubMenu));
		return ::ModifyMenu(m_hMenu, nPosition, nFlags | MF_POPUP, (UINT_PTR)hSubMenu, lpszNewItem);
	}

	BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, HBITMAP hBmp)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::ModifyMenu(m_hMenu, nPosition, nFlags | MF_BITMAP, nIDNewItem, (LPCTSTR)hBmp);
	}

	BOOL ModifyMenu(UINT nPosition, UINT nFlags, HMENU hSubMenu, HBITMAP hBmp)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		ATLASSERT(::IsMenu(hSubMenu));
		return ::ModifyMenu(m_hMenu, nPosition, nFlags | (MF_BITMAP | MF_POPUP), (UINT_PTR)hSubMenu, (LPCTSTR)hBmp);
	}
#endif //!_WIN32_WCE

	BOOL RemoveMenu(UINT nPosition, UINT nFlags)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::RemoveMenu(m_hMenu, nPosition, nFlags);
	}

#ifndef _WIN32_WCE
	BOOL SetMenuItemBitmaps(UINT nPosition, UINT nFlags, HBITMAP hBmpUnchecked, HBITMAP hBmpChecked)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::SetMenuItemBitmaps(m_hMenu, nPosition, nFlags, hBmpUnchecked, hBmpChecked);
	}
#endif //!_WIN32_WCE

	BOOL CheckMenuRadioItem(UINT nIDFirst, UINT nIDLast, UINT nIDItem, UINT nFlags)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::CheckMenuRadioItem(m_hMenu, nIDFirst, nIDLast, nIDItem, nFlags);
	}

	BOOL GetMenuItemInfo(UINT uItem, BOOL bByPosition, LPMENUITEMINFO lpmii) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return (BOOL)::GetMenuItemInfo(m_hMenu, uItem, bByPosition, lpmii);
	}

	BOOL SetMenuItemInfo(UINT uItem, BOOL bByPosition, LPMENUITEMINFO lpmii)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return (BOOL)::SetMenuItemInfo(m_hMenu, uItem, bByPosition, lpmii);
	}

#ifndef _WIN32_WCE
	BOOL InsertMenuItem(UINT uItem, BOOL bByPosition, LPMENUITEMINFO lpmii)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return (BOOL)::InsertMenuItem(m_hMenu, uItem, bByPosition, lpmii);
	}

	UINT GetMenuDefaultItem(BOOL bByPosition = FALSE, UINT uFlags = 0U) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::GetMenuDefaultItem(m_hMenu, (UINT)bByPosition, uFlags);
	}

	BOOL SetMenuDefaultItem(UINT uItem = (UINT)-1,  BOOL bByPosition = FALSE)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::SetMenuDefaultItem(m_hMenu, uItem, (UINT)bByPosition);
	}

	BOOL GetMenuItemRect(HWND hWnd, UINT uItem, LPRECT lprcItem) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::GetMenuItemRect(hWnd, m_hMenu, uItem, lprcItem);
	}

	int MenuItemFromPoint(HWND hWnd, POINT point) const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::MenuItemFromPoint(hWnd, m_hMenu, point);
	}

// Context Help Functions
	BOOL SetMenuContextHelpId(DWORD dwContextHelpId)
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::SetMenuContextHelpId(m_hMenu, dwContextHelpId);
	}

	DWORD GetMenuContextHelpId() const
	{
		ATLASSERT(::IsMenu(m_hMenu));
		return ::GetMenuContextHelpId(m_hMenu);
	}
#endif //!_WIN32_WCE
};


///////////////////////////////////////////////////////////////////////////////
// CAccelerator

template <bool t_bManaged>
class CAcceleratorT
{
public:
	HACCEL m_hAccel;

// Constructor/destructor/operators
	CAcceleratorT(HACCEL hAccel = NULL) : m_hAccel(hAccel)
	{ }

	~CAcceleratorT()
	{
		if(t_bManaged && m_hAccel != NULL)
			::DestroyAcceleratorTable(m_hAccel);
	}

	CAcceleratorT<t_bManaged>& operator =(HACCEL hAccel)
	{
		Attach(hAccel);
		return *this;
	}

	void Attach(HACCEL hAccel)
	{
		if(t_bManaged && m_hAccel != NULL)
			::DestroyAcceleratorTable(m_hAccel);
		m_hAccel = hAccel;
	}

	HACCEL Detach()
	{
		HACCEL hAccel = m_hAccel;
		m_hAccel = NULL;
		return hAccel;
	}

	operator HACCEL() const { return m_hAccel; }

	bool IsNull() const { return m_hAccel == NULL; }

// Create/destroy methods
	HACCEL LoadAccelerators(ATL::_U_STRINGorID accel)
	{
		ATLASSERT(m_hAccel == NULL);
#if (_ATL_VER >= 0x0700)
		m_hAccel = ::LoadAccelerators(ATL::_AtlBaseModule.GetResourceInstance(), accel.m_lpstr);
#else //!(_ATL_VER >= 0x0700)
		m_hAccel = ::LoadAccelerators(ATL::_pModule->GetResourceInstance(), accel.m_lpstr);
#endif //!(_ATL_VER >= 0x0700)
		return m_hAccel;
	}

	HACCEL CreateAcceleratorTable(LPACCEL pAccel, int cEntries)
	{
		ATLASSERT(m_hAccel == NULL);
		ATLASSERT(!::IsBadReadPtr(pAccel, sizeof(ACCEL) * cEntries));
		m_hAccel = ::CreateAcceleratorTable(pAccel, cEntries);
		return m_hAccel;
	}

	void DestroyObject()
	{
		if(m_hAccel != NULL)
		{
			::DestroyAcceleratorTable(m_hAccel);
			m_hAccel = NULL;
		}
	}

// Operations
#ifndef _WIN32_WCE
	int CopyAcceleratorTable(LPACCEL lpAccelDst, int cEntries)
	{
		ATLASSERT(m_hAccel != NULL);
		ATLASSERT(!::IsBadWritePtr(lpAccelDst, sizeof(ACCEL) * cEntries));
		return ::CopyAcceleratorTable(m_hAccel, lpAccelDst, cEntries);
	}
#endif //!_WIN32_WCE

	BOOL TranslateAccelerator(HWND hWnd, LPMSG pMsg)
	{
		ATLASSERT(m_hAccel != NULL);
		ATLASSERT(::IsWindow(hWnd));
		ATLASSERT(pMsg != NULL);
		return ::TranslateAccelerator(hWnd, m_hAccel, pMsg);
	}
};

typedef CAcceleratorT<false>   CAcceleratorHandle;
typedef CAcceleratorT<true>    CAccelerator;


///////////////////////////////////////////////////////////////////////////////
// CIcon

template <bool t_bManaged>
class CIconT
{
public:
	HICON m_hIcon;

// Constructor/destructor/operators
	CIconT(HICON hIcon = NULL) : m_hIcon(hIcon)
	{ }

	~CIconT()
	{
		if(t_bManaged && m_hIcon != NULL)
			::DestroyIcon(m_hIcon);
	}

	CIconT<t_bManaged>& operator =(HICON hIcon)
	{
		Attach(hIcon);
		return *this;
	}

	void Attach(HICON hIcon)
	{
		if(t_bManaged && m_hIcon != NULL)
			::DestroyIcon(m_hIcon);
		m_hIcon = hIcon;
	}

	HICON Detach()
	{
		HICON hIcon = m_hIcon;
		m_hIcon = NULL;
		return hIcon;
	}

	operator HICON() const { return m_hIcon; }

	bool IsNull() const { return m_hIcon == NULL; }

// Create/destroy methods
	HICON LoadIcon(ATL::_U_STRINGorID icon)
	{
		ATLASSERT(m_hIcon == NULL);
#if (_ATL_VER >= 0x0700)
		m_hIcon = ::LoadIcon(ATL::_AtlBaseModule.GetResourceInstance(), icon.m_lpstr);
#else //!(_ATL_VER >= 0x0700)
		m_hIcon = ::LoadIcon(ATL::_pModule->GetResourceInstance(), icon.m_lpstr);
#endif //!(_ATL_VER >= 0x0700)
		return m_hIcon;
	}

	HICON LoadIcon(ATL::_U_STRINGorID icon, int cxDesired, int cyDesired, UINT fuLoad = 0)
	{
		ATLASSERT(m_hIcon == NULL);
#if (_ATL_VER >= 0x0700)
		m_hIcon = (HICON) ::LoadImage(ATL::_AtlBaseModule.GetResourceInstance(), icon.m_lpstr, IMAGE_ICON, cxDesired, cyDesired, fuLoad);
#else //!(_ATL_VER >= 0x0700)
		m_hIcon = (HICON) ::LoadImage(ATL::_pModule->GetResourceInstance(), icon.m_lpstr, IMAGE_ICON, cxDesired, cyDesired, fuLoad);
#endif //!(_ATL_VER >= 0x0700)
		return m_hIcon;
	}

	HICON LoadOEMIcon(UINT nIDIcon)   // for IDI_* types
	{
		ATLASSERT(m_hIcon == NULL);
		m_hIcon = ::LoadIcon(NULL, MAKEINTRESOURCE(nIDIcon));
		return m_hIcon;
	}

#ifndef _WIN32_WCE
	HICON CreateIcon(int nWidth, int nHeight, BYTE cPlanes, BYTE cBitsPixel, CONST BYTE* lpbANDbits, CONST BYTE *lpbXORbits)
	{
		ATLASSERT(m_hIcon == NULL);
		ATLASSERT(lpbANDbits != NULL);
		ATLASSERT(lpbXORbits != NULL);
#if (_ATL_VER >= 0x0700)
		m_hIcon = ::CreateIcon(ATL::_AtlBaseModule.GetResourceInstance(), nWidth, nHeight, cPlanes, cBitsPixel, lpbANDbits, lpbXORbits);
#else //!(_ATL_VER >= 0x0700)
		m_hIcon = ::CreateIcon(ATL::_pModule->GetResourceInstance(), nWidth, nHeight, cPlanes, cBitsPixel, lpbANDbits, lpbXORbits);
#endif //!(_ATL_VER >= 0x0700)
		return m_hIcon;
	}

	HICON CreateIconFromResource(PBYTE pBits, DWORD dwResSize, DWORD dwVersion = 0x00030000)
	{
		ATLASSERT(m_hIcon == NULL);
		ATLASSERT(pBits != NULL);
		m_hIcon = ::CreateIconFromResource(pBits, dwResSize, TRUE, dwVersion);
		return m_hIcon;
	}

	HICON CreateIconFromResourceEx(PBYTE pbBits, DWORD cbBits, DWORD dwVersion = 0x00030000, int cxDesired = 0, int cyDesired = 0, UINT uFlags = LR_DEFAULTCOLOR)
	{
		ATLASSERT(m_hIcon == NULL);
		ATLASSERT(pbBits != NULL);
		ATLASSERT(cbBits > 0);
		m_hIcon = ::CreateIconFromResourceEx(pbBits, cbBits, TRUE, dwVersion, cxDesired, cyDesired, uFlags);
		return m_hIcon;
	}
#endif //!_WIN32_WCE

	HICON CreateIconIndirect(PICONINFO pIconInfo)
	{
		ATLASSERT(m_hIcon == NULL);
		ATLASSERT(pIconInfo != NULL);
		m_hIcon = ::CreateIconIndirect(pIconInfo);
		return m_hIcon;
	}

#ifndef _WIN32_WCE
	HICON ExtractIcon(LPCTSTR lpszExeFileName, UINT nIconIndex)
	{
		ATLASSERT(m_hIcon == NULL);
		ATLASSERT(!::IsBadStringPtr(lpszExeFileName, (UINT_PTR)-1));
#if (_ATL_VER >= 0x0700)
		m_hIcon = ::ExtractIcon(ATL::_AtlBaseModule.GetModuleInstance(), lpszExeFileName, nIconIndex);
#else //!(_ATL_VER >= 0x0700)
		m_hIcon = ::ExtractIcon(ATL::_pModule->GetModuleInstance(), lpszExeFileName, nIconIndex);
#endif //!(_ATL_VER >= 0x0700)
		return m_hIcon;
	}

	HICON ExtractAssociatedIcon(HINSTANCE hInst, LPTSTR lpIconPath, LPWORD lpiIcon)
	{
		ATLASSERT(m_hIcon == NULL);
		ATLASSERT(!::IsBadStringPtr(lpIconPath, (UINT_PTR)-1));
		ATLASSERT(lpiIcon != NULL);
		m_hIcon = ::ExtractAssociatedIcon(hInst, lpIconPath, lpiIcon);
		return m_hIcon;
	}
#endif //!_WIN32_WCE

	BOOL DestroyIcon()
	{
		ATLASSERT(m_hIcon != NULL);
		BOOL bRet = ::DestroyIcon(m_hIcon);
		if(bRet != FALSE)
			m_hIcon = NULL;
		return bRet;
	}

// Operations
#ifndef _WIN32_WCE
	HICON CopyIcon()
	{
		ATLASSERT(m_hIcon != NULL);
		return ::CopyIcon(m_hIcon);
	}

	HICON DuplicateIcon()
	{
		ATLASSERT(m_hIcon != NULL);
		return ::DuplicateIcon(NULL, m_hIcon);
	}
#endif //!_WIN32_WCE

	BOOL DrawIcon(HDC hDC, int x, int y)
	{
		ATLASSERT(m_hIcon != NULL);
#ifndef _WIN32_WCE
		return ::DrawIcon(hDC, x, y, m_hIcon);
#else // CE specific
		return ::DrawIconEx(hDC, x, y, m_hIcon, 0, 0, 0, NULL, DI_NORMAL);
#endif //_WIN32_WCE
	}

	BOOL DrawIcon(HDC hDC, POINT pt)
	{
		ATLASSERT(m_hIcon != NULL);
#ifndef _WIN32_WCE
		return ::DrawIcon(hDC, pt.x, pt.y, m_hIcon);
#else // CE specific
		return ::DrawIconEx(hDC, pt.x, pt.y, m_hIcon, 0, 0, 0, NULL, DI_NORMAL);
#endif //_WIN32_WCE
	}

	BOOL DrawIconEx(HDC hDC, int x, int y, int cxWidth, int cyWidth, UINT uStepIfAniCur = 0, HBRUSH hbrFlickerFreeDraw = NULL, UINT uFlags = DI_NORMAL)
	{
		ATLASSERT(m_hIcon != NULL);
		return ::DrawIconEx(hDC, x, y, m_hIcon, cxWidth, cyWidth, uStepIfAniCur, hbrFlickerFreeDraw, uFlags);
	}

	BOOL DrawIconEx(HDC hDC, POINT pt, SIZE size, UINT uStepIfAniCur = 0, HBRUSH hbrFlickerFreeDraw = NULL, UINT uFlags = DI_NORMAL)
	{
		ATLASSERT(m_hIcon != NULL);
		return ::DrawIconEx(hDC, pt.x, pt.y, m_hIcon, size.cx, size.cy, uStepIfAniCur, hbrFlickerFreeDraw, uFlags);
	}

#ifndef _WIN32_WCE
	BOOL GetIconInfo(PICONINFO pIconInfo)
	{
		ATLASSERT(m_hIcon != NULL);
		ATLASSERT(pIconInfo != NULL);
		return ::GetIconInfo(m_hIcon, pIconInfo);
	}
#endif //!_WIN32_WCE
};

typedef CIconT<false>   CIconHandle;
typedef CIconT<true>    CIcon;


///////////////////////////////////////////////////////////////////////////////
// CCursor

// protect template member from a winuser.h macro
#ifdef CopyCursor
  #undef CopyCursor
#endif

template <bool t_bManaged>
class CCursorT
{
public:
	HCURSOR m_hCursor;

// Constructor/destructor/operators
	CCursorT(HCURSOR hCursor = NULL) : m_hCursor(hCursor)
	{ }

	~CCursorT()
	{
		if(t_bManaged && m_hCursor != NULL)
			DestroyCursor();
	}

	CCursorT<t_bManaged>& operator =(HCURSOR hCursor)
	{
		Attach(hCursor);
		return *this;
	}

	void Attach(HCURSOR hCursor)
	{
		if(t_bManaged && m_hCursor != NULL)
			DestroyCursor();
		m_hCursor = hCursor;
	}

	HCURSOR Detach()
	{
		HCURSOR hCursor = m_hCursor;
		m_hCursor = NULL;
		return hCursor;
	}

	operator HCURSOR() const { return m_hCursor; }

	bool IsNull() const { return m_hCursor == NULL; }

// Create/destroy methods
	HCURSOR LoadCursor(ATL::_U_STRINGorID cursor)
	{
		ATLASSERT(m_hCursor == NULL);
#if (_ATL_VER >= 0x0700)
		m_hCursor = ::LoadCursor(ATL::_AtlBaseModule.GetResourceInstance(), cursor.m_lpstr);
#else //!(_ATL_VER >= 0x0700)
		m_hCursor = ::LoadCursor(ATL::_pModule->GetResourceInstance(), cursor.m_lpstr);
#endif //!(_ATL_VER >= 0x0700)
		return m_hCursor;
	}

	HCURSOR LoadOEMCursor(UINT nIDCursor)   // for IDC_ types
	{
		ATLASSERT(m_hCursor == NULL);
		m_hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(nIDCursor));
		return m_hCursor;
	}

	HCURSOR LoadCursor(ATL::_U_STRINGorID cursor, int cxDesired, int cyDesired, UINT fuLoad = 0)
	{
		ATLASSERT(m_hCursor == NULL);
#if (_ATL_VER >= 0x0700)
		m_hCursor = (HCURSOR) ::LoadImage(ATL::_AtlBaseModule.GetResourceInstance(), cursor.m_lpstr, IMAGE_CURSOR, cxDesired, cyDesired, fuLoad);
#else //!(_ATL_VER >= 0x0700)
		m_hCursor = (HCURSOR) ::LoadImage(ATL::_pModule->GetResourceInstance(), cursor.m_lpstr, IMAGE_CURSOR, cxDesired, cyDesired, fuLoad);
#endif //!(_ATL_VER >= 0x0700)
		return m_hCursor;
	}

#ifndef _WIN32_WCE
	HCURSOR LoadCursorFromFile(LPCTSTR pstrFilename)
	{
		ATLASSERT(m_hCursor == NULL);
		ATLASSERT(!::IsBadStringPtr(pstrFilename, (UINT_PTR)-1));
		m_hCursor = ::LoadCursorFromFile(pstrFilename);
		return m_hCursor;
	}
#endif //!_WIN32_WCE

#if !defined(_WIN32_WCE) || ((_WIN32_WCE >= 0x400) && !(defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)))
	HCURSOR CreateCursor(int xHotSpot, int yHotSpot, int nWidth, int nHeight, CONST VOID *pvANDPlane, CONST VOID *pvXORPlane)
	{
		ATLASSERT(m_hCursor == NULL);
#if (_ATL_VER >= 0x0700)
		m_hCursor = ::CreateCursor(ATL::_AtlBaseModule.GetResourceInstance(), xHotSpot, yHotSpot, nWidth, nHeight, pvANDPlane, pvXORPlane);
#else //!(_ATL_VER >= 0x0700)
		m_hCursor = ::CreateCursor(ATL::_pModule->GetResourceInstance(), xHotSpot, yHotSpot, nWidth, nHeight, pvANDPlane, pvXORPlane);
#endif //!(_ATL_VER >= 0x0700)
		return m_hCursor;
	}
#endif //!defined(_WIN32_WCE) || ((_WIN32_WCE >= 0x400) && !(defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)))

#ifndef _WIN32_WCE
	HCURSOR CreateCursorFromResource(PBYTE pBits, DWORD dwResSize, DWORD dwVersion = 0x00030000)
	{
		ATLASSERT(m_hCursor == NULL);
		ATLASSERT(pBits != NULL);
		m_hCursor = (HCURSOR)::CreateIconFromResource(pBits, dwResSize, FALSE, dwVersion);
		return m_hCursor;
	}

	HCURSOR CreateCursorFromResourceEx(PBYTE pbBits, DWORD cbBits, DWORD dwVersion = 0x00030000, int cxDesired = 0, int cyDesired = 0, UINT uFlags = LR_DEFAULTCOLOR)
	{
		ATLASSERT(m_hCursor == NULL);
		ATLASSERT(pbBits != NULL);
		ATLASSERT(cbBits > 0);
		m_hCursor = (HCURSOR)::CreateIconFromResourceEx(pbBits, cbBits, FALSE, dwVersion, cxDesired, cyDesired, uFlags);
		return m_hCursor;
	}
#endif //!_WIN32_WCE

	BOOL DestroyCursor()
	{
		ATLASSERT(m_hCursor != NULL);
#if !defined(_WIN32_WCE) || ((_WIN32_WCE >= 0x400) && !(defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)))
		BOOL bRet = ::DestroyCursor(m_hCursor);
		if(bRet != FALSE)
			m_hCursor = NULL;
		return bRet;
#else //!(!defined(_WIN32_WCE) || ((_WIN32_WCE >= 0x400) && !(defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP))))
		ATLTRACE2(atlTraceUI, 0, _T("Warning: This version of Windows CE does not have ::DestroyCursor()\n"));
		return FALSE;
#endif //!(!defined(_WIN32_WCE) || ((_WIN32_WCE >= 0x400) && !(defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP))))
	}

// Operations
#ifndef _WIN32_WCE
	HCURSOR CopyCursor()
	{
		ATLASSERT(m_hCursor != NULL);
		return (HCURSOR)::CopyIcon((HICON)m_hCursor);
	}
#endif //!_WIN32_WCE

#if (WINVER >= 0x0500) && !defined(_WIN32_WCE)
	BOOL GetCursorInfo(LPCURSORINFO pCursorInfo)
	{
		ATLASSERT(m_hCursor != NULL);
		ATLASSERT(pCursorInfo != NULL);
		return ::GetCursorInfo(pCursorInfo);
	}
#endif //(WINVER >= 0x0500) && !defined(_WIN32_WCE)
};

typedef CCursorT<false>   CCursorHandle;
typedef CCursorT<true>    CCursor;


///////////////////////////////////////////////////////////////////////////////
// CResource - Wraps a generic Windows resource.
//             Use it with custom resource types other than the
//             standard RT_CURSOR, RT_BITMAP, etc.

class CResource
{
public:
	HGLOBAL m_hGlobal;
	HRSRC m_hResource;

// Constructor/destructor
	CResource() : m_hGlobal(NULL), m_hResource(NULL)
	{ }

	~CResource()
	{
		Release();
	}

// Load methods
	bool Load(ATL::_U_STRINGorID Type, ATL::_U_STRINGorID ID)
	{
		ATLASSERT(m_hResource == NULL);
		ATLASSERT(m_hGlobal == NULL);

#if (_ATL_VER >= 0x0700)
		m_hResource = ::FindResource(ATL::_AtlBaseModule.GetResourceInstance(), ID.m_lpstr, Type.m_lpstr);
#else //!(_ATL_VER >= 0x0700)
		m_hResource = ::FindResource(ATL::_pModule->GetResourceInstance(), ID.m_lpstr, Type.m_lpstr);
#endif //!(_ATL_VER >= 0x0700)
		if(m_hResource == NULL)
			return false;

#if (_ATL_VER >= 0x0700)
		m_hGlobal = ::LoadResource(ATL::_AtlBaseModule.GetResourceInstance(), m_hResource);
#else //!(_ATL_VER >= 0x0700)
		m_hGlobal = ::LoadResource(ATL::_pModule->GetResourceInstance(), m_hResource);
#endif //!(_ATL_VER >= 0x0700)
		if(m_hGlobal == NULL)
		{
			m_hResource = NULL;
			return false;
		}

		return true;
	}

#ifndef _WIN32_WCE
	bool LoadEx(ATL::_U_STRINGorID Type, ATL::_U_STRINGorID ID, WORD wLanguage)
	{
		ATLASSERT(m_hResource == NULL);
		ATLASSERT(m_hGlobal == NULL);

#if (_ATL_VER >= 0x0700)
		m_hResource = ::FindResourceEx(ATL::_AtlBaseModule.GetResourceInstance(), ID.m_lpstr, Type.m_lpstr, wLanguage);
#else //!(_ATL_VER >= 0x0700)
		m_hResource = ::FindResourceEx(ATL::_pModule->GetResourceInstance(), ID.m_lpstr, Type.m_lpstr, wLanguage);
#endif //!(_ATL_VER >= 0x0700)
		if(m_hResource == NULL)
			return false;

#if (_ATL_VER >= 0x0700)
		m_hGlobal = ::LoadResource(ATL::_AtlBaseModule.GetResourceInstance(), m_hResource);
#else //!(_ATL_VER >= 0x0700)
		m_hGlobal = ::LoadResource(ATL::_pModule->GetResourceInstance(), m_hResource);
#endif //!(_ATL_VER >= 0x0700)
		if(m_hGlobal == NULL)
		{
			m_hResource = NULL;
			return false;
		}

		return true;
	}
#endif //!_WIN32_WCE

// Misc. operations
	DWORD GetSize() const
	{
		ATLASSERT(m_hResource != NULL);
#if (_ATL_VER >= 0x0700)
		return ::SizeofResource(ATL::_AtlBaseModule.GetResourceInstance(), m_hResource);
#else //!(_ATL_VER >= 0x0700)
		return ::SizeofResource(ATL::_pModule->GetResourceInstance(), m_hResource);
#endif //!(_ATL_VER >= 0x0700)
	}

	LPVOID Lock()
	{
		ATLASSERT(m_hResource != NULL);
		ATLASSERT(m_hGlobal != NULL);
		LPVOID pVoid = ::LockResource(m_hGlobal);
		ATLASSERT(pVoid != NULL);
		return pVoid;
	}

	void Release()
	{
		if(m_hGlobal != NULL)
		{
			FreeResource(m_hGlobal);
			m_hGlobal = NULL;
			m_hResource = NULL;
		}
	}
};

}; //namespace WTL

#endif // __ATLUSER_H__
