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
// OutputBox.cpp : implementation of the COutputBox class
//

#include "stdafx.h"

#include "OutputBox.h"

LRESULT COutputBox::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
  PARAFORMAT2 pf;
  pf.cbSize = sizeof(pf);
  pf.dwMask = PFM_ALL2;
  GetParaFormat(pf);
  pf.dwMask = PFM_OFFSET;
  pf.dxOffset = 200;
  SetParaFormat(pf);

  SetAutoURLDetect(TRUE);
  SetEventMask(ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_SELCHANGE | ENM_LINK | ENM_SCROLL | ENM_SCROLLEVENTS);
  //SetBackgroundColor(0xE9F2F7);
  bHandled = FALSE;

  return 0;
}
BOOL COutputBox::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}
LRESULT COutputBox::OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CMenu menu;
	if(!menu.CreatePopupMenu())
		return 0;
	menu.AppendMenu(MF_STRING, 1, "Clear");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, 2, "Copy");
	int nCmd = menu.TrackPopupMenu(TPM_RETURNCMD,LOWORD(lParam),HIWORD(lParam),m_hWnd);
	if(nCmd == 1) {
		SetRedraw(FALSE);
		SetSel(0,-1);
		ReplaceSel("");
		SetRedraw(TRUE);
		RedrawWindow();
	} else if(nCmd == 2) {
		long nStartChar, nEndChar;
		SetRedraw(FALSE);
		GetSel(nStartChar, nEndChar);
		if(nStartChar == nEndChar) SetSel(0,-1);
		Copy();
		SetSel(nStartChar, nEndChar);
		SetRedraw(TRUE);
		RedrawWindow();
	}

	return 0;
}

// we need to scroll to end.
LRESULT COutputBox::OnRequestScrollToEnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	return 1;
}

void COutputBox::WriteMsg(LPCTSTR lpszNewText, CHARFORMAT2 *pcFmt)
{
	SetRedraw(FALSE);
	SetSel(-1,-1);
	SetSelectionCharFormat(*pcFmt);  
	ReplaceSel(lpszNewText);
	SendMessage(WM_VSCROLL, SB_BOTTOM, NULL);
	SetSel(-1,-1); // deselect and move to end
	SetRedraw(TRUE);
	RedrawWindow();
}

void COutputBox::Empty()
{
	SetRedraw(FALSE);
	SetSel(0,-1);
	ReplaceSel("");
	SetSel(-1,-1); // deselect and move to end
	SetRedraw(TRUE);
	RedrawWindow();
}
