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
// HtmlView.cpp : implementation of the CHtmlView class
//

#include "stdafx.h"

#include "MainFrm.h"
#include "HtmlView.h"
#include "HtmlFrm.h"

CHtmlView::CHtmlView(CHtmlFrame *pParentFrame) :
	CChildView(pParentFrame)
{
}

BOOL CHtmlView::PreTranslateMessage(MSG *pMsg)
{
	if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
		(pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
		return FALSE;

	// give HTML page a chance to translate this message
	return (BOOL)SendMessage(WM_FORWARDMSG, 0, (LPARAM)pMsg);
}
void CHtmlView::OnFinalMessage(HWND /*hWnd*/)
{
	delete this;
}

LRESULT CHtmlView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// "base::OnCreate"
	LRESULT nResult = DefWindowProc();

	//CAxWindow::QueryControl(&m_pBrowser);
	//if(m_pBrowser)
	//{
	//	DispEventAdvise(m_pBrowser, &DIID_DWebBrowserEvents2);
	//}

	bHandled = TRUE;
	return nResult;
}

LRESULT CHtmlView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	//if(m_pBrowser)
	//{
	//	DispEventUnadvise(m_pBrowser, &DIID_DWebBrowserEvents2);
	//}

	bHandled = FALSE;
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT CHtmlView::get_Control(IUnknown** ppControl)
{
	if(m_hWnd != NULL) {
		return CAxWindow::QueryControl(ppControl);
	}
	return NULL;
}

HRESULT CHtmlView::get_Browser(IWebBrowser2** ppBrowser)
{
	if(m_hWnd != NULL) {
		return CAxWindow::QueryControl(ppBrowser);
	}
	return NULL;
}
