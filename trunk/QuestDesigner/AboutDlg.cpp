/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germán Méndez Bravo)
   Copyright (C) 2001-2003. Open Zelda's Project
 
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

	LPSTR szLicense;
	FILE *arch = fopen("License.txt", "rb");
	if(arch) {
		szLicense = new char[20*1024];
		memset(szLicense, 0, 20*1024);
		fread(szLicense, 1, 20*1024-1, arch);
		fclose(arch);
	} else szLicense = _T("\
Open Zelda Project\r\n\
Copyright (C) 2001-2003\r\n\
\r\n\
This program is free software; you can redistribute it and/or modify it \
under the terms of the GNU General Public License as published by the Free \
Software Foundation; either version 2 of the License, or (at your option) \
any later version.\r\n\
\r\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY \
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more \
details.\r\n\
\r\n\
You should have received a copy of the GNU General Public License along with this \
program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, \
Suite 330, Boston, MA 02111-1307 USA");

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
	SetDlgItemText(IDC_THANKS_EDIT, _T("\
I want to thank all people who helped me and collaborated in one way or the other and helped OpenZelda to become what it is today:\r\n\r\n\
      + GD for his magnificent idea and original design,\r\n\
      + Fenris (a.k.a. Sk8erHacker) for testing and \r\n\
          excellent support,\r\n\
      + Hylian for his support and the Open Zelda \r\n\
          domain name,\r\n\
      + Lukex for maintaining the official website and \r\n\
          starting a very useful help file,\r\n\
      + Menne for his great Z3C editor and its nice \r\n\
          source code,\r\n\
      + LittleBuddy for his support as a consultant and \r\n\
          his scripting expertise,\r\n\
      + Strider for his support,\r\n\
      + Alias Jargon,\r\n\
      + Raichu19192,\r\n\
      + HocusPocus,\r\n\
      + GodGinrai,\r\n\
      + and finally the guys at GU that hold that great \r\n\
          website and their support.\r\n\r\n\
If you feel I'm missing sombody, please tell me so I can add her/him/you to the list. "));
	

	return TRUE;
}

LRESULT CAckDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
