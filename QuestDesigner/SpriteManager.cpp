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
	if(m_bDefined) return m_pSpriteSheet; 
	return NULL;
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

bool CBackground::NeedToDraw(const CDrawableContext &scontext) 
{ 
	if(m_bDefined) {
		IGraphics *pGraphics = scontext.GetGraphicsDevice();
		CRect rcLocation;
		CRect rcVisible = pGraphics->GetVisibleRect();
		scontext.GetAbsRect(rcLocation);
		rcLocation.IntersectRect(rcVisible, rcLocation);
		if(!rcLocation.IsRectEmpty()) return true;
	}
	return false; 
}
bool CBackground::Draw(CDrawableContext &context) 
{ 
	CSpriteContext *scontext = static_cast<CSpriteContext*>(&context);
	CSpriteSheet *pSpriteSheet = GetSpriteSheet();
	if(!pSpriteSheet) return false;

	IGraphics *pGraphics = context.GetGraphicsDevice();
	// We handle texture stuff
	ITexture *pTexture = NULL;
	if(pSpriteSheet->m_pTexture != NULL) {
		if(pSpriteSheet->m_pTexture->GetTexture() && pSpriteSheet->m_pTexture->GetDeviceID() == pGraphics->GetDeviceID()) {
			pTexture = pSpriteSheet->m_pTexture;
		} 
	}
	if(pTexture == NULL) {
		// We make use of lazy evaluation here to load the textures.
		CFileName fn = pSpriteSheet->GetFileName();
		CString str;
		str.Format("%s%s.bmp", fn.GetPath(), fn.GetFileTitle());
		pGraphics->CreateTextureFromFile(str, &pTexture);
		if(!pTexture) return false;
		if(pSpriteSheet->m_pTexture) pSpriteSheet->m_pTexture->Release();
		pSpriteSheet->m_pTexture = pTexture->AddRef();
	}

	if(m_pSpriteData->iAnimSpd && pGraphics->GetCurrentZoom()>=1) { // fps
		int nFrame = ( (m_pSpriteData->iAnimSpd * GetTickCount())/1000 ) % m_Boundaries.GetSize();
		if(scontext->m_pBuffer && scontext->m_nFrame!=nFrame) {
			scontext->m_pBuffer->Invalidate();
			scontext->m_nFrame = nFrame;
		}
	}

	if(scontext->isTiled()) {
		CRect Position;
		scontext->GetAbsRect(Position);
		pGraphics->Render(pTexture, m_Boundaries[scontext->m_nFrame], Position, scontext->Rotation(), scontext->Transformation(), scontext->getAlpha(), &(scontext->m_pBuffer));
	} else {
		CPoint Position;
		scontext->GetAbsPosition(Position);
		pGraphics->Render(pTexture, m_Boundaries[scontext->m_nFrame], Position, scontext->Rotation(), scontext->Transformation(), scontext->getAlpha(), &(scontext->m_pBuffer));
	}
	CRect Rect;
	scontext->GetAbsRect(Rect);
	if((scontext->Rotation() == SROTATE_90 ||scontext->Rotation() == SROTATE_270) && !scontext->isTiled()) {
		int w = Rect.Width();
		int h = Rect.Height();
		Rect.bottom = Rect.top+w;
		Rect.right = Rect.left+h;;
	}

	//pGraphics->DrawRect(Rect,128,255,255,255,1);
/*
		if(scontext->isTiled())
			pGraphics->SelectionBox(Rect,200,255,255,225);
		else
			pGraphics->BoundingBox(Rect,255,0,0,0);
/**/
	return true; 
}

CMaskMap::CMaskMap(LPCSTR szName) :
	CSprite(szName)
{
	m_SptType = tMask;
}

// Srite Sheets
CSpriteSheet::CSpriteSheet(CProjectManager *pProjectManager) :
	CNamedObj(""),
	m_pProjectManager(pProjectManager),
	m_pTexture(NULL)
{
	m_ArchiveIn = new CSpriteSheetTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
CSpriteSheet::~CSpriteSheet()
{
	if(m_pTexture) m_pTexture->Release();
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

bool CScript::NeedToCompile()
{
	//m_fnScriptFile.GetFilePath()
	return true;
}
