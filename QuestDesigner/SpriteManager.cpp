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

#include "ArchiveText.h"

CSprite::CSprite(LPCSTR szName) :
	CNamedObj(szName),
	m_bDefined(false),
	m_SptType(tUnknown),
	m_pSpriteSheet(NULL),
	m_pSpriteData(NULL)
{
}
CSprite::~CSprite() 
{
	delete m_pSpriteData;
}
_spt_type CSprite::GetSpriteType() 
{ 
	return m_SptType; 
}
CSpriteSheet* CSprite::GetSpriteSheet() 
{ 
	return m_pSpriteSheet; 
}
bool CSprite::IsDefined() 
{ 
	return m_bDefined; 
}
void CSprite::SetSpriteSheet(CSpriteSheet *pSpriteSheet) 
{
	m_pSpriteSheet = pSpriteSheet;
	if(m_pSpriteSheet) m_bDefined = true;
}
void CSprite::AddRect(RECT rcRect) 
{
	m_Boundaries.Add(rcRect);
}
void CSprite::SetSpriteData(SSpriteData *pSpriteData) {
	m_pSpriteData = pSpriteData;
}

CEntity::CEntity(LPCSTR szName) :
	CBackground(szName)
{
	m_SptType = tEntity;
}
CBackground::CBackground(LPCSTR szName) :
	CSprite(szName)
{
	m_SptType = tBackground;
}
CMaskMap::CMaskMap(LPCSTR szName) :
	CSprite(szName)
{
	m_SptType = tMask;
}

// Srite Sheets
CSpriteSheet::CSpriteSheet(CProjectManager *pProjectManager) :
	CNamedObj(""),
	m_pProjectManager(pProjectManager)
{
	m_ArchiveIn = new CSpriteSheetTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
CSpriteSheet::~CSpriteSheet()
{
	while(m_Sprites.GetSize()) {
		delete m_Sprites.GetValueAt(0);
		m_Sprites.RemoveAt(0);
	}
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
