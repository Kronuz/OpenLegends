/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		SpriteManager.cpp
	\brief		Implementation of the sprite classes.
	\date		April 15, 2003:
					* Pre-alpha state.
				April 17, 2003:
					* Initial release.
				September 3, 2003:
					- Bug Fix. Pasted objects could not be rotated correctly.
					- Bug Fix. Rotated objects not showing correctly.
				September 10, 2003: 
					- Bug Fix. After changing the Alpha value buffer was needed to be touch. 
					  (not anymore due a change in the Graphics engine, it now 
					   checks every time for color change)
				September 16, 2003: 
					+ Full ARGB values instead of just alpha in contexts.
				September 17, 2003: 
					+ Optimization. Sprite sets now have indexed names and the alpha-RGB are now separated.
				September 14, 2004: 
					+ Bug Fix. Sprite Sets didn't load entities well. (it changed some sizes to null)

	This file implements all the classes that manage the sprites,
	including backgrounds, sprite sheets, animations, mask maps and
	entities.
*/

#include "stdafx.h"

#include "SpriteManager.h"
#include "GameManager.h"
#include "ScriptManager.h"

#include "ArchiveText.h"

#include <set>

bool g_bBounds = false;
bool g_bMasks = false;
bool g_bEntities = false;

CSprite::CSprite(LPCSTR szName) :
	m_bDefined(false),
	m_pSpriteSheet(NULL),
	m_pSpriteData(NULL),
	m_nCatalog(-1),
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
		CRect rcVisible;
		pGraphics->GetVisibleRect(&rcVisible);
		rcVisible.InflateRect(1, 1); // adjacent sprites are painted (just in case, for zooms)
		scontext.GetAbsFinalRect(rcLocation);
		rcLocation.IntersectRect(rcVisible, rcLocation);
		if(!rcLocation.IsRectEmpty()) return true;
	}
	return false; 
}
inline bool CSprite::Draw(const CDrawableContext &context, bool bBounds, const ARGBCOLOR *rgbColorModulation, int nBuffer)
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
		if(GetTickCount() - pSpriteSheet->m_dwLastTry < 10000) return false;
		pSpriteSheet->m_dwLastTry = 0;

		// We make use of lazy evaluation here to load the textures.
		CVFile fnFile = pSpriteSheet->GetFile();
		float scale = 1.0f;
		fnFile.SetFileExt(".png");
		if(!fnFile.FileExists()) {
			scale = 2.0f;
			fnFile.SetFileExt(".bmp");
			CONSOLE_PRINTF("Kernel Warning: Couldn't find Sprite Sheet from PNG file,\n    trying to load from '%s' instead.\n", fnFile.GetFileName());
			if(!fnFile.FileExists()) {
				CONSOLE_PRINTF("Kernel Error: Couldn't find Sprite Sheet bitmap for '%s.spt'.\n", fnFile.GetFileTitle());
				pSpriteSheet->m_dwLastTry = GetTickCount();
				return false;
			}
		}
		if(fnFile.Open("r")) {
			int filesize = fnFile.GetFileSize();
			LPCVOID pData = fnFile.ReadFile();
			if(!pData) {
				CONSOLE_PRINTF("Kernel Fatal Error: Not enough memory to hold %d bytes!\n", fnFile.GetFileSize());
			} else pGraphics->CreateTextureFromFileInMemory(fnFile.GetFileName(), pData, filesize, &pTexture, scale);
			fnFile.Close();
		}
		if(!pTexture) {
			CONSOLE_PRINTF("Kernel Error: Couldn't open Sprite Sheet bitmap for '%s.spt'.\n", fnFile.GetFileTitle());
			pSpriteSheet->m_dwLastTry = GetTickCount();
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

	float fDelta = CGameManager::GetFPSDelta();
	if( fDelta > 0.0f && pGraphics->GetCurrentZoom() >= 0.5f ) {
		if( m_pSpriteData->iAnimSpd && 
			CGameManager::GetPauseLevel() == 0 ) { // fps

			int TotalFrames = m_Boundaries.size();
			if(m_pSpriteData->eAnimDir != _d_down) {
				// How long would have taken to play all remaining frames since last render?
				// Less than the time it took to get here?
				float AnimTime = (float)(TotalFrames - nFrame) / (float)m_pSpriteData->iAnimSpd;
				int nTmp = ((m_pSpriteData->iAnimSpd * CGameManager::GetLastTick()/1000) % TotalFrames);
				if((nTmp < nFrame || AnimTime < fDelta) && !m_pSpriteData->bAnimLoop) {
					nTmp = TotalFrames-1;
				} 
				nFrame = nTmp;
			} else {
				float AnimTime = (float)(nFrame + 1) / (float)m_pSpriteData->iAnimSpd;
				int nTmp = (TotalFrames-1) - ((m_pSpriteData->iAnimSpd * CGameManager::GetLastTick()/1000) % TotalFrames);
				if((nTmp > nFrame || AnimTime < fDelta) && !m_pSpriteData->bAnimLoop) {
					nTmp = 0;
				}
				nFrame = nTmp;
			}
		} 
	} else nFrame = 0;

	if(context.m_pBuffer[nBuffer] && scontext->m_nFrame[nBuffer]!=nFrame) {
		if( m_Boundaries[nFrame].Width() != m_Boundaries[scontext->m_nFrame[nBuffer]].Width() ||
			m_Boundaries[nFrame].Height() != m_Boundaries[scontext->m_nFrame[nBuffer]].Height()) {
			context.m_pBuffer[nBuffer]->Invalidate(true); // the next frame has different size, invalidate.
		} else {
			context.m_pBuffer[nBuffer]->Touch(); // same size, the same buffer can be used, just touch.
		}
	}/**/

	scontext->m_nFrame[nBuffer] = nFrame;

	ARGBCOLOR rgbColor = scontext->getARGB();
	if(rgbColorModulation) {
		rgbColor.rgbAlpha	= (BYTE)(((int)rgbColorModulation->rgbAlpha	* (int)rgbColor.rgbAlpha)	/ 255);
		rgbColor.rgbRed		= (BYTE)(((int)rgbColorModulation->rgbRed	* (int)rgbColor.rgbRed)		/ 255);
		rgbColor.rgbGreen	= (BYTE)(((int)rgbColorModulation->rgbGreen	* (int)rgbColor.rgbGreen)	/ 255);
		rgbColor.rgbBlue	= (BYTE)(((int)rgbColorModulation->rgbBlue	* (int)rgbColor.rgbBlue)	/ 255);
	}

	CRect Rect;
	scontext->GetAbsFinalRect(Rect);
	pGraphics->Render(pTexture,							// texture
		m_Boundaries[scontext->m_nFrame[nBuffer]],		// rectSrc
		Rect,											// rectDest
		scontext->Rotation(),							// rotation (0 = 0 degrees, 1 = 90 degrees, etc.)
		scontext->Transformation(),						// transform
		rgbColor,										// rgbColor
		(float)scontext->getLightness() / 255.0f,		// lightness
		&(context.m_pBuffer[nBuffer]),					// buffer
		scontext->RelRotation(),						// relative rotation (in radians)
		scontext->RelScale()							// relative scale
		/*/(( (20 * GetTickCount())/1000 ) % 360) * 0.01745329252f, // just for testing
		(float)(((10 * GetTickCount())/1000 ) % 50) / 10 /**/
	);

	if(bBounds) pGraphics->BoundingBox(Rect, COLOR_ARGB(255,0,0,0));

	return true;
}
bool CBackground::Draw(const CDrawableContext &context, const ARGBCOLOR *rgbColorModulation) 
{ 
	if(!NeedToDraw(context)) return true;

	if(m_SptType == tMask) {
		ARGBCOLOR rgbColor = COLOR_ARGB(192,255,255,255);
		// Draw mask not showing any boundaries, and using buffer #1
		// the Mask is to be drawn  with alpha blending
		if(!CSprite::Draw(context, false, &rgbColor, 1)) {
			return false;
		}
	} else {
		if(!CSprite::Draw(context, g_bBounds, rgbColorModulation, 0)) {
			return false;
		}

		if(g_bMasks) {
			const SBackgroundData *pBackgroundData = static_cast<const SBackgroundData*>(m_pSpriteData);
			if(pBackgroundData->pMaskMap)
				pBackgroundData->pMaskMap->Draw(context, rgbColorModulation);
		}
	}
	return true; 
}
bool CEntity::Draw(const CDrawableContext &context, const ARGBCOLOR *rgbColorModulation) 
{ 
	if(g_bEntities)
		return CBackground::Draw(context, rgbColorModulation); 
	return true;
}
const IScript* CEntity::GetScript() const
{
	const SEntityData *pEntityData = static_cast<const SEntityData*>(m_pSpriteData);
	ASSERT(pEntityData);
	if(pEntityData->pScript) {
		return pEntityData->pScript;
	}
	return NULL;
}
bool CEntity::Run(const CDrawableContext &context, RUNACTION action)
{
	const SEntityData *pEntityData = static_cast<const SEntityData*>(m_pSpriteData);
	ASSERT(pEntityData);
	// If there's a script for the entity, run it:
	if(pEntityData->pScript)
		pEntityData->pScript->RunScript(context, action);
	else {
		CONSOLE_DEBUG("The entity %s has no script.\n", pEntityData->pScript);
	}
	
	return true;
}

// Srite Sheets
CSpriteSheet::CSpriteSheet(CGameManager *pGameManager) :
	CNamedObj(""),
	m_pGameManager(pGameManager),
	m_pTexture(NULL),
	m_dwLastTry(0)
{
	m_ArchiveIn = new CSpriteSheetTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
CSpriteSheet::~CSpriteSheet()
{
	if(m_pTexture) m_pTexture->Release();

	/*
	for_each(m_Sprites.begin(), m_Sprites.end(), ptr_delete());
	/*/
	mapSprite::iterator Iterator = m_Sprites.begin();
	while(Iterator != m_Sprites.end()) {
		delete Iterator->second;
		Iterator++;
	}
	/**/
	m_Sprites.clear();
}
int CSpriteSheet::ForEachSprite(FOREACHPROC ForEach, LPARAM lParam)
{
	int cnt = 0;

	BuildCatalog();

	vectorSprite::iterator Iterator = m_Catalog.begin();
	while(Iterator != m_Catalog.end()) {
		ASSERT(*Iterator);
		int aux = ForEach((LPVOID)(*Iterator), lParam);
		if(aux < 0) return aux-cnt;
		cnt += aux;
		Iterator++;
	}
	/*/
	mapSprite::iterator Iterator = m_Sprites.begin();
	while(Iterator != m_Sprites.end()) {
		ASSERT(Iterator->second);
		int aux = ForEach((LPVOID)(Iterator->second), lParam);
		if(aux < 0) return aux-cnt;
		cnt += aux;
		Iterator++;
	}
	/**/

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
		pPL->AddRange("Alpha", getAlpha(), 0, 255, SIMPLE_SLIDER, false);
		pPL->AddRange("Red Color", getRed(), -256, 255, RED_SLIDER, false);
		pPL->AddRange("Green Color", getGreen(), -256, 255, GREEN_SLIDER, false);
		pPL->AddRange("Blue Color", getBlue(), -256, 255, BLUE_SLIDER, false);
		pPL->AddRGBColor("RGB Color", getARGB(), false);
		pPL->AddRange("Lightness", getLightness(), 0, 255, SIMPLE_SLIDER, false);
	} else {
		pPL->AddValue("Width", Rect.Width());
		pPL->AddValue("Height", Rect.Height());
		pPL->AddBoolean("IsMirrored", isMirrored());
		pPL->AddBoolean("IsFlipped", isFlipped());
		pPL->AddList("Rotation", Rotation(), "0 degrees, 90 degrees, 180 degrees, 270 degrees");
		pPL->AddRange("Alpha", getAlpha(), 0, 255, SIMPLE_SLIDER);
		pPL->AddRange("Red Color", getRed(), -256, 255, RED_SLIDER);
		pPL->AddRange("Green Color", getGreen(), -256, 255, GREEN_SLIDER);
		pPL->AddRange("Blue Color", getBlue(), -256, 255, BLUE_SLIDER);
		pPL->AddRGBColor("RGB Color", getARGB());
		pPL->AddRange("Lightness", getLightness(), 0, 255, SIMPLE_SLIDER);
	}

	pPL->AddCategory("Misc");

	char szLayersList[MAX_LAYERS + sizeof(g_szLayerNames)] = {0};
	for(int i=0; i<MAX_LAYERS; i++) {
		strcat(szLayersList, g_szLayerNames[i]);
		if(i != MAX_LAYERS-1) {
			if(*g_szLayerNames[i+1] == '\0') break;
			strcat(szLayersList, ",");
		}
	}
	char szSubLayersList[MAX_SUBLAYERS + sizeof(g_szSubLayerNames)] = {0};
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		strcat(szSubLayersList, g_szSubLayerNames[i]);
		if(i != MAX_SUBLAYERS-1) {
			if(*g_szSubLayerNames[i+1] == '\0') break;
			strcat(szSubLayersList, ",");
		}
	}

	pPL->AddList("Layer", GetObjLayer(), szLayersList);
	pPL->AddList("SubLayer", GetObjSubLayer(), szSubLayersList);
	pPL->AddString("Sprite Sheet", static_cast<CSprite*>(m_pDrawableObj)->GetSpriteSheet()->GetName(), false);
	
	pPL->AddCategory("Behavior");
	pPL->AddBoolean("IsVisible", isVisible(), false);

	return true;
}

void CSpriteContext::Commit() const
{
	// Save all variables that can be indirectly changed by other property:
	// These will be secured in case a property Cancel() is called.
	Commit_rgbColor = getARGB();
}
void CSpriteContext::Cancel()
{
	ARGB(Commit_rgbColor);
}

bool CSpriteContext::SetProperties(SPropertyList &PL) 
{
	bool bChanged = false;

	SProperty* pP;

	CRect Rect;
	GetAbsFinalRect(Rect);

	pP = PL.FindProperty("Name", "Appearance", SProperty::ptString);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(strcmp(GetName(), pP->szString)) {
			SetName(pP->szString);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("X", "Appearance", SProperty::ptValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(pP->nValue - Rect.left) {
			Rect.OffsetRect(pP->nValue - Rect.left, 0);
			bChanged = true;
		}
	}
	
	pP = PL.FindProperty("Y", "Appearance", SProperty::ptValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(pP->nValue - Rect.top) {
			Rect.OffsetRect(0, pP->nValue - Rect.top);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("Width", "Appearance", SProperty::ptValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(Rect.right - Rect.left != pP->nValue) {
			Rect.right = Rect.left + pP->nValue;
			bChanged = true;
		}
	}

	pP = PL.FindProperty("Height", "Appearance", SProperty::ptValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(Rect.bottom - Rect.top != pP->nValue) {
			Rect.bottom = Rect.top + pP->nValue;
			bChanged = true;
		}
	}

	SetAbsFinalRect(Rect);

	pP = PL.FindProperty("Alpha", "Appearance", SProperty::ptRangeValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(getAlpha() != pP->nValue) {
			Alpha(pP->nValue);
			bChanged = true;
		}
	}
	pP = PL.FindProperty("Red Color", "Appearance", SProperty::ptRangeValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(getRed() != pP->nValue) {
			Red(pP->nValue);
			bChanged = true;
		}
	}
	pP = PL.FindProperty("Green Color", "Appearance", SProperty::ptRangeValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(getGreen() != pP->nValue) {
			Green(pP->nValue);
			bChanged = true;
		}
	}
	pP = PL.FindProperty("Blue Color", "Appearance", SProperty::ptRangeValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(getBlue() != pP->nValue) {
			Blue(pP->nValue);
			bChanged = true;
		}
	}
	pP = PL.FindProperty("Lightness", "Appearance", SProperty::ptRangeValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(getLightness() != pP->nValue) {
			ARGB(Commit_rgbColor); 
			Lightness(pP->nValue); // dependant variable (needs commit)
			bChanged = true;
		}
	}

	pP = PL.FindProperty("RGB Color", "Appearance", SProperty::ptRGBColor);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(getARGB().dwColor != pP->rgbColor) {
			ARGB(pP->rgbColor);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("IsMirrored", "Appearance", SProperty::ptBoolean);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(isMirrored() != pP->bBoolean) {
			Mirror(pP->bBoolean);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("IsFlipped", "Appearance", SProperty::ptBoolean);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(isFlipped() != pP->bBoolean) {
			Flip(pP->bBoolean);
			bChanged = true;
		}
	}
	
	pP = PL.FindProperty("Rotation", "Appearance", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(Rotation() != pP->nIndex) {
			Rotate(pP->nIndex);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("Layer", "Misc", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(GetObjLayer() != pP->nIndex) {
			SetObjLayer(pP->nIndex);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("SubLayer", "Misc", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(GetObjSubLayer() != pP->nIndex) {
			SetObjSubLayer(pP->nIndex);
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
	ARGB(COLOR_ARGB(255,128,128,128));
	Rotate(SROTATE_0);
	Tile(false);
}

void CSpriteSelection::BuildRealSelectionBounds()
{
	CRect RectTmp;
	m_rcSelection.SetRectEmpty();
	vectorObject::iterator Iterator;
	for(Iterator = m_Objects[0].begin(); Iterator != m_Objects[0].end(); Iterator++) {
		Iterator->pContext->GetAbsFinalRect(RectTmp);
		m_rcSelection.UnionRect(m_rcSelection, RectTmp);
		// We need to keep the initial size and location of every selected object:
		Iterator->rcRect = RectTmp;
	}
}

void CSpriteSelection::SetSelectionName(LPCSTR szName)
{
	m_ObjectsNames[m_nCurrentGroup] = szName;
}

LPCSTR CSpriteSelection::GetSelectionName(LPSTR szName, int size)
{
	SObjProp *pSel = GetFirstSelection();
	*szName = '\0';

	ASSERT(m_Objects.size() == m_ObjectsNames.size());
	if(m_nCurrentGroup>0 && m_nCurrentGroup<(int)m_ObjectsNames.size()) {
		strncpy(szName+1, m_ObjectsNames[m_nCurrentGroup].c_str(), size-2);
		*szName = '@'; // To recognize them from regular sprites, Sprite Sets names internally start with '@' (as long as the name is not changed)
	} else if(pSel) {
		strncpy(szName, (LPCSTR)pSel->pContext->GetObjName(), size-1);
	}

	szName[size - 1] = '\0';

	return szName;
}

bool CSpriteSelection::Draw(const IGraphics *pGraphics_) {
	CRect Rect(0,0,0,0);
	CRect RectTmp;

	// Always draws what's in group 0
	int nObjects = (int)m_Objects[0].size();
	const CSpriteContext *scontext = NULL;

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
		scontext = static_cast<const CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);
		Rect.UnionRect(Rect, RectTmp);
		if(nObjects>1) {
			if(!m_bHighlightOnly && !m_bFloating) {
				if(Iterator->bSubselected || !m_bHoldSelection) {
					if(m_bHoldSelection) {
						// if the object is subselected and the selection is held, draw in other color:
						pGraphics_->FillRect(RectTmp, COLOR_ARGB(92,128,128,255));
					} else {
						// only fill the selection if it isn't held:
//						if(!m_bHoldSelection) pGraphics_->FillRect(RectTmp, COLOR_ARGB(25,255,255,225));
					}
					// draw a bounding rect over the selected object:
					pGraphics_->BoundingBox(RectTmp, COLOR_ARGB(80,255,255,225));
				}
			}
			if(!m_bHighlightOnly) {
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
		}

		Iterator++;
	}
	m_bCanMove = true;
	m_bCanResize = true;

	if(!m_bFloating) {
		if(m_bHighlightOnly) {
			if(nObjects>0) {
				// draw a bounding rect over the selected objects:
				pGraphics_->FillRect(Rect, COLOR_ARGB(64,255,255,0));
				pGraphics_->BoundingBox(Rect, COLOR_ARGB(255,255,0,0));
			}
		} else {
			if(nObjects>1) {
				if(m_bHoldSelection) pGraphics_->SelectionBox(Rect, COLOR_ARGB(128,160,160,160));
				else {
					if(isGroup()) pGraphics_->SelectionBox(Rect, COLOR_ARGB(255,255,192,96));
 					else pGraphics_->SelectionBox(Rect, COLOR_ARGB(255,255,255,200));
				}
			} else if(nObjects==1) {
				if(scontext->isTiled()) {
					pGraphics_->SelectionBox(Rect, COLOR_ARGB(200,255,255,200));
				} else {
					m_bCanResize = false;
					pGraphics_->BoundingBox(Rect, COLOR_ARGB(255,255,255,200));
				}
			}
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
	// Won't resize to a null size.
	if(w<=0 || h<=0) return;

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
		if(/*(bAllowResize_ && xChain == relative) ||*/ (bAllowResize_ && xChain == stretch)) {
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
		if(/*(bAllowResize_ && yChain == relative) ||*/ (bAllowResize_ && yChain == stretch)) {
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

	if(m_Objects[0].size() == 0) {
		if(bDeleteBitmap && ppBitmap) {
			*ppBitmap = NULL;
			delete []pBitmap;
		}
		return NULL;
	}

	int bmpsize = 0;
	int indexsize = 0;
	int datasize = sizeof(_SpriteSet::_SpriteSetInfo);
	if(pBitmap) bmpsize = sizeof(BITMAP) + pBitmap->bmHeight * pBitmap->bmWidthBytes;

	///////////////////////////////////////////////////////////////////////////////////////
	// Analize the selected objeects to find out the needed size for the sprite set:

	// we need a map to hold every different object (to make an object index.)
	std::map<CDrawableObject*, int> ObjectsNames;

	vectorObject::iterator Iterator = m_Objects[0].begin();
	for(int i=0; Iterator != m_Objects[0].end(); i++) {
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
		if( scontext->getARGB().rgbAlpha != 255 ) {
			datasize += sizeof(_SpriteSet::_SpriteSetData03);
		}

		// if it has an alpha value set, it will need mask: SSD_RGBL
		if( scontext->getRGB().dwColor != COLOR_RGB(128,128,128).dwColor ) {
			datasize += sizeof(_SpriteSet::_SpriteSetData04);
		}

		// try to insert the drawable object in the index...
		if( ObjectsNames.insert(
				pair<CDrawableObject*, int>(scontext->GetDrawableObj(), -1)
			).second == true) {
			// a new item has been found (and inserted), so we increase the index size
			int nNameLen = strlen(scontext->GetObjName()) + 1;
			ASSERT(nNameLen < 30);
			indexsize += nNameLen + sizeof(WORD); // name length + offset of the name in the index.
		}

		Iterator++;
	}
	int asize = ((datasize + indexsize + 15) / 16) * 16; // align to 16 bytes.

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
	LPBYTE pRawBuffer = (LPBYTE)::GlobalLock(hglbCopy);
	ASSERT(pRawBuffer);
#ifdef _DEBUG
	memset(pRawBuffer, 0, asize + bmpsize);
#else 
	memset(pRawBuffer, 0, sizeof(_SpriteSet::_SpriteSetInfo)); // clear the header.
#endif

	CRect rcBoundaries, RectTmp;
	GetBoundingRect(&rcBoundaries);

	_SpriteSet *CopyBoard = (_SpriteSet*)pRawBuffer;
	strcpy(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID);
	strcat(CopyBoard->Info.Header.ID, "\nUntitled Sprite Set\nDescription goes here.");
	CopyBoard->Info.Header.dwSignature = OLF_SPRITE_SET_SIGNATURE;
	CopyBoard->Info.Header.dwSize = asize + bmpsize;
	CopyBoard->Info.nObjects = (int)m_Objects[0].size();

	CopyBoard->Info.rcBoundaries = rcBoundaries;
	CopyBoard->Info.rcBoundaries.OffsetRect(-rcBoundaries.TopLeft());
	CopyBoard->Info.Header.dwBitmapOffset = 0;
	CopyBoard->Info.Header.dwDataOffset = sizeof(_SpriteSet::_SpriteSetInfo) + indexsize;

	LPWORD pIndexOffset = (LPWORD)(pRawBuffer + sizeof(_SpriteSet::_SpriteSetInfo));
	LPSTR szIndexNames = (LPSTR)(pIndexOffset + ObjectsNames.size());

	LPBYTE pData = pRawBuffer + CopyBoard->Info.Header.dwDataOffset;

	// fill the index:
	int index = 0;
	std::map<CDrawableObject*, int>::iterator IndexIterator = ObjectsNames.begin();
	while(IndexIterator != ObjectsNames.end()) {
		ASSERT(pIndexOffset < (LPWORD)szIndexNames);
		ASSERT((LPBYTE)szIndexNames < pData);
		*pIndexOffset++ = (LPBYTE)szIndexNames - pRawBuffer;
		strcpy(szIndexNames, IndexIterator->first->GetName());
		szIndexNames += (strlen(szIndexNames) + 1);
		IndexIterator->second = index++;
		IndexIterator++;
	}

	Iterator = m_Objects[0].begin();
	for(i=0; Iterator != m_Objects[0].end(); i++) {
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		ASSERT(scontext);
		SObjProp *pObjProp = static_cast<SObjProp *>(Iterator.operator ->());
		CSprite *pSprite = static_cast<CSprite *>(scontext->GetDrawableObj());
		ASSERT(pSprite);
		ASSERT(pObjProp);

		int Mask = 0;
		_SpriteSet::_SpriteSetData01 *pSpriteSetData01 = NULL;
		_SpriteSet::_SpriteSetData02 *pSpriteSetData02 = NULL;
		_SpriteSet::_SpriteSetData03 *pSpriteSetData03 = NULL;
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
		if( scontext->getARGB().rgbAlpha != 255 ) {
			Mask |= SSD_ALPHA;
			pSpriteSetData03 = (_SpriteSet::_SpriteSetData03 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData03);
		}

		// if it has an alpha value set, it will need mask: SSD_RGBL
		if( scontext->getRGB().dwColor != COLOR_RGB(128,128,128).dwColor ) {
			Mask |= SSD_RGBL;
			pSpriteSetData04 = (_SpriteSet::_SpriteSetData04 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData04);
		}

		///////////////////////////////////////////////////////////////////////////
		// Now fill in the needed data:

		scontext->GetAbsFinalRect(RectTmp);
		RectTmp.OffsetRect(-rcBoundaries.TopLeft());

		ASSERT(pSpriteSetData);

		ASSERT(Mask < 0x0040);
		ASSERT(scontext->GetObjLayer() < 0x0008);
		ASSERT(scontext->GetObjSubLayer() < 0x0008);

		pSpriteSetData->Mask = Mask;
		pSpriteSetData->Layer = scontext->GetObjLayer();
		pSpriteSetData->SubLayer = scontext->GetObjSubLayer();
		pSpriteSetData->X = (WORD)RectTmp.left;
		pSpriteSetData->Y = (WORD)RectTmp.top;

		IndexIterator = ObjectsNames.find(scontext->GetDrawableObj());
		ASSERT(IndexIterator != ObjectsNames.end());
		pSpriteSetData->ObjIndex = IndexIterator->second;
		ASSERT(pSpriteSetData->ObjIndex != -1);

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
		if(pSpriteSetData03) { // SSD_ALPHA
			pSpriteSetData03->Alpha = scontext->getARGB().rgbAlpha;
		}
		if(pSpriteSetData04) { // SSD_RGBL
			pSpriteSetData04->Red = scontext->getARGB().rgbRed;
			pSpriteSetData04->Green = scontext->getARGB().rgbGreen;
			pSpriteSetData04->Blue = scontext->getARGB().rgbBlue;
		}

		Iterator++;
		ASSERT(pData <= pRawBuffer + indexsize + datasize);
	}

	ASSERT(pData == pRawBuffer + indexsize + datasize);
	CONSOLE_DEBUG("Sprite Set created, %d bytes: %d bytes used for data, and %d bytes for the thumbnail\n", asize + bmpsize, datasize, bmpsize);

	if(pBitmap) {
		CopyBoard->Info.Header.dwBitmapOffset = (DWORD)asize; // offset of the bitmap
		memcpy(pRawBuffer + asize, pBitmap, sizeof(BITMAP));
		memcpy(pRawBuffer + asize + sizeof(BITMAP), pBitmap->bmBits, bmpsize - sizeof(BITMAP));
	}

	::GlobalUnlock(hglbCopy); 

	if(bDeleteBitmap && ppBitmap) {
		*ppBitmap = NULL;
		delete []pBitmap; // free bitmap memory
	}
	return hglbCopy;
}
CRect CSpriteSelection::PasteSprite(CLayer *pLayer, CSprite *pSprite, const CPoint *pPoint, bool bPaste) 
{
	CRect RetRect(0,0,0,0);

	// PasteSprite needs no paste group, so we clear the paste group if set:
	m_nPasteGroup = 0;
	m_nCurrentGroup = 0;

	if(pSprite->GetSpriteType() == tMask) {
		if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error: Attempt to use mask '%s' as a sprite\n", pSprite->GetName());
		return RetRect;
	}
	if(!pLayer) {
		if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error : Couldn't paste '%s' in the requested layer!\n", pSprite->GetName());
		return RetRect;
	}

	CSize Size;
	pSprite->GetSize(Size);
	RetRect.SetRect(0, 0, Size.cx, Size.cy);

	if(bPaste) {
		if(!pPoint) CleanSelection();

		CSpriteContext *pSpriteContext = new CSpriteContext("");
		pSpriteContext->SetDrawableObj(pSprite);

		if(pSprite->GetSpriteType() == tBackground) pSpriteContext->Tile();
		pSpriteContext->SetObjSubLayer(static_cast<CBackground *>(pSprite)->GetObjSubLayer());
		pSpriteContext->Alpha(static_cast<CBackground *>(pSprite)->GetAlphaValue());

		pLayer->AddSpriteContext(pSpriteContext, true); // insert the sprite in the current layer
		if(!pPoint) {
			m_Objects[0].push_back(SObjProp(this, pSpriteContext));
			pSpriteContext->MoveTo(0, 0);
			pSpriteContext->SelectContext();

			m_bFloating = true;
			StartMoving(CPoint(Size.cx/2, Size.cy/2));
		} else {
			pSpriteContext->MoveTo(*pPoint);
		}
	}

	if(pPoint) RetRect.OffsetRect(*pPoint);

	return RetRect;
}

CRect CSpriteSelection::PasteSprite(CLayer *pLayer, LPCSTR szSprite, const CPoint *pPoint, bool bPaste) 
{
	CSprite *pSprite = CGameManager::Instance()->FindSprite(szSprite);

	if(!pSprite) {
		if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error : Couldn't find the requested sprite (1): '%s'!\n", szSprite);
		return CRect(0,0,0,0);
	}
	return PasteSprite(pLayer, pSprite, pPoint, bPaste);
}
CRect CSpriteSelection::PasteSpriteSet(CLayer *pLayer, const LPBYTE pRawBuffer, const CPoint *pPoint, bool bPaste)
{
	CRect RetRect(0,0,0,0);

	ASSERT(pRawBuffer);

	_SpriteSet *CopyBoard = (_SpriteSet*)pRawBuffer;

	if(!pPoint && bPaste) CleanSelection();
	LPSTR szName = NULL;

	LPWORD pIndexOffset = (LPWORD)((LPBYTE)CopyBoard + sizeof(_SpriteSet::_SpriteSetInfo));
	LPBYTE pData = (LPBYTE)CopyBoard + CopyBoard->Info.Header.dwDataOffset;

	UINT nLayer = -1;

	try {
		char szSpriteSetName[200];
		// get the next availible paste group:
		m_nCurrentGroup = SetNextPasteGroup(GetNameFromOLFile(&CopyBoard->Info.Header, szSpriteSetName, sizeof(szSpriteSetName)));
		CONSOLE_DEBUG("Sprite Set '%s' to be pasted as group #%d\n", szSpriteSetName, m_nPasteGroup);

		for(UINT i=0; i<CopyBoard->Info.nObjects; i++) {

			_SpriteSet::_SpriteSetData01 *pSpriteSetData01 = NULL;
			_SpriteSet::_SpriteSetData02 *pSpriteSetData02 = NULL;
			_SpriteSet::_SpriteSetData03 *pSpriteSetData03 = NULL;
			_SpriteSet::_SpriteSetData04 *pSpriteSetData04 = NULL;

			// Initialize structures from data and mask:
			_SpriteSet::_SpriteSetData *pSpriteSetData = (_SpriteSet::_SpriteSetData *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData);

			if((pSpriteSetData->Mask & SSD_WIDTHHEIGHT) == SSD_WIDTHHEIGHT) {
				pSpriteSetData01 = (_SpriteSet::_SpriteSetData01 *)pData;
				pData += sizeof(_SpriteSet::_SpriteSetData01);
			}
			if((pSpriteSetData->Mask & SSD_TRANS) == SSD_TRANS) {
				pSpriteSetData02 = (_SpriteSet::_SpriteSetData02 *)pData;
				pData += sizeof(_SpriteSet::_SpriteSetData02);
			}
			if((pSpriteSetData->Mask & SSD_ALPHA) == SSD_ALPHA) {
				pSpriteSetData03 = (_SpriteSet::_SpriteSetData03 *)pData;
				pData += sizeof(_SpriteSet::_SpriteSetData03);
			}
			if((pSpriteSetData->Mask & SSD_RGBL) == SSD_RGBL) {
				pSpriteSetData04 = (_SpriteSet::_SpriteSetData04 *)pData;
				pData += sizeof(_SpriteSet::_SpriteSetData04);
			}
			szName = (LPSTR)CopyBoard + pIndexOffset[pSpriteSetData->ObjIndex];

			// Build sprite:
			if(nLayer!=-1 || !pLayer) {
				if(nLayer != pSpriteSetData->Layer) {
					nLayer = pSpriteSetData->Layer;
					pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(nLayer));
				}
			} // else if there is a current layer selected, use that layer instead

			CSprite *pSprite = CGameManager::Instance()->FindSprite(szName);
			if(!pSprite) {
				if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error : Couldn't find the requested sprite (2): '%s'!\n", szName);
				continue;
			}
			if(pSprite->GetSpriteType() == tMask) {
				if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error: Attempt to use mask '%s' as a sprite\n", szName);
				continue;
			}
			if(!pLayer) {
				if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error : Couldn't paste '%s' in the requested layer!\n", szName);
				continue;
			}

			// Start the Rect with the width and heignt of the sprite (defaults for entities)
			CRect RectTmp(pSpriteSetData->X, pSpriteSetData->Y, pSpriteSetData->X - 1, pSpriteSetData->Y - 1);

			if(bPaste) {
				// Fill sprite context data:

				ARGBCOLOR rgbColor = COLOR_ARGB(255,128,128,128);
				int rotation = 0;
				bool mirrored = false;
				bool flipped = false;
				
				_Chain XChain = relative;
				_Chain YChain = relative;

				CSpriteContext *pSpriteContext = new CSpriteContext(""); // New sprite context with no name.
				pSpriteContext->SetDrawableObj(pSprite);

				pSpriteContext->SetObjSubLayer(pSpriteSetData->SubLayer);

				// We ignore Width and Height from entities (shouldn't be there anyway) ...
				if(pSprite->GetSpriteType() != tEntity) {
					if(pSpriteSetData01) { //SSD_WIDTHHEIGHT
						ASSERT((pSpriteSetData->Mask & SSD_WIDTHHEIGHT) == SSD_WIDTHHEIGHT);
						RectTmp.right = RectTmp.left + pSpriteSetData01->Width;
						RectTmp.bottom = RectTmp.top + pSpriteSetData01->Height;
						pSpriteContext->Tile();
						if(RectTmp.right < RectTmp.left || RectTmp.bottom < RectTmp.top) {
							CONSOLE_PRINTF("Paste error: Corrupted Sprite: '%s' (in the Sprite Set '%s')\n", szName, szSpriteSetName);
							continue;
						}
					} else {
						CSize Size;
						pSprite->GetSize(Size);
						RectTmp.right = RectTmp.left + Size.cx;
						RectTmp.bottom = RectTmp.top + Size.cy;
					}
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
				if(pSpriteSetData03) { //SSD_ALPHA
					ASSERT((pSpriteSetData->Mask & SSD_ALPHA) == SSD_ALPHA);
					rgbColor.rgbAlpha = pSpriteSetData03->Alpha;
					if(rgbColor.rgbAlpha == 0) {
						CONSOLE_PRINTF("Paste warning: Invisible Sprite: '%s'\n", szName);
					}
				}
				if(pSpriteSetData04) { //SSD_RGBL
					ASSERT((pSpriteSetData->Mask & SSD_RGBL) == SSD_RGBL);
					rgbColor.rgbRed = pSpriteSetData04->Red;
					rgbColor.rgbGreen = pSpriteSetData04->Green;
					rgbColor.rgbBlue = pSpriteSetData04->Blue;
				}

				pSpriteContext->Rotate(rotation);
				pSpriteContext->Mirror(mirrored);
				pSpriteContext->Flip(flipped);
				pSpriteContext->ARGB(rgbColor);

				pLayer->AddSpriteContext(pSpriteContext, true); // insert the sprite in the current layer
				if(!pPoint) {
					// the sprite absolute postion must be set after inserting it in the layer.
					pSpriteContext->SelectContext();
					pSpriteContext->SetAbsFinalRect(RectTmp);
					// push to the main group:
					m_Objects[0].push_back(SObjProp(this, pSpriteContext, RectTmp, XChain, YChain));
					// push to the current paste group:
					if(m_nPasteGroup != 0) m_Objects[m_nPasteGroup].push_back(SObjProp(this, pSpriteContext, RectTmp, XChain, YChain));
				} else {
					RectTmp.OffsetRect(*pPoint);
					pSpriteContext->SetAbsFinalRect(RectTmp);
					// push to the current paste group:
					if(m_nPasteGroup != 0) m_Objects[m_nPasteGroup].push_back(SObjProp(this, pSpriteContext, RectTmp, XChain, YChain));
				}
			} else {
				if(pSpriteSetData01) { //SSD_WIDTHHEIGHT
					ASSERT((pSpriteSetData->Mask & SSD_WIDTHHEIGHT) == SSD_WIDTHHEIGHT);
					RectTmp.right = RectTmp.left + pSpriteSetData01->Width;
					RectTmp.bottom = RectTmp.top + pSpriteSetData01->Height;
				}
				if(pSpriteSetData02) { //SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
					ASSERT((pSpriteSetData->Mask & SSD_TRANS) == SSD_TRANS);
				}
				if(pSpriteSetData03) { //SSD_ALPHA
					ASSERT((pSpriteSetData->Mask & SSD_ALPHA) == SSD_ALPHA);
				}
				if(pSpriteSetData04) { //SSD_RGBL
					ASSERT((pSpriteSetData->Mask & SSD_RGBL) == SSD_RGBL);
				}
				if(pPoint) RectTmp.OffsetRect(*pPoint);
			}

			RetRect.UnionRect(RetRect, RectTmp);
		}

		// there was only a single sprite in the sprite set, so we set the 
		// paste group to 0 and clear the used group:
		if(m_nPasteGroup != 0 && m_Objects[m_nPasteGroup].size() <= 1) {
			m_Objects[m_nPasteGroup].clear();
			m_nCurrentGroup = m_nPasteGroup = 0;
		}

		if(!pPoint && bPaste) {
			m_bFloating = true;
			StartMoving(CopyBoard->Info.rcBoundaries.CenterPoint());
		}
	} 
	catch(...) {
		CONSOLE_PRINTF("Paste error : Couldn't paste corrupted sprite set!\n");
	}

	return RetRect;
}
CRect CSpriteSelection::PasteFile(CLayer *pLayer, LPCSTR szFilePath, const CPoint *pPoint, bool bPaste)
{
	CRect RetRect(0,0,0,0);

	// validate the possibility of receiving a group:
	if(*szFilePath == '@') szFilePath++;

	CVFile fnFile(szFilePath);
	if(!fnFile.FileExists()) return RetRect;

	BYTE Header[sizeof(_SpriteSet::_SpriteSetInfo)];
	_SpriteSet *CopyBoard = (_SpriteSet*)Header;

	fnFile.Open("r");
	fnFile.Read(Header, sizeof(_SpriteSet::_SpriteSetInfo));
	fnFile.Close();
	if(strncmp(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID, sizeof(OLF_SPRITE_SET_ID)-1)) return RetRect;
	else if(CopyBoard->Info.Header.dwSignature != OLF_SPRITE_SET_SIGNATURE) return RetRect;

	fnFile.Open("r");
	LPBYTE pRawBuffer = (LPBYTE)fnFile.ReadFile();
	RetRect = PasteSpriteSet(pLayer, pRawBuffer, pPoint, bPaste);
	fnFile.Close();

	return RetRect;
}
bool CSpriteSelection::GetPastedSize(LPCVOID pBuffer, SIZE *pSize)
{
	pSize->cx = 0;
	pSize->cy = 0;

	CPoint Point(0,0);
	CRect RetRect(0,0,0,0);

	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	_SpriteSet *CopyBoard = (_SpriteSet*)pBuffer;
	// is it a sprite set?
	if(strncmp(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID, sizeof(OLF_SPRITE_SET_ID)-1)) {
		RetRect = PasteFile(pLayer, (LPCSTR)pBuffer, &Point, false);
		if(RetRect.IsRectNull()) {
			RetRect = PasteSprite(pLayer, (LPCSTR)pBuffer, &Point, false);
			if(RetRect.IsRectNull()) return false;
		}
	} else {
		if(CopyBoard->Info.Header.dwSignature != OLF_SPRITE_SET_SIGNATURE) {
			CONSOLE_PRINTF("Paste error : Attempt to paste an invalid Quest Designer Sprite Set!\n");
			return false;
		}
		RetRect = PasteSpriteSet(pLayer, (LPBYTE)pBuffer, &Point, false);
		if(RetRect.IsRectNull()) return false;
	}
	pSize->cx = RetRect.Width();
	pSize->cy = RetRect.Height();
	return true;
}
bool CSpriteSelection::GetPastedSize(CSprite *pSprite, SIZE *pSize)
{
	CSize Size;

	if(pSprite->GetSpriteType() == tMask) {
		pSize->cx = 0;
		pSize->cy = 0;
		return false;
	}

	pSprite->GetSize(Size);
	*pSize = Size;
	return true;
}

bool CSpriteSelection::FastPaste(CSprite *pSprite, const CPoint &point_ )
{
	CRect RetRect(0,0,0,0);

	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));
	RetRect = PasteSprite(pLayer, pSprite, &point_);
	return (RetRect.IsRectNull() == false);
}

bool CSpriteSelection::FastPaste(LPCVOID pBuffer, const CPoint &point_ )
{
	CRect RetRect(0,0,0,0);

	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	_SpriteSet *CopyBoard = (_SpriteSet*)pBuffer;
	// is it a sprite set?
	if(strncmp(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID, sizeof(OLF_SPRITE_SET_ID)-1)) {
		RetRect = PasteFile(pLayer, (LPCSTR)pBuffer, &point_);
		if(RetRect.IsRectNull()) {
			RetRect = PasteSprite(pLayer, (LPCSTR)pBuffer, &point_);
			if(RetRect.IsRectNull()) return false;
		}
	} else {
		if(CopyBoard->Info.Header.dwSignature != OLF_SPRITE_SET_SIGNATURE) {
			CONSOLE_PRINTF("Paste error : Attempt to paste an invalid Quest Designer Sprite Set!\n");
			return false;
		}
		RetRect = PasteSpriteSet(pLayer, (LPBYTE)pBuffer, &point_);
		if(RetRect.IsRectNull()) return false;
	}

	return true;
}

bool CSpriteSelection::Paste(LPCVOID pBuffer, const CPoint &point_)
{
	ASSERT(pBuffer);
	if(m_bHoldSelection) {
		CONSOLE_PRINTF("Warning: Can not paste on held selection, release the selection first.\n");
		return false;
	}

	CRect RetRect(0,0,0,0);
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	_SpriteSet *CopyBoard = (_SpriteSet*)pBuffer;
	// is it a sprite set?
	if(strncmp(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID, sizeof(OLF_SPRITE_SET_ID)-1)) {
		RetRect = PasteFile(pLayer, (LPCSTR)pBuffer);
		if(RetRect.IsRectNull()) {
			RetRect = PasteSprite(pLayer, (LPCSTR)pBuffer);
			if(RetRect.IsRectNull()) return false;
		}
	} else {
		if(CopyBoard->Info.Header.dwSignature != OLF_SPRITE_SET_SIGNATURE) {
			CONSOLE_PRINTF("Paste error : Attempt to paste an invalid Quest Designer Sprite Set!\n");
			return false;
		}
		RetRect = PasteSpriteSet(pLayer, (LPBYTE)pBuffer);
		if(RetRect.IsRectNull()) return false;
	}

	if(m_Objects[m_nPasteGroup].empty()) {
		EndMoving(point_);
		if(m_nPasteGroup) CONSOLE_PRINTF("Paste error : Attempt to paste an empty Sprite!\n");
		else CONSOLE_PRINTF("Paste error : Attempt to paste an empty Quest Designer Sprite Set!\n");
		return false;
	}

	MoveTo(point_);

	CONSOLE_DEBUG("%d of %d objects selected.\n", m_Objects[m_nPasteGroup].size(), (*m_ppMainDrawable)->Objects());
	return true;
}

void CSpriteSelection::SelectionToGroup(LPCSTR szGroupName)
{
	if(m_bHoldSelection) return;
	// no new groups can be created from an empty selection or a single sprite selection.
	if(m_Objects[0].size() <= 1) return;

	// get the next availible paste group:
	m_nCurrentGroup = SetNextPasteGroup(szGroupName);
	CONSOLE_DEBUG("New group #%d created\n", m_nCurrentGroup);

	ASSERT(m_nCurrentGroup > 0);

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
		m_Objects[m_nCurrentGroup].push_back(*Iterator);
		Iterator++;
	}

}
// converts the currently selected group to a simple selection:
void CSpriteSelection::GroupToSelection()
{
	if(m_bHoldSelection) return;
	// no new groups can be created from an empty selection or a single sprite selection.
	if(m_Objects[0].size() <= 1) return;
	if(m_nCurrentGroup < 1) return;

	m_Objects[m_nCurrentGroup].clear();
	m_nCurrentGroup = 0;
}
void CSpriteSelection::SelectionToTop()
{
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	// we start by sorting the selected objects (in the selection):
	SortSelection();
	int nNextOrder = pLayer->ReOrder(1);

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
		if(m_bHoldSelection && Iterator->bSubselected || !m_bHoldSelection) {
			Iterator->pContext->SetObjOrder(nNextOrder++);
		}
		Iterator++;
	}
}
void CSpriteSelection::SelectionToBottom()
{
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	// we start by sorting the selected objects (in the selection):
	SortSelection();
	int nNextOrder = pLayer->ReOrder(1, 0, m_Objects[0].size());

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
		if(m_bHoldSelection && Iterator->bSubselected || !m_bHoldSelection) {
			Iterator->pContext->SetObjOrder(nNextOrder++);
		}
		Iterator++;
	}
}
void CSpriteSelection::SelectionDown()
{
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	// we start by sorting the selected objects (in the selection):
	SortSelection();
	int nNextOrder = pLayer->ReOrder(2); // reorder everything leaving a space between objects.

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
		if(m_bHoldSelection && Iterator->bSubselected || !m_bHoldSelection) {
			int nNextOrder = Iterator->pContext->GetObjOrder() - 3;
			if(nNextOrder < 0) nNextOrder = 0;
			Iterator->pContext->SetObjOrder(nNextOrder);
		}
		Iterator++;
	}
}
void CSpriteSelection::SelectionUp()
{
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	// we start by sorting the selected objects (in the selection):
	SortSelection();
	int nNextOrder = pLayer->ReOrder(2); // reorder everything leaving a space between objects.

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
		if(m_bHoldSelection && Iterator->bSubselected || !m_bHoldSelection) {
			int nNextOrder = Iterator->pContext->GetObjOrder() + 3;
			Iterator->pContext->SetObjOrder(nNextOrder);
		}
		Iterator++;
	}
}

void CSpriteSelection::FlipSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(&rcBoundaries);

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
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

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
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

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
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

	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
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
