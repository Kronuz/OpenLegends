/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
   Copyright (C) 2001-2003. Open Zelda's Project

   Based on the drag and drop code by Leon Finker (Jan/2001)
 
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

#ifndef __WINDOWDROPTARGET_H__
#define __WINDOWDROPTARGET_H__

#include "DragDropImpl.h"

#define WDT_DROPOBJ WMQD_DROPOBJ

class CWindowDropTarget : public CIDropTarget
{
public:
	CWindowDropTarget(HWND hTargetWnd) : CIDropTarget(hTargetWnd) {}

	virtual bool DropAt(FORMATETC* pFmtEtc, STGMEDIUM& medium, LPARAM lParam)
	{
		if(pFmtEtc->cfFormat == CF_TEXT && medium.tymed == TYMED_ISTREAM) {
			if(medium.pstm != NULL) {
				// this is a MESS and a better code is REQUIRED!
				// unfortunately you can't rely on STAT to be supported by all streams
				// so you can't get the size in advance
				//maybe it's better to use GetHGlobalFromStream
				const int BUF_SIZE = 10000;
				TCHAR buff[BUF_SIZE+1];
				ULONG cbRead=0;
				HRESULT hr = medium.pstm->Read(buff, BUF_SIZE, &cbRead);
				if( SUCCEEDED(hr) && cbRead > 0 && cbRead < BUF_SIZE) {
					buff[cbRead]=0;
					::SendMessage(m_hTargetWnd, WDT_DROPOBJ, (WPARAM)buff, lParam);
				} else {
					while((hr==S_OK && cbRead >0) && SUCCEEDED(hr)) {
						buff[cbRead]=0;
						::SendMessage(m_hTargetWnd, WDT_DROPOBJ, (WPARAM)buff, lParam);
						cbRead=0;
						hr = medium.pstm->Read(buff, BUF_SIZE, &cbRead);
					}
				}
			}
		}
		if(pFmtEtc->cfFormat == CF_TEXT && medium.tymed == TYMED_HGLOBAL)
		{
			CHAR* pStr = (CHAR*)GlobalLock(medium.hGlobal);
			if(pStr != NULL) {
				::SendMessage(m_hTargetWnd, WDT_DROPOBJ, (WPARAM)pStr, lParam);
			}
			GlobalUnlock(medium.hGlobal);
		}
		if(pFmtEtc->cfFormat == CF_HDROP && medium.tymed == TYMED_HGLOBAL)
		{
			HDROP hDrop = (HDROP)GlobalLock(medium.hGlobal);
			if(hDrop != NULL) {
				TCHAR szFileName[MAX_PATH];

				UINT cFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); 
				for(UINT i = 0; i < cFiles; ++i) {
				DragQueryFile(hDrop, i, szFileName, sizeof(szFileName)); 
				::SendMessage(m_hTargetWnd, WDT_DROPOBJ, (WPARAM)szFileName, lParam);
				}  
				//DragFinish(hDrop); // base class calls ReleaseStgMedium
			}
			GlobalUnlock(medium.hGlobal);
		}
		return true; //let base free the medium

	}

	virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
	{
		return DropAt(pFmtEtc, medium, 0);
	}

};
#endif //__DROPTARGET_H__