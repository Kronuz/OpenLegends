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
/*! \file		SpriteManager.cpp
	\brief		Implementation of the sprite classes.
	\date		April 17, 2003

	This file implements all the classes that manage the sprites,
	including backgrounds, sprite sheets, animations, mask maps and
	entities.
*/

#include "stdafx.h"

#include "SpriteManager.h"
#include "GameManager.h"
#include "ScriptManager.h"

#include "ArchiveText.h"

bool g_bBounds = false;
bool g_bMasks = false;
bool g_bEntities = false;

CSprite::CSprite(LPCSTR szName) :
	m_bDefined(false),
	m_pSpriteSheet(NULL),
	m_pSpriteData(NULL),
	CDrawableObject(szName)
{
}
CSprite::~CSprite() 
{
	delete m_pSpriteData;
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
		const IGraphics *pGraphics = scontext.GetGraphicsDevice();
		CRect rcLocation;
		CRect rcVisible = pGraphics->GetVisibleRect();
		scontext.GetAbsRect(rcLocation);
		rcLocation.IntersectRect(rcVisible, rcLocation);
		if(!rcLocation.IsRectEmpty()) return true;
	}
	return false; 
}
inline bool CSprite::Draw(const CDrawableContext &context, bool bBounds, ARGBCOLOR *rgbColorOverride, int nBuffer)
{
	CSpriteSheet *pSpriteSheet = GetSpriteSheet();
	ASSERT(pSpriteSheet);

	const CSpriteContext *scontext = static_cast<const CSpriteContext*>(&context);

	const IGraphics *pGraphics = context.GetGraphicsDevice();
	// We handle texture stuff
	ITexture *pTexture = NULL;
	if(pSpriteSheet->m_pTexture != NULL) {
		if(pSpriteSheet->m_pTexture->GetTexture() && pSpriteSheet->m_pTexture->GetDeviceID() == pGraphics->GetDeviceID()) {
			pTexture = pSpriteSheet->m_pTexture;
		} 
	}
	if(pTexture == NULL) {
		// We make use of lazy evaluation here to load the textures.
		CVFile fn = pSpriteSheet->GetFileName();
		float scale = 1.0f;
		fn.SetFileExt(".png");
		if(!fn.FileExists()) {
			scale = 2.0f;
			fn.SetFileExt(".bmp");
			CONSOLE_PRINTF("Kernel Warning: Couldn't find Sprite Sheet from PNG file,\n    trying to load from '%s' instead.\n", fn.GetFileName());
			if(!fn.FileExists()) {
				CONSOLE_PRINTF("Kernel Error: Couldn't find Sprite Sheet bitmap for '%s.spt'.\n", fn.GetFileTitle());
				return false;
			}
		}
		if(fn.Open()) {
			int filesize = fn.GetFileSize();
			LPCVOID pData = fn.ReadFile();
			if(!pData) {
				CONSOLE_PRINTF("Kernel Fatal Error: Not enough memory to hold %d bytes!\n", fn.GetFileSize());
			} else pGraphics->CreateTextureFromFileInMemory(fn.GetFileName(), pData, filesize, &pTexture, scale);
			fn.Close();
		}
		if(!pTexture) {
			CONSOLE_PRINTF("Kernel Error: Couldn't open Sprite Sheet bitmap for '%s.spt'.\n", fn.GetFileTitle());
			return false;
		}
		if(pSpriteSheet->m_pTexture) pSpriteSheet->m_pTexture->Release();
		pSpriteSheet->m_pTexture = pTexture->AddRef();
	}

	// Initialize the first frame
	int nFrame = scontext->m_nFrame[nBuffer];
	if(m_pSpriteData->eAnimDir != _d_down) {
		if(nFrame == -1) nFrame = 0;
	} else {
		if(nFrame == -1) nFrame = m_Boundaries.size()-1;
	}

	if( m_pSpriteData->iAnimSpd && 
		CGameManager::GetPauseLevel() == 0 && 
		pGraphics->GetCurrentZoom() >= 0.5f ) { // fps

		int TotalFrames = m_Boundaries.size();
		if(m_pSpriteData->eAnimDir != _d_down) {
			// How long would have taken to play all remaining frames since last render?
			// Less than the time it took to get here?
			float AnimTime = (float)(TotalFrames - nFrame) / (float)m_pSpriteData->iAnimSpd;
			int nTmp = ((m_pSpriteData->iAnimSpd * CGameManager::GetLastTick()/1000) % TotalFrames);
			if((nTmp < nFrame || AnimTime < CGameManager::GetFPSDelta()) && !m_pSpriteData->bAnimLoop) {
				nTmp = TotalFrames-1;
			} 
			nFrame = nTmp;
		} else {
			float AnimTime = (float)(nFrame + 1) / (float)m_pSpriteData->iAnimSpd;
			int nTmp = (TotalFrames-1) - ((m_pSpriteData->iAnimSpd * CGameManager::GetLastTick()/1000) % TotalFrames);
			if((nTmp > nFrame || AnimTime < CGameManager::GetFPSDelta()) && !m_pSpriteData->bAnimLoop) {
				nTmp = 0;
			}
			nFrame = nTmp;
		}

		if(context.m_pBuffer[nBuffer] && scontext->m_nFrame[nBuffer]!=nFrame) {
			if( m_Boundaries[nFrame].Width() != m_Boundaries[scontext->m_nFrame[nBuffer]].Width() ||
				m_Boundaries[nFrame].Height() != m_Boundaries[scontext->m_nFrame[nBuffer]].Height()) {
				context.m_pBuffer[nBuffer]->Invalidate(); // the next frame has different size, invalidate.
			} else {
				context.m_pBuffer[nBuffer]->Touch(); // same size, the same buffer can be used, just touch.
			}
		}/**/
	}
	scontext->m_nFrame[nBuffer] = nFrame;

	ARGBCOLOR rgbColor;
	if(!rgbColorOverride) {
		rgbColor = COLOR_ARGB(scontext->getAlpha(),255,255,255);
		rgbColorOverride = &rgbColor;
	}

	CRect Rect;
	scontext->GetAbsFinalRect(Rect);
	pGraphics->Render(pTexture,							// texture
		m_Boundaries[scontext->m_nFrame[nBuffer]],		// rectSrc
		Rect,											// rectDest
		scontext->Rotation(),							// rotation
		scontext->Transformation(),						// transform
		*rgbColorOverride,								// rgbColor
		&(context.m_pBuffer[nBuffer]),					// buffer
		scontext->RelRotation(),						// relarive rotation
		scontext->RelScale()							// relative scale
		/*/(( (20 * GetTickCount())/1000 ) % 360) * 0.01745329252f, // just for testing
		(float)(((10 * GetTickCount())/1000 ) % 50) / 10 /**/
	);

	if(bBounds) pGraphics->BoundingBox(Rect, COLOR_ARGB(255,0,0,0));

	return true;
}
bool CBackground::Draw(const CDrawableContext &context) 
{ 
	if(!NeedToDraw(context)) return true;

	if(m_SptType == tMask) {
		ARGBCOLOR rgbColor = COLOR_ARGB(200,255,255,255);
		if(!CSprite::Draw(context, false, &rgbColor, 1)) {
			return false;
		}
	} else {
		if(!CSprite::Draw(context, g_bBounds, NULL, 0)) {
			return false;
		}

		if(g_bMasks) {
			const SBackgroundData *pBackgroundData = static_cast<const SBackgroundData*>(m_pSpriteData);
			if(pBackgroundData->pMaskMap)
				pBackgroundData->pMaskMap->Draw(context);
		}
	}
	return true; 
}
bool CEntity::Draw(const CDrawableContext &context) 
{ 
	if(g_bEntities)
		return CBackground::Draw(context); 
	return true;
}

bool CEntity::Run(const CDrawableContext &context, RUNACTION action)
{
	const SEntityData *pEntityData = static_cast<const SEntityData*>(m_pSpriteData);
	pEntityData->pScript->RunScript(context, action);
	
	return true;
}

// Srite Sheets
CSpriteSheet::CSpriteSheet(CGameManager *pGameManager) :
	CNamedObj(""),
	m_pGameManager(pGameManager),
	m_pTexture(NULL)
{
	m_ArchiveIn = new CSpriteSheetTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
CSpriteSheet::~CSpriteSheet()
{
	if(m_pTexture) m_pTexture->Release();

	std::map<CBString, CSprite*>::iterator Iterator = m_Sprites.begin();
	while(Iterator != m_Sprites.end()) {
		delete Iterator->second;
		Iterator++;
	}
	m_Sprites.clear();
}
int CSpriteSheet::ForEachSprite(FOREACHPROC ForEach, LPARAM lParam)
{
	int cnt = 0;
	std::map<CBString, CSprite*>::iterator Iterator = m_Sprites.begin();
	while(Iterator != m_Sprites.end()) {
		ASSERT(Iterator->second);
		int aux = ForEach((LPVOID)(Iterator->second), lParam);
		if(aux < 0) return aux-cnt;
		cnt += aux;
		Iterator++;
	}
	return cnt;
}
bool CSpriteContext::GetInfo(SInfo *pI) const 
{
	ASSERT(m_pDrawableObj);
	_spt_type eType = static_cast<CSprite *>(m_pDrawableObj)->GetSpriteType();
		 if(eType == tMask) pI->eType = itMask;
	else if(eType == tBackground) pI->eType = itBackground;
	else if(eType == tEntity) pI->eType = itEntity;

	strncpy(pI->szName, GetName(), 29);
	strncpy(pI->szScope, m_pDrawableObj->GetName(), 29);
	pI->pPropObject = (IPropertyEnabled*)this;
	return true;
}
bool CSpriteContext::GetProperties(SPropertyList *pPL) const 
{
	ASSERT(m_pDrawableObj);
	ASSERT(pPL->nProperties == 0);
	GetInfo(&pPL->Information);

	CRect Rect;
	pPL->AddCategory("Appearance");
	if(pPL->Information.eType == itEntity) pPL->AddString("Name", GetName());
	GetAbsRect(Rect);
	pPL->AddValue("X", Rect.left);
	pPL->AddValue("Y", Rect.top);
	if(pPL->Information.eType == itEntity) {
		pPL->AddValue("Width", Rect.Width(), false);
		pPL->AddValue("Height", Rect.Height(), false);
		pPL->AddBoolean("IsMirrored", isMirrored(), false);
		pPL->AddBoolean("IsFlipped", isFlipped(), false);
		pPL->AddList("Rotation", Rotation(), "0 degrees, 90 degrees, 180 degrees, 270 degrees", false);
	} else {
		pPL->AddValue("Width", Rect.Width());
		pPL->AddValue("Height", Rect.Height());
		pPL->AddBoolean("IsMirrored", isMirrored());
		pPL->AddBoolean("IsFlipped", isFlipped());
		pPL->AddList("Rotation", Rotation(), "0 degrees, 90 degrees, 180 degrees, 270 degrees");
	}

	pPL->AddCategory("Misc");
	pPL->AddList("Layer", GetObjLayer(), "0 - Ground, 1 - First Level, 2 - Second Level, 3 - Third Level, 4 - Fourth Level, 5 - Fifth Level ");
	pPL->AddList("SubLayer", GetObjSubLayer(), "0 - Background, 1 - Sprites, 2 - Entities, 3 - Top Background, 4 - Top Sprites, 5 - Top Entities");
	pPL->AddString("Sprite Sheet", static_cast<CSprite*>(m_pDrawableObj)->GetSpriteSheet()->GetName(), false);
	
	pPL->AddCategory("Behavior");
	pPL->AddBoolean("IsVisible", isVisible(), false);

	return true;
}
bool CSpriteContext::SetProperties(SPropertyList &PL) 
{
	SProperty* pP;

	CRect Rect;
	GetAbsRect(Rect);

	pP = PL.FindProperty("X", SProperty::ptValue);
	if(pP) if(pP->bEnabled) Rect.left = (_Chain)pP->nValue;
	
	pP = PL.FindProperty("Y", SProperty::ptValue);
	if(pP) if(pP->bEnabled) Rect.top = (_Chain)pP->nValue;

	pP = PL.FindProperty("Width", SProperty::ptValue);
	if(pP) if(pP->bEnabled) Rect.right = Rect.left + (_Chain)pP->nValue;

	pP = PL.FindProperty("Height", SProperty::ptValue);
	if(pP) if(pP->bEnabled) Rect.bottom = Rect.top + (_Chain)pP->nValue;

	SetAbsRect(Rect);

	pP = PL.FindProperty("IsMirrored", SProperty::ptBoolean);
	if(pP) if(pP->bEnabled) Mirror((_Chain)pP->bBoolean);

	pP = PL.FindProperty("IsFlipped", SProperty::ptBoolean);
	if(pP) if(pP->bEnabled) Flip((_Chain)pP->bBoolean);
	
	pP = PL.FindProperty("Rotation", SProperty::ptList);
	if(pP) if(pP->bEnabled) Rotate((_Chain)pP->nIndex);

	pP = PL.FindProperty("SubLayer", SProperty::ptList);
	if(pP) if(pP->bEnabled) SetObjSubLayer((_Chain)pP->nIndex);

	pP = PL.FindProperty("Layer", SProperty::ptList);
	if(pP) if(pP->bEnabled) SetObjLayer((_Chain)pP->nIndex);

	return true;
}

CSpriteContext::CSpriteContext(LPCSTR szName) : 
	CDrawableContext(szName)
{
	memset(m_nFrame, -1, sizeof(m_nFrame));
	Mirror(false);
	Flip(false);
	Alpha(255);
	Rotate(SROTATE_0);
	Tile(false);
}

void CSpriteSelection::BuildRealSelectionBounds()
{
	m_rcSelection.SetRectEmpty();
	// We need to keep the initial size and location of every selected object:

	mapObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		CRect Rect;
		Iterator->first->GetAbsFinalRect(Rect);
		Iterator->second.rcRect = Rect;
		m_rcSelection.UnionRect(m_rcSelection, Rect);
		Iterator++;
	}
}
SObjProp* CSpriteSelection::GetFirstSelection()
{
	m_CurrentSel = m_Objects.begin();
	if(m_CurrentSel == m_Objects.end()) return NULL;
	return &(m_CurrentSel++)->second;
}
SObjProp* CSpriteSelection::GetNextSelection() 
{
	if(m_CurrentSel == NULL) return NULL;
	if(m_CurrentSel == m_Objects.end()) return NULL;
	return &(m_CurrentSel++)->second;
}

bool CSpriteSelection::Draw(const IGraphics *pGraphics_) {
	CRect Rect(0,0,0,0);
	CRect RectTmp;

	int nObjects = (int)m_Objects.size();
	const CSpriteContext *scontext = NULL;

	mapObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		scontext = static_cast<const CSpriteContext*>(Iterator->first);
		scontext->GetAbsFinalRect(RectTmp);
		Rect.UnionRect(Rect, RectTmp);
		if(nObjects>1) {
			pGraphics_->FillRect(RectTmp, COLOR_ARGB(25,255,255,225));
			pGraphics_->BoundingBox(RectTmp, COLOR_ARGB(80,255,255,225));

			CRect rcSpecial, rcArrow;
			rcSpecial = RectTmp;
			if(Iterator->second.eXChain == left) {
				rcSpecial.right = rcSpecial.left+1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
			} else if(Iterator->second.eXChain == right) {
				rcSpecial.left = rcSpecial.right-1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
			} else if(Iterator->second.eXChain == fixed) {
				rcSpecial.right = rcSpecial.left+1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
				rcSpecial = RectTmp;
				rcSpecial.left = rcSpecial.right-1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
			} else if(Iterator->second.eXChain == stretch) {
				rcSpecial.top = rcSpecial.top+rcSpecial.Height()/2;
				rcSpecial.bottom = rcSpecial.top+1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));

				rcArrow = rcSpecial;
				rcArrow.bottom++;
				rcArrow.top--;
				rcArrow.left++;
				rcArrow.right = rcArrow.left+1;
				pGraphics_->FillRect(rcArrow, COLOR_ARGB(255,0,0,255));

				rcArrow = rcSpecial;
				rcArrow.bottom++;
				rcArrow.top--;
				rcArrow.right--;
				rcArrow.left = rcArrow.right-1;
				pGraphics_->FillRect(rcArrow, COLOR_ARGB(255,0,0,255));
			}

			rcSpecial = RectTmp;
			if(Iterator->second.eYChain == up) {
				rcSpecial.bottom = rcSpecial.top+1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
			} else if(Iterator->second.eYChain == down) {
				rcSpecial.top = rcSpecial.bottom-1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
			} else if(Iterator->second.eYChain == fixed) {
				rcSpecial.bottom = rcSpecial.top+1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
				rcSpecial = RectTmp;
				rcSpecial.top = rcSpecial.bottom-1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
			} else if(Iterator->second.eYChain == stretch) {
				rcSpecial.left = rcSpecial.left+rcSpecial.Width()/2;
				rcSpecial.right = rcSpecial.left+1;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));

				rcArrow = rcSpecial;
				rcArrow.right++;
				rcArrow.left--;
				rcArrow.top++;
				rcArrow.bottom = rcArrow.top+1;
				pGraphics_->FillRect(rcArrow, COLOR_ARGB(255,0,0,255));

				rcArrow = rcSpecial;
				rcArrow.right++;
				rcArrow.left--;
				rcArrow.bottom--;
				rcArrow.top = rcArrow.bottom-1;
				pGraphics_->FillRect(rcArrow, COLOR_ARGB(255,0,0,255));
			}
		}

		Iterator++;
	}
	m_bCanMove = true;
	m_bCanResize = true;

	if(nObjects>1) {
		pGraphics_->SelectionBox(Rect, COLOR_ARGB(255,255,255,200));
	} else if(nObjects==1) {
		if(scontext->isTiled()) {
			pGraphics_->SelectionBox(Rect, COLOR_ARGB(200,255,255,200));
		} else {
			m_bCanResize = false;
			pGraphics_->BoundingBox(Rect, COLOR_ARGB(255,255,255,200));
		}
	}

	if(m_eCurrentState==eSelecting) {
		RectTmp = m_rcSelection;
		RectTmp.NormalizeRect();
		if(RectTmp.Width()>1 && RectTmp.Height()>1) {
			pGraphics_->SelectingBox(m_rcSelection, COLOR_ARGB(128,255,255,255));
		}
	}
	return true;
}

// this function expects normalized rects
// simpler, yet much better ResizeObject()
void CSpriteSelection::ResizeObject(CDrawableContext *Object, const SObjProp &ObjProp_, const CRect &rcOldBounds_, const CRect &rcNewBounds_, bool bAllowResize_)
{
	const CSpriteContext *scontext = static_cast<const CSpriteContext*>(Object);
	if(!scontext->isTiled()) bAllowResize_ = false;

	int w = ObjProp_.rcRect.Width();
	int h = ObjProp_.rcRect.Height();

	float xFactor=1.0f, yFactor=1.0f;
	_Chain xChain = ObjProp_.eXChain;
	_Chain yChain = ObjProp_.eYChain;

	if(!bAllowResize_) {
		if(ObjProp_.rcRect.left == rcOldBounds_.left && ObjProp_.rcRect.right == rcOldBounds_.right) {
			if(m_bCursorLeft) xChain = right;
			else  xChain = left;
		} else if(ObjProp_.rcRect.left == rcOldBounds_.left) xChain = left;
		else if(ObjProp_.rcRect.right == rcOldBounds_.right) xChain = right;

		if(ObjProp_.rcRect.top == rcOldBounds_.top && ObjProp_.rcRect.bottom == rcOldBounds_.bottom) {
			if(m_bCursorTop) yChain = down;
			else yChain = up;
		} else if(ObjProp_.rcRect.top == rcOldBounds_.top) yChain = up;
		else if(ObjProp_.rcRect.bottom == rcOldBounds_.bottom) yChain = down;
	}

	CRect Rect;
	if(xChain == left) {
		Rect.left = rcNewBounds_.left + (ObjProp_.rcRect.left - rcOldBounds_.left);
		Rect.right = Rect.left + w;
	} else if(xChain == right) {
		Rect.right = rcNewBounds_.right - (rcOldBounds_.right - ObjProp_.rcRect.right);
		Rect.left = Rect.right - w;
	} else if(xChain == fixed || !bAllowResize_) {
		if(rcOldBounds_.Width()) xFactor = (float)rcNewBounds_.Width()/(float)rcOldBounds_.Width();
		Rect.left = rcNewBounds_.left + (int)(((float)ObjProp_.rcRect.left + (float)w/2.0f - (float)rcOldBounds_.left) * xFactor - (float)w/2.0f);
		Rect.right = Rect.left + w;
	} else if(xChain == stretch) {
		Rect.left = rcNewBounds_.left + (ObjProp_.rcRect.left - rcOldBounds_.left);
		Rect.right = rcNewBounds_.right - (rcOldBounds_.right - ObjProp_.rcRect.right);
	} else {
		if(rcOldBounds_.Width()) xFactor = (float)rcNewBounds_.Width()/(float)rcOldBounds_.Width();
		Rect.left = rcNewBounds_.left + (int)((float)(ObjProp_.rcRect.left - rcOldBounds_.left) * xFactor);
		Rect.right = rcNewBounds_.right - (int)((float)(rcOldBounds_.right - ObjProp_.rcRect.right) * xFactor);
	}

	if(yChain == up) {
		Rect.top = rcNewBounds_.top + (ObjProp_.rcRect.top - rcOldBounds_.top);
		Rect.bottom = Rect.top + h;
	} else if(yChain == down) {
		Rect.bottom = rcNewBounds_.bottom - (rcOldBounds_.bottom - ObjProp_.rcRect.bottom);
		Rect.top = Rect.bottom - h;
	} else if(yChain == fixed || !bAllowResize_) {
		if(rcOldBounds_.Height()) yFactor = (float)rcNewBounds_.Height()/(float)rcOldBounds_.Height();
		Rect.top = rcNewBounds_.top + (int)(((float)ObjProp_.rcRect.top + (float)h/2.0f - (float)rcOldBounds_.top) * yFactor - (float)h/2.0f);
		Rect.bottom = Rect.top + h;
	} else if(yChain == stretch) {
		Rect.top = rcNewBounds_.top + (ObjProp_.rcRect.top - rcOldBounds_.top);
		Rect.bottom = rcNewBounds_.bottom - (rcOldBounds_.bottom - ObjProp_.rcRect.bottom);
	} else {
		if(rcOldBounds_.Height()) yFactor = (float)rcNewBounds_.Height()/(float)rcOldBounds_.Height();
		Rect.top = rcNewBounds_.top + (int)((float)(ObjProp_.rcRect.top - rcOldBounds_.top) * yFactor);
		Rect.bottom = rcNewBounds_.bottom - (int)((float)(rcOldBounds_.bottom - ObjProp_.rcRect.bottom) * yFactor);
	}

	// Adjust non-resizable sprites:
	if(!bAllowResize_ || yChain == fixed || yChain == up || yChain == down) {
		if(m_bCursorTop) {
			if(Rect.top > rcNewBounds_.bottom-h) Rect.top = rcNewBounds_.bottom - h;
			Rect.bottom = Rect.top + h;
		} else {
			if(Rect.bottom < rcNewBounds_.top+h) Rect.bottom = rcNewBounds_.top + h;
			Rect.top = Rect.bottom - h;
		}
	}
	if(!bAllowResize_ || xChain == fixed || xChain == left || xChain == right) {
		if(m_bCursorLeft) {
			if(Rect.left > rcNewBounds_.right-w) Rect.left = rcNewBounds_.right - w;
			Rect.right  = Rect.left + w;
		} else {
			if(Rect.right < rcNewBounds_.left+w) Rect.right = rcNewBounds_.left + w;
			Rect.left  = Rect.right - w;
		}
	}

	if(rcOldBounds_.Width() != rcNewBounds_.Width()) {
		if((bAllowResize_ && xChain == relative) || (bAllowResize_ && xChain == stretch)) {
			w = Rect.Width();
			Rect.left = m_nSnapSize*(Rect.left/m_nSnapSize);
			if(Rect.Width() <= 0) Rect.left = Rect.right-m_nSnapSize;
			if(Rect.left<rcNewBounds_.left) Rect.left = rcNewBounds_.left;
			Rect.right = m_nSnapSize*((Rect.left+w+m_nSnapSize-1)/m_nSnapSize);
			if(Rect.Width() <= 0) Rect.right = Rect.left+m_nSnapSize;
			if(Rect.right>rcNewBounds_.right) Rect.right = rcNewBounds_.right;
		}
	}

	if(rcOldBounds_.Height() != rcNewBounds_.Height()) {
		if((bAllowResize_ && yChain == relative) || (bAllowResize_ && yChain == stretch)) {
			h = Rect.Height();
			Rect.top = m_nSnapSize*(Rect.top/m_nSnapSize);
			if(Rect.Height() <= 0) Rect.top = Rect.bottom-m_nSnapSize;
			if(Rect.top<rcNewBounds_.top) Rect.top = rcNewBounds_.top;
			Rect.bottom = m_nSnapSize*((Rect.top+h+m_nSnapSize-1)/m_nSnapSize);
			if(Rect.Height() <= 0) Rect.bottom = Rect.top+m_nSnapSize;
			if(Rect.bottom>rcNewBounds_.bottom) Rect.bottom = rcNewBounds_.bottom;
		}
	}

	ASSERT(Rect.right>Rect.left && Rect.bottom>Rect.top);
	Object->SetAbsFinalRect(Rect);
}

HGLOBAL CSpriteSelection::Copy()
{
	if(m_Objects.size() == 0) return NULL;

	CRect rcBoundaries, RectTmp;
	GetBoundingRect(rcBoundaries);

	int size = (int)(sizeof(_SpriteSet::_SpriteSetInfo)+sizeof(_SpriteSet::_SpriteSetData)*m_Objects.size());
	HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, size); 
	if(hglbCopy == NULL) return NULL;

 	// Lock the handle and copy the text to the buffer. 
	_SpriteSet *CopyBoard = (_SpriteSet*)::GlobalLock(hglbCopy);
	strcpy(CopyBoard->Info.ID, "Quest Designer Sprite Set");
	CopyBoard->Info.dwSignature = QUEST_SET_SIGNATURE;
	CopyBoard->Info.nSize = (int)m_Objects.size();

	CopyBoard->Info.rcBoundaries = rcBoundaries;
	CopyBoard->Info.rcBoundaries.OffsetRect(-rcBoundaries.TopLeft());
	
	_SpriteSet::_SpriteSetData *pData = (_SpriteSet::_SpriteSetData *)((char*)CopyBoard + sizeof(_SpriteSet::_SpriteSetInfo));

	mapObject::iterator Iterator = m_Objects.begin();
	for(int i=0; Iterator != m_Objects.end(); i++) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->first);
		
		scontext->GetAbsFinalRect(RectTmp);
		RectTmp.OffsetRect(-rcBoundaries.TopLeft());
		LPCSTR szName = scontext->GetObjName();

		strncpy(pData[i].szName, szName, 30);
		pData[i].Rect = RectTmp;
		pData[i].dwStatus = scontext->GetStatus();
		pData[i].nSubLayer = scontext->GetObjSubLayer();
		pData[i].nLayer = scontext->GetObjLayer();
		pData[i].nOrder = scontext->GetObjOrder();
		pData[i].XChain = Iterator->second.eXChain;
		pData[i].YChain = Iterator->second.eYChain;

		Iterator++;
	}

	::GlobalUnlock(hglbCopy); 

	return hglbCopy;
}
bool CSpriteSelection::PasteSprite(CLayer *pLayer, LPCSTR szSprite) 
{
	CSprite *pSprite = CGameManager::Instance()->FindSprite(szSprite);
	if(!pSprite) {
		CONSOLE_PRINTF("Paste error : Couldn't find the requested sprite: '%s'!\n", szSprite);
		return false;
	}
	if(pSprite->GetSpriteType() == tMask) {
		CONSOLE_PRINTF("Paste error: Attempt to use mask '%s' as a sprite\n", szSprite);
		return false;
	}
	if(!pLayer) {
		CONSOLE_PRINTF("Paste error : Couldn't paste '%s' in the requested layer!\n", szSprite);
		return false;
	}

	CSpriteContext *pSpriteContext = new CSpriteContext("");

	CSize Size;
	pSpriteContext->GetSize(Size);
	pSpriteContext->SetDrawableObj(pSprite);
	if(pSprite->GetSpriteType() == tBackground) pSpriteContext->Tile();
	pSpriteContext->SetObjSubLayer(static_cast<CBackground *>(pSprite)->GetObjSubLayer());
	pSpriteContext->Alpha(static_cast<CBackground *>(pSprite)->GetAlphaValue());

	pLayer->AddSpriteContext(pSpriteContext, true); // insert the sprite in the current layer
	pSpriteContext->MoveTo(-Size.cx/2, -Size.cy/2);

	m_Objects.clear();
	m_Objects.insert(pairObject(pSpriteContext, SObjProp(pSpriteContext)));
	return true;
}
bool CSpriteSelection::PasteObj(CLayer *pLayer, _SpriteSet::_SpriteSetData *pData)
{
	CSprite *pSprite = CGameManager::Instance()->FindSprite(pData->szName);
	if(!pSprite) {
		CONSOLE_PRINTF("Paste error : Couldn't find the requested sprite: '%s'!\n", pData->szName);
		return false;
	}
	if(pSprite->GetSpriteType() == tMask) {
		CONSOLE_PRINTF("Paste error: Attempt to use mask '%s' as a sprite\n", pData->szName);
		return false;
	}
	if(!pLayer) {
		CONSOLE_PRINTF("Paste error : Couldn't paste '%s' in the requested layer!\n", pData->szName);
		return false;
	}

	CSpriteContext *pSpriteContext = new CSpriteContext("");
	pSpriteContext->SetDrawableObj(pSprite);
	pSpriteContext->SetStatus(pData->dwStatus);
	pSpriteContext->SetObjSubLayer(pData->nSubLayer);

	pLayer->AddSpriteContext(pSpriteContext, true); // insert the sprite in the current layer
	m_Objects.insert(pairObject(pSpriteContext, SObjProp(pSpriteContext, pData->Rect, pData->XChain, pData->YChain)));

	// the sprite absolute postion must be set after inserting it in the layer.
	pSpriteContext->SetAbsFinalRect(pData->Rect);
	pSpriteContext->SelectContext();

	return true;
}
bool CSpriteSelection::Paste(LPVOID pBuffer, const CPoint &point_)
{
	CLayer *pLayer = NULL;
	UINT nLayer = -1;

	ASSERT(pBuffer);

	CPoint Point(0,0);
	_SpriteSet *CopyBoard = (_SpriteSet*)pBuffer;
	// is it a sprite set?
	if(strcmp(CopyBoard->Info.ID, "Quest Designer Sprite Set")) {
		pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));
		if(!PasteSprite(pLayer, (LPCSTR)pBuffer)) return false;
		m_bFloating = true;
	} else {
		if(CopyBoard->Info.dwSignature != QUEST_SET_SIGNATURE) {
			CONSOLE_PRINTF("Paste error : Attempt to paste an invalid Quest Designer Sprite Set!\n");
			return false;
		}

		m_Objects.clear();
		Point = CopyBoard->Info.rcBoundaries.CenterPoint();

		_SpriteSet::_SpriteSetData *pData = (_SpriteSet::_SpriteSetData *)((char*)CopyBoard + sizeof(_SpriteSet::_SpriteSetInfo));
		for(UINT i=0; i<CopyBoard->Info.nSize; i++) {
			if(nLayer != pData[i].nLayer) {
				nLayer = pData[i].nLayer;
				pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(nLayer));
			}
			PasteObj(pLayer, &pData[i]);
		}
	}

	m_bFloating = true;
	StartMoving(Point);
	MoveTo(point_);

	CONSOLE_DEBUG("%d of %d objects selected.\n", m_Objects.size(), (*m_ppMainDrawable)->Objects());
	return true;
}

void CSpriteSelection::FlipSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(rcBoundaries);

	mapObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->first);
		scontext->GetAbsFinalRect(RectTmp);

		if(Iterator->second.eYChain == up) Iterator->second.eYChain = down;
		else if(Iterator->second.eYChain == down) Iterator->second.eYChain = up;

		// no rotations nor transormations are allowed for entities 
		// (that should be handled in the scripts instead)
		if(static_cast<CSprite*>(scontext->GetDrawableObj())->GetSpriteType() != tEntity) {
			if(scontext->isMirrored() && !scontext->isFlipped()) {
				scontext->Mirror(false);
				scontext->Rotate((scontext->Rotation()-2)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			} else {
				scontext->Flip();
			}
		}
		Rect.top = rcBoundaries.top + (rcBoundaries.bottom - RectTmp.bottom);
		Rect.bottom = rcBoundaries.bottom - (RectTmp.top - rcBoundaries.top);
		Rect.left = RectTmp.left;
		Rect.right = RectTmp.right;

		Rect.NormalizeRect();

		scontext->SetAbsFinalRect(Rect);
		Iterator++;
	}
	m_bModified = m_bChanged = true;
}
void CSpriteSelection::MirrorSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(rcBoundaries);

	mapObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->first);
		scontext->GetAbsFinalRect(RectTmp);

		if(Iterator->second.eXChain == left) Iterator->second.eXChain = right;
		else if(Iterator->second.eXChain == right) Iterator->second.eXChain = left;

		// no rotations nor transormations are allowed for entities 
		// (that should be handled in the scripts instead)
		if(static_cast<CSprite*>(scontext->GetDrawableObj())->GetSpriteType() != tEntity) {
			if(!scontext->isMirrored() && scontext->isFlipped()) {
				scontext->Flip(false);
				scontext->Rotate((scontext->Rotation()-2)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			} else {
				scontext->Mirror();
			}
		}
		Rect.top = RectTmp.top;
		Rect.bottom = RectTmp.bottom;
		Rect.left = rcBoundaries.left + (rcBoundaries.right - RectTmp.right);
		Rect.right = rcBoundaries.right - (RectTmp.left - rcBoundaries.left);

		Rect.NormalizeRect();

		scontext->SetAbsFinalRect(Rect);
		Iterator++;
	}
	m_bModified = m_bChanged = true;
}
void CSpriteSelection::CWRotateSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(rcBoundaries);

	mapObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->first);
		scontext->GetAbsFinalRect(RectTmp);

		_Chain cTmp = Iterator->second.eXChain;
		Iterator->second.eXChain = Iterator->second.eYChain;
		if(cTmp == left) Iterator->second.eYChain = up;
		else if(cTmp == right) Iterator->second.eYChain = down;
		else Iterator->second.eYChain = cTmp;

		// no rotations nor transormations are allowed for entities 
		// (that should be handled in the scripts instead)
		if(static_cast<CSprite*>(scontext->GetDrawableObj())->GetSpriteType() != tEntity) {
			if(scontext->isMirrored() && scontext->isFlipped()) {
				scontext->Flip(false);
				scontext->Mirror(false);
				scontext->Rotate((scontext->Rotation()+1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			} else {
				if(scontext->isMirrored() || scontext->isFlipped()) {
					scontext->Flip();
					scontext->Mirror();
				}
				scontext->Rotate((scontext->Rotation()-1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			}
			Rect.top = rcBoundaries.top + (RectTmp.left - rcBoundaries.left);
			Rect.bottom = rcBoundaries.top + (RectTmp.right - rcBoundaries.left);
			Rect.left = rcBoundaries.left + (rcBoundaries.bottom - RectTmp.bottom);
			Rect.right = rcBoundaries.left + (rcBoundaries.bottom - RectTmp.top);
		} else {
			Rect.top = rcBoundaries.top + (RectTmp.left - rcBoundaries.left);
			Rect.bottom = Rect.top + RectTmp.Height();
			Rect.left = rcBoundaries.left + (rcBoundaries.bottom - RectTmp.bottom);
			Rect.right = Rect.left + RectTmp.Width();
		}

		Rect.NormalizeRect();

		scontext->SetAbsFinalRect(Rect);
		Iterator++;
	}
	m_bModified = m_bChanged = true;
}
void CSpriteSelection::CCWRotateSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(rcBoundaries);

	mapObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->first);
		scontext->GetAbsFinalRect(RectTmp);

		_Chain cTmp = Iterator->second.eYChain;
		Iterator->second.eYChain = Iterator->second.eXChain;
		if(cTmp == up) Iterator->second.eXChain = left;
		else if(cTmp == down) Iterator->second.eXChain = right;
		else Iterator->second.eYChain = cTmp;

		// no rotations nor transormations are allowed for entities 
		// (that should be handled in the scripts instead)
		if(static_cast<CSprite*>(scontext->GetDrawableObj())->GetSpriteType() != tEntity) {
			if(scontext->isMirrored() && scontext->isFlipped()) {
				scontext->Flip(false);
				scontext->Mirror(false);
				scontext->Rotate((scontext->Rotation()-1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			} else {
				if(scontext->isMirrored() || scontext->isFlipped()) {
					scontext->Flip();
					scontext->Mirror();
				}
				scontext->Rotate((scontext->Rotation()+1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			}
			Rect.top = rcBoundaries.top + (rcBoundaries.right - RectTmp.right);
			Rect.bottom = rcBoundaries.top + (rcBoundaries.right - RectTmp.left);
			Rect.left = rcBoundaries.left + (RectTmp.top - rcBoundaries.top);
			Rect.right = rcBoundaries.left + (RectTmp.bottom - rcBoundaries.top);
		} else {
			Rect.top = rcBoundaries.top + (rcBoundaries.right - RectTmp.right);
			Rect.bottom = Rect.top + RectTmp.Height();
			Rect.left = rcBoundaries.left + (RectTmp.top - rcBoundaries.top);
			Rect.right = Rect.left + RectTmp.Width();
		}

		Rect.NormalizeRect();

		scontext->SetAbsFinalRect(Rect);
		Iterator++;
	}
	m_bModified = m_bChanged = true;
}
