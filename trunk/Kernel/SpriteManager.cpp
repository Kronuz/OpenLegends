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
	\date		April 17, 2003:
					* Initial release.
				September 3, 2003:
					- Bug Fix. Pasted objects could not be rotated correctly.
					- Bug Fix. Rotated objects not showing correctly.

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
		scontext.GetAbsFinalRect(rcLocation);
		rcLocation.IntersectRect(rcVisible, rcLocation);
		if(!rcLocation.IsRectEmpty()) return true;
	}
	return false; 
}
inline bool CSprite::Draw(const CDrawableContext &context, bool bBounds, const ARGBCOLOR *rgbColorOverride, int nBuffer)
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
	if(rgbColorOverride) {
		rgbColor.rgbAlpha = (BYTE)(((int)rgbColorOverride->rgbAlpha + scontext->getAlpha()) / 2);
		rgbColor.rgbRed = rgbColorOverride->rgbRed;
		rgbColor.rgbGreen = rgbColorOverride->rgbGreen;
		rgbColor.rgbBlue = rgbColorOverride->rgbBlue;
	} else {
		rgbColor = COLOR_ARGB(scontext->getAlpha(),255,255,255);
	}

	CRect Rect;
	scontext->GetAbsFinalRect(Rect);
	pGraphics->Render(pTexture,							// texture
		m_Boundaries[scontext->m_nFrame[nBuffer]],		// rectSrc
		Rect,											// rectDest
		scontext->Rotation(),							// rotation
		scontext->Transformation(),						// transform
		rgbColor,										// rgbColor
		&(context.m_pBuffer[nBuffer]),					// buffer
		scontext->RelRotation(),						// relarive rotation
		scontext->RelScale()							// relative scale
		/*/(( (20 * GetTickCount())/1000 ) % 360) * 0.01745329252f, // just for testing
		(float)(((10 * GetTickCount())/1000 ) % 50) / 10 /**/
	);

	if(bBounds) pGraphics->BoundingBox(Rect, COLOR_ARGB(255,0,0,0));

	return true;
}
bool CBackground::Draw(const CDrawableContext &context, const ARGBCOLOR *rgbColorOverride) 
{ 
	if(!NeedToDraw(context)) return true;

	if(m_SptType == tMask) {
		ARGBCOLOR rgbColor = COLOR_ARGB(200,255,255,255);
		if(!CSprite::Draw(context, false, &rgbColor, 1)) {
			return false;
		}
	} else {
		if(!CSprite::Draw(context, g_bBounds, rgbColorOverride, 0)) {
			return false;
		}

		if(g_bMasks) {
			const SBackgroundData *pBackgroundData = static_cast<const SBackgroundData*>(m_pSpriteData);
			if(pBackgroundData->pMaskMap)
				pBackgroundData->pMaskMap->Draw(context, rgbColorOverride);
		}
	}
	return true; 
}
bool CEntity::Draw(const CDrawableContext &context, const ARGBCOLOR *rgbColorOverride) 
{ 
	if(g_bEntities)
		return CBackground::Draw(context, rgbColorOverride); 
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

	strncpy(pI->szName, GetName(), sizeof(pI->szName) - 1);
	strncpy(pI->szScope, m_pDrawableObj->GetName(), sizeof(pI->szScope) - 1);

	pI->szName[sizeof(pI->szName) - 1] = '\0';
	pI->szScope[sizeof(pI->szScope) - 1] = '\0';

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
	GetAbsFinalRect(Rect);
	pPL->AddValue("X", Rect.left);
	pPL->AddValue("Y", Rect.top);
	if(pPL->Information.eType == itEntity) {
		pPL->AddValue("Width", Rect.Width(), false);
		pPL->AddValue("Height", Rect.Height(), false);
		pPL->AddBoolean("IsMirrored", isMirrored(), false);
		pPL->AddBoolean("IsFlipped", isFlipped(), false);
		pPL->AddList("Rotation", Rotation(), "0 degrees, 90 degrees, 180 degrees, 270 degrees", false);
		pPL->AddValue("Alpha", getAlpha(), false);
	} else {
		pPL->AddValue("Width", Rect.Width());
		pPL->AddValue("Height", Rect.Height());
		pPL->AddBoolean("IsMirrored", isMirrored());
		pPL->AddBoolean("IsFlipped", isFlipped());
		pPL->AddList("Rotation", Rotation(), "0 degrees, 90 degrees, 180 degrees, 270 degrees");
		pPL->AddValue("Alpha", getAlpha());
	}

	pPL->AddCategory("Misc");

	char szLayersList[MAX_LAYERS + sizeof(g_szLayerNames)] = {0};
	for(int i=0; i<MAX_LAYERS; i++) {
		strcat(szLayersList, g_szLayerNames[i]);
		if(i != MAX_LAYERS-1) strcat(szLayersList, ",");
	}
	char szSubLayersList[MAX_SUBLAYERS + sizeof(g_szSubLayerNames)] = {0};
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		strcat(szSubLayersList, g_szSubLayerNames[i]);
		if(i != MAX_SUBLAYERS-1) strcat(szSubLayersList, ",");
	}

	pPL->AddList("Layer", GetObjLayer(), szLayersList);
	pPL->AddList("SubLayer", GetObjSubLayer(), szSubLayersList);
	pPL->AddString("Sprite Sheet", static_cast<CSprite*>(m_pDrawableObj)->GetSpriteSheet()->GetName(), false);
	
	pPL->AddCategory("Behavior");
	pPL->AddBoolean("IsVisible", isVisible(), false);

	return true;
}
bool CSpriteContext::SetProperties(SPropertyList &PL) 
{
	bool bChanged = false;

	SProperty* pP;

	CRect Rect;
	GetAbsFinalRect(Rect);

	pP = PL.FindProperty("X", SProperty::ptValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(pP->nValue - Rect.left) {
			Rect.OffsetRect(pP->nValue - Rect.left, 0);
			bChanged = true;
		}
	}
	
	pP = PL.FindProperty("Y", SProperty::ptValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(pP->nValue - Rect.top) {
			Rect.OffsetRect(0, pP->nValue - Rect.top);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("Width", SProperty::ptValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(Rect.right - Rect.left != pP->nValue) {
			Rect.right = Rect.left + pP->nValue;
			bChanged = true;
		}
	}

	pP = PL.FindProperty("Height", SProperty::ptValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(Rect.bottom - Rect.top != pP->nValue) {
			Rect.bottom = Rect.top + pP->nValue;
			bChanged = true;
		}
	}

	SetAbsFinalRect(Rect);

	pP = PL.FindProperty("Alpha", SProperty::ptValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(getAlpha() != pP->nValue) {
			Alpha(pP->nValue);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("IsMirrored", SProperty::ptBoolean);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(isMirrored() != pP->bBoolean) {
			Mirror(pP->bBoolean);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("IsFlipped", SProperty::ptBoolean);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(isFlipped() != pP->bBoolean) {
			Flip(pP->bBoolean);
			bChanged = true;
		}
	}
	
	pP = PL.FindProperty("Rotation", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(Rotation() != pP->nIndex) {
			Rotate(pP->nIndex);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("SubLayer", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(GetObjSubLayer() != pP->nIndex) {
			SetObjSubLayer(pP->nIndex);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("Layer", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(GetObjLayer() != pP->nIndex) {
			SetObjLayer(pP->nIndex);
			bChanged = true;
		}
	}

	return bChanged;
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
	CRect RectTmp;
	m_rcSelection.SetRectEmpty();
	vectorObject::iterator Iterator;
	for(Iterator = m_Objects.begin(); Iterator != m_Objects.end(); Iterator++) {
		Iterator->pContext->GetAbsFinalRect(RectTmp);
		m_rcSelection.UnionRect(m_rcSelection, RectTmp);
		// We need to keep the initial size and location of every selected object:
		Iterator->rcRect = RectTmp;
	}
}
SObjProp* CSpriteSelection::GetFirstSelection()
{
	m_CurrentSel = m_Objects.begin();
	if(m_CurrentSel == m_Objects.end()) return NULL;
	return (m_CurrentSel++).operator ->();
}
SObjProp* CSpriteSelection::GetNextSelection() 
{
	if(m_CurrentSel == NULL) return NULL;
	if(m_CurrentSel == m_Objects.end()) return NULL;
	return (m_CurrentSel++).operator ->();
}

bool CSpriteSelection::Draw(const IGraphics *pGraphics_) {
	CRect Rect(0,0,0,0);
	CRect RectTmp;

	int nObjects = (int)m_Objects.size();
	const CSpriteContext *scontext = NULL;

	vectorObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		scontext = static_cast<const CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);
		Rect.UnionRect(Rect, RectTmp);
		if(nObjects>1) {
			if(Iterator->bSubselected && m_bHoldSelection) {
				// if the object is subselected and the selection is held, draw in other color:
				pGraphics_->FillRect(RectTmp, COLOR_ARGB(100,0,0,128));
			} else {
				// only fill the selection if it isn't held:
				if(!m_bHoldSelection) pGraphics_->FillRect(RectTmp, COLOR_ARGB(25,255,255,225));
			}
			// draw a bounding rect over the selected object:
			pGraphics_->BoundingBox(RectTmp, COLOR_ARGB(80,255,255,225));

			// Draw the horizontal chain of the object:
			CRect rcSpecial, rcArrow;
			rcSpecial = RectTmp;
			if(Iterator->eXChain == left) {
				rcSpecial.right = rcSpecial.left+2;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
			} else if(Iterator->eXChain == right) {
				rcSpecial.left = rcSpecial.right-2;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
			} else if(Iterator->eXChain == fixed) {
				rcSpecial.right = rcSpecial.left+2;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
				rcSpecial = RectTmp;
				rcSpecial.left = rcSpecial.right-2;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
			} else if(Iterator->eXChain == stretch) {
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

			// Draw the vertical chain of the object:
			rcSpecial = RectTmp;
			if(Iterator->eYChain == up) {
				rcSpecial.bottom = rcSpecial.top+2;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
			} else if(Iterator->eYChain == down) {
				rcSpecial.top = rcSpecial.bottom-2;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
			} else if(Iterator->eYChain == fixed) {
				rcSpecial.bottom = rcSpecial.top+2;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
				rcSpecial = RectTmp;
				rcSpecial.top = rcSpecial.bottom-2;
				pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
			} else if(Iterator->eYChain == stretch) {
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
		if(m_bHoldSelection) pGraphics_->SelectionBox(Rect, COLOR_ARGB(255,160,160,160));
		else pGraphics_->SelectionBox(Rect, COLOR_ARGB(255,255,255,200));
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
void CSpriteSelection::ResizeObject(const SObjProp &ObjProp_, const CRect &rcOldBounds_, const CRect &rcNewBounds_, bool bAllowResize_)
{
	CSpriteContext *scontext = static_cast<CSpriteContext*>(ObjProp_.pContext);
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
	scontext->SetAbsFinalRect(Rect);
}

// pBitmap must point to a block of memory allocated by the Kernel (i.e. via CaptureSelection or similars)
HGLOBAL CSpriteSelection::Copy(BITMAP **ppBitmap, bool bDeleteBitmap)
{
	BITMAP *pBitmap = NULL;
	if(ppBitmap) pBitmap = *ppBitmap;

	// Now we are ready to copy the objects's information into the sprite set,
	// we start by sorting the selected objects (in the selection):
	SortSelection();

	if(m_Objects.size() == 0) {
		if(bDeleteBitmap && ppBitmap) {
			*ppBitmap = NULL;
			delete []pBitmap;
		}
		return NULL;
	}

	int bmpsize = 0;
	if(pBitmap) bmpsize = sizeof(BITMAP) + pBitmap->bmHeight * pBitmap->bmWidthBytes;
	int datasize = sizeof(_SpriteSet::_SpriteSetInfo);
	// Analize the selected objeects to find out the needed size for the sprite set:
	CSprite *pOldSprite = NULL;
	vectorObject::iterator Iterator = m_Objects.begin();
	for(int i=0; Iterator != m_Objects.end(); i++) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		ASSERT(scontext);
		SObjProp *pObjProp = static_cast<SObjProp *>(Iterator.operator ->());
		CSprite *pSprite = static_cast<CSprite *>(scontext->GetDrawableObj());
		ASSERT(pSprite);
		ASSERT(pObjProp);

		datasize += sizeof(_SpriteSet::_SpriteSetData);

		// if it is not an entity, with and height it will need mask: SSD_WIDTHHEIGHT
		if(pSprite->GetSpriteType() != tEntity) {
			datasize += sizeof(_SpriteSet::_SpriteSetData01);
		}

		// if it has chains or transformations it will need mask: SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
		if( pObjProp->eXChain != relative || pObjProp->eYChain != relative ||
			scontext->isMirrored() || scontext->isFlipped() || scontext->Rotation() ) {
			datasize += sizeof(_SpriteSet::_SpriteSetData02);
		}

		// if it has an alpha value set, it will need mask: SSD_ALPHA
		if(scontext->getAlpha() != 255) {
			datasize += sizeof(_SpriteSet::_SpriteSetData04);
		}

		if(pOldSprite != pSprite) {
			pOldSprite = pSprite;
			int nNameLen = 0;
			nNameLen = strlen(scontext->GetObjName()) + 1;
			ASSERT(nNameLen < 30);
			datasize += nNameLen;
		}

		Iterator++;
	}
	int asize = (datasize + sizeof(DWORD) - 1 / sizeof(DWORD)) * sizeof(DWORD); // align to 32 bits.

	// Now that we know the exact space all the sprites will need, we allocate it:
	HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, asize + bmpsize); 
	if(hglbCopy == NULL) {
		if(bDeleteBitmap && ppBitmap) {
			*ppBitmap = NULL;
			delete []pBitmap;
		}
		return NULL;
	}

 	// Lock the handle and copy the text to the buffer. 
	LPBYTE RawBuffer = (LPBYTE)::GlobalLock(hglbCopy);
	ASSERT(RawBuffer);

	CRect rcBoundaries, RectTmp;
	GetBoundingRect(&rcBoundaries);

	_SpriteSet *CopyBoard = (_SpriteSet*)RawBuffer;
	strcpy(CopyBoard->Info.ID, QUEST_SET_ID);
	strcat(CopyBoard->Info.ID, "\nUnnamed Sprite Set\nDescription goes here.");
	CopyBoard->Info.dwSignature = QUEST_SET_SIGNATURE;
	CopyBoard->Info.dwSize = asize + bmpsize;
	CopyBoard->Info.nObjects = (int)m_Objects.size();

	CopyBoard->Info.rcBoundaries = rcBoundaries;
	CopyBoard->Info.rcBoundaries.OffsetRect(-rcBoundaries.TopLeft());
	CopyBoard->Info.dwBitmapOffset = 0;
	
	LPBYTE pData = (LPBYTE)(RawBuffer + sizeof(_SpriteSet::_SpriteSetInfo));

	pOldSprite = NULL;
	Iterator = m_Objects.begin();
	for(i=0; Iterator != m_Objects.end(); i++) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		ASSERT(scontext);
		SObjProp *pObjProp = static_cast<SObjProp *>(Iterator.operator ->());
		CSprite *pSprite = static_cast<CSprite *>(scontext->GetDrawableObj());
		ASSERT(pSprite);
		ASSERT(pObjProp);

		int Mask = 0;
		LPSTR szName = NULL;
		int nNameLen = 0;
		_SpriteSet::_SpriteSetData01 *pSpriteSetData01 = NULL;
		_SpriteSet::_SpriteSetData02 *pSpriteSetData02 = NULL;
		_SpriteSet::_SpriteSetData04 *pSpriteSetData04 = NULL;

		// Search for all required options and mask. 

		_SpriteSet::_SpriteSetData *pSpriteSetData = (_SpriteSet::_SpriteSetData *)pData;
		pData += sizeof(_SpriteSet::_SpriteSetData);

		// if it is not an entity, with and height it will need mask: SSD_WIDTHHEIGHT
		if(pSprite->GetSpriteType() != tEntity) {
			Mask |= SSD_WIDTHHEIGHT;
			pSpriteSetData01 = (_SpriteSet::_SpriteSetData01 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData01);
		}

		// if it has chains or transformations it will need mask: SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
		if( pObjProp->eXChain != relative || pObjProp->eYChain != relative ||
			scontext->isMirrored() || scontext->isFlipped() || scontext->Rotation() ) {
			Mask |= SSD_TRANS;
			if(pObjProp->eXChain != relative) Mask |= SSD_CHAIN_X;
			if(pObjProp->eYChain != relative) Mask |= SSD_CHAIN_Y;
			pSpriteSetData02 = (_SpriteSet::_SpriteSetData02 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData02);
		}

		// if it has an alpha value set, it will need mask: SSD_ALPHA
		if(scontext->getAlpha() != 255) {
			Mask |= SSD_ALPHA;
			pSpriteSetData04 = (_SpriteSet::_SpriteSetData04 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData04);
		}

		if(pOldSprite != pSprite) {
			pOldSprite = pSprite;
			nNameLen = strlen(scontext->GetObjName()) + 1;
			ASSERT(nNameLen < 30);
			szName = (LPSTR)pData;
			pData += nNameLen;
		}

		// Fill the needed data:

		scontext->GetAbsFinalRect(RectTmp);
		RectTmp.OffsetRect(-rcBoundaries.TopLeft());

		ASSERT(pSpriteSetData);

		ASSERT(Mask < 0x0020);
		ASSERT(scontext->GetObjLayer() < 0x0008);
		ASSERT(scontext->GetObjSubLayer() < 0x0008);
		ASSERT(nNameLen < 0x0030);

		pSpriteSetData->Mask = Mask;
		pSpriteSetData->NameLen = nNameLen;
		pSpriteSetData->Layer = scontext->GetObjLayer();
		pSpriteSetData->SubLayer = scontext->GetObjSubLayer();
		pSpriteSetData->X = (WORD)RectTmp.left;
		pSpriteSetData->Y = (WORD)RectTmp.top;

		if(pSpriteSetData01) { // SSD_WIDTHHEIGHT
			ASSERT(RectTmp.Width() < 0x1000);
			ASSERT(RectTmp.Height() < 0x1000);

			pSpriteSetData01->Width = RectTmp.Width();
			pSpriteSetData01->Height = RectTmp.Height();
		}
		if(pSpriteSetData02) { // SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
			ASSERT(scontext->Rotation() < 0x0004);

			pSpriteSetData02->rotation = scontext->Rotation();
			pSpriteSetData02->mirrored = scontext->isMirrored();
			pSpriteSetData02->flipped = scontext->isFlipped();
			pSpriteSetData02->XChain = ((unsigned)pObjProp->eXChain - 1);
			pSpriteSetData02->YChain = ((unsigned)pObjProp->eYChain - 1);
		}
		if(pSpriteSetData04) { // SSD_ALPHA
			pSpriteSetData04->alpha = (BYTE)scontext->getAlpha();
		}
		if(szName) {
			strcpy(szName, scontext->GetObjName());
		}

		Iterator++;
		ASSERT(pData <= RawBuffer + datasize);
	}

	ASSERT(pData == RawBuffer + datasize);
	CONSOLE_DEBUG("Sprite Set created, %d bytes: %d bytes used for data, and %d bytes for the thumbnail\n", asize + bmpsize, datasize, bmpsize);

	if(pBitmap) {
		CopyBoard->Info.dwBitmapOffset = (DWORD)asize; // offset of the bitmap
		memcpy(RawBuffer + asize, pBitmap, sizeof(BITMAP));
		memcpy(RawBuffer + asize + sizeof(BITMAP), pBitmap->bmBits, bmpsize - sizeof(BITMAP));
	}

	::GlobalUnlock(hglbCopy); 

	if(bDeleteBitmap && ppBitmap) {
		*ppBitmap = NULL;
		delete []pBitmap; // free bitmap memory
	}
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
	m_Objects.push_back(SObjProp(this, pSpriteContext));
	return true;
}
bool CSpriteSelection::PasteObj(CLayer *pLayer, LPBYTE *ppData)
{
	return true;
}
bool CSpriteSelection::Paste(LPVOID pBuffer, const CPoint &point_)
{
	ASSERT(pBuffer);
	if(m_bHoldSelection) {
		CONSOLE_PRINTF("Warning: Can not paste on held selection, release the selection first.\n");
		return false;
	}

	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));
	UINT nLayer = -1;

	CPoint Point(0,0);
	_SpriteSet *CopyBoard = (_SpriteSet*)pBuffer;
	// is it a sprite set?
	if(strncmp(CopyBoard->Info.ID, QUEST_SET_ID, QUEST_SET_IDLEN)) {
		if(!PasteSprite(pLayer, (LPCSTR)pBuffer)) return false;
		m_bFloating = true;
	} else {
		if(CopyBoard->Info.dwSignature != QUEST_SET_SIGNATURE) {
			CONSOLE_PRINTF("Paste error : Attempt to paste an invalid Quest Designer Sprite Set!\n");
			return false;
		}

		m_Objects.clear();
		Point = CopyBoard->Info.rcBoundaries.CenterPoint();

		LPSTR szName = NULL;

		LPBYTE pData = (LPBYTE)((char*)CopyBoard + sizeof(_SpriteSet::_SpriteSetInfo));
		for(UINT i=0; i<CopyBoard->Info.nObjects; i++) {

			_SpriteSet::_SpriteSetData01 *pSpriteSetData01 = NULL;
			_SpriteSet::_SpriteSetData02 *pSpriteSetData02 = NULL;
			_SpriteSet::_SpriteSetData04 *pSpriteSetData04 = NULL;

			// Initialize structures from data and mask:
			_SpriteSet::_SpriteSetData *pSpriteSetData = (_SpriteSet::_SpriteSetData *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData);

			if((pSpriteSetData->Mask & SSD_WIDTHHEIGHT) == SSD_WIDTHHEIGHT) {
				pSpriteSetData01 = (_SpriteSet::_SpriteSetData01 *)pData;;
				pData += sizeof(_SpriteSet::_SpriteSetData01);
			}
			if((pSpriteSetData->Mask & SSD_TRANS) == SSD_TRANS) {
				pSpriteSetData02 = (_SpriteSet::_SpriteSetData02 *)pData;;
				pData += sizeof(_SpriteSet::_SpriteSetData02);
			}
			if((pSpriteSetData->Mask & SSD_ALPHA) == SSD_ALPHA) {
				pSpriteSetData04 = (_SpriteSet::_SpriteSetData04 *)pData;;
				pData += sizeof(_SpriteSet::_SpriteSetData04);
			}
			if(pSpriteSetData->NameLen) {
				szName = (LPSTR)pData;
				pData += pSpriteSetData->NameLen;
			}

			// Build sprite:
			if(nLayer!=-1 || !pLayer) {
				if(nLayer != pSpriteSetData->Layer) {
					nLayer = pSpriteSetData->Layer;
					pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(nLayer));
				}
			} // else if there is a current layer selected, use that layer instead

			CSprite *pSprite = CGameManager::Instance()->FindSprite(szName);
			if(!pSprite) {
				CONSOLE_PRINTF("Paste error : Couldn't find the requested sprite: '%s'!\n", szName);
				continue;
			}
			if(pSprite->GetSpriteType() == tMask) {
				CONSOLE_PRINTF("Paste error: Attempt to use mask '%s' as a sprite\n", szName);
				continue;
			}
			if(!pLayer) {
				CONSOLE_PRINTF("Paste error : Couldn't paste '%s' in the requested layer!\n", szName);
				continue;
			}

			CSpriteContext *pSpriteContext = new CSpriteContext(""); // New sprite context with no name.
			pSpriteContext->SetDrawableObj(pSprite);

			// Fill sprite context data:

			int alpha = 255;
			int rotation = 0;
			bool mirrored = false;
			bool flipped = false;
			
			_Chain XChain = relative;
			_Chain YChain = relative;
			CRect RectTmp(pSpriteSetData->X, pSpriteSetData->Y, pSpriteSetData->X - 1, pSpriteSetData->Y - 1);
			pSpriteContext->SetObjSubLayer(pSpriteSetData->SubLayer);

			if(pSpriteSetData01) { //SSD_WIDTHHEIGHT
				ASSERT((pSpriteSetData->Mask & SSD_WIDTHHEIGHT) == SSD_WIDTHHEIGHT);
				RectTmp.right = RectTmp.left + pSpriteSetData01->Width;
				RectTmp.bottom = RectTmp.top + pSpriteSetData01->Height;
				pSpriteContext->Tile();
			}
			if(pSpriteSetData02) { //SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
				ASSERT((pSpriteSetData->Mask & SSD_TRANS) == SSD_TRANS);
				rotation = pSpriteSetData02->rotation;
				mirrored = pSpriteSetData02->mirrored;
				flipped = pSpriteSetData02->flipped;
				if((pSpriteSetData->Mask & SSD_CHAIN_X) == SSD_CHAIN_X)
					XChain = (_Chain)(pSpriteSetData02->XChain + 1);
				if((pSpriteSetData->Mask & SSD_CHAIN_Y) == SSD_CHAIN_Y)
					YChain = (_Chain)(pSpriteSetData02->YChain + 1);
			}
			if(pSpriteSetData04) { //SSD_ALPHA
				ASSERT((pSpriteSetData->Mask & SSD_ALPHA) == SSD_ALPHA);
				alpha = pSpriteSetData04->alpha;
			}

			pSpriteContext->Rotate(rotation);
			pSpriteContext->Mirror(mirrored);
			pSpriteContext->Flip(flipped);
			pSpriteContext->Alpha(alpha);

			pLayer->AddSpriteContext(pSpriteContext, true); // insert the sprite in the current layer
			m_Objects.push_back(SObjProp(this, pSpriteContext, RectTmp, XChain, YChain));

			// the sprite absolute postion must be set after inserting it in the layer.
			pSpriteContext->SetAbsFinalRect(RectTmp);
			pSpriteContext->SelectContext();

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
	GetBoundingRect(&rcBoundaries);

	vectorObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);

		if(Iterator->eYChain == up) Iterator->eYChain = down;
		else if(Iterator->eYChain == down) Iterator->eYChain = up;

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
		if(m_eCurrentState==eMoving) {
			Iterator->rcRect = Rect; // for floating selections
		}

		Iterator++;
	}

	if(m_eCurrentState==eMoving) {
		GetBoundingRect(&m_rcSelection);
		SetInitialMovingPoint(m_rcSelection.CenterPoint());
	}

	m_bModified = m_bChanged = true;
}
void CSpriteSelection::MirrorSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(&rcBoundaries);

	vectorObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);

		if(Iterator->eXChain == left) Iterator->eXChain = right;
		else if(Iterator->eXChain == right) Iterator->eXChain = left;

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
		if(m_eCurrentState==eMoving) {
			Iterator->rcRect = Rect; // for floating selections
		}

		Iterator++;
	}

	if(m_eCurrentState==eMoving) {
		GetBoundingRect(&m_rcSelection);
		SetInitialMovingPoint(m_rcSelection.CenterPoint());
	}

	m_bModified = m_bChanged = true;
}
void CSpriteSelection::CWRotateSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(&rcBoundaries);

	vectorObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);

		_Chain cTmp = Iterator->eXChain;
		Iterator->eXChain = Iterator->eYChain;
		if(cTmp == left) Iterator->eYChain = up;
		else if(cTmp == right) Iterator->eYChain = down;
		else Iterator->eYChain = cTmp;

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
		if(m_eCurrentState==eMoving) {
			Iterator->rcRect = Rect; // for floating selections
		}

		Iterator++;
	}

	if(m_eCurrentState==eMoving) {
		GetBoundingRect(&m_rcSelection);
		SetInitialMovingPoint(m_rcSelection.CenterPoint());
	}

	m_bModified = m_bChanged = true;
}
void CSpriteSelection::CCWRotateSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(&rcBoundaries);

	vectorObject::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);

		_Chain cTmp = Iterator->eYChain;
		Iterator->eYChain = Iterator->eXChain;
		if(cTmp == up) Iterator->eXChain = left;
		else if(cTmp == down) Iterator->eXChain = right;
		else Iterator->eYChain = cTmp;

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
		if(m_eCurrentState==eMoving) {
			Iterator->rcRect = Rect; // for floating selections
		}

		Iterator++;
	}

	if(m_eCurrentState==eMoving) {
		GetBoundingRect(&m_rcSelection);
		SetInitialMovingPoint(m_rcSelection.CenterPoint());
	}

	m_bModified = m_bChanged = true;
}
