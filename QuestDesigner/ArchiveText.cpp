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

#include "stdafx.h"
#include "ArchiveText.h"

#include "ProjectManager.h"
#include "SpriteManager.h"

bool CSpriteSheetTxtArch::ReadObject(LPCSTR szFile)
{
	CFileName &fnSheetFile = m_pSpriteSheet->GetFileName();
	CString &sSheetName = m_pSpriteSheet->GetName();

	FILE *fInFile = fopen(szFile, "rt");
	m_nLines = 0;
	if(fInFile) {
		fnSheetFile.SetFilePath(szFile);
		CHAR buff[100];
		if(fgets(buff, sizeof(buff), fInFile)) {
			m_nLines++;
			sSheetName = buff;
			sSheetName.Trim();
			if(sSheetName=="") 
				sSheetName = fnSheetFile.GetFileTitle();

			while(ReadSprite(fInFile)>=0);
		}
		fclose(fInFile);
	}
	return true;
}

/*!
	\param fInFile the file from where the sprite is to be read.
	\return Returns the number of errors found in the sprite, or 0 if there
	are no errors. Returns < 0 if the end of file has been reached.
	\remarks This method is used to load one sprite at the time, checking and
	validating every property, and creating the sprite in the list.

	If there is any reference to an object as part of the properties (such a mask), 
	the object is searched, declared, and referenced.

	This function should be smaller, but this keeps only temporary compatibility 
	with the old Quest Designer by GD.
*/
int CSpriteSheetTxtArch::ReadSprite(FILE *fInFile)
{
	SSpriteData *pSpriteData = NULL;
	SMaskData *pMaskData = NULL;
	SBackgroundData *pBackgroundData = NULL;
	SEntityData *pEntityData = NULL;

	CHAR buff[100];
	CString sLine;
	int iFrame=0, iFrames = 1;

	CFileName &fnSheetFile = m_pSpriteSheet->GetFileName();
	CProjectManager *pProjectManager = m_pSpriteSheet->GetProjectManager();

	CRect rcRect;
	_spt_type sptType;
	enum {eLeft, eTop, eRight, eBottom} nextPoint = eLeft;
	enum {eEOF=-1, eEnd, eType, eName, eAnim, eAnimFrms, eAnimSpd, eRect, eFill, eMask, eLayer, eAlpha, eDefine, eError} state = eEOF;
	CSprite *pSprite = NULL;
	while(fgets(buff, sizeof(buff), fInFile)) {
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
				pMaskData = new SMaskData;
				memset(pMaskData, 0, sizeof(SMaskData));
				pSpriteData = pMaskData;
				sptType = tMask;
				state = eName;
				goto retry;
			}
		} else if(state == eName) {
			pSprite = pProjectManager->ReferSprite(sLine, sptType);
			if(!pSprite) {
				state = eError;
				printf("Sprite error[1] in '%s'(%d)\n", fnSheetFile.GetFileName(), m_nLines);
			}
			else {
				if(pSprite->IsDefined()) {
					state = eError;
					printf("Sprite '%s' already defined in '%s'\n", pSprite->GetName(), pSprite->GetSpriteSheet()->GetName());
					printf("\t'%s' redefinition attempt in '%s'\n", pSprite->GetName(), m_pSpriteSheet->GetName());
				} else state = eAnim;
			}
		} else if(state == eAnim) {
			if(sLine=="ANIMATED") state = eAnimFrms;
			else {
				state = eRect;
				goto retry;
			}
		} else if(state == eAnimFrms) {
			iFrames = atoi(sLine);
			state = eAnimSpd;
		} else if(state == eAnimSpd) {
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

					if( rcRect.right-rcRect.left == 0 || 
						rcRect.bottom-rcRect.top == 0 ||
						rcRect.top < 0 || rcRect.bottom < 0 ||
						rcRect.right < 0 || rcRect.left < 0 ) {
						if(iFrames == 1) state = eError;
						printf("Sprite error[2] in '%s'(%d)\n", fnSheetFile.GetFileName(), m_nLines);
					} else {
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
					pBackgroundData->pMaskMap = static_cast<CMaskMap*>(pProjectManager->ReferSprite(sLine, tMask));
					if(!pBackgroundData->pMaskMap) {
						state = eError;
						printf("Sprite error[3] in '%s'(%d)\n", fnSheetFile.GetFileName(), m_nLines);
					}
				}
			} else goto retry;
		} else if(state == eLayer) {
			state = eAlpha;
			if(	sptType == tBackground ||
				sptType == tEntity ) {
					if(sLine == "TRUE") pBackgroundData->nSubLayer = 4;
					else pBackgroundData->nSubLayer = 1;
					if(sptType == tEntity) pBackgroundData->nSubLayer++;
			} else goto retry;
		} else if(state == eAlpha) {
			state = eDefine;
			if(	sptType == tBackground ||
				sptType == tEntity ) {
					pBackgroundData->cAlphaValue = atoi(sLine);
			} else goto retry;
		} else if(state == eDefine) {
			state = eEnd;
			pSprite->SetSpriteData(pSpriteData);
			if(sptType == tEntity ) {
				pEntityData->pScript = pProjectManager->MakeScript(pSprite->GetName());
				if(!pEntityData->pScript) {
					state = eError;
					printf("Sprite error[4] in '%s'(%d): Couldn't load the script file for the entity!\n", fnSheetFile.GetFileName(), m_nLines);
				}
			}
			if(state != eError) {
				if(pProjectManager->MakeSprite(pSprite->GetName(), sptType, m_pSpriteSheet) == NULL) 
					state = eError;
			}
		}
		if(sLine=="[EE]") break;
	}
	return (int)state;
}
bool CSpriteSheetTxtArch::WriteObject(LPCSTR szFile)
{
	return false;
}


bool CProjectTxtArch::ReadObject(LPCSTR szFile)
{
	return false;
}
bool CProjectTxtArch::WriteObject(LPCSTR szFile)
{
	return false;
}

bool CLayerTxtArch::ReadObject(LPCSTR szFile)
{
	FILE *fInFile = fopen(szFile, "rt");
	CString sLine;
	m_nLines = 0;
	if(fInFile) {
		CHAR buff[100];
		if(fgets(buff, sizeof(buff), fInFile)) {
			m_nLines++;
			sLine = buff;
			sLine.Trim();
			// read the sprites locations (sprite context)
			int nObjects = atoi(sLine);
			for(int i=0; i<nObjects; i++) {
				if(ReadSprite(fInFile)<0) {
					fclose(fInFile);
					return false;
				}
			}
		}
		if(fgets(buff, sizeof(buff), fInFile)) {
			m_nLines++;
			sLine = buff;
			sLine.Trim();
			// read the tiles positions
			int nObjects = atoi(sLine);
			for(int i=0; i<nObjects; i++) {
				if(ReadTile(fInFile)<0) {
					fclose(fInFile);
					return false;
				}
			}
		}
		fclose(fInFile);
	}
	return true;
}
int CLayerTxtArch::ReadSprite(FILE *fInFile)
{
	CHAR buff[100];
	CString sLine;
	// sprite contexts on lands are 4 lines long (old Greg format):

	// First line (x position):
	if(!fgets(buff, sizeof(buff), fInFile)) return -1;
	m_nLines++; sLine = buff; sLine.Trim();
	int x = atoi(sLine);
	
	// Second line (y position):
	if(!fgets(buff, sizeof(buff), fInFile)) return -1;
	m_nLines++; sLine = buff; sLine.Trim();
	int y = atoi(sLine);
	
	// Third line (sprite name):
	if(!fgets(buff, sizeof(buff), fInFile)) return -1;
	m_nLines++; 
	CString sName = buff; 
	sName.Trim();

	// Fourth line (sprite context ID):
	if(!fgets(buff, sizeof(buff), fInFile)) return -1;
	m_nLines++; 
	CString sID = buff; 
	sID.Trim();

	CSprite *pSprite = CProjectManager::Instance()->FindSprite(sName);
	if(!pSprite) {
		printf("Layer error: Couldn't find the requested sprite!\n");
		return 0;
	}

	CSpriteContext *pSpriteContext = new CSpriteContext(sID);
	pSpriteContext->SetDrawableObj(pSprite);
	pSpriteContext->MoveTo(x/2, y/2);

	if(pSprite->GetSpriteType() == tBackground || pSprite->GetSpriteType() == tEntity) {
		pSpriteContext->SetSubLayer(static_cast<CBackground *>(pSprite)->GetSubLayer());
		pSpriteContext->Alpha(static_cast<CBackground *>(pSprite)->GetAlphaValue());
	} else {
		printf("Map error: Attempt to use mask '%s' as a sprite in line %d\n", sName, m_nLines);
		delete pSpriteContext;
		return 0;
	}


	m_pLayer->AddSpriteContext(pSpriteContext);

	return 1;
}
int CLayerTxtArch::ReadTile(FILE *fInFile)
{
	CHAR buff[100];
	CString sLine;
	// sprite contexts on lands are 4 lines long (old Greg format):

	// First line (x1 position):
	if(!fgets(buff, sizeof(buff), fInFile)) return -1;
	m_nLines++; sLine = buff; sLine.Trim();
	int x1 = atoi(sLine);
	
	// Second line (y1 position):
	if(!fgets(buff, sizeof(buff), fInFile)) return -1;
	m_nLines++; sLine = buff; sLine.Trim();
	int y1 = atoi(sLine);

	// Third line (x2 position):
	if(!fgets(buff, sizeof(buff), fInFile)) return -1;
	m_nLines++; sLine = buff; sLine.Trim();
	int x2 = atoi(sLine);
	
	// Fourth line (y2 position):
	if(!fgets(buff, sizeof(buff), fInFile)) return -1;
	m_nLines++; sLine = buff; sLine.Trim();
	int y2 = atoi(sLine);
	
	// Fifth line (sprite name):
	if(!fgets(buff, sizeof(buff), fInFile)) return -1;
	m_nLines++; 
	CString sName = buff; 
	sName.Trim();

	CSprite *pSprite = CProjectManager::Instance()->FindSprite(sName);
	if(!pSprite) {
		printf("Layer error: Couldn't find the requested fill!\n");
		return 0;
	}

	CSpriteContext *pSpriteContext = new CSpriteContext("");
	pSpriteContext->SetDrawableObj(pSprite);
	pSpriteContext->MoveTo(x1/2, y1/2);
	pSpriteContext->Tile();
	pSpriteContext->SetSize((x2-x1)/2, (y2-y1)/2);

	if(pSprite->GetSpriteType() == tBackground || pSprite->GetSpriteType() == tEntity) {
		pSpriteContext->SetSubLayer(static_cast<CBackground *>(pSprite)->GetSubLayer()-1);
		pSpriteContext->Alpha(static_cast<CBackground *>(pSprite)->GetAlphaValue());
	} else {
		printf("Map error: Attempt to use mask '%s' as a fill in line %d\n", sName, m_nLines);
		delete pSpriteContext;
		return 0;
	}

	m_pLayer->AddSpriteContext(pSpriteContext);

	return 1;
}

bool CLayerTxtArch::WriteObject(LPCSTR szFile)
{
	return false;
}
