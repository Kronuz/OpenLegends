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
/*! \file		MapEditorView.h 
	\brief		Interface of the CMapEditorView class.
	\date		April 27, 2003
*/

#pragma once

#include "ChildView.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CMapEditorFrame;

class CMapEditorView : 
	public CChildView,
	public CScrollWindowImpl<CMapEditorView>
{
private:

	CSize m_MapSize;

public:
	// Construction/Destruction
	CMapEditorView(CMapEditorFrame *pParentFrame);

	DECLARE_WND_CLASS_EX(NULL, 0, -1)

	// Called to translate window messages before they are dispatched 
	BOOL PreTranslateMessage(MSG *pMsg);

	// Called to clean up after window is destroyed
	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CWorldEditView)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)

		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)

		CHAIN_MSG_MAP(CScrollWindowImpl<CMapEditorView>);
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void DoPaint(CDCHandle dc);

};