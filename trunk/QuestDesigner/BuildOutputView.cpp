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
/*! \file		BuildOutputView.cpp 
	\brief		Implementation of the CBuildOutputView class.
	\date		April 15, 2003
*/

#include "stdafx.h"

#include "MainFrm.h"
#include "BuildOutputView.h"

BOOL CBuildOutputView::PreTranslateMessage(MSG* /*pMsg*/)
{
	return FALSE;
}


LRESULT CBuildOutputView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	COutputView::OnCreate(uMsg, wParam, lParam, bHandled);

	SetReadOnly();
	return 0;
}

LRESULT CBuildOutputView::OnWriteMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	char buff[2000];
	InfoStruct *pInfo = (InfoStruct *)lParam;
	if(pInfo->type==t_printf) {
		vsprintf(buff, pInfo->message, pInfo->argptr);
	} else if(pInfo->type==t_error) {
		static char *prefix[3]={ "Error", "Fatal", "Warning" };
		if(pInfo->number!=0) {
			char *pre;
			pre = prefix[pInfo->number/100];
			if(pInfo->firstline>=0)
				sprintf(buff, "%s(%d - %d) : %s [%03d]: ", pInfo->filename, pInfo->firstline, pInfo->lastline, pre, pInfo->number);
			else
				sprintf(buff, "%s(%d) : %s [%03d]: ", pInfo->filename, pInfo->lastline, pre, pInfo->number);
			WriteMsg(buff);
		} 
		vsprintf(buff, pInfo->message, pInfo->argptr);
	}
	WriteMsg(buff);
	return 0;
}

LRESULT CBuildOutputView::OnLButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	CString strLineText;
	int nLineIndex = LineFromChar(LineIndex(-1));
	int nLineLength = LineLength(LineIndex(nLineIndex));
	int nRead = GetLine(nLineIndex, strLineText.GetBuffer(nLineLength + 3), 
		nLineLength + 1);
	strLineText.ReleaseBuffer(nRead);
	strLineText.SpanExcluding(_T("\r\n"));

	// Don't do anything on an empty line:
	if(strLineText.GetLength() > 0)	{
		int show = strLineText.Find(_T(") : Warning"));
		if(show==-1) show = strLineText.Find(_T(") : Error"));

		if(show >= 0) {
			// The selected line is a warning or a error:
			long lSelStart	= LineIndex(nLineIndex);
			long lSelEnd	= lSelStart + strLineText.GetLength();
			SetSel(lSelStart, lSelEnd);
/////////////////////
			CString sFile = strLineText.Mid(0,show);
			int line = sFile.ReverseFind('(');
			CString sLine= sFile.Mid(line+1);
			m_pMainFrame->FileOpen(sFile.Mid(0,line),atol(sLine));

/////////////////////
		}
	}
	return 0;
}

LRESULT CBuildOutputView::BeginBuildMsg(WPARAM wParam, LPARAM lParam)
{
	char line[500];
	m_sProject = (LPCSTR)lParam;
	
	CHARFORMAT2 fmt;
	fmt.dwMask = CFM_SIZE | CFM_COLOR | CFM_FACE | CFM_FACE | CFM_BOLD;
	strcpy(fmt.szFaceName,"Courier New");
	fmt.dwEffects = 0;
	fmt.yHeight = 8*20;
	fmt.crTextColor = 0;
	fmt.cbSize = sizeof(CHARFORMAT2);
	
	Empty();

	sprintf(line, "------ Build started: Project: %s, QuestDesigner ------\n\nCompiling...\n", m_sProject);
	COutputView::WriteMsg(line, &fmt);
	m_Errors = 0;
	m_Warnings = 0;
	return TRUE;
}

LRESULT CBuildOutputView::EndBuildMsg(WPARAM wParam, LPARAM lParam)
{
	char line[500];

	CHARFORMAT2 fmt;
	fmt.dwMask = CFM_SIZE | CFM_COLOR | CFM_FACE | CFM_FACE | CFM_BOLD;
	strcpy(fmt.szFaceName,"Courier New");
	fmt.dwEffects = 0;
	fmt.yHeight = 8*20;
	fmt.crTextColor = 0;
	fmt.cbSize = sizeof(CHARFORMAT2);

	sprintf(line, "\
\n%s - %d error(s), %d warning(s)\n\n\
---------------------- Done ----------------------\n\n\
Build: %d succeeded, %d failed\n\n\n\
", m_sProject, m_Errors, m_Warnings, m_Errors?0:1, m_Errors?1:0);
	COutputView::WriteMsg(line, &fmt);
	return TRUE;
}

void CBuildOutputView::WriteMsg(LPCTSTR lpszNewText, CHARFORMAT2 *pcFmt)
{
	COLORREF color = 0x000000;
	if(strstr(lpszNewText, "Small compiler")) return;
	if(strstr(lpszNewText, " Error.")) return;
	if(strstr(lpszNewText, " Errors.")) return;
	if(strstr(lpszNewText, " Warning.")) return;
	if(strstr(lpszNewText, " Warnings.")) return;

	if(strstr(lpszNewText, "Done.")) return;
	if(strstr(lpszNewText, "Compilation aborted.")) return;

	if(strstr(lpszNewText, ") : Warning")) { color=0x005500; m_Warnings++; }
	if(strstr(lpszNewText, ") : Error")) { color=0x000055; m_Errors++; }
	if(strstr(lpszNewText, ") : Fatal")) { color=0x0000ff; m_Errors++; }

	CHARFORMAT2 fmt;
	if(!pcFmt) {
		fmt.dwMask = CFM_SIZE | CFM_COLOR | CFM_FACE | CFM_FACE | CFM_BOLD;
		strcpy(fmt.szFaceName,"Courier New");
		fmt.dwEffects = 0;
		fmt.yHeight = 8*20;
		fmt.crTextColor = color;
		fmt.cbSize = sizeof(CHARFORMAT2);
		pcFmt = &fmt;
	}

	COutputView::WriteMsg(lpszNewText, pcFmt);
}
