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
// OutputView.cpp : implementation of the COutputView class
//

#include "stdafx.h"

#include "OutputView.h"

LRESULT COutputView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
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
BOOL COutputView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

void COutputView::WriteMsg(LPCTSTR lpszNewText, CHARFORMAT2 *pcFmt)
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

void COutputView::Empty()
{
	SetRedraw(FALSE);
	SetSel(0,-1);
	ReplaceSel("");
	SetSel(-1,-1); // deselect and move to end
	SetRedraw(TRUE);
	RedrawWindow();
}
