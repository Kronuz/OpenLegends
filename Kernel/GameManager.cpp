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
/*! \file		GameManager.cpp
	\brief		Implementation of the CGameManager class.
	\date		April 16, 2003

	This file implements all the classes that manage the project,
	this includes the methods to write and read from the 
	configuration files (i.e. sprite sheet files).
*/

#include "stdafx.h"
#include "GameManager.h"

#include "ArchiveText.h"

#define SetCallback(funct, callback, param) { ##funct.Callback = callback; ##funct.lParam = param; }

#define CallbackProc(funct, obj, str, reason) \
	if(##funct.Callback) { \
		GameInfo Info; \
		CBString String = ##str; \
		Info.eInfoType = it##funct; \
		Info.##funct.eInfoReason = ir##reason; \
		Info.##funct.lpszString = (LPCSTR)String; \
		Info.##funct.pInterface = ##obj; \
		##funct.Callback(&Info, ##funct.lParam); \
	}

CGameManager *CGameManager::_instance = NULL;

CGameManager::CGameManager() :
	m_World("New Quest")
{
	m_ArchiveIn = new CProjectTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}

CGameManager::~CGameManager()
{
	Clean();
}

CSprite *CGameManager::CreateSprite(_spt_type sptType, LPCSTR szName)
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
	if(!pSprite) CONSOLE_OUTPUT("The sprite '%s' couldn't be created!\n", szName);
	ASSERT(pSprite);
	return pSprite;
}

CSprite *CGameManager::FindSprite(LPCSTR szName)
{
	std::vector<CSpriteSheet*>::iterator SheetIterator = m_SpriteSheets.begin();
	while(SheetIterator != m_SpriteSheets.end()) {
		if(*SheetIterator) {
			std::map<CBString, CSprite*>::iterator SpriteIterator;
			SpriteIterator = (*SheetIterator)->m_Sprites.find(szName);
			if(SpriteIterator != (*SheetIterator)->m_Sprites.end()) {
				return SpriteIterator->second;
			}

		}
		SheetIterator++;
	}

	std::map<CBString, SUndefSprite>::iterator SpriteIterator = m_UndefSprites.find(szName);
	if(SpriteIterator != m_UndefSprites.end()) {
		return (SpriteIterator->second).pSprite;
	}
	return NULL;
}

CSound *CGameManager::FindSound(LPCSTR szName)
{
	std::map<CBString, CSound*>::iterator Iterator = m_Sounds.find(szName);
	if(Iterator != m_Sounds.end()) return Iterator->second;
	return NULL;
}
CScript *CGameManager::FindScript(LPCSTR szName)
{
	std::map<CBString, CScript*>::iterator Iterator = m_Scripts.find(szName);
	if(Iterator != m_Scripts.end()) return Iterator->second;
	return NULL;
}

CScript *CGameManager::MakeScript(LPCSTR szName)
{
	// create a new script object and try to find its file
	CScript *pScript = new CScript;
	CBString sPath = CBString("Entities\\") + szName + ".zes"; // relative by default
	pScript->m_fnScriptFile.SetFilePath(sPath); 
	if(!pScript->m_fnScriptFile.FileExists()) {
		CBString sFileTitle = szName;
		sFileTitle.SetAt(sFileTitle.GetLength()-1, '#');
		pScript->m_fnScriptFile.SetFileTitle(sFileTitle);
		if(!pScript->m_fnScriptFile.FileExists()) {
			delete pScript;
			return NULL; // couldn't find the script file
		}
	}
	// Now we find the script file name in the map of scripts:
	std::map<CBString, CScript*>::iterator Iterator = m_Scripts.find(pScript->m_fnScriptFile.GetFileTitle());
	if(Iterator == m_Scripts.end()) { // there's no script file in the map (first time script):
		m_Scripts.insert(pairScript(pScript->m_fnScriptFile.GetFileTitle(), pScript)); //add it to the map and ...
		// ... make a call to the client, to notify the script creation.
		CallbackProc(
			Script, 
			pScript, 
			(LPCSTR)(m_sProjectName + "\\Entities\\" + pScript->m_fnScriptFile.GetFileTitle()), 
			Added);
	} else { // otherwise, the script already is in the map, so we don't need the newly created object.
		delete pScript; 
		pScript = Iterator->second;
	}

	return pScript; // return the located or created script object.
}

/*!
	This function returns NULL if there is a conflict of types.
	(i.e. two sprites with the same name but different types)
*/
CSprite *CGameManager::ReferSprite(LPCSTR szName, _spt_type sptType, LPCSTR szFile_, int nLine_)
{
	if(!*szName) return NULL;
	if(sptType == tMask) { if(!isalnum(*szName) && *szName!='_') return NULL; }
	else { if(!isalpha(*szName) && *szName!='_') return NULL; }

	CSprite *pSprite = FindSprite(szName);
	if(pSprite) {
		if(pSprite->GetSpriteType() != sptType) 
			return NULL;
	} else {
		SUndefSprite UndefSprite;
		pSprite = CreateSprite(sptType, szName);
		if(pSprite)	{
			UndefSprite.pSprite = pSprite;
			UndefSprite.nLine = nLine_;
			UndefSprite.sFile = szFile_;
			m_UndefSprites.insert(pairUndefSprite(szName, UndefSprite));
		}
	}

	return pSprite;
}

CSprite *CGameManager::MakeSprite(LPCSTR szName, _spt_type sptType, CSpriteSheet *pSpriteSheet)
{
	ASSERT(pSpriteSheet);

	CSprite *pSprite = NULL;

	// MakeSprite defines an undefined sprite. The sprite must be already in the referenced list.
	std::map<CBString, SUndefSprite>::iterator Iterator = m_UndefSprites.find(szName);
	if(Iterator != m_UndefSprites.end()) {
		pSprite = (Iterator->second).pSprite;
		if(pSprite->GetSpriteData() == NULL) return NULL;
		if(pSprite->GetSpriteType() != sptType) return NULL;
		m_UndefSprites.erase(Iterator);
	} 

	if(pSprite) {
		pSpriteSheet->m_Sprites.insert(CSpriteSheet::pairSprite(szName, pSprite));
		pSprite->SetSpriteSheet(pSpriteSheet);
		// We send a message letting know that a new sprite has just been defined

		CallbackProc(
			Sprite, 
			pSprite, 
			m_sProjectName + "\\Sprite sheets\\" + pSpriteSheet->GetName() + (LPCSTR)((sptType==tBackground)?"\\Backgrounds\\":(sptType==tEntity)?"\\Entities\\":"\\Mask maps\\") + szName,
			Added);
	}

	return pSprite;
}

int CALLBACK CGameManager::LoadSheet(LPCTSTR szFile, LPARAM lParam)
{
	CGameManager *pGameManager = reinterpret_cast<CGameManager*>(lParam);
	CSpriteSheet *sstmp = new CSpriteSheet(pGameManager);
	ASSERT(sstmp);
	pGameManager->m_SpriteSheets.push_back(sstmp);

	CVFile vfFile(szFile);
	sstmp->Load(vfFile);

	return 1;
}

void CGameManager::DeleteScript(int nIndex)
{
	std::map<CBString, CScript*>::iterator Iterator =  m_Scripts.begin();
	for(int idx=0; idx!=nIndex && Iterator != m_Scripts.end(); idx++, Iterator++);

	if(Iterator != m_Scripts.end()) {
		CScript *pScript = Iterator->second;

		CallbackProc(
			Script, 
			NULL, 
			m_sProjectName + "\\Entities\\" + pScript->m_fnScriptFile.GetFileTitle(),
			Deleted);

		delete pScript;
		m_Scripts.erase(Iterator);
	}
}
void CGameManager::DeleteSpriteSheet(int nIndex)
{
	std::vector<CSpriteSheet*>::iterator Iterator = m_SpriteSheets.begin();
	for(int idx=0; idx!=nIndex && Iterator != m_SpriteSheets.end(); idx++, Iterator++);

	if(Iterator != m_SpriteSheets.end()) {
		CSpriteSheet *pSpriteSheet = *Iterator;

		CallbackProc(
			SpriteSheet, 
			NULL, 
			m_sProjectName + "\\Sprite sheets\\" + pSpriteSheet->GetName(),
			Deleted);

		delete pSpriteSheet;
		m_SpriteSheets.erase(Iterator);
	}
}
void CGameManager::DeleteSprite(LPCSTR szName)
{ 
	CSprite *pSprite = FindSprite(szName);
	if(pSprite) {
		CallbackProc(
			Sprite, 
			NULL, 
			m_sProjectName + "\\Sprite sheets\\" + pSprite->GetSpriteSheet()->GetName() + (LPCSTR)((pSprite->GetSpriteType()==tBackground)?"\\Backgrounds\\":(pSprite->GetSpriteType()==tEntity)?"\\Entities\\":"\\Mask maps\\") + szName,
			Deleted);

		//FIXME this method still has to really delete the sprite from the sprite sheet
	}
}

void CGameManager::Clean()
{
	if(m_SpriteSheets.size()) CONSOLE_OUTPUT("Freeing Sprite Sheets...\n");
	while(m_SpriteSheets.size()) {
		DeleteSpriteSheet(0);
	}

	// Now we delete all undefined sprites (referred, but never really created)
	if(m_UndefSprites.size()) CONSOLE_OUTPUT("Freeing Unreferred Sprites...\n");
	while(m_UndefSprites.size()) {
		delete m_UndefSprites.begin()->second.pSprite;
		m_UndefSprites.erase(m_UndefSprites.begin());
	}

	// Delete all loaded scripts
	if(m_Scripts.size()) CONSOLE_OUTPUT("Freeing Scripts...\n");
	while(m_Scripts.size()) {
		DeleteScript(0);
	}

	// Delete all loaded sounds
	if(m_Sounds.size()) CONSOLE_OUTPUT("Freeing Sounds...\n");
	while(m_Sounds.size()) {
		delete m_Sounds.begin()->second;
		m_UndefSprites.erase(m_UndefSprites.begin());
	}
}
int CALLBACK RemoveMask(LPVOID sprite, LPARAM lParam)
{
	CSprite *pSprite = static_cast<CSprite*>(sprite);
	if(pSprite->GetSpriteType() == tEntity || pSprite->GetSpriteType()== tBackground) {
		CBackground *pBackground = static_cast<CBackground*>(sprite);
		SBackgroundData *pBackgroundData =
			static_cast<SBackgroundData*>(pBackground->GetSpriteData());
		if(pBackgroundData->pMaskMap == static_cast<CMaskMap*>((LPVOID)lParam)) {
			pBackgroundData->pMaskMap = NULL;
			return 1;
		}
	}
	return 0;
}
bool CGameManager::Load(CVFile &vfFile)
{

	DWORD dwInitTicks = GetTickCount();
	m_sProjectName = vfFile.GetFileDesc();
	g_sHomeDir = vfFile.GetPath();
	CONSOLE_OUTPUT("Loading project: '%s' at %s...\n", vfFile.GetFileName(), vfFile.GetPath());

	if(m_sProjectName == "") m_sProjectName = "Unnamed Project";
	CBString sPath = g_sHomeDir + "Sprite Sheets\\*.spt";
	ForEachFile(sPath, CGameManager::LoadSheet, (LPARAM)this, 0);

	if(m_UndefSprites.size()) {
		CONSOLE_OUTPUT("\nReferenced but undefined sprites:\n");
	}
	std::map<CBString, SUndefSprite>::iterator Iterator = m_UndefSprites.begin();
	while(Iterator != m_UndefSprites.end()) {
		SUndefSprite &UndefSprite = Iterator->second;
		// If the referenced, but not defined sprite is a Mask, we need to look for it on every object in every
		// sprite sheet and clear its mask reference if needed.
		if(UndefSprite.pSprite->GetSpriteType() == tMask) {
			if(ForEachSprite(RemoveMask, (LPARAM)UndefSprite.pSprite)) {
				CONSOLE_OUTPUT("  Error: Undefined mask: '%s', first referred at: '%s' (%d)\n", Iterator->first, (LPCSTR)UndefSprite.sFile, UndefSprite.nLine);
			} else {
				CONSOLE_OUTPUT("  Warning: Undefined mask: '%s', first referred at: '%s' (%d)\n", Iterator->first, (LPCSTR)UndefSprite.sFile, UndefSprite.nLine);
			}
		} else {
			CONSOLE_OUTPUT("  Warning: Undefined sprite: '%s', first referred at: '%s' (%d)\n", Iterator->first, (LPCSTR)UndefSprite.sFile, UndefSprite.nLine);
		}
		delete UndefSprite.pSprite;
		Iterator++;
	}
	m_UndefSprites.clear();

	CONSOLE_OUTPUT("Done! (%d milliseconds)\n", GetTickCount()-dwInitTicks);

	return true;
}
void CGameManager::DeleteSpriteSelection(CSpriteSelection *pSelection)
{
	delete pSelection;
}
CSpriteSelection* CGameManager::CreateSpriteSelection(CDrawableContext **ppDrawableContext_)
{
	return new CSpriteSelection(ppDrawableContext_);
}
void CGameManager::GetMapSize(CSize &mapSize) const
{
	mapSize = m_World.m_szMapSize;
}
void CGameManager::SetMapSize(const CSize &mapSize)
{
	m_World.m_szMapSize = mapSize;
}

void CGameManager::GetWorldSize(CSize &worldSize) const
{
	worldSize = m_World.m_szWorldSize;
}
void CGameManager::SetWorldSize(const CSize &worldSize)
{
	m_World.m_szWorldSize = worldSize;
}

CMapGroup* CGameManager::BuildMapGroup(int x, int y, int width, int height)
{
	return m_World.BuildMapGroup(x, y, width, height);
}
CMapGroup* CGameManager::FindMapGroup(int x, int y) const
{
	return m_World.FindMapGroup(x, y);
}
void CGameManager::SetSoundCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam)
{
	SetCallback(Sound, StatusChanged, lParam);
}
void CGameManager::SetSpriteCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam)
{
	SetCallback(Sprite, StatusChanged, lParam);
}
void CGameManager::SetScriptCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam)
{
	SetCallback(Script, StatusChanged, lParam);
}
void CGameManager::SetSpriteSheetCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam)
{
	SetCallback(SpriteSheet, StatusChanged, lParam);
}
void CGameManager::SetMapCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam)
{
	SetCallback(Map, StatusChanged, lParam);
}
void CGameManager::SetMapGroupCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam)
{
	SetCallback(MapGroup, StatusChanged, lParam);
}

int CGameManager::ForEachSound(FOREACHPROC ForEach, LPARAM lParam)
{
	int cnt = 0;
	std::map<CBString, CSound*>::iterator Iterator = m_Sounds.begin();
	while(Iterator != m_Sounds.end()) {
		ASSERT(Iterator->second);
		int aux = ForEach((LPVOID)(Iterator->second), lParam);
		if(aux < 0) return aux-cnt;
		cnt += aux;
		Iterator++;
	}
	return cnt;
}
int CGameManager::ForEachSprite(FOREACHPROC ForEach, LPARAM lParam)
{
	int cnt = 0;
	std::vector<CSpriteSheet*>::iterator Iterator = m_SpriteSheets.begin();
	while(Iterator != m_SpriteSheets.end()) {
		ASSERT(*Iterator);
		int aux = (*Iterator)->ForEachSprite(ForEach, lParam);
		if(aux < 0) return aux-cnt;
		cnt += aux;
		Iterator++;
	}
	return cnt;
}
int CGameManager::ForEachScript(FOREACHPROC ForEach, LPARAM lParam)
{
	int cnt = 0;
	std::map<CBString, CScript*>::iterator Iterator = m_Scripts.begin();
	while(Iterator != m_Scripts.end()) {
		ASSERT(Iterator->second);
		int aux = ForEach((LPVOID)(Iterator->second), lParam);
		if(aux < 0) return aux-cnt;
		cnt += aux;
		Iterator++;
	}
	return cnt;
}
int CGameManager::ForEachSpriteSheet(FOREACHPROC ForEach, LPARAM lParam)
{
	int cnt = 0;
	std::vector<CSpriteSheet*>::iterator Iterator = m_SpriteSheets.begin();
	while(Iterator != m_SpriteSheets.end()) {
		ASSERT(*Iterator);
		int aux = ForEach((LPVOID)(*Iterator), lParam);
		if(aux < 0) return aux-cnt;
		cnt += aux;
		Iterator++;
	}
	return cnt;
}
int CGameManager::ForEachMap(FOREACHPROC ForEach, LPARAM lParam)
{
	return 0;
}
int CGameManager::ForEachMapGroup(FOREACHPROC ForEach, LPARAM lParam)
{
	return 0;
}
