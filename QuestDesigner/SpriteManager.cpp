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

	CRect Rect;
	scontext->GetAbsFinalRect(Rect);
	pGraphics->Render(pTexture, m_Boundaries[scontext->m_nFrame], Rect, scontext->Rotation(), scontext->Transformation(), scontext->getAlpha(), &(scontext->m_pBuffer));
	if(!scontext->isTiled()) {
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

CSpriteContext::CSpriteContext(LPCSTR szName) : CNamedObj(szName), CDrawableContext(), m_nFrame(0) {
	Mirror(false);
	Flip(false);
	Alpha(255);
	Rotate(SROTATE_0);
	Tile(false);
}
inline void CSpriteContext::Mirror() 
{
	if(isMirrored()) Mirror(false);
	else Mirror(true);
}
inline void CSpriteContext::Flip() 
{
	if(isFlipped()) Flip(false);
	else Flip(true);
}
inline void CSpriteContext::Mirror(bool bMirror) 
{
	if(bMirror) m_dwStatus |= (SMIRRORED<<_SPT_TRANSFORM);
	else		m_dwStatus &= ~(SMIRRORED<<_SPT_TRANSFORM);
}
inline void CSpriteContext::Flip(bool bFlip) 
{
	if(bFlip)	m_dwStatus |= (SFLIPPED<<_SPT_TRANSFORM);
	else		m_dwStatus &= ~(SFLIPPED<<_SPT_TRANSFORM);
}
inline void CSpriteContext::Alpha(int alpha) 
{
	m_dwStatus &= ~SPT_ALPHA;
	m_dwStatus |= ((alpha<<_SPT_ALPHA)&SPT_ALPHA);
}
inline void CSpriteContext::Rotate(int rotate) 
{
	m_dwStatus &= ~SPT_ROT;
	m_dwStatus |= ((rotate<<_SPT_ROT)&SPT_ROT);
}
inline void CSpriteContext::Tile(bool bTile) 
{
	if(!bTile)	m_dwStatus |= (SNTILED<<_SPT_INFO);
	else		m_dwStatus &= ~(SNTILED<<_SPT_INFO);
}
inline bool CSpriteContext::isTiled() const
{
	return !((m_dwStatus&(SNTILED<<_SPT_INFO))==(SNTILED<<_SPT_INFO));
}
inline bool CSpriteContext::isMirrored() const
{
	return ((m_dwStatus&(SMIRRORED<<_SPT_TRANSFORM))==(SMIRRORED<<_SPT_TRANSFORM));
}
inline bool CSpriteContext::isFlipped() const
{
	return ((m_dwStatus&(SFLIPPED<<_SPT_TRANSFORM))==(SFLIPPED<<_SPT_TRANSFORM));
}
inline int CSpriteContext::getAlpha() const
{
	return ((m_dwStatus&SPT_ALPHA)>>_SPT_ALPHA);
}
inline int CSpriteContext::Transformation() const 
{
	return ((m_dwStatus&SPT_TRANSFORM)>>_SPT_TRANSFORM);
}
inline int CSpriteContext::Rotation() const
{
	return ((m_dwStatus&SPT_ROT)>>_SPT_ROT);
}

void CSpriteSelection::BuildRealSelectionBounds()
{
	m_rcSelection.SetRectEmpty();
	// We need to keep the initial size and location of every selected object:
	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		CRect Rect;
		(m_Objects.GetKeyAt(i))->GetAbsFinalRect(Rect);
		(m_Objects.GetValueAt(i)) = Rect;
		m_rcSelection.UnionRect(m_rcSelection, Rect);
	}
}

void CSpriteSelection::Draw(const IGraphics *pGraphics_) {
	CRect Rect(0,0,0,0);
	CRect RectTmp;

	const CSpriteContext *scontext = NULL;

	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		scontext = static_cast<const CSpriteContext*>(m_Objects.GetKeyAt(i));
		scontext->GetAbsFinalRect(RectTmp);
		Rect.UnionRect(Rect, RectTmp);
		if(nObjects>1) {
			pGraphics_->FillRect(RectTmp,25,255,255,225);
			pGraphics_->BoundingBox(RectTmp, 80,255,255,225);
		}
	}
	m_bCanMove = true;
	m_bCanResize = true;
	if(nObjects>1) {
		pGraphics_->SelectionBox(Rect, 255, 255, 255, 200);
	} else if(nObjects==1) {
		if(scontext->isTiled()) pGraphics_->SelectionBox(Rect, 200, 255, 255, 200);
		else {
			m_bCanResize = false;
			pGraphics_->BoundingBox(Rect, 225, 255, 255, 200);
		}
	}

	if(m_eCurrentState==eSelecting) {
		RectTmp = m_rcSelection;
		RectTmp.NormalizeRect();
		if(RectTmp.Width()>1 && RectTmp.Height()>1) {
			pGraphics_->BoundingBox(m_rcSelection, 128, 0, 0, 0);
		}
	}
}

// this function expects normalized rects
void CSpriteSelection::ResizeObject(CDrawableContext *Object, const CRect &rcObject_, const CRect &rcOldBounds_, const CRect &rcNewBounds_, bool bAllowResize_)
{
	const CSpriteContext *scontext = static_cast<const CSpriteContext*>(Object);
	if(!scontext->isTiled()) bAllowResize_ = false;

	int w = rcObject_.Width();
	int h = rcObject_.Height();

	float xFactor=1.0f, yFactor=1.0f;
	if(rcOldBounds_.Width()) xFactor = (float)rcNewBounds_.Width()/(float)rcOldBounds_.Width();
	if(rcOldBounds_.Height()) yFactor = (float)rcNewBounds_.Height()/(float)rcOldBounds_.Height();

	CRect Rect;
	if(m_bCursorLeft) Rect.right = (rcOldBounds_.left+rcNewBounds_.Width()) - ((float)(rcOldBounds_.right-rcObject_.right) * xFactor);
	else Rect.left = rcOldBounds_.left + ((float)(rcObject_.left-rcOldBounds_.left) * xFactor);
	if(m_bCursorTop) Rect.bottom = (rcOldBounds_.top+rcNewBounds_.Height()) - ((float)(rcOldBounds_.bottom-rcObject_.bottom) * yFactor);
	else Rect.top = rcOldBounds_.top + ((float)(rcObject_.top-rcOldBounds_.top) * yFactor);

	if(bAllowResize_) {
		if(m_bCursorLeft) Rect.left = rcOldBounds_.left + ((float)(rcObject_.left-rcOldBounds_.left) * xFactor);
		else Rect.right = (rcOldBounds_.left+rcNewBounds_.Width()) - ((float)(rcOldBounds_.right-rcObject_.right) * xFactor);
		if(m_bCursorTop) Rect.top = rcOldBounds_.top + ((float)(rcObject_.top-rcOldBounds_.top) * yFactor);
		else Rect.bottom = (rcOldBounds_.top+rcNewBounds_.Height()) - ((float)(rcOldBounds_.bottom-rcObject_.bottom) * yFactor);
	} else {
		if(m_bCursorLeft) Rect.left  = Rect.right - w;
		else Rect.right  = Rect.left + w;
		if(m_bCursorTop) Rect.top = Rect.bottom - h;
		else Rect.bottom = Rect.top + h;
	}

	Rect.OffsetRect(rcNewBounds_.left-rcOldBounds_.left, rcNewBounds_.top-rcOldBounds_.top);
	Rect.NormalizeRect();

	if(!bAllowResize_) {
		if(m_bCursorLeft) if(Rect.left>rcNewBounds_.right-w) Rect.left = rcNewBounds_.right - w;
		if(m_bCursorTop) if(Rect.top>rcNewBounds_.bottom-h) Rect.top = rcNewBounds_.bottom - h;
		Rect.right  = Rect.left + w;
		Rect.bottom = Rect.top + h;
	}

	w = Rect.Width();
	h = Rect.Height();
	Rect.left = m_nSnapSize*(Rect.left/m_nSnapSize);
	if(bAllowResize_) {
		if(Rect.Width() == 0) Rect.left--;
		if(Rect.left<rcNewBounds_.left) Rect.left = rcNewBounds_.left;
		Rect.right = m_nSnapSize*((Rect.left+w+m_nSnapSize-1)/m_nSnapSize);
		if(Rect.Width() == 0) Rect.right++;
		if(Rect.right>rcNewBounds_.right) Rect.right = rcNewBounds_.right;
	} else {
		Rect.right = Rect.left + w;
	}

	Rect.top = m_nSnapSize*(Rect.top/m_nSnapSize);
	if(bAllowResize_) {
		if(Rect.Height() == 0) Rect.top--;
		if(Rect.top<rcNewBounds_.top) Rect.top = rcNewBounds_.top;
		Rect.bottom = m_nSnapSize*((Rect.top+h+m_nSnapSize-1)/m_nSnapSize);
		if(Rect.Height() == 0) Rect.bottom++;
		if(Rect.bottom>rcNewBounds_.bottom) Rect.bottom = rcNewBounds_.bottom;
	} else {
		Rect.bottom = Rect.top + h;
	}

	Object->SetAbsFinalRect(Rect);
}

void CSpriteSelection::FlipSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(rcBoundaries);

	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(m_Objects.GetKeyAt(i));
		scontext->GetAbsFinalRect(RectTmp);
		Rect.left = RectTmp.left;
		Rect.right = RectTmp.right;
		Rect.top = rcBoundaries.top + (rcBoundaries.bottom - RectTmp.bottom);
		Rect.bottom = rcBoundaries.bottom - (RectTmp.top - rcBoundaries.top);
		Rect.NormalizeRect();

		scontext->Flip();
		scontext->SetAbsFinalRect(Rect);
	}
}
void CSpriteSelection::MirrorSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(rcBoundaries);

	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(m_Objects.GetKeyAt(i));
		scontext->GetAbsFinalRect(RectTmp);
		Rect.left = rcBoundaries.left + (rcBoundaries.right - RectTmp.right);
		Rect.right = rcBoundaries.right - (RectTmp.left - rcBoundaries.left);
		Rect.top = RectTmp.top;
		Rect.bottom = RectTmp.bottom;
		Rect.NormalizeRect();

		scontext->Mirror();
		scontext->SetAbsFinalRect(Rect);
	}
}
void CSpriteSelection::CWRotateSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(rcBoundaries);

	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(m_Objects.GetKeyAt(i));
		scontext->GetAbsFinalRect(RectTmp);
		Rect.top = rcBoundaries.top + (RectTmp.left - rcBoundaries.left);
		Rect.bottom = rcBoundaries.top + (RectTmp.right - rcBoundaries.left);
		Rect.left = rcBoundaries.left + (rcBoundaries.bottom - RectTmp.bottom);
		Rect.right = rcBoundaries.left + (rcBoundaries.bottom - RectTmp.top);

		Rect.NormalizeRect();

		scontext->Rotate((scontext->Rotation()-1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
		scontext->SetAbsFinalRect(Rect);
	}
}
void CSpriteSelection::CCWRotateSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(rcBoundaries);

	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(m_Objects.GetKeyAt(i));
		scontext->GetAbsFinalRect(RectTmp);

		Rect.top = rcBoundaries.top + (rcBoundaries.right - RectTmp.right);
		Rect.left = rcBoundaries.left + (RectTmp.top - rcBoundaries.top);
		Rect.right = rcBoundaries.left + (RectTmp.bottom - rcBoundaries.top);
		Rect.bottom = rcBoundaries.top + (rcBoundaries.right - RectTmp.left);

		Rect.NormalizeRect();

		scontext->Rotate((scontext->Rotation()+1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
		scontext->SetAbsFinalRect(Rect);
	}
}
