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
/*! \file		ProjectFactory.cpp
	\brief		Implementation of the CProjectFactory class.
	\date		June 26, 2003
*/

#include "stdafx.h"
#include "ProjectFactory.h"

#include "FoldersTreeBox.h"

const WORD IGame::Version = 0x0401;
CProjectFactory *CProjectFactory::_instance = NULL;

CProjectFactory::CProjectFactory() :
	m_pGameI(NULL),
	m_hDLL(NULL),
	m_hWnd(NULL),
	m_bBuilding(false),
	m_iStep(-1), m_iCnt1(-1), m_iCnt2(-1)
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
	if(IsWindow(hWnd)) _instance->m_hWnd = hWnd;
	if(_instance->m_pGameI == NULL) {
		if(SUCCEEDED(New(&(_instance->m_pGameI), "Kernel.dll"))) {
			ASSERT(IsWindow(_instance->m_hWnd));
			_instance->m_pGameI->SetSoundCallback(CProjectFactory::StatusChanged, (LPARAM)_instance);
			_instance->m_pGameI->SetScriptCallback(CProjectFactory::StatusChanged, (LPARAM)_instance);
			_instance->m_pGameI->SetSpriteCallback(CProjectFactory::StatusChanged, (LPARAM)_instance);
			_instance->m_pGameI->SetSpriteSheetCallback(CProjectFactory::StatusChanged, (LPARAM)_instance);
		} else {
			MessageBox(NULL, 
				"Couldn't load kernel, check kernel version.", 
				"Quest Designer - Fatal Error", MB_OK | MB_ICONERROR);
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
			CString sMsg = "Error loading up " + _instance->sIName;
			::MessageBox(NULL,
				sMsg,
				"Quest Designer - Fatal Error",
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

/*!
	This function sets up the project manager and starts the process of building.
*/
LRESULT CProjectFactory::StartBuild()
{
	ASSERT(IsWindow(m_hWnd));
	if(!IsWindow(m_hWnd)) return FALSE;
	m_iStep = 0;
	m_iCnt1 = Interface()->CountScripts();
	if(m_iCnt1 == 0) m_iStep++;
	::SendMessage(m_hWnd, WMQD_BUILDBEGIN, 0, (LPARAM)(LPCSTR)Interface()->GetProjectName());
	m_bBuilding = true;
	::PostMessage(m_hWnd, WMQD_STEPEND, 0, 0); /**/
	return TRUE;
}

/*!
	\param wParam Reserved parameter.
	\param lParam Reserved parameter.
	\return returns the number of characters written, 
		not including the terminating null character, 
		or a negative value if an output error occurs
	\remarks Returns TRUE if the step was managed.

	The project files to be compiled are only the necessary files to the quest,
	unnecessary files will not be compiled nor linked inside the quest file.

	To accomplish this objective, we need to compile and link only the
	files referenced on the world, and not any other file. This gives us the
	job to walk over each referred file (sprites, sounds, etc.)  by the class 
	CWorldManager.
*/
LRESULT CProjectFactory::BuildNextStep(WPARAM wParam, LPARAM lParam)
{
	ASSERT(IsWindow(m_hWnd));

	char szScriptFile[MAX_PATH];
	char szCompiledFile[MAX_PATH];
	if(m_iStep == 0) {
		if(--m_iCnt1 == 0) 
			m_iStep++;
	}
	if(m_iStep == 0) {
		const IScript *pScript = Interface()->GetScript(m_iCnt1);
		ASSERT(pScript);
		if(pScript->NeedToCompile()) {
			CString sIncludeDir = g_sHomeDir + "compiler\\INCLUDE";
			SCompiler::Instance()->Compile(
				sIncludeDir, 
				pScript->GetScriptFilePath(szScriptFile, MAX_PATH), 
				pScript->GetCompiledFilePath(szCompiledFile, MAX_PATH) );
		} else ::PostMessage(m_hWnd, WMQD_STEPEND, 0, 0);
	} else {
		m_bBuilding = false;
		::PostMessage(m_hWnd, WMQD_BUILDEND, m_iStep, 0);
	}
	return TRUE;
}

int CALLBACK CProjectFactory::StatusChanged(GameInfo *NewStatus, LPARAM lParam)
{
	char szFilePath[MAX_PATH];
	CTreeInfo *pTreeInfo = NULL;
	CProjectFactory *_this = reinterpret_cast<CProjectFactory*>(lParam);
	ASSERT(_this->m_hWnd);
	if(IsWindow(_this->m_hWnd)) {
		switch(NewStatus->eInfoType) {
			case itSpriteSheet: {
				if(NewStatus->SpriteSheet.eInfoReason == irAdded) {
					NewStatus->SpriteSheet.pInterface->GetFilePath(szFilePath, MAX_PATH);
					pTreeInfo = new CTreeInfo('S', NewStatus->SpriteSheet.lpszString, szFilePath);
					SendMessage(_this->m_hWnd, 
						WMGP_ADDTREE, 
						ICO_SPTSHT, 
						(LPARAM)pTreeInfo
					);
					return 1;
				}
				if(NewStatus->SpriteSheet.eInfoReason == irDeleted) {
					SendMessage(_this->m_hWnd, 
						WMGP_DELTREE, 
						0, 
						(LPARAM)new CTreeInfo('*', NewStatus->SpriteSheet.lpszString, NULL)
					);
					return 1;
				}
				break;
			}
			case itSprite:
			case itMask:
			case itEntity:
			case itBackground: {
				if(NewStatus->Sprite.eInfoReason == irAdded) {
					NewStatus->Sprite.pInterface->GetSpriteSheet()->GetFilePath(szFilePath, MAX_PATH);
					pTreeInfo = new CTreeInfo('S', NewStatus->Sprite.lpszString, szFilePath);
					pTreeInfo->m_eType = titFile;
					SendMessage(_this->m_hWnd, 
						WMGP_ADDTREE, 
						ICO_SPRITE, 
						(LPARAM)pTreeInfo
					);
					return 1;
				}
				if(NewStatus->Sprite.eInfoReason == irDeleted) {
					SendMessage(_this->m_hWnd, 
						WMGP_DELTREE, 
						0, 
						(LPARAM)new CTreeInfo('*', NewStatus->Sprite.lpszString, NULL)
					);
					return 1;
				}
				break;
			}
			case itSound: {
				if(NewStatus->Sound.eInfoReason == irAdded) {
					NewStatus->Sound.pInterface->GetSoundFilePath(szFilePath, MAX_PATH);
					pTreeInfo = new CTreeInfo('N', NewStatus->Sound.lpszString, szFilePath);
					pTreeInfo->m_eType = titFile;
					SendMessage(_this->m_hWnd, 
						WMQ_ADDTREE, 
						ICO_WAV, 
						(LPARAM)pTreeInfo
					);
					return 1;
				}
				if(NewStatus->Sound.eInfoReason == irDeleted) {
					SendMessage(_this->m_hWnd, 
						WMQ_DELTREE, 
						0, 
						(LPARAM)new CTreeInfo('*', NewStatus->Sound.lpszString, NULL)
					);
					return 1;
				}
				break;
			}
			case itScript: {
				if(NewStatus->Script.eInfoReason == irAdded) {
					NewStatus->Script.pInterface->GetScriptFilePath(szFilePath, MAX_PATH);
					pTreeInfo = new CTreeInfo('E', NewStatus->Script.lpszString, szFilePath);
					pTreeInfo->m_eType = titFile;
					SendMessage(_this->m_hWnd, 
						WMGP_ADDTREE, 
						ICO_SCRIPT, 
						(LPARAM)pTreeInfo
					);
					return 1;
				}
				if(NewStatus->Script.eInfoReason == irDeleted) {
					SendMessage(_this->m_hWnd, 
						WMGP_DELTREE, 
						0, 
						(LPARAM)new CTreeInfo('*', NewStatus->Script.lpszString, NULL)
					);
					return 1;
				}
				break;
			}
		};
	}
	return 0;
}