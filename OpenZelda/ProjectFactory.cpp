/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germ�n M�ndez Bravo)
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
/*! \file		ProjectFactory.cpp
	\brief		Implementation of the CProjectFactory class.
	\date		June 26, 2003
*/

#include "stdafx.h"
#include "ProjectFactory.h"

#include "Console.h"

const WORD IGame::Version = 0x0300;
CProjectFactory *CProjectFactory::_instance = NULL;

CProjectFactory::CProjectFactory() :
	m_pGameI(NULL),
	m_hDLL(NULL),
	m_hWnd(NULL),
	m_iStep(0), m_iCnt1(0), m_iCnt2(0)
{
}

CProjectFactory::~CProjectFactory()
{
}
IGame* CProjectFactory::Interface(HWND hWnd)
{ 
	return Instance(hWnd)->m_pGameI; 
}

CProjectFactory* CProjectFactory::Instance(HWND hWnd)
{
	if(_instance == NULL) {
		_instance = new CProjectFactory;
	}
	if(hWnd) _instance->m_hWnd = hWnd;
	if(_instance->m_pGameI == NULL) {
		if(SUCCEEDED(New(&(_instance->m_pGameI), "Kernel.dll"))) {
			_instance->m_pGameI->SetScriptCallback(CProjectFactory::StatusChanged, (LPARAM)_instance);
			_instance->m_pGameI->SetSpriteCallback(CProjectFactory::StatusChanged, (LPARAM)_instance);
			_instance->m_pGameI->SetSpriteSheetCallback(CProjectFactory::StatusChanged, (LPARAM)_instance);
		} else {
			MessageBox(NULL, 
				"Couldn't load kernel, check kernel version.", 
				"Open Zelda - Fatal Error", MB_OK | MB_ICONERROR);
		}
	}
	return _instance;
}

HRESULT CProjectFactory::Delete(IGame **pGameI) 
{
	ASSERT(pGameI);
	if(*pGameI == NULL) return S_OK;
	if(_instance == NULL) return E_FAIL;
	if(_instance->m_hDLL == NULL) return E_FAIL;

	RELEASEGAMEINTERFACE ReleaseGameInterface = 
		(RELEASEGAMEINTERFACE)GetProcAddress(_instance->m_hDLL, "ReleaseGameInterface");
	if(ReleaseGameInterface == NULL) return E_FAIL;
	if(FAILED(ReleaseGameInterface(pGameI))) return E_FAIL;
	*pGameI = NULL;

	FreeLibrary(_instance->m_hDLL);
	_instance->m_hDLL = NULL;
	_instance->sIName = "";

	return S_OK;
}

HRESULT CProjectFactory::New(IGame **pGameI, LPCSTR lpszIName)
{
	if(_instance == NULL) {
		_instance = new CProjectFactory;
	}
	ASSERT(pGameI);
	if(lpszIName && _instance->sIName != "") {
		if(_instance->sIName != lpszIName) return E_FAIL; // There can be only one Game Kernel loaded
	}

	if(_instance->m_hDLL == NULL) {
		_instance->sIName = lpszIName;
		_instance->m_hDLL = LoadLibraryEx(_instance->sIName, NULL, 0);
		if(_instance->m_hDLL == NULL) {
			CBString sMsg = "Error loading up " + _instance->sIName;
			::MessageBox(NULL,
				sMsg,
				"Open Zelda - Fatal Error",
				MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}

	QUERYGAMEINTERFACE QueryGameInterface = 
		(QUERYGAMEINTERFACE)GetProcAddress(_instance->m_hDLL, "QueryGameInterface");
	if(QueryGameInterface == NULL) return E_FAIL;
	if(FAILED(QueryGameInterface(IGame::Version, pGameI, CConsole::Instance()))) return E_FAIL;

	return S_OK;
}

HRESULT CProjectFactory::New(CSpriteSelection **pSpriteSelectionI, CDrawableContext **ppDrawableContext_)
{
	ASSERT(pSpriteSelectionI);
	if(_instance == NULL) return E_FAIL;
	if(_instance->m_pGameI == NULL) return E_FAIL; // Main interface must be already initialized.

	*pSpriteSelectionI = _instance->m_pGameI->CreateSpriteSelection(ppDrawableContext_);
	if(*pSpriteSelectionI == NULL) return E_FAIL;

	return S_OK;
}

HRESULT CProjectFactory::Delete(CSpriteSelection **pSpriteSelectionI)
{
	ASSERT(pSpriteSelectionI);
	if(*pSpriteSelectionI == NULL) return S_OK;

	_instance->m_pGameI->DeleteSpriteSelection(*pSpriteSelectionI);
	*pSpriteSelectionI = NULL;

	return S_OK;
}

int CALLBACK CProjectFactory::StatusChanged(GameInfo *NewStatus, LPARAM lParam)
{
	return 0;
}
