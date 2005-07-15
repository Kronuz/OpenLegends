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
/*! \file		AboutDlg.cpp
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the CAboutDlg class
	\date		April 15, 2003:
						* Creation date.
*/

#include "stdafx.h"
#include "resource.h"

#include "AboutDlg.h"
LRESULT CAboutDlg::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
/*	HBITMAP hPicture = LoadBitmap(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_LOGO));

	BITMAP bm;
	GetObject(hPicture, sizeof (BITMAP), (LPSTR)&bm);

	CMemDC memdc((HDC)wParam, NULL);
	HDC hdcCompatible = CreateCompatibleDC(memdc);
	HBITMAP hOldBMP = (HBITMAP)SelectObject(hdcCompatible, hPicture);
	RECT rcClient;
	GetClientRect(&rcClient);
	memdc.FillSolidRect(&rcClient, GetSysColor(COLOR_3DFACE));

	memdc.TransparentBlt(7, 15, bm.bmWidth, bm.bmHeight, hdcCompatible, 
						 0, 0, bm.bmWidth, bm.bmHeight, 0xff00ff);

	SelectObject(hdcCompatible, hOldBMP);
	DeleteDC(hdcCompatible);
	DeleteObject(hPicture);

/*/
	CImage Image;
	LoadImage(&Image, _Module.GetModuleInstance(), IDB_LOGO);

	CMemDC memdc((HDC)wParam, NULL);

	RECT rcClient;
	GetClientRect(&rcClient);
	memdc.FillRect(&rcClient, COLOR_3DFACE);

	Image.AlphaBlend(memdc, 7, 25, Image.GetWidth(), Image.GetHeight(), 0, 0, Image.GetWidth(), Image.GetHeight());

	return 0;

}
LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	SetWindowText("About " QD_NAME);
	GetDlgItem(IDC_STATIC1).SetWindowText(QD_FULLNAME "\nfor " OL_NAME);
	GetDlgItem(IDC_STATIC2).SetWindowText(OL_NAME " Website:");

	// set-up the dialogs hyperlinks
	m_wndHyperLinkURL1.SubclassWindow ( GetDlgItem ( IDC_URL1_STATIC ) );
	m_wndHyperLinkURL1.SetHyperLink ( OL_WEBSITE );

	m_wndHyperLinkEmail1.SubclassWindow ( GetDlgItem ( IDC_EMAIL1_STATIC ) );
	m_wndHyperLinkEmail1.SetHyperLink ( "mailto:" OL_EMAIL );

	m_wndHyperLinkSupport1.SubclassWindow ( GetDlgItem ( IDC_SUPPORT1_STATIC ) );
	m_wndHyperLinkSupport1.SetHyperLink ( OL_DONATION_URL );

	LPSTR szLicense;

	char szPath[MAX_PATH];
	::GetModuleFileNameA(_Module.GetModuleInstance(), szPath, MAX_PATH);
	char *aux = strrchr(szPath, '\\');
	if(aux) *(aux+1) = '\0';
	strcat(szPath, "License.txt");

	FILE *arch = fopen(szPath, "rb");
	if(arch) {
		szLicense = new char[20*1024];
		memset(szLicense, 0, 20*1024);
		fread(szLicense, 1, 20*1024-1, arch);
		fclose(arch);
	} else szLicense = OL_LICENSE;
	SetDlgItemText(IDC_GPL_EDIT, szLicense);
	if(arch) delete []szLicense;

	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
LRESULT CAboutDlg::OnAcknowlegments(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAckDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CAckDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	SetDlgItemText(IDC_ACK_EDIT, OL_ACK);
	SetDlgItemText(IDC_THANKS_EDIT, OL_THANKS);

	return TRUE;
}

LRESULT CAckDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
