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
/*! \file		AboutDlg.cpp
	\brief		Implementation of the CAboutDlg class
	\date		April 15, 2003
*/

#include "stdafx.h"
#include "resource.h"

#include "AboutDlg.h"
LRESULT CAboutDlg::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
/*
	HGLOBAL hLogo = LoadResource(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDR_LOGO));
	LPVOID pLogo = LockResource(hLogo);
	DWORD dwSize = SizeofResource(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDR_LOGO));

	BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = 200;
    bmi.bmiHeader.biHeight      = 182; // top-down image
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 0;
    bmi.bmiHeader.biCompression = BI_PNG;
    bmi.bmiHeader.biSizeImage   = dwSize;

	HDC hdcCompatible = CreateCompatibleDC(memdc);
	SetDIBitsToDevice(hdcCompatible, 
		ulDstX, ulDstY,
		ulDstWidth, ulDstHeight,
		0, 0,
		0, ulJpgHeight,
		pLogo,
		&bmi,
		DIB_RGB_COLORS);

/*/
	HBITMAP hPicture = LoadBitmap(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_LOGO));

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

/**/
	return 0;

}
LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	// set-up the dialogs hyperlinks
	m_wndHyperLinkURL1.SubclassWindow ( GetDlgItem ( IDC_URL1_STATIC ) );
	m_wndHyperLinkURL1.SetHyperLink ( _T ("http://www.openzelda.com") );		

	m_wndHyperLinkEmail1.SubclassWindow ( GetDlgItem ( IDC_EMAIL1_STATIC ) );
	m_wndHyperLinkEmail1.SetHyperLink ( _T ("mailto:kronuz@users.sourceforge.net") );		

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

	return TRUE;
}

LRESULT CAckDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
