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

CProjectManager::~CProjectManager()
{
	for(int i=0; i<m_SpriteSheets.GetSize(); i++){
		delete m_SpriteSheets[i];
		m_SpriteSheets[i] = NULL;
	}
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

CSprite *CProjectManager::CreateSprite(_spt_type sptType, LPCSTR sName)
{
	CSprite *pSprite = NULL;
	switch(sptType) {
		case tBackground: {
			pSprite = new CBackground;
			break;
		}
		case tEntity: {
			pSprite = new CEntity;
			break;
		}
		case tMask: {
			pSprite = new CMaskMap;
			break;
		}
	}
	ATLASSERT(pSprite);
	pSprite->m_bDefined = false;
	pSprite->m_sName = sName;
	return pSprite;
}

CScript *CProjectManager::DefineScript(CString &sName)
{

	CScript *pScript = new CScript;
	pScript->m_fnScriptFile.SetFilePath(g_sHomeDir + "Entities\\" + sName + ".zes");
	if(!pScript->m_fnScriptFile.FileExists()) {
		CString sFileTitle = sName;
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
			(LPARAM)(LPCSTR)(m_sProjectName + "\\Entities\\" + 
				pScript->m_fnScriptFile.GetFileTitle()));
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
CSprite *CProjectManager::ReferSprite(CString &sName, _spt_type sptType)
{
	int idx = -1;
	CSprite *pSprite = NULL;

	for(int i=0; i<m_SpriteSheets.GetSize(); i++) {
		idx = m_SpriteSheets[i]->m_Sprites.FindKey(sName);
		if(idx!=-1) {
			pSprite = m_SpriteSheets[i]->m_Sprites.GetValueAt(idx);
			break;
		}
	}
	if(idx==-1) {
		idx = m_UndefSprites.FindKey(sName);
		if(idx!=-1) pSprite = m_UndefSprites.GetValueAt(idx);
	}
	if(pSprite) {
		if(pSprite->m_SptType != sptType) 
			return NULL;
	} else {
		pSprite = CreateSprite(sptType, sName);
		m_UndefSprites.Add(sName, pSprite);
	}

	return pSprite;
}
CSprite *CProjectManager::DefineSprite(CString &sName, _spt_type sptType, CSpriteSheet *pSpriteSheet)
{
	ATLASSERT(pSpriteSheet);

	int idx = -1;
	CSprite *pSprite = NULL;
/*
	for(int i=0; i<m_SpriteSheets.GetSize(); i++) {
		idx = m_SpriteSheets[i]->m_Sprites.FindKey(sName);
		if(idx!=-1) {
			pSprite = m_SpriteSheets[i]->m_Sprites.GetValueAt(idx);
			break;
		}
	}
	if(idx!=-1) {
		printf("Sprite '%s' already defined in '%s'\n", sName, pSprite->m_pSpriteSheet->m_sSheetName);
		printf("\t'%s' redefinition attempt in '%s'\n", sName, pSpriteSheet->m_sSheetName);
		return NULL;
	}
*/
	idx = m_UndefSprites.FindKey(sName);
	if(idx!=-1) {
		pSprite = m_UndefSprites.GetValueAt(idx);
		if(pSprite->m_SptType!=sptType) return NULL;
		m_UndefSprites.RemoveAt(idx);
	} else {
		pSprite = CreateSprite(sptType, sName);
	}

	pSpriteSheet->m_Sprites.Add(sName, pSprite);
	pSprite->m_pSpriteSheet = pSpriteSheet;
	pSprite->m_bDefined = true;
	// We send a message letting know that a new sprite has just been defined
	SendMessage(m_shWnd, 
		WMQD_ADDTREE, 
		ICO_PICTURE, 
		(LPARAM)(LPCSTR)(m_sProjectName + "\\Sprite sheets\\" + pSpriteSheet->m_sSheetName + (LPCSTR)((sptType==tBackground)?"\\Backgrounds\\":(sptType==tEntity)?"\\Entities\\":"\\Mask maps\\") + sName));

	return pSprite;
}

int CALLBACK CProjectManager::LoadSheet(LPCTSTR szFile, LPARAM lParam)
{
	CSpriteSheet *sstmp = new CSpriteSheet;
	CProjectManager *pProjectManager = (CProjectManager*)lParam;
	sstmp->m_pProjectManager = pProjectManager;
	pProjectManager->m_SpriteSheets.Add(sstmp);
	sstmp->Load(szFile);

	return 1;
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
	return false;
}
