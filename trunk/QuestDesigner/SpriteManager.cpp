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
/*! \file		SpriteManager.cpp
	\brief		Implementation of the sprite classes.
	\date		April 17, 2003

	This file implements all the classes that manage the sprites,
	including backgrounds, sprite sheets, animations, mask maps and
	entities.
*/

#include "stdafx.h"
#include "SpriteManager.h"
#include "ProjectManager.h"

CEntity::CEntity()
{
	m_SptType = tEntity;
}
CBackground::CBackground()
{
	m_SptType = tBackground;
}
CMaskMap::CMaskMap()
{
	m_SptType = tMask;
}

bool CSpriteSheet::Load(LPCSTR szFile) 
{ 
	FILE *fInFile = fopen(szFile, "rt");
	m_iLines = 0;
	static int iUnnamed=1;
	if(fInFile) {
		m_fnSheetFile.SetFilePath(szFile);
		CHAR buff[100];
		if(fgets(buff, sizeof(buff), fInFile)) {
			m_iLines++;
			m_sSheetName = buff;
			m_sSheetName.Trim();
			if(m_sSheetName=="") m_sSheetName = m_fnSheetFile.GetFileTitle();

			while(LoadSprite(fInFile)>=0);
		}
		fclose(fInFile);
	}
	return false; 
}

/*!
	\param fInFile the file from where the sprite is to be read.
	\return Returns the number of errors found in the sprite, or 0 if there
	are no error. Returns < 0 if the end of file has been reached.
	\remarks This method is used to load one sprite at the time, checking and
	validating every property, and creating the sprite in the list.

	If there is any reference to an object as part of the properties (such a mask), 
	the object is searched, declared, and referenced.
*/
int CSpriteSheet::LoadSprite(FILE *fInFile)
{
	CHAR buff[100];
	CString sLine;
	int iFrame=0, iFrames = 1;

	CRect rcRect;
	_spt_type sptType;
	enum {eLeft, eTop, eRight, eBottom} nextPoint = eLeft;
	enum {eEOF=-1, eEnd, eType, eName, eAnim, eAnimFrms, eAnimSpd, eRect, eFill, eMask, eLayer, eAlpha, eDefine, eError} state = eEOF;
	CSprite *pSprite = NULL;
	while(fgets(buff, sizeof(buff), fInFile)) {
		m_iLines++;
		if(state == eEOF) state = eType;
		sLine = buff;
		sLine.Trim();
retry:
		if(state == eType) {
			if(sLine=="[ENTITY]") {
				sptType = tEntity;
				state = eName;
			} else if(sLine=="[BACKGROUND]") {
				sptType = tBackground;
				state = eName;
			} else {
				sptType = tMask;
				state = eName;
				goto retry;
			}
		} else if(state == eName) {
			pSprite = m_pProjectManager->ReferSprite(sLine, sptType);
			if(!pSprite) {
				state = eError;
				printf("Sprite error[1] in '%s'(%d)\n", m_fnSheetFile.GetFileName(), m_iLines);
			}
			else {
				if(pSprite->m_bDefined) {
					state = eError;
					printf("Sprite '%s' already defined in '%s'\n", pSprite->m_sName, pSprite->m_pSpriteSheet->m_sSheetName);
					printf("\t'%s' redefinition attempt in '%s'\n", pSprite->m_sName, m_sSheetName);
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
			pSprite->m_iAnimSpd = atoi(sLine);
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
						printf("Sprite error[2] in '%s'(%d)\n", m_fnSheetFile.GetFileName(), m_iLines);
					} else {
						pSprite->m_Boundaries.Add(rcRect);
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
			if(	pSprite->m_SptType == tBackground ||
				pSprite->m_SptType == tEntity) {
				if(sLine!="") { // there is a mask for this sprite
					static_cast<CBackground*>(pSprite)->m_pMaskMap =
						static_cast<CMaskMap*>(m_pProjectManager->ReferSprite(sLine, tMask));
					if(!static_cast<CBackground*>(pSprite)->m_pMaskMap) {
						state = eError;
						printf("Sprite error[3] in '%s'(%d)\n", m_fnSheetFile.GetFileName(), m_iLines);
					}
				}
			} else goto retry;
		} else if(state == eLayer) {
			state = eAlpha;
			if(	pSprite->m_SptType == tBackground ||
				pSprite->m_SptType == tEntity ) {
					static_cast<CBackground*>(pSprite)->m_sSubLayer = sLine;
			} else goto retry;
		} else if(state == eAlpha) {
			state = eDefine;
			if(	pSprite->m_SptType == tBackground ||
				pSprite->m_SptType == tEntity ) {
					static_cast<CBackground*>(pSprite)->m_cAlphaValue = atoi(sLine);
			} else goto retry;
		} else if(state == eDefine) {
			state = eEnd;
			if(pSprite->m_SptType == tEntity ) {
				static_cast<CEntity*>(pSprite)->m_pScript =
					m_pProjectManager->DefineScript(pSprite->m_sName);
				if(!static_cast<CEntity*>(pSprite)->m_pScript) {
					state = eError;
					printf("Sprite error[4] in '%s'(%d)\n", m_fnSheetFile.GetFileName(), m_iLines);
				}
			}
			if(state != eError) {
				if(m_pProjectManager->DefineSprite(pSprite->m_sName, sptType, this) == NULL) state = eError;
			}
		}
		if(sLine=="[EE]") break;
	}
	return (int)state;
}

CString CScript::GetScriptFile()
{
	return m_fnScriptFile.GetFilePath();
}
CString CScript::GetAmxFile()
{
	return m_fnScriptFile.GetPath() + "amx\\" + m_fnScriptFile.GetFileTitle() + ".amx";
}

bool CScript::NeedsToCompile()
{
	//m_fnScriptFile.GetFilePath()
	return true;
}
