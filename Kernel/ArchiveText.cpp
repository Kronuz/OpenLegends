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

#include "stdafx.h"
#include "ArchiveText.h"

#include "WorldManager.h"
#include "GameManager.h"
#include "SpriteManager.h"

#define ReadFloatFromFile(file) \
	((file).GetLine(buff, sizeof(buff))==NULL) ? (0.0f) : (m_nLines++, atof(buff))

#define ReadLongFromFile(file) \
	((file).GetLine(buff, sizeof(buff))==NULL) ? (0) : (m_nLines++, atol(buff))

#define ReadStringFromFile(string, file) \
	((file).GetLine(buff, sizeof(buff))==NULL) ? (string="[eof]") : (m_nLines++, string=buff, string.Trim())

bool CSpriteSheetTxtArch::ReadObject(CVFile &vfFile)
{
	m_nLines = 0;
	CHAR buff[100];

	if(!vfFile.Open("r")) return false;

	CONSOLE_DEBUG("Loading %s (%d bytes)...\n", vfFile.GetFileName(), vfFile.GetFileSize());

	CBString sSheetName;
	ReadStringFromFile(sSheetName, vfFile);
	if(sSheetName=="") {
		m_pSpriteSheet->SetName(vfFile.GetFileTitle());
	} else {
		m_pSpriteSheet->SetName(sSheetName);
	}

	CGameManager *pGameManager = CGameManager::Instance();
	if(!pGameManager->MakeSpriteSheet(m_pSpriteSheet)) return false;

	while(ReadSprite(vfFile)>=0);

	vfFile.Close();
	return true;
}

/*!
	\param vfFile the file from where the sprite is to be read.
	\return Returns the number of errors found in the sprite, or 0 if there
	are no errors. Returns < 0 if the end of file has been reached.
	\remarks This method is used to load one sprite at the time, checking and
	validating every property, and creating the sprite in the list.

	If there is any reference to an object as part of the properties (such a mask), 
	the object is searched, declared, and referenced.

	This function should be smaller, but this keeps only temporary compatibility 
	with the old Quest Designer by GD.
*/
int CSpriteSheetTxtArch::ReadSprite(CVFile &vfFile)
{
	SSpriteData *pSpriteData = NULL;
	SBackgroundData *pMaskData = NULL;
	SBackgroundData *pBackgroundData = NULL;
	SEntityData *pEntityData = NULL;

	CHAR buff[100];
	CBString sLine;
	int iFrame=0, iFrames = 1;

	CVFile &fnSheetFile = m_pSpriteSheet->GetFileName();
	CGameManager *pGameManager = CGameManager::Instance();

	CRect rcRect;
	_spt_type sptType;
	enum {eLeft, eTop, eRight, eBottom} nextPoint = eLeft;
	enum {eEOF=-1, eEnd, eType, eName, eAnim, eAnimFrms, eAnimSpd, eRect, eFill, eMask, eLayer, eAlpha, eDefine, eError} state = eEOF;
	CSprite *pSprite = NULL;
	while(vfFile.GetLine(buff, sizeof(buff))) {
		m_nLines++;
		if(state == eEOF) state = eType;
		sLine = buff;
		sLine.Trim();

retry:
		if(state == eType) {
			if(sLine=="[ENTITY]") {
				pEntityData = new SEntityData;
				memset(pEntityData, 0, sizeof(SEntityData));
				pBackgroundData = pEntityData;
				pSpriteData = pEntityData;
				sptType = tEntity;
				state = eName;
			} else if(sLine=="[BACKGROUND]") {
				pBackgroundData = new SBackgroundData;
				memset(pBackgroundData, 0, sizeof(SBackgroundData));
				pSpriteData = pBackgroundData;
				sptType = tBackground;
				state = eName;
			} else {
				pMaskData = new SBackgroundData;
				memset(pMaskData, 0, sizeof(SBackgroundData));
				pBackgroundData = pMaskData;
				pSpriteData = pMaskData;
				sptType = tBackground;
				state = eName;
				goto retry;
			}
		} else if(state == eName) {
			pSprite = pGameManager->ReferSprite(sLine, sptType, fnSheetFile.GetFileName(), m_nLines);
			if(!pSprite) {
				state = eError;
				CONSOLE_PRINTF("Sprite error[0] in '%s'(%d): Expected a valid %s name instead of: '%s'\n", fnSheetFile.GetFileName(), m_nLines, (sptType==tMask)?"Mask":(sptType==tEntity)?"Entity":"Sprite", sLine);
			}
			else {
				if(pSprite->IsDefined()) {
					state = eError;
					CONSOLE_PRINTF("%s error[1]: '%s' already defined in '%s' (%s)\n    '%s' redefinition attempt in '%s' (%s)\n", 
						(sptType==tMask)?"Mask":(sptType==tEntity)?"Entity":"Sprite", pSprite->GetName(), pSprite->GetSpriteSheet()->GetName(), pSprite->GetSpriteSheet()->GetFileName().GetFileName(),
						pSprite->GetName(), m_pSpriteSheet->GetName(), fnSheetFile.GetFileName());
				} else state = eAnim;
			}
		} else if(state == eAnim) {
			// Check for the ANIMATED keyword. Sprites can either
			// have just the one frame (which is normal), or they
			// can have a few frames.
			if(sLine=="ANIMATED") state = eAnimFrms;
			else {
				state = eRect;
				goto retry;
			}
		} else if(state == eAnimFrms) {
			iFrames = atoi(sLine);
			state = eAnimSpd;
		} else if(state == eAnimSpd) {
			pSpriteData->eAnimDir = _d_up;
			pSpriteData->bAnimLoop = true;
			pSpriteData->iAnimSpd = atoi(sLine);
			state = eRect;
		} else if(state == eRect) {
			if(iFrame<iFrames) {
				if(nextPoint == eLeft) {
					rcRect.left = atoi(sLine);
					nextPoint = eTop;
				} else if(nextPoint == eTop) {
					rcRect.top = atoi(sLine);
					nextPoint = eRight;
				} else if(nextPoint == eRight) {
					rcRect.right = rcRect.left + atoi(sLine);
					nextPoint = eBottom;
				} else if(nextPoint == eBottom) {
					rcRect.bottom = rcRect.top + atoi(sLine);
					nextPoint = eLeft;
					rcRect.NormalizeRect();
					if( rcRect.IsRectEmpty()) {
						if(iFrames == 1) state = eError;
						CONSOLE_PRINTF("Sprite error[2] in '%s'(%d): The size of the %s is not well defined (size is: %d x %d)\n", 
							fnSheetFile.GetFileName(), m_nLines, 
							(sptType==tMask)?"Mask":(sptType==tEntity)?"Entity":"Sprite", 
							rcRect.Width(), rcRect.Height());
					} else {
						// To make sprites and maps 640x480 by default:
						rcRect.top *= 2;
						rcRect.bottom *= 2;
						rcRect.left *= 2;
						rcRect.right *= 2;
/**/
						pSprite->AddRect(rcRect);
					}
						iFrame++;
				}
			} else {
				state = eFill;
				goto retry;
			}
		} else if(state == eFill) {
			state = eMask;
			if(sLine!="[FILL]") goto retry;
		} else if(state == eMask) {
			state = eLayer;
			if(	sptType == tBackground ||
				sptType == tEntity) {
				if(sLine!="") { // there is a mask for this sprite
					pBackgroundData->pMaskMap = static_cast<CMaskMap*>(pGameManager->ReferSprite(sLine, tMask, fnSheetFile.GetFileName(), m_nLines));
					if(!pBackgroundData->pMaskMap) {
						state = eError;
						CONSOLE_PRINTF("Sprite error[3] in '%s'(%d)\n", fnSheetFile.GetFileName(), m_nLines);
					}
				}
			} 
		} else if(state == eLayer) {
			state = eAlpha;
			if(sLine == "TRUE") pSpriteData->nSubLayer = 4;
			else if(sLine == "FALSE") pSpriteData->nSubLayer = 1;
			else {
				state = eError;
				CONSOLE_PRINTF("Sprite error[4] in '%s'(%d): Invalid layer type.\n", fnSheetFile.GetFileName(), m_nLines);
			}
			if(sptType == tEntity) pSpriteData->nSubLayer++;
		} else if(state == eAlpha) {
			state = eDefine;
			pSpriteData->cAlphaValue = atoi(sLine);
		} else if(state == eDefine) {
			state = eEnd;
			pSprite->SetSpriteData(pSpriteData);
			pSpriteData = NULL;
			if(sptType == tEntity ) {
				pEntityData->pScript = pGameManager->MakeScript(pSprite->GetName());
				if(!pEntityData->pScript) {
//					state = eError;
					CONSOLE_PRINTF("Sprite warning in '%s'(%d): Couldn't find the script file for the entity %s.\n", fnSheetFile.GetFileName(), m_nLines, pSprite->GetName());
				}
			}
			if(state != eError) {
				if(pGameManager->MakeSprite(pSprite->GetName(), sptType, m_pSpriteSheet) == NULL) 
					state = eError;
			}
		}
		if(sLine=="[EE]") break;
	}
	delete pSpriteData;

	return (int)state;
}
bool CSpriteSheetTxtArch::WriteObject(CVFile &vfFile)
{
	return false;
}

bool CProjectTxtArch::ReadObject(CVFile &vfFile)
{
	return false;
}
bool CProjectTxtArch::WriteObject(CVFile &vfFile)
{
	return false;
}

bool CMapTxtArch::ReadObject(CVFile &vfFile)
{
	vfFile.Open("r");

	int nObjects;
	m_nLines = 0;
	CHAR buff[100];

	// read the sprites locations (sprite context)
	nObjects = ReadLongFromFile(vfFile);
	for(int i=0; i<nObjects; i++) {
		if(ReadSprite(vfFile)<0) {
			vfFile.Close();
			return false;
		}
	}
	
	// read the tiles positions
	nObjects = ReadLongFromFile(vfFile);
	for(int i=0; i<nObjects; i++) {
		if(ReadTile(vfFile)<0) {
			vfFile.Close();
			return false;
		}
	}

	vfFile.Close();
	return true;
}
int CMapTxtArch::ReadSprite(CVFile &vfFile)
{
	CHAR buff[100];
	// sprite contexts on lands are 4 lines long (old Greg format):

	// First line (x position):
	int x = ReadLongFromFile(vfFile);
	// Second line (y position):
	int y = ReadLongFromFile(vfFile);
	// Third line (sprite name):
	CBString sName;
	ReadStringFromFile(sName,vfFile);
	// Fourth line (sprite context ID):
	CBString sID; 
	ReadStringFromFile(sID,vfFile);

	CSprite *pSprite = CGameManager::Instance()->FindSprite(sName);
	if(!pSprite) {
		CONSOLE_PRINTF("Map error in '%s' (%d): Couldn't find the requested sprite: '%s'!\n", vfFile.GetFileName(), m_nLines, sName);
		return 0;
	}

	CSpriteContext *pSpriteContext = new CSpriteContext(sID);
	pSpriteContext->SetDrawableObj(pSprite);
	pSpriteContext->MoveTo(x, y); // x/2, y/2
	if(pSprite->GetSpriteType() == tBackground) pSpriteContext->Tile();

	if(pSprite->GetSpriteType() == tBackground || pSprite->GetSpriteType() == tEntity) {
		pSpriteContext->SetObjSubLayer(static_cast<CBackground *>(pSprite)->GetObjSubLayer());
		pSpriteContext->Alpha(static_cast<CBackground *>(pSprite)->GetAlphaValue());
	} else {
		CONSOLE_PRINTF("Map error in '%s' (%d): Attempt to use mask '%s' as a sprite\n", vfFile.GetFileName(), m_nLines, sName);
		delete pSpriteContext;
		return 0;
	}

	if(!m_pLayer->AddSpriteContext(pSpriteContext, false)) {
		CONSOLE_PRINTF("Map warning in '%s' (%d): Duplicated sprite '%s' not added at (%d, %d).\n", vfFile.GetFileName(), m_nLines, sName, x, y); // x/2, y/2
		delete pSpriteContext;
	}

	return 1;
}
int CMapTxtArch::ReadTile(CVFile &vfFile)
{
	CHAR buff[100];
	CBString sLine;
	// sprite contexts on lands are 4 lines long (old Greg format):

	// First line (x1 position):
	int x1 = ReadLongFromFile(vfFile);
	// Second line (y1 position):
	int y1 = ReadLongFromFile(vfFile);
	// Third line (x2 position):
	int x2 = ReadLongFromFile(vfFile);
	// Fourth line (y2 position):
	int y2 = ReadLongFromFile(vfFile);
	// Fifth line (sprite name):
	CBString sName = buff; 
	ReadStringFromFile(sName, vfFile);

	CSprite *pSprite = CGameManager::Instance()->FindSprite(sName);
	if(!pSprite) {
		CONSOLE_PRINTF("Map error in '%s' (%d): Couldn't find the requested fill: '%s'!\n", vfFile.GetFileName(), m_nLines, sName);
		return 0;
	}

	CSpriteContext *pSpriteContext = new CSpriteContext("");
	pSpriteContext->SetDrawableObj(pSprite);
	pSpriteContext->MoveTo(x1, y1); // x1/2, y1/2
	pSpriteContext->Tile();
	pSpriteContext->SetSize(x2-x1, y2-y1); // (x2-x1)/2, (y2-y1)/2

	if(pSprite->GetSpriteType() == tBackground || pSprite->GetSpriteType() == tEntity) {
		pSpriteContext->SetObjSubLayer(static_cast<CBackground *>(pSprite)->GetObjSubLayer()-1);
		pSpriteContext->Alpha(static_cast<CBackground *>(pSprite)->GetAlphaValue());
	} else {
		CONSOLE_PRINTF("Map error in '%s' (%d): Attempt to use mask '%s' as a fill\n", vfFile.GetFileName(), m_nLines, sName);
		delete pSpriteContext;
		return 0;
	}

	if(!m_pLayer->AddSpriteContext(pSpriteContext, false)) {
		CONSOLE_PRINTF("Map warning in '%s' (%d): Duplicated sprite '%s' not added at (%d, %d).\n", vfFile.GetFileName(), m_nLines, sName, x2-x1, y2-y1); // (x2-x1)/2, (y2-y1)/2
		delete pSpriteContext;
	}

	return 1;
}

bool CMapTxtArch::WriteObject(CVFile &vfFile)
{
	return false;
}


bool CWorldTxtArch::ReadObject(CVFile &vfFile)
{
	bool bRet = true;

	if(!vfFile.Open("r")) return false;
	CONSOLE_DEBUG("Loading %s (%d bytes)...\n", vfFile.GetFileName(), vfFile.GetFileSize());

	m_nLines = 0;
	CHAR buff[100];

	CBString sID;
	ReadStringFromFile(sID, vfFile);
	if(sID == "Open Zelda Quest Designer Map File") {
		if(ReadMaps(vfFile)) {
			if(ReadMapGroups(vfFile)) {
				if(!ReadProperties(vfFile)) bRet = false;
			} else bRet = false;
		} else bRet = false;
	} else bRet = false;

	vfFile.Close();
	return bRet;
}
bool CWorldTxtArch::WriteObject(CVFile &vfFile)
{
	return false;
}

bool CWorldTxtArch::ReadMaps(CVFile &vfFile)
{
	CHAR buff[100];

	int nMaps = ReadLongFromFile(vfFile);
	for(int i=0; i<nMaps; i++) {
		// x position
		ReadLongFromFile(vfFile);
		// y position
		ReadLongFromFile(vfFile);
		// indoors
		int nIndoors = ReadLongFromFile(vfFile);
	}
	return true;
}
bool CWorldTxtArch::ReadMapGroups(CVFile &vfFile)
{
	CHAR buff[100];
	// Get the Number of groups
	int nGroups = ReadLongFromFile(vfFile);
	for(int i=0; i<nGroups; i++) {
		CBString sMusic;
		CRect Rect;
		int nIdent		= ReadLongFromFile(vfFile);

		Rect.left		= ReadLongFromFile(vfFile) / 128;
		Rect.top		= ReadLongFromFile(vfFile) / 96;
		Rect.right		= ReadLongFromFile(vfFile) / 128;
		Rect.bottom		= ReadLongFromFile(vfFile) / 96;

		int nLoopBack	= ReadLongFromFile(vfFile);

		// Music fot the map group
		ReadStringFromFile(sMusic, vfFile);

		CMapGroup *pMapGroup = m_pWorld->BuildMapGroup(Rect.left, Rect.top, Rect.Width(), Rect.Height());
		if(pMapGroup && sMusic!="") {
			CGameManager *pGameManager = CGameManager::Instance();
			pMapGroup->SetMusic(pGameManager->MakeSound(sMusic));
		}
	}

	return true;
}
bool CWorldTxtArch::ReadProperties(CVFile &vfFile)
{
	CHAR buff[100];
	CBString sQuestName;

	int lStartX = ReadLongFromFile(vfFile);
	int lStartY = ReadLongFromFile(vfFile);
	ReadStringFromFile(sQuestName, vfFile);
	bool bFinalVersion = (ReadLongFromFile(vfFile)==1);
	bool bSaveSounds = (ReadLongFromFile(vfFile)==1);

	return true;
}
