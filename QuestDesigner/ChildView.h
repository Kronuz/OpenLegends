/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germán Méndez Bravo (Kronuz)
   Copyright (C) 2001-2003. Open Legends's Project
 
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
/////////////////////////////////////////////////////////////////////////////
/*! \file		ChildView.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the CChildView class.
	\date		April 26, 2003
*/

#pragma once

#include "Console.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CChildFrame;

class CChildView {
protected:
	// Pointer to parent frame
	CChildFrame *m_pParentFrame;
	// The view's file name
	CString m_sFilePath;
	// The view's title
	CString m_sTitle;

	// Construction/Destruction
	CChildView(CChildFrame *pParentFrame);
public:
	CChildFrame *GetParentFrame() { return m_pParentFrame; }
	CMainFrame* GetMainFrame() { if(!m_pParentFrame) return NULL; return m_pParentFrame->GetMainFrame(); }

	CString& GetFilePath() { return m_sFilePath; }
	CString& GetTitle() { return m_sTitle; }

	virtual BOOL PreTranslateMessage(MSG* pMsg) = 0;
	// Called to do idle processing by the parent frame:
	virtual BOOL OnIdle() = 0;
	virtual bool hasChanged() = 0;
	virtual HWND SetFocus() = 0;

	UINT GetProfileInt( LPCSTR pszSection, LPCSTR pszEntry, int nDefault ) const
		{ return RegGetProfileInt( COMPANY, APPNAME, pszSection, pszEntry, nDefault ); }
	LPCSTR GetProfileString( LPCSTR pszSection, LPCSTR pszEntry, LPSTR pszValue, LPCSTR pszDefault ) const
		{ return RegGetProfileString( COMPANY, APPNAME, pszSection, pszEntry, pszValue, pszDefault ); }
	BOOL GetProfileBinary( LPCSTR pszSection, LPCSTR pszEntry, BYTE** ppData, UINT* pBytes ) const
		{ return RegGetProfileBinary( COMPANY, APPNAME, pszSection, pszEntry, ppData, pBytes ); }
	BOOL WriteProfileInt( LPCSTR pszSection, LPCSTR pszEntry, int nValue ) const
		{ return RegWriteProfileInt( COMPANY, APPNAME, pszSection, pszEntry, nValue ); }
	BOOL WriteProfileString( LPCSTR pszSection, LPCSTR pszEntry, LPCSTR pszValue ) const
		{ return RegWriteProfileString( COMPANY, APPNAME, pszSection, pszEntry, pszValue ); }
	BOOL WriteProfileBinary( LPCSTR pszSection, LPCSTR pszEntry, LPBYTE pData, UINT nBytes ) const
		{ return RegWriteProfileBinary( COMPANY, APPNAME, pszSection, pszEntry, pData, nBytes ); }

};
