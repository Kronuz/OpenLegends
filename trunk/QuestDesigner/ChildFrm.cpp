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
// ChildFrm.cpp : implementation of the CChildFrame base class
//

#include "stdafx.h"

#include "MainFrm.h"
#include "ChildFrm.h"

CChildFrame::CChildFrame(CMainFrame *pMainFrame, _child_type ChildType) :
	m_pMainFrame(pMainFrame),
	m_ChildType(ChildType),
	m_pCmdBar(NULL)
{
}
void CChildFrame::SetCommandBarCtrlForContextMenu(CTabbedMDICommandBarCtrl* pCmdBar)
{
	m_pCmdBar = pCmdBar;
}
LRESULT CChildFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
	bHandled = FALSE;
	if(wParam == SIZE_MINIMIZED) ShowWindow(FALSE);
	return 0;
}
LRESULT CChildFrame::Register(_child_type ChildType)
{
	ATLASSERT(m_pMainFrame);

	m_pMainFrame->m_ChildList.Add(this);
	m_ChildType = ChildType;

	return TRUE;
}
LRESULT CChildFrame::Unregister()
{
	ATLASSERT(m_pMainFrame);
	m_pMainFrame->m_ChildList.Remove(this);
	return 0;
}

LRESULT CChildFrame::OnShowTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	POINT ptPopup = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

	// Build up the menu to show
	CMenu mnuContext;

	// or build dynamically
	// (being sure to enable/disable menu items as appropriate,
	// and giving the appropriate IDs)
	if(mnuContext.CreatePopupMenu())
	{
		int cchWindowText = this->GetWindowTextLength();
		CString sWindowText;
		this->GetWindowText(sWindowText.GetBuffer(cchWindowText+1), cchWindowText+1);
		sWindowText.ReleaseBuffer();

		CString sSave(_T("&Save '"));
		sSave += sWindowText;
		sSave += _T("'");

		mnuContext.AppendMenu((MF_ENABLED | MF_STRING), ID_FILE_SAVE, sSave);
		mnuContext.AppendMenu((MF_ENABLED | MF_STRING), ID_FILE_CLOSE, _T("&Close\tCtrl+F4"));
		mnuContext.AppendMenu(MF_SEPARATOR);
		//mnuContext.AppendMenu((MF_ENABLED | MF_STRING), ID_VIEW_SOURCE, _T("&View Source"));

		if(m_pCmdBar != NULL)
		{
			// NOTE: The CommandBarCtrl in our case is the mainframe's, so the commands
			//  would actually go to the main frame if we don't specify TPM_RETURNCMD.
			//  In the main frame's message map, if we don't specify
			//  CHAIN_MDI_CHILD_COMMANDS, we are not going to see those command
			//  messages. We have 2 choices here - either specify TPM_RETURNCMD,
			//  then send/post the message to our window, or don't specify
			//  TPM_RETURNCMD, and be sure to have CHAIN_MDI_CHILD_COMMANDS
			//  in the main frame's message map.

			//m_pCmdBar->TrackPopupMenu(mnuContext, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_VERTICAL,
			//	ptPopup.x, ptPopup.y);

			DWORD nSelection = m_pCmdBar->TrackPopupMenu(mnuContext, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_VERTICAL | TPM_RETURNCMD,
				ptPopup.x, ptPopup.y);
			if(nSelection != 0)
			{
				this->PostMessage(WM_COMMAND, MAKEWPARAM(nSelection, 0));
			}
		}
		else
		{
			mnuContext.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_VERTICAL,
				ptPopup.x, ptPopup.y, m_hWnd, NULL);
		}
	}
	return 0;
}
