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
/*! \file		GraphicsFactory.cpp
	\brief		Implementation of the CGraphicsFactory class.
	\date		June 26, 2003
*/

#include "stdafx.h"
#include "GraphicsFactory.h"

#include "FoldersTreeBox.h"

#include "../Console.h"

const WORD IGraphics::Version = 0x0300;
CGraphicsFactory *CGraphicsFactory::_instance = NULL;

CGraphicsFactory::CGraphicsFactory() :
	m_hDLL(NULL)
{
}

CGraphicsFactory::~CGraphicsFactory()
{
}

HRESULT CGraphicsFactory::Delete(IGraphics **pGraphics) {
	ASSERT(pGraphics);
	if(*pGraphics == NULL) return S_OK;

	int cnt = (*pGraphics)->Finalize();

	RELEASEGFXINTERFACE ReleaseGfxInterface = 
		(RELEASEGFXINTERFACE)GetProcAddress(_instance->m_hDLL, "ReleaseGfxInterface");
	if(ReleaseGfxInterface == NULL) return E_FAIL;
	if(FAILED(ReleaseGfxInterface(pGraphics))) return E_FAIL;
	*pGraphics = NULL;


	if(cnt == 0) {
		// FreeLibrary(_instance->m_hDLL);
		_instance->m_hDLL = NULL;
	}

	return S_OK;
}

HRESULT CGraphicsFactory::New(IGraphics **pGraphics, LPCSTR lpszIName) {
	ASSERT(pGraphics);

	if(_instance == NULL) {
		_instance = new CGraphicsFactory;
	}

	if(lpszIName) {
		if(_instance->sIName != lpszIName) {
			// Before obtaining a new graphics device, all instances of
			// the current device must be properly finalized.
			Delete(pGraphics);
			if(_instance->m_hDLL != NULL) return E_FAIL; // There are still unfinalized instances.
		}
	}
	if(_instance->m_hDLL == NULL) {
		_instance->sIName = lpszIName;
		_instance->m_hDLL = LoadLibraryEx(_instance->sIName, NULL, 0);
		if(_instance->m_hDLL == NULL) {
			CString sMsg = "Error loading up " + _instance->sIName;
			::MessageBox(NULL,
				sMsg,
				"Quest Designer - Fatal Error",
				MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}

	QUERYGFXINTERFACE QueryGfxInterface = 
		(QUERYGFXINTERFACE)GetProcAddress(_instance->m_hDLL, "QueryGfxInterface");
	if(QueryGfxInterface == NULL) return E_FAIL;
	if(FAILED(QueryGfxInterface(IGraphics::Version, pGraphics, CConsole::Instance()))) return E_FAIL;

	return S_OK;
}
