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
// HtmlFrm.cpp : implementation of the CHtmlFrame class
//

#include "stdafx.h"
#include "HtmlFrm.h"

CHtmlFrame::CHtmlFrame(CMainFrame *pMainFrame) :
	CChildFrame(pMainFrame, tHtmlView),
	m_pHtmlView(NULL)
{
}

void CHtmlFrame::OnFinalMessage(HWND /*hWnd*/)
{
	delete this;
}
LRESULT CHtmlFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	LRESULT lRet = DefWindowProc();
	bHandled = TRUE;

	m_pHtmlView = new CHtmlView(this);

	// create our view
	m_hWndClient = m_pHtmlView->Create(	m_hWnd, 
										rcDefault, 
										_T(""), 
										WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 
										WS_EX_CLIENTEDGE);
	if ( NULL == m_hWndClient ) {
		ATLTRACE ( _T ( "Error: failed to create child window\n" ) );
		return FALSE;
	}

	m_pHtmlView->get_Control(&m_punkBrowser);
	if(m_punkBrowser)
		DispEventAdvise(m_punkBrowser, &DIID_DWebBrowserEvents2);

	CChildFrame::Register(tHtmlView);

	return TRUE;
}
LRESULT CHtmlFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CChildFrame::Unregister();

	if(m_punkBrowser)
		DispEventUnadvise(m_punkBrowser, &DIID_DWebBrowserEvents2);
	bHandled = FALSE;
	return 0;
}
LRESULT CHtmlFrame::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMSG pMsg = (LPMSG)lParam;

	if(baseClass::PreTranslateMessage(pMsg))
		return TRUE;

	return m_pHtmlView->PreTranslateMessage(pMsg);
}
LRESULT CHtmlFrame::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// System settings or metrics have changed.  Propogate this message
	// to all the child windows so they can update themselves as appropriate.
	SendMessageToDescendants(uMsg, wParam, lParam, TRUE);
	
	return 0;
}
LRESULT CHtmlFrame::OnShowTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	POINT ptPopup = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

	// Build up the menu to show
	CMenu mnuContext;

	// Load from resource
	//mnuContext.LoadMenu(IDR_CONTEXT);

	// or build dynamically
	// (being sure to enable/disable menu items as appropriate,
	// and giving the appropriate IDs)
	if(mnuContext.CreatePopupMenu()) {
		int cchWindowText = GetWindowTextLength();
		CString sWindowText;
		GetWindowText(sWindowText.GetBuffer(cchWindowText+1), cchWindowText+1);
		sWindowText.ReleaseBuffer();

		CString sSave(_T("&Save '"));
		sSave += sWindowText;
		sSave += _T("'");

		mnuContext.AppendMenu((MF_ENABLED | MF_STRING), ID_FILE_SAVE, sSave);
		mnuContext.AppendMenu((MF_ENABLED | MF_STRING), ID_FILE_CLOSE, _T("&Close\tCtrl+F4"));
		mnuContext.AppendMenu(MF_SEPARATOR);

		if(m_pCmdBar != NULL) {
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
				PostMessage(WM_COMMAND, MAKEWPARAM(nSelection, 0));
		} else {
			mnuContext.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_VERTICAL,
				ptPopup.x, ptPopup.y, m_hWnd, NULL);
		}
	}

	return 0;
}
LRESULT CHtmlFrame::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MessageBox(_T("OnFileSave"));
	return 0;
}
LRESULT CHtmlFrame::OnFileClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L);
	return 0;
}

/////////////////////

void CALLBACK CHtmlFrame::OnTitleChange(BSTR sTitle)
{
	USES_CONVERSION;
	baseClass::SetTitle(OLE2CT(sTitle));
}

void CALLBACK CHtmlFrame::OnNavigateComplete2(IDispatch* pDisp, VARIANT* vtURL)
{
	HRESULT hr = S_OK;
	CComQIPtr<IWebBrowser2> pBrowser(pDisp);
	if(pBrowser) {
		//CComPtr<IHTMLDocument> pDocument;
		CComPtr<IDispatch> pDocument;
		hr = pBrowser->get_Document(&pDocument);

		CComQIPtr<IHTMLDocument2> pHtmlDocument(pDocument);
		if(pHtmlDocument) {
			CComPtr<IHTMLLocation> pHtmlLocation;
			pHtmlDocument->get_location(&pHtmlLocation);
			if(pHtmlLocation) {
				CComBSTR sHref;
				pHtmlLocation->get_href(&sHref);
				if(sHref.Length() > 0) {
					USES_CONVERSION;
					baseClass::SetTabToolTip(OLE2CT(sHref));
				}
			}
		}
	}
}

void CALLBACK CHtmlFrame::OnDocumentComplete(IDispatch* pDisp, VARIANT* vtURL)
{
	HRESULT hr = S_OK;
	CComQIPtr<IWebBrowser2> pBrowser(pDisp);
	if(pBrowser) {
		//CComPtr<IHTMLDocument> pDocument;
		CComPtr<IDispatch> pDocument;
		hr = pBrowser->get_Document(&pDocument);

		CComQIPtr<IPersistStreamInit> pPersistStreamInit(pDocument);
		if(pPersistStreamInit) {
			ULARGE_INTEGER cbSize = {0};
			pPersistStreamInit->GetSizeMax(&cbSize);

			HGLOBAL hGlobal = ::GlobalAlloc(GHND, (DWORD)cbSize.QuadPart);

			if(hGlobal != NULL) {
				CComPtr<IStream> pStream;
				hr = ::CreateStreamOnHGlobal(hGlobal, FALSE, &pStream);
				if(pStream) {
					hr = pPersistStreamInit->Save(pStream, TRUE);

					// TODO: Figure out the character encoding,
					//  and translate to LPTSTR
					LPVOID sText = ::GlobalLock(hGlobal);
					DWORD cbText = ::GlobalSize(hGlobal);

//					m_pHtmlSourceView->SetWindowText((LPTSTR)sText);

					::GlobalUnlock(hGlobal);
				}

				::GlobalFree(hGlobal);
			}
		}
	}
}
