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
/*! \file		ProjectManager.cpp
	\brief		Implementation of the CProjectManager class.
	\date		April 16, 2003

	This file implements all the classes that manage the project,
	this includes the methods to write and read from the 
	configuration files (i.e. sprite sheet files).
*/

#include "stdafx.h"
#include "ProjectManager.h"

#include "ArchiveText.h"

#include "FoldersTreeView.h"

CProjectManager::CProjectManager() :
	m_iStep(0), m_iCnt1(0), m_iCnt2(0),
	m_World(NULL)
{
	m_ArchiveIn = new CProjectTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}

CProjectManager::~CProjectManager()
{
	Clean();
}

/*!
	This function sets up the project manager and starts the process of building.
*/
LRESULT CProjectManager::StartBuild()
{
	m_iStep = 0;
	m_iCnt1 = m_Scripts.GetSize();
	if(m_iCnt1 == 0) m_iStep++;
	SendMessage(m_shWnd, WMQD_BUILDBEGIN, 0, (LPARAM)(LPCSTR)m_sProjectName);
	SendMessage(m_shWnd, WMQD_STEPEND, 0, 0);
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
LRESULT CProjectManager::BuildNextStep(WPARAM wParam, LPARAM lParam)
{
	if(m_iStep == 0) {
		if(--m_iCnt1 == 0) 
			m_iStep++;
	}
	if(m_iStep == 0) {
		CScript *pScript = m_Scripts.GetValueAt(m_iCnt1);
		if(pScript->NeedsToCompile()) {
			CString sIncludeDir = g_sHomeDir + "compiler\\INCLUDE";
			SCompiler::Instance()->Compile(
				sIncludeDir, 
				pScript->GetScriptFile(), 
				pScript->GetAmxFile() );
		} else SendMessage(m_shWnd, WMQD_STEPEND, 0, 0);
	} else
		SendMessage(m_shWnd, WMQD_BUILDEND, 0, 0);
	return TRUE;
}

CSprite *CProjectManager::CreateSprite(_spt_type sptType, LPCSTR szName)
{
	CSprite *pSprite = NULL;
	switch(sptType) {
		case tBackground: {
			pSprite = new CBackground(szName);
			break;
		}
		case tEntity: {
			pSprite = new CEntity(szName);
			break;
		}
		case tMask: {
			pSprite = new CMaskMap(szName);
			break;
		}
	}
	ASSERT(pSprite);
	return pSprite;
}
CSprite *CProjectManager::FindSprite(LPCSTR szName)
{
	int idx = -1;
	CSprite *pSprite = NULL;

	for(int i=0; i<m_SpriteSheets.GetSize(); i++) {
		idx = m_SpriteSheets[i]->m_Sprites.FindKey(szName);
		if(idx!=-1) {
			pSprite = m_SpriteSheets[i]->m_Sprites.GetValueAt(idx);
			break;
		}
	}
	if(idx==-1) {
		idx = m_UndefSprites.FindKey(szName);
		if(idx!=-1) pSprite = m_UndefSprites.GetValueAt(idx);
	}
	return pSprite;
}
CSound *CProjectManager::FindSound(LPCSTR szName)
{
	CSound *pSound = NULL;

	int idx = m_Sounds.FindKey(szName);
	if(idx!=-1) pSound = m_Sounds.GetValueAt(idx);

	return pSound;
}
CScript *CProjectManager::FindScript(LPCSTR szName)
{
	CScript *pScript = NULL;

	int idx = m_Scripts.FindKey(szName);
	if(idx!=-1) pScript = m_Scripts.GetValueAt(idx);

	return pScript;
}

CScript *CProjectManager::MakeScript(LPCSTR szName)
{

	CScript *pScript = new CScript;
	pScript->m_fnScriptFile.SetFilePath(g_sHomeDir + "Entities\\" + szName + ".zes");
	if(!pScript->m_fnScriptFile.FileExists()) {
		CString sFileTitle = szName;
		sFileTitle.SetAt(sFileTitle.GetLength()-1, '#');
		pScript->m_fnScriptFile.SetFileTitle(sFileTitle);
		if(!pScript->m_fnScriptFile.FileExists()) {
			delete pScript;
			return NULL;
		}
	}

	int idx = m_Scripts.FindKey(pScript->m_fnScriptFile.GetFileTitle());
	if(idx == -1) {
		m_Scripts.Add(pScript->m_fnScriptFile.GetFileTitle(), pScript);

		SendMessage(m_shWnd, 
			WMQD_ADDTREE, 
			ICO_DOC1, 
			(LPARAM)new CTreeInfo(
				(LPCSTR)(m_sProjectName + "\\Entities\\" + pScript->m_fnScriptFile.GetFileTitle()), 
				(DWORD_PTR)pScript 
			)
		);
	} else {
		delete pScript;
		pScript = m_Scripts.GetValueAt(idx);
	}

	return pScript;
}

/*!
	This function returns NULL if there is a conflict of types.
	(i.e. two sprites with the same name but different types)
*/
CSprite *CProjectManager::ReferSprite(LPCSTR szName, _spt_type sptType)
{
	CSprite *pSprite = FindSprite(szName);
	if(pSprite) {
		if(pSprite->GetSpriteType() != sptType) 
			return NULL;
	} else {
		pSprite = CreateSprite(sptType, szName);
		m_UndefSprites.Add(szName, pSprite);
	}

	return pSprite;
}
CSprite *CProjectManager::MakeSprite(LPCSTR szName, _spt_type sptType, CSpriteSheet *pSpriteSheet)
{
	ASSERT(pSpriteSheet);

	CSprite *pSprite = NULL;
	int idx = m_UndefSprites.FindKey(szName);
	if(idx!=-1) {
		pSprite = m_UndefSprites.GetValueAt(idx);
		if(pSprite->GetSpriteType() != sptType) return NULL;
		m_UndefSprites.RemoveAt(idx);
	} else {
		pSprite = CreateSprite(sptType, szName);
	}

	pSpriteSheet->m_Sprites.Add(szName, pSprite);
	pSprite->SetSpriteSheet(pSpriteSheet);
	// We send a message letting know that a new sprite has just been defined
	SendMessage(m_shWnd, 
		WMQD_ADDTREE, 
		ICO_PICTURE, 
		(LPARAM)new CTreeInfo(
			(LPCSTR)(m_sProjectName + "\\Sprite sheets\\" + pSpriteSheet->GetName() + (LPCSTR)((sptType==tBackground)?"\\Backgrounds\\":(sptType==tEntity)?"\\Entities\\":"\\Mask maps\\") + szName),
			(DWORD_PTR)pSprite
		)
	);

	return pSprite;
}

int CALLBACK CProjectManager::LoadSheet(LPCTSTR szFile, LPARAM lParam)
{
	CProjectManager *pProjectManager = (CProjectManager*)lParam;
	CSpriteSheet *sstmp = new CSpriteSheet(pProjectManager);
	pProjectManager->m_SpriteSheets.Add(sstmp);
	sstmp->Load(szFile);

	return 1;
}

void CProjectManager::DeleteScript(int nIndex)
{
	CScript *pScript = m_Scripts.GetValueAt(nIndex);
	ASSERT(pScript);
	SendMessage(m_shWnd, 
		WMQD_DELTREE, 
		NULL, 
		(LPARAM)new CTreeInfo(		
			(LPCSTR)(m_sProjectName + "\\Entities\\" + pScript->m_fnScriptFile.GetFileTitle()),
			NULL
		)
	);
	delete pScript;
	m_Scripts.RemoveAt(nIndex);
}
void CProjectManager::DeleteSpriteSheet(int nIndex)
{
	CSpriteSheet *pSpriteSheet = m_SpriteSheets[nIndex];
	ASSERT(pSpriteSheet);
	SendMessage(m_shWnd, 
		WMQD_DELTREE, 
		0, 
		(LPARAM)new CTreeInfo(		
			(LPCSTR)(m_sProjectName + "\\Sprite sheets\\" + pSpriteSheet->GetName()),
			NULL
		)
	);
	delete pSpriteSheet;
	m_SpriteSheets.RemoveAt(nIndex);
}
void CProjectManager::DeleteSprite(LPCSTR szName)
{
	CSprite *pSprite = FindSprite(szName);
	if(pSprite) {
		SendMessage(m_shWnd, 
			WMQD_DELTREE, 
			0, 
			(LPARAM)new CTreeInfo(
			(LPCSTR)(m_sProjectName + "\\Sprite sheets\\" + pSprite->GetSpriteSheet()->GetName() + (LPCSTR)((pSprite->GetSpriteType()==tBackground)?"\\Backgrounds\\":(pSprite->GetSpriteType()==tEntity)?"\\Entities\\":"\\Mask maps\\") + szName),
				NULL
			)
		);
	}
}

void CProjectManager::Clean()
{
	int i;
	delete m_World;
	m_World = NULL;

	if(m_SpriteSheets.GetSize()) printf("Freeing Sprite Sheets...\n");
	while(m_SpriteSheets.GetSize()) {
		DeleteSpriteSheet(0);
	}

	// Now we delete all undefined sprites (referred, but never really created)
	if(m_UndefSprites.GetSize()) printf("Freeing Unreferred Sprites...\n");
	while(m_UndefSprites.GetSize()) {
		delete m_UndefSprites.GetValueAt(0);
		m_UndefSprites.RemoveAt(0);
	}

	// Delete all loaded scripts
	if(m_Scripts.GetSize()) printf("Freeing Scripts...\n");
	while(m_Scripts.GetSize()) {
		DeleteScript(0);
	}

	// Delete all loaded sounds
	if(m_Sounds.GetSize()) printf("Freeing Sounds...\n");
	while(m_Sounds.GetSize()) {
		delete m_Sounds.GetValueAt(0);
		m_Sounds.RemoveAt(0);
	}
}

bool CProjectManager::Load(LPCSTR szFile)
{

	m_sProjectName = szFile;
	g_sHomeDir = "C:\\qd\\Quest Designer 2.0.4\\";
	printf("Loading project: '%s' at %s...\n", szFile, g_sHomeDir);

	if(m_sProjectName == "") m_sProjectName = "Unnamed Project";
	ForEachFile(g_sHomeDir + "Sprite Sheets\\*.spt", CProjectManager::LoadSheet, (LPARAM)this, 0);

	for(int i=0; i<m_UndefSprites.GetSize(); i++) {
		CString str = m_UndefSprites.GetKeyAt(i);
		printf("Undefined sprite: '%s'\n", str);
	}

	printf("Done!\n");

	DeleteSprite("_itemheart");
	//Clean();

	return false;
}
