/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		ProjectFactory.h 
	\brief		Interface of the CProjectFactory class.
	\date		June 26, 2003
*/

#pragma once

#include "../IGame.h"

/////////////////////////////////////////////////////////////////////////////
/*! \class		CProjectFactory
	\brief		CProjectFactory class.
	\author		Kronuz
	\version	1.0
	\date		June 26, 2003

	This class generates the interface to the core game engine and project
	management.
*/
class CProjectFactory
{
	static CProjectFactory *_instance;
	CBString sIName;
   	HMODULE m_hDLL;
	HWND m_hWnd;

	IGame *m_pGameI;

	// Current state of the object:
	int m_iStep, m_iCnt1, m_iCnt2;

public:
	CProjectFactory();
	~CProjectFactory();

	static HRESULT New(IGame **pGameI, LPCSTR lpszIName = NULL);
	static HRESULT Delete(IGame **pGameI);

	static HRESULT New(CSpriteSelection **pSpriteSelectionI, CDrawableContext **ppDrawableContext_);
	static HRESULT Delete(CSpriteSelection **pSpriteSelectionI);

	static CProjectFactory* Instance(HWND hWnd = NULL);
	static IGame* Interface(HWND hWnd = NULL);

	static int CALLBACK StatusChanged(GameInfo *NewStatus, LPARAM lParam);
};
