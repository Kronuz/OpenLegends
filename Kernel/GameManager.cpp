/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germán Méndez Bravo (Kronuz)
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
/*! \file		GameManager.cpp
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the CGameManager class.
	\date		April 16, 2003:
					* Creation date.
				July 07, 2005 by Littlebuddy:
					+ Linked ForEachMapGroup to it's corresponding CWorld function.
				Feb, 2006 by Littlebuddy:
					+ Added sprite buffer for script threads.
				Feb 14, 2006:
					+ Sprite buffer now uses std::pair<CSpriteContext*, int> to also store layer to draw sprite in.
					+ DrawSprite() has been added as an inline boolean function.

	This file implements all the classes that manage the project,
	this includes the methods to write and read from the 
	configuration files (i.e. sprite sheet files).
*/

#include "stdafx.h"
#include "GameManager.h"
#include "ScriptManager.h"
#include "Debugger.h"

#include "ArchiveText.h"

float CGameManager::ms_fDelta = 0.0f;
DWORD CGameManager::ms_dwLastTick = 0L;
IGraphics **CGameManager::ms_ppGraphicsI = NULL;

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

int arr=0;
void CGameManager::TheSecretsOfDebugging(){
	arr+=1;
	CBString sName = "__pstn1";
	CSize *size = new CSize;
	CPoint pt;
	int x; int y;
	x=y=0;
	CSprite *pSprite = CGameManager::Instance()->FindSprite(sName);
	if(!pSprite) {
		return;
	}
	
	pSprite->GetSize(*size);
			CSpriteContext *pSpriteContext = new CSpriteContext("");
			pSpriteContext->SetDrawableObj(pSprite);
			//if(pSprite->GetSpriteType() == tBackground) pSpriteContext->Tile();	
			pSpriteContext->SetObjSubLayer(2);
			pSpriteContext->SetTemp();
			pSpriteContext->MoveTo(x*size->cx+arr, y*size->cy);
			if(arr > 620) arr = 0;
			BufferPair Pair;
			Pair.first = pSpriteContext;
			Pair.second = 3;
			m_SpriteBuffer.push_back(Pair); //*used to add objects*
	return;
}

CMapGroup* CGameManager::Wiping(){
	if(!m_bWiping) return NULL;	//Cool part is, Wiping is totally separated from script threads and will be smooth no matter what the script-fps is.
	
	
	
	
	
	return m_pWipeTarget;
}
bool CGameManager::Wipe(int dir, LPCSTR szName, int edgedistX, int edgedistY){
	if(m_bWiping) return true;
	ASSERT(ms_ppGraphicsI);
	ASSERT(*ms_ppGraphicsI);
	if(!(*ms_ppGraphicsI)) return false;
	CRect mapRect;
	CPoint mapPos;
	m_pActiveMapGroup->GetMapGroupRect(mapRect);
	(*ms_ppGraphicsI)->GetWorldPosition(&mapPos);
	/*SCRIPT ENUM:
		north = 0
		west = 1
		south = 2
		east = 3
	*/
	mapPos.x /= DEF_MAPSIZEX; //These mods make sure we find the correct location if the player
	mapPos.y /= DEF_MAPSIZEY; //has walked down a bit and crossed an imaginary group border for the group next to his location.
	CMapGroup *pTargetGroup = NULL;
	switch(dir){
		case 0: //north ^
			pTargetGroup = FindMapGroup(mapRect.left+mapPos.x, mapRect.top-1);
			break;
		case 1: //west <--
			pTargetGroup = FindMapGroup(mapRect.left-1, mapRect.top+mapPos.y);
			break;
		case 2: //south \/
			pTargetGroup = FindMapGroup(mapRect.left+mapPos.x, mapRect.bottom+1);
			break;
		case 3: //east -->
			pTargetGroup = FindMapGroup(mapRect.right+1, mapRect.top+mapPos.y);
			break;
		default: return false;
	}
	if(pTargetGroup == NULL) return false;	//No group in this location, map design flaw.
	m_pWipeTarget = pTargetGroup;
	CPoint groupPos, targetPos;
	m_pActiveMapGroup->GetAbsPosition(groupPos);
	m_pWipeTarget->GetAbsPosition(targetPos);
	//groupPos += mapPos; //Offset to find the correct location to draw the target group.
	CPoint *delta = new CPoint(groupPos.x - targetPos.x, groupPos.y - targetPos.y);
	m_pWipeOffset = delta;
	m_bWiping = true;	//The wipe will happen. (Do this when all data has been finalized.)
	return true;
}
//Flush sprites to the correct location
void CGameManager::FlushSprites(){
	while(m_SpriteBuffer.begin() != m_SpriteBuffer.end()){
		CLayer *pLayer = static_cast<CLayer *>(m_pActiveMapGroup->GetChild((*m_SpriteBuffer.begin()).second));
		pLayer->AddSpriteContext((*m_SpriteBuffer.begin()).first);
		m_SpriteBuffer.erase(m_SpriteBuffer.begin());
	}
}
void CGameManager::QueueFull(){
	CScript::QueueFull();
}
bool CGameManager::QueueAccepting(){
	return CScript::QueueAccepting();
}

CGameManager *CGameManager::_instance = NULL;

CGameManager::CGameManager() :
	CDocumentObject(),
	m_World("New Quest"),
	m_pDummyDebug(NULL)
{
	m_bWiping = false;
	m_ArchiveIn = new CProjectTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
	m_pSoundManager = CSoundManager::Instance();
	CONSOLE_PRINTF("Using the " KERNEL_FULLNAME "\n");
}

CGameManager::~CGameManager()
{
	BEGIN_DESTRUCTOR
	Close(true);
	delete m_pDummyDebug;
	
	delete m_ArchiveIn;
	if(m_ArchiveIn != m_ArchiveOut) delete m_ArchiveOut;
	DeleteCriticalSection(&CScript::XCritical);
	END_DESTRUCTOR
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
			pSprite->SetSpriteType(tMask);
			break;
		}
	}
	if(!pSprite) CONSOLE_PRINTF("The sprite '%s' couldn't be created!\n", szName);
	ASSERT(pSprite);
	return pSprite;
}

CSprite *CGameManager::FindSprite(LPCSTR szName)
{
	std::vector<CSpriteSheet*>::iterator SheetIterator = m_SpriteSheets.begin();
	while(SheetIterator != m_SpriteSheets.end()) {
		if(*SheetIterator) {
			CSprite *pSprite = (*SheetIterator)->FindSprite(szName);
			if(pSprite) return pSprite;
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

CSound *CGameManager::MakeSound(LPCSTR szName)
{
	// create a new sound object and try to find its file
	CSound *pSound = new CSound;
	CBString sPath = CBString("Sounds\\") + szName; // relative by default
	pSound->SetSource(sPath);
	if(!pSound->SourceExists()) {
		delete pSound;
		return NULL; // couldn't find the sound file
	}

	// Now we find the sound file name in the map of sounds:
	std::map<CBString, CSound*>::iterator Iterator = m_Sounds.find(pSound->GetTitle());
	if(Iterator == m_Sounds.end()) { // there's no sound file in the map (first time sound):
		m_Sounds.insert(pairSound(szName, pSound)); //add it to the map and ...
		// ... make a call to the client, to notify the sound creation.
		CallbackProc(
			Sound, 
			pSound, 
			(LPCSTR)(m_sProjectName + "\\Sounds\\" + pSound->GetTitle()), 
			Added);
	} else { // otherwise, the sound already is in the map, so we don't need the newly created object.
		delete pSound; 
		pSound = Iterator->second;
	}

	return pSound; // return the located or created script object.
}

CScript *CGameManager::MakeScript(LPCSTR szName)
{
	// create a new script object and try to find its file
	CScript *pScript = new CScript;
	CBString sPath = CBString("Entities\\") + szName + ".zes"; // relative by default
	pScript->SetSource(sPath);
	if(!pScript->SourceExists()) {
		CBString sFileTitle = szName;
		sFileTitle.SetAt(sFileTitle.GetLength()-1, '#');
		pScript->SetTitle(sFileTitle);
		if(!pScript->SourceExists()) {
			delete pScript;
			return NULL; // couldn't find the script file
		}
	}

	// Now we find the script file name in the map of scripts:
	std::map<CBString, CScript*>::iterator Iterator = m_Scripts.find(pScript->GetTitle());
	if(Iterator == m_Scripts.end()) { // there's no script file in the map (first time script):
		m_Scripts.insert(pairScript(pScript->GetTitle(), pScript)); //add it to the map and ...
		// ... make a call to the client, to notify the script creation.
		CallbackProc(
			Script, 
			pScript, 
			(LPCSTR)(m_sProjectName + "\\Entities\\" + pScript->GetTitle()), 
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
	if(sptType == tEntity) { if(!isalpha(*szName) && *szName!='_') return NULL; }
	else { if(!isalnum(*szName) && *szName!='_') return NULL; }

	CSprite *pSprite = FindSprite(szName);
	if(pSprite) {
		if(sptType == tBackground && pSprite->GetSpriteType() == tMask) {
			sptType = tMask;
		} else if(sptType == tMask && pSprite->GetSpriteType() == tBackground) {
			pSprite->SetSpriteType(tMask);
		} else if(pSprite->GetSpriteType() != sptType) {
			return NULL;
		}
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
		ASSERT(pSprite);
		if(pSprite->GetSpriteData() == NULL) return NULL;

		if(sptType == tBackground && pSprite->GetSpriteType() == tMask) {
			sptType = tMask;
		} else if(sptType == tMask && pSprite->GetSpriteType() == tBackground) {
			pSprite->SetSpriteType(tMask);
		} else if(pSprite->GetSpriteType() != sptType) {
			return NULL;
		}

		m_UndefSprites.erase(Iterator);
	} 

	if(pSprite) {
		pSprite->SetSpriteSheet(pSpriteSheet);

		/*
		pSpriteSheet->m_Sprites.push_back(pSprite);
		/*/
		pSpriteSheet->m_Sprites.insert(CSpriteSheet::pairSprite(szName, pSprite));
		/**/

		// We send a message letting know that a new sprite has just been defined
		CallbackProc(
			Sprite, 
			pSprite, 
			m_sProjectName + "\\Sprite sheets\\" + pSpriteSheet->GetName() + (LPCSTR)((sptType==tEntity)?"\\Entities\\":"\\Sprites\\") + szName,
			Added);
	}

	return pSprite;
}

void CGameManager::DeleteSound(int nIndex)
{
	std::map<CBString, CSound*>::iterator Iterator =  m_Sounds.begin();
	for(int idx=0; idx!=nIndex && Iterator != m_Sounds.end(); idx++, Iterator++);

	if(Iterator != m_Sounds.end()) {
		CSound *pSound = Iterator->second;

		CallbackProc(
			Sound, 
			NULL, 
			m_sProjectName + "\\Sounds\\" + pSound->GetTitle(),
			Deleted);

		delete pSound;
		m_Sounds.erase(Iterator);
	}
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
			m_sProjectName + "\\Entities\\" + pScript->GetTitle(),
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

		m_SpriteSheets.erase(Iterator);
		delete pSpriteSheet;
	}
}
void CGameManager::DeleteSprite(LPCSTR szName)
{ 
	CSprite *pSprite = FindSprite(szName);
	if(pSprite) {
		CallbackProc(
			Sprite, 
			NULL, 
			m_sProjectName + "\\Sprite sheets\\" + pSprite->GetSpriteSheet()->GetName() + (LPCSTR)((pSprite->GetSpriteType()==tEntity)?"\\Entities\\":"\\Sprites\\") + szName,
			Deleted);

		//FIXME this method still has to really delete the sprite from the sprite sheet
	}
}

bool CGameManager::_Close(bool bForce)
{
	if(!CloseWorld(bForce)) return false;

	CONSOLE_PRINTF("Closing project: '%s'...\n", m_sProjectName);

	if(m_SpriteSheets.size()) CONSOLE_PRINTF("Cleaning Sprite Sheets...\n");
	while(m_SpriteSheets.size()) {
		DeleteSpriteSheet(0);
	}

	// Now we delete all undefined sprites (referred, but never really created)
	if(m_UndefSprites.size()) CONSOLE_PRINTF("Cleaning Unreferred Sprites...\n");
	while(m_UndefSprites.size()) {
		delete m_UndefSprites.begin()->second.pSprite;
		m_UndefSprites.erase(m_UndefSprites.begin());
	}

	// Delete all loaded scripts
	if(m_Scripts.size()) CONSOLE_PRINTF("Cleaning Scripts...\n");
	while(m_Scripts.size()) {
		DeleteScript(0);
	}

	// Delete all loaded sounds
	if(m_Sounds.size()) CONSOLE_PRINTF("Cleaning Sounds...\n");
	while(m_Sounds.size()) {
		delete m_Sounds.begin()->second;
		m_Sounds.erase(m_Sounds.begin());
	}

	m_sProjectName = "";
	return true;
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
// This really just adds the sprite sheet to the game, the Sprite sheet object
// should have been already created. This just validates the addition and
// returns NULL on error. (so far, never NULL)
CSpriteSheet* CGameManager::MakeSpriteSheet(CSpriteSheet *pSpriteSheet)
{
	ASSERT(pSpriteSheet);
	m_SpriteSheets.push_back(pSpriteSheet);

	CallbackProc(
		SpriteSheet, 
		pSpriteSheet, 
		m_sProjectName + "\\Sprite sheets\\" + pSpriteSheet->GetName(),
		Added);

	return pSpriteSheet;
}

int CALLBACK CGameManager::LoadSheet(LPCTSTR szFile, DWORD dwFileAttributes, LPARAM lParam)
{
	CGameManager *pGameManager = reinterpret_cast<CGameManager*>(lParam);
	CSpriteSheet *pSpriteSheet = new CSpriteSheet(pGameManager);
	ASSERT(pSpriteSheet);

	// if it is a directory:
	if((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) return 0;

	CVFile vfFile(szFile);
	pSpriteSheet->Load(vfFile);

	if( find(pGameManager->m_SpriteSheets.begin(), 
			 pGameManager->m_SpriteSheets.end(), 
			 pSpriteSheet) == pGameManager->m_SpriteSheets.end() ) {
		ASSERT(!*"Sprite sheet not added to the game. Use MakeSpriteSheet() at loading time.");
		delete pSpriteSheet;
	}

	return 1;
}

void CGameManager::CleanUndefs()
{
	if(m_UndefSprites.size()) {
		CONSOLE_PRINTF("\nReferenced but undefined sprites:\n");
	}
	std::map<CBString, SUndefSprite>::iterator Iterator = m_UndefSprites.begin();
	while(Iterator != m_UndefSprites.end()) {
		SUndefSprite &UndefSprite = Iterator->second;
		// If the referenced, but not defined sprite is a Mask, we need to look for it on every object in every
		// sprite sheet and clear its mask reference if needed.
		if(UndefSprite.pSprite->GetSpriteType() == tMask) {
			if(ForEachSprite(RemoveMask, (LPARAM)UndefSprite.pSprite)) {
				CONSOLE_PRINTF("  Error: Undefined mask: '%s', first referred at: '%s' (%d)\n", Iterator->first, (LPCSTR)UndefSprite.sFile, UndefSprite.nLine);
			} else {
				CONSOLE_PRINTF("  Warning: Undefined mask: '%s', first referred at: '%s' (%d)\n", Iterator->first, (LPCSTR)UndefSprite.sFile, UndefSprite.nLine);
			}
		} else {
			CONSOLE_PRINTF("  Warning: Undefined sprite: '%s', first referred at: '%s' (%d)\n", Iterator->first, (LPCSTR)UndefSprite.sFile, UndefSprite.nLine);
		}
		delete UndefSprite.pSprite;
		Iterator++;
	}
	m_UndefSprites.clear();
}

void CGameManager::StopWaiting()
{
	CScript::StopWaiting();
}

bool CGameManager::isDebugging()
{
	return CScript::isDebugging();
}

float CGameManager::UpdateFPS(float fpsLock)
{
	// Perform Timing calculations
    static DWORD s_dwFrames = 0L;
	static DWORD s_dwLastTime = 0L;
	static DWORD s_dwLastTick = 0L;	// this is the real last tick
	static float s_fFPS = 0.0f;
	static int s_nAdjust = 0;

	if(fpsLock == 0.0f) {
		s_fFPS = 0.0f;
		s_nAdjust = 0;

		s_dwLastTick = GetTickCount();
		s_dwLastTime = s_dwLastTick;
		ms_dwLastTick = s_dwLastTick;
		ms_fDelta = 0.0f;
		return s_fFPS;
	}

	// Find out the wasted milliseconds and sleep
	int msw = 0;
	if(fpsLock != -1.0f) { // sleep only if we have a fixed frame rate
		msw = (int)(1000.0f / fpsLock) - s_nAdjust;
		// Is the adjustment too high or too low? recover:
		if(msw > (int)(1000.0f / fpsLock)*2) {
			msw = (int)(1000.0f / fpsLock)*2;
			s_nAdjust = 0;
		} else if(msw < 0) {
			msw = 0;
			s_nAdjust = 0;
		}
		// sleep for a while
		int tosleep = (2*msw) / 3; // sleep 2/3 of the wasted time
		if(tosleep > 5) Sleep(tosleep); // minimum milliseconds to sleep (no less than 5 ms)
	}

	// Calculate how much time has passed since the last time
    DWORD dwCurrTick = GetTickCount();
	DWORD dwDelta = dwCurrTick - s_dwLastTick;

	if(fpsLock != -1.0f) {
		if( dwDelta < (DWORD)msw ) {
			return -1.0f; // nothing to do
		}
	}

	// Count the number of rendered frames 
	s_dwFrames++;

	// Calculate the current frame rate
	float fOldFPS = s_fFPS;
	if( dwCurrTick - s_dwLastTime > 1000 || fpsLock != -1.0f )
		s_fFPS = (float)s_dwFrames / ((float)((dwCurrTick - s_dwLastTime)) / 1000);

	// Make time adjustments to stabilize the frame rate and the sleep time
	if(fpsLock != -1.0f) {
		if(s_fFPS < fpsLock-4.0f) {
			s_nAdjust+=3;
		} else if(s_fFPS < fpsLock) {
			if(s_fFPS <= fOldFPS) s_nAdjust++;
		} else if(s_fFPS > fpsLock+4.0f) {
			s_nAdjust-=3;
		} else if(s_fFPS > fpsLock) {
			if(s_fFPS >= fOldFPS) s_nAdjust--;
		}
	}

	if( dwCurrTick - s_dwLastTime > 1000 ) {
		s_dwLastTime = dwCurrTick;
		s_dwFrames   = 0;
	}

	// Limit the max Time Delta in case of very slow circumstances
	// This will have the effect of slowing the game down rather than
	// dropping frames making everything jumpy
	if(dwDelta > 50) {
		dwDelta = 50;
	}

	// Record the virtual last tick time
	ms_dwLastTick += dwDelta;

	// Record our Global Time Delta value
	ms_fDelta = (float)dwDelta / 1000.0f;

	// Record the real last tick time
	s_dwLastTick = dwCurrTick;

	return s_fFPS;
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
CMapGroup* CGameManager::FindMapGroup(LPCSTR szMapID) const
{
	return m_World.FindMapGroup(szMapID);
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

int CGameManager::ForEachSound(SIMPLEPROC ForEach, LPARAM lParam)
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
int CGameManager::ForEachSprite(SIMPLEPROC ForEach, LPARAM lParam)
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
int CGameManager::ForEachScript(SIMPLEPROC ForEach, LPARAM lParam)
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
int CGameManager::ForEachSpriteSheet(SIMPLEPROC ForEach, LPARAM lParam)
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
int CGameManager::ForEachMap(SIMPLEPROC ForEach, LPARAM lParam)
{
	return 0;
}
int CGameManager::ForEachMapGroup(SIMPLEPROC ForEach, LPARAM lParam)
{
	return m_World.ForEachMapGroup(ForEach, lParam);
}

int CGameManager::CountScripts() const
{
	return m_Scripts.size();
}

bool CGameManager::Configure(IGraphics **ppGraphicsI, bool bDebug) 
{ 
	delete m_pDummyDebug; m_pDummyDebug = NULL;
	if(bDebug) m_pDummyDebug = new CDebugScript(NULL, NULL);

	ms_ppGraphicsI = ppGraphicsI; 
	CScript::ms_bDebug = bDebug;
	InitializeCriticalSection(&CScript::XCritical);
	return true; 
}

LPCSTR CGameManager::GetProjectName() const
{
	return m_sProjectName;
}
void CGameManager::SetProjectName(LPCSTR szName)
{
	m_sProjectName = szName;
}
ISoundManager* CGameManager::GetSoundManager() const
{
	return (ISoundManager*)m_pSoundManager;
}

const IScript* CGameManager::GetScript(int idx) const
{
	ASSERT(idx < (int)m_Scripts.size());
	std::map<CBString, CScript*>::const_iterator Iter=m_Scripts.begin();
	for(int i=0; Iter!=m_Scripts.end(); Iter++, i++) {
		if(i == idx) return Iter->second;
	}
	return NULL;
}

const IScript* CGameManager::GetScript(CSprite *pSprite) const
{
	if(pSprite && pSprite->GetSpriteType() == tEntity) {
		CEntity *pEntity = static_cast<CEntity *>(pSprite);
		return pEntity->GetScript();
	}
	return NULL;
}

