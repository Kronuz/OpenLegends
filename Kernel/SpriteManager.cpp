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
/*! \file		SpriteManager.cpp
	\author		Germán Méndez Bravo (Kronuz)
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
				July 15, 2005:
						* Added CMutable Touch() calls
				July 19, 2005:
						* Moved CSpriteSelection to the file SelectionManager.(cpp|h)
				February 12, 2006:
						* CEntityData creation in CSpriteContext (logical storage area since similar data will be manipulated.)
						* Added implementation of CEntityData.
				Feb 13, 2006:
						+ Implemented CEntityData static functions for retrieval of indexed Contexts.

	This file implements all the classes that manage the sprites,
	including backgrounds, sprite sheets, animations, mask maps and
	entities.
*/

#include "stdafx.h"
#include "SpriteManager.h"

#include <set>

#include "GameManager.h"
#include "ScriptManager.h"
#include "Script.h"
#include "ArchiveText.h"

bool g_bBounds = false;
bool g_bMasks = false;
bool g_bEntities = false;

std::vector<CEntityData::contextPair> CEntityData::ms_ContextIndex;

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
	BEGIN_DESTRUCTOR
	delete m_pSpriteData;
	END_DESTRUCTOR
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

void CEntityData::UpdateVariables(){
	CPoint pos;
	if(strcmp(m_pParent->GetName(), SPECIALENTITYWORLD)){	//We won't do the same checks for this entity.
		m_pParent->GetPosition(pos);
		SetValue("_x", pos.x);
		SetValue("_y", pos.y);
	}
}
void CEntityData::UpdateSpecialVariable(LPCSTR Id, int Value, bool in){
	if(in){
		if(!strcmp("_x", Id)){
			CPoint pos;
			m_pParent->GetPosition(pos);
			pos.x = Value;
			m_pParent->MoveTo(pos);
			if(!strcmp(m_pParent->GetName(), SPECIALENTITYWORLD)){
				CPoint *pt = CGameManager::Instance()->GetWorldCo();
				CGameManager::Instance()->UpdateWorldCo(pos.x, pos.y);
			}
		} else if (!strcmp("_y", Id)){
			CPoint pos;
			m_pParent->GetPosition(pos);
			pos.y = Value;
			m_pParent->MoveTo(pos);
			if(!strcmp(m_pParent->GetName(), SPECIALENTITYWORLD)){
				CPoint *pt = CGameManager::Instance()->GetWorldCo();
				CGameManager::Instance()->UpdateWorldCo(pos.x, pos.y);
			}
		}
		
	} else {
		if(!strcmp(m_pParent->GetName(), SPECIALENTITYWORLD)){
			CPoint *pt = CGameManager::Instance()->GetWorldCo();
			if(!pt) return;
			SetValue("_x", pt->x);
			SetValue("_y", pt->y);
		}
	}
}
void CEntityData::UpdateSpecialVariable(LPCSTR Id, LPCSTR Text, bool in){

}
void CEntityData::UpdateSpecialVariable(LPCSTR Id, bool Set, bool in){

}

CDrawableContext* CEntityData::FindContext(LPCSTR szName){
	std::vector<contextPair>::iterator iter = ms_ContextIndex.begin();
	for(;iter < ms_ContextIndex.end(); iter++){
		if(!(*iter).first.Compare(szName)){
			return (*iter).second;
		}
	}
	return NULL;	//Not found.
}

void CEntityData::RemoveContext(LPCSTR szName){
	std::vector<contextPair>::iterator iter = ms_ContextIndex.begin();
	for(;iter < ms_ContextIndex.end(); iter++){
		if(!(*iter).first.Compare(szName)){
			ms_ContextIndex.erase(iter);
			return;
		}
	}
}

int CEntityData::GetValue(int Id){
	std::vector<vvPair>::iterator iter = m_vvStorage.begin();
	for(;iter < m_vvStorage.end(); iter++){
		if((*iter).first == Id) return (*iter).second;
	}
	return 0;
}
int CEntityData::GetValue(LPCSTR Id){
	UpdateSpecialVariable(Id, NULL, false);
	std::vector<svPair>::iterator iter = m_svStorage.begin();
	for(;iter < m_svStorage.end(); iter++){
		if(!(*iter).first.Compare(Id)) return (*iter).second;
	}
	return 0;	
}

CBString CEntityData::GetString(int Id){
	std::vector<vsPair>::iterator iter = m_vsStorage.begin();
	for(;iter < m_vsStorage.end(); iter++){
		if((*iter).first == Id) return ((*iter).second);
	}
	return "";
}
CBString CEntityData::GetString(LPCSTR Id){
	UpdateSpecialVariable(Id, NULL, false);
	std::vector<ssPair>::iterator iter = m_ssStorage.begin();
	for(;iter < m_ssStorage.end(); iter++){
		if(!(*iter).first.Compare(Id)) return ((*iter).second);
	}
	return "";
}

bool CEntityData::GetFlag(int Id){
	std::vector<vfPair>::iterator iter = m_vfStorage.begin();
	for(;iter < m_vfStorage.end(); iter++){
		if((*iter).first == Id) return (*iter).second;
	}
	return false;
}
bool CEntityData::GetFlag(LPCSTR Id){
	UpdateSpecialVariable(Id, NULL, false);
	std::vector<sfPair>::iterator iter = m_sfStorage.begin();
	for(;iter < m_sfStorage.end(); iter++){
		if(!(*iter).first.Compare(Id)) return (*iter).second;
	}
	return false;
}

bool CEntityData::SetValue(int Id, int Value){
	std::vector<vvPair>::iterator iter = m_vvStorage.begin();
	for(;iter < m_vvStorage.end(); iter++){
		if((*iter).first == Id){
			(*iter).second = Value;
			return true;
		}
	}
	vvPair obj;
	obj.first = Id;
	obj.second = Value;
	m_vvStorage.push_back(obj);
	return false;
}
bool CEntityData::SetValue(LPCSTR Id, int Value){
	std::vector<svPair>::iterator iter = m_svStorage.begin();
	UpdateSpecialVariable(Id, Value);
	for(;iter < m_svStorage.end(); iter++){
		if(!(*iter).first.Compare(Id)){
			(*iter).second = Value;
			return true;
		}
	}
	svPair obj;
	obj.first = CBString(Id);
	obj.second = Value;
	m_svStorage.push_back(obj);
	
	return false;
}

bool CEntityData::SetString(int Id, LPCSTR Text){
	std::vector<vsPair>::iterator iter = m_vsStorage.begin();
	for(;iter < m_vsStorage.end(); iter++){
		if((*iter).first == Id){
			(*iter).second = CBString(Text);
			return true;
		}
	}
	vsPair obj;
	obj.first = Id;
	obj.second = CBString(Text);
	m_vsStorage.push_back(obj);
	return false;
}
bool CEntityData::SetString(LPCSTR Id, LPCSTR Text){
	std::vector<ssPair>::iterator iter = m_ssStorage.begin();
	UpdateSpecialVariable(Id, Text);
	for(;iter < m_ssStorage.end(); iter++){
		if(!(*iter).first.Compare(Id)){
			(*iter).second = CBString(Text);
			return true;
		}
	}
	ssPair obj;
	obj.first = CBString(Id);
	obj.second = CBString(Text);
	m_ssStorage.push_back(obj);
	
	return false;
}

bool CEntityData::SetFlag(int Id, bool Set){
	std::vector<vfPair>::iterator iter = m_vfStorage.begin();
	for(;iter < m_vfStorage.end(); iter++){
		if((*iter).first == Id){
			(*iter).second = Set;
			return true;
		}
	}
	vfPair obj;
	obj.first = Id;
	obj.second = Set;
	m_vfStorage.push_back(obj);
	return false;
}
bool CEntityData::SetFlag(LPCSTR Id, bool Set){
	std::vector<sfPair>::iterator iter = m_sfStorage.begin();
	UpdateSpecialVariable(Id, Set);
	for(;iter < m_sfStorage.end(); iter++){
		if(!(*iter).first.Compare(Id)){
			(*iter).second = Set;
			return true;
		}
	}
	sfPair obj;
	obj.first = CBString(Id);
	obj.second = Set;
	m_sfStorage.push_back(obj);
	
	return false;
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
		//CONSOLE_DEBUG("The entity %s has no script.\n", pEntityData->pScript);
		//TODO: Disable this message after one run to avoid game-slowdowns.
		//Disabled for now.
	}
	
	return true;
}

// Srite Sheets
CSpriteSheet::CSpriteSheet(CGameManager *pGameManager) :
	CNamedObj(""),
	CDocumentObject(),
	m_pGameManager(pGameManager),
	m_pTexture(NULL),
	m_dwLastTry(0)
{
	m_ArchiveIn = new CSpriteSheetTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
CSpriteSheet::~CSpriteSheet()
{
	BEGIN_DESTRUCTOR
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

	delete m_ArchiveIn;
	if(m_ArchiveIn != m_ArchiveOut) delete m_ArchiveOut;

	END_DESTRUCTOR
}
int CSpriteSheet::ForEachSprite(SIMPLEPROC ForEach, LPARAM lParam)
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSpriteContext Implementation:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrawableContext* CSpriteContext::MakeGroup(LPCSTR szGroupName)
{
	CSpriteSetContext *pGroupContext = new CSpriteSetContext(szGroupName);
	return pGroupContext;
}

// Memento interface
void CSpriteContext::ReadState(StateData *data)
{
	CDrawableContext::ReadState(data);
	StateSpriteContext *curr = static_cast<StateSpriteContext *>(data);
	curr->rgbColor = m_rgbColor;
	curr->eXChain = m_eXChain;
	curr->eYChain = m_eYChain;
}
void CSpriteContext::WriteState(StateData *data)
{
	CDrawableContext::WriteState(data);
	StateSpriteContext *curr = static_cast<StateSpriteContext *>(data);
	m_rgbColor = curr->rgbColor;
	m_eXChain = curr->eXChain;
	m_eYChain = curr->eYChain;

}
int CSpriteContext::_SaveState(UINT checkpoint)
{
	// This is needed to delete no longer used objects (garbage collector):
	if(m_pParent && isDeleted() && !StateCount(checkpoint)) {
		VERIFY(m_pParent->PopChild(this));
		delete this; //since we are deleted, and we won't be able to reappear from an undo, we commit suicide
		return 0;
	}
	// Create a new state and read the object's information into it:
	StateSpriteContext *curr = new StateSpriteContext;
	ReadState(curr);
	// Save the object's state (SaveState decides if there are changes to be saved)
	return SetState(checkpoint, curr);
}
int CSpriteContext::_RestoreState(UINT checkpoint)
{
	StateSpriteContext *curr = static_cast<StateSpriteContext *>(GetState(checkpoint));
	if(curr) {
		WriteState(curr);
	} else {
		if(isDeleted()) return 0;
		// Set the sprite "deleted" flag
		DeleteContext();
	}
	return 1;
}
int CALLBACK CSpriteContext::DestroyCheckpoint(LPVOID Interface, LPARAM lParam)
{
	StateSpriteContext *curr = static_cast<StateSpriteContext *>(Interface);
	delete curr;
	return 1;
}

bool CSpriteContext::GetInfo(SInfo *pI) const 
{
	if(m_pDrawableObj) {
		_spt_type eType = static_cast<CSprite *>(m_pDrawableObj)->GetSpriteType();
			if(eType == tMask) pI->eType = itMask;
		else if(eType == tBackground) pI->eType = itBackground;
		else if(eType == tEntity) pI->eType = itEntity;
		strncpy(pI->szScope, m_pDrawableObj->GetName(), sizeof(pI->szScope) - 1);
		pI->szScope[sizeof(pI->szScope) - 1] = '\0';
	} else {
		pI->eType = itSpriteSet;
		strncpy(pI->szScope, "group", sizeof(pI->szScope) - 1);
		pI->szScope[sizeof(pI->szScope) - 1] = '\0';
	}

	strncpy(pI->szName, GetName(), sizeof(pI->szName) - 1);
	pI->szName[sizeof(pI->szName) - 1] = '\0';

	pI->pPropObject = (IPropertyEnabled*)this;
	return true;
}
bool CSpriteContext::GetProperties(SPropertyList *pPL) const 
{
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
	if(m_pDrawableObj) {
		pPL->AddString("Sprite Sheet", static_cast<CSprite*>(m_pDrawableObj)->GetSpriteSheet()->GetName(), false);
	}
	
	pPL->AddCategory("Behavior");
	pPL->AddBoolean("IsVisible", isVisible(), false);
	if(pPL->Information.eType == itEntity) {
		pPL->AddList("Horizontal Chain", m_eXChain, "0 - Relative, 1 - Fill, 2 - Right, 3 - Left, 4 - Fixed");
		pPL->AddList("Vertical Chain", m_eYChain, "0 - Relative, 1 - Fill, 2 - Up, 3 - Down, 4 - Fixed");
	}

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

	pP = PL.FindProperty("Horizontal Chain", "Behavior", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(m_eXChain != (_Chain)pP->nIndex) {
			m_eXChain = (_Chain)pP->nIndex;
			bChanged = true;
		}
	}
	
	pP = PL.FindProperty("Vertical Chain", "Behavior", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged)  {
		if(m_eYChain != (_Chain)pP->nIndex) {
			m_eYChain = (_Chain)pP->nIndex;
			bChanged = true;
		}
	}

	if(bChanged) Touch();
	return bChanged;
}

CSpriteContext::~CSpriteContext(){
	delete m_pEntityData;
}

CSpriteContext::CSpriteContext(LPCSTR szName) : 
	CDrawableContext(szName),
	m_eXChain(relative),
	m_eYChain(relative)
{
	DestroyStateCallback(CSpriteContext::DestroyCheckpoint, (LPARAM)this);
	
	
	if(CGameManager::Instance()->isPlaying()){
		m_pEntityData = new CEntityData;
		
		if(strlen(szName) < 1){
			char szName[256];
			sprintf(szName, "%p", (DWORD)this);
			SetName(szName);
			CEntityData::InsertContext(szName, static_cast<CDrawableContext *>(this));	
		} else CEntityData::InsertContext(szName, static_cast<CDrawableContext *>(this));	
		m_pEntityData->m_pParent = this;
	}
	

	memset(m_nFrame, -1, sizeof(m_nFrame));
	Mirror(false);
	Flip(false);
	ARGB(COLOR_ARGB(255,128,128,128));
	Rotate(SROTATE_0);
	Tile(false);
}
