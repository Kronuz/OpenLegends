/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003 Kronuz
   Copyright (C) 2001/2003 Open Zelda's Project
 
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
/*! \file		HtmlFrm.h 
	\brief		Interface of the CHtmlFrame class.
	\date		April 15, 2003
*/

#pragma once

#ifndef __WTL_TABBED_FRAME_H__
	#error HtmlFrame.h requires TabbedFrame.h to be included first
#endif

#include <ExDispid.h>

#include "ChildFrm.h"
#include "HtmlView.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMainFrame;

static _ATL_FUNC_INFO TitleChangeInfo = {CC_STDCALL, VT_EMPTY, 1, { VT_BSTR } };
static _ATL_FUNC_INFO NavigateComplete2Info = {CC_STDCALL, VT_EMPTY, 2, { VT_DISPATCH, (VT_BYREF | VT_VARIANT)}};
static _ATL_FUNC_INFO DocumentCompleteInfo = {CC_STDCALL, VT_EMPTY, 2, { VT_DISPATCH, (VT_BYREF | VT_VARIANT) } };

/////////////////////////////////////////////////////////////////////////////
// This class manages the HTML browser frame (for the help files)
class CHtmlFrame :
	public CChildFrame,
	public IDispEventSimpleImpl<0, CHtmlFrame, &DIID_DWebBrowserEvents2>
{
protected:
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MDICHILD)

	// The windows view (the child control)
	CHtmlView *m_pHtmlView;

	CComPtr<IUnknown> m_punkBrowser;

	// Construction/Destruction:
	CHtmlFrame(CMainFrame *pMainFrame);
	virtual void OnFinalMessage(HWND /*hWnd*/);

	void SetCommandBarCtrlForContextMenu(CTabbedMDICommandBarCtrl* pCmdBar);

	BEGIN_MSG_MAP(CHtmlFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)

		MESSAGE_HANDLER(UWM_MDICHILDSHOWTABCONTEXTMENU, OnShowTabContextMenu)

		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
		COMMAND_ID_HANDLER(ID_FILE_CLOSE, OnFileClose)

		CHAIN_MSG_MAP(CChildFrame)
		// Pass all unhandled WM_COMMAND messages to the client window or 'view'
		CHAIN_CLIENT_COMMANDS ()
		// Reflect all the WM_NOTIFY messages to the client window
		REFLECT_NOTIFICATIONS()

	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	// Return the window's child control
	CHtmlView* GetView() { return m_pHtmlView; }
	// Return the main frame
	CMainFrame* GetMainFrame() { return m_pMainFrame; }

	LRESULT OnShowTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	BEGIN_SINK_MAP(CHtmlFrame)
		SINK_ENTRY_INFO(0, DIID_DWebBrowserEvents2, DISPID_TITLECHANGE, OnTitleChange, &TitleChangeInfo)
		SINK_ENTRY_INFO(0, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, OnNavigateComplete2, &NavigateComplete2Info)
		SINK_ENTRY_INFO(0, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete, &DocumentCompleteInfo)
	END_SINK_MAP()

	void __stdcall OnTitleChange(BSTR sTitle);
	void __stdcall OnNavigateComplete2(IDispatch* pDisp, VARIANT* vtURL);
	void __stdcall OnDocumentComplete(IDispatch* pDisp, VARIANT* vtURL);
};
