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
/*! \file		WorldManager.cpp
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the CGameManager class.
	\date		April 28, 2003:
						* Creation date.
				July 07, 2005 by Littlebuddy:
						+ Added ForEachMapGroup to CWorld.

	This file implements the classes that manage a world in the game,
	this includes the methods to create a new world, maps for that
	world, and being living on it.

*/

#include "stdafx.h"
#include "WorldManager.h"
#include "GameManager.h"
#include "SoundManager.h"

CLayer::CLayer() :
	CDrawableContext(),
	CDocumentObject(),
	m_ptLoadPoint(0,0)
{
	DestroyStateCallback(CLayer::DestroyCheckpoint, (LPARAM)this);
	m_ArchiveIn = new CMapTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
CLayer::~CLayer()
{
	delete m_ArchiveIn;
}
// Memento interface
void CLayer::ReadState(StateData *data)
{
	CDrawableContext::ReadState(data);
	StateLayer *curr = static_cast<StateLayer *>(data);
	// no data
}
void CLayer::WriteState(StateData *data)
{
	CDrawableContext::WriteState(data);
	StateLayer *curr = static_cast<StateLayer *>(data);
	// no data
}
int CLayer::_SaveState(UINT checkpoint)
{
	StateLayer *curr = new StateLayer;
	ReadState(curr);
	// This is needed to delete no longer used objects (garbage collector):
	if(m_pParent && m_bDeleted && !StateCount(checkpoint)) {
		m_pParent->KillChildEx(this);
		return 0;
	}
	// Save the object's state (SaveState decides if there are changes to be saved)
	return SetState(checkpoint, curr);
}
int CLayer::_RestoreState(UINT checkpoint)
{
	StateLayer *curr = static_cast<StateLayer *>(GetState(checkpoint));
	if(curr) {
		WriteState(curr);
	} else {
		if(m_bDeleted) return 0;
		// Set the sprite "deleted" flag
		m_bDeleted = true;
	}
	return 1;
}
int CALLBACK CLayer::DestroyCheckpoint(LPVOID Interface, LPARAM lParam)
{
	StateLayer *curr = static_cast<StateLayer *>(Interface);
	delete curr;
	return 1;
}

CThumbnails::CThumbnails() :
	CDrawableContext()
{
	// Build all layers for the map:
	CLayer *pLayer = NULL;
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		pLayer = new CLayer;
		pLayer->SetName(g_szLayerNames[i]);
		pLayer->SetObjSubLayer(i);	// tell the newly created layer what layer it is.
		AddChild(pLayer);			// add the new layer to the map group
	}
}
void CThumbnails::CleanThumbnails()
{
	// Clean all layers of the map:
	CLayer *pLayer = NULL;
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		pLayer = static_cast<CLayer *>(GetChild(i));
		pLayer->Clean();
	}
}

CMapGroup::CMapGroup() :
	CDrawableContext(),
	CDocumentObject(),
	m_pWorld(NULL),
	m_pBitmap(NULL),
	m_pOriginalBitmap(NULL),
	m_rcPosition(0,0,0,0),
	m_sMapID("New Map Group"),
	m_bFlagged(false),
	m_pMusic(NULL)
{
	DestroyStateCallback(CMapGroup::DestroyCheckpoint, (LPARAM)this);
	m_ArchiveIn = new CMapGroupTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;

	// Build all layers for the map:
	CLayer *pLayer = NULL;
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		pLayer = new CLayer;
		pLayer->SetName(g_szLayerNames[i]);
		pLayer->SetObjSubLayer(i);	// tell the newly created layer what layer it is.
		AddChild(pLayer);			// add the new layer to the map group
	}
}
CMapGroup::~CMapGroup()
{
	if(m_pBitmap != m_pOriginalBitmap) delete []m_pBitmap;
	delete []m_pOriginalBitmap;
}

// Memento interface
void CMapGroup::ReadState(StateData *data)
{
	CDrawableContext::ReadState(data);
	StateMapGroup *curr = static_cast<StateMapGroup *>(data);
	curr->bFlagged = m_bFlagged;
	curr->sMapID = m_sMapID;
	curr->rcPosition = m_rcPosition;
}
void CMapGroup::WriteState(StateData *data)
{
	CDrawableContext::WriteState(data);
	StateMapGroup *curr = static_cast<StateMapGroup *>(data);
	m_bFlagged = curr->bFlagged;
	m_sMapID = curr->sMapID;
	m_rcPosition = curr->rcPosition;
}
int CMapGroup::_SaveState(UINT checkpoint)
{
	StateMapGroup *curr = new StateMapGroup;
	ReadState(curr);
	// This is needed to delete no longer used objects (garbage collector):
	if(m_pParent && m_bDeleted && !StateCount(checkpoint)) {
		ASSERT(!m_pParent); // This shouldn't happen since MapGropus don't currently have parents
		m_pParent->KillChildEx(this);
		return 0;
	}
	// Save the object's state (SaveState decides if there are changes to be saved)
	return SetState(checkpoint, curr);
}
int CMapGroup::_RestoreState(UINT checkpoint)
{
	StateMapGroup *curr = static_cast<StateMapGroup *>(GetState(checkpoint));
	if(curr) {
		WriteState(curr);
	} else {
		if(m_bDeleted) return 0;
		// Set the sprite "deleted" flag
		m_bDeleted = true;
	} 
	return 1;
}
int CALLBACK CMapGroup::DestroyCheckpoint(LPVOID Interface, LPARAM lParam)
{
	StateMapGroup *curr = static_cast<StateMapGroup *>(Interface);
	delete curr;
	return 1;
}

bool CMapGroup::isFlagged()
{
	return m_bFlagged;
}
void CMapGroup::Flag(bool bFlag)
{
	m_bFlagged = bFlag;
}
bool CMapGroup::GetInfo(SInfo *pI) const
{
	pI->eType = itMapGroup;

	strncpy(pI->szName, GetMapGroupID(), sizeof(pI->szName) - 1);
	strncpy(pI->szScope, "Map Group", sizeof(pI->szScope) - 1);

	pI->szName[sizeof(pI->szName) - 1] = '\0';
	pI->szScope[sizeof(pI->szScope) - 1] = '\0';

	pI->pPropObject = (IPropertyEnabled*)this;
	return true;
}
bool CMapGroup::GetProperties(SPropertyList *pPL) const
{
	ASSERT(pPL->nProperties == 0);
	GetInfo(&pPL->Information);
	char szFileName[MAX_PATH] = { 0 };

	pPL->AddCategory("Map Group");
	pPL->AddString("Name", GetMapGroupID());
	pPL->AddValue("X", m_rcPosition.left, false);
	pPL->AddValue("Y", m_rcPosition.top, false);
	pPL->AddValue("Width", m_rcPosition.Width(), false);
	pPL->AddValue("Height", m_rcPosition.Height(), false);
	pPL->AddRange("Red BkColor", m_rgbBkColor.rgbRed, 0, 255, RED_SLIDER);
	pPL->AddRange("Green BkColor", m_rgbBkColor.rgbGreen, 0, 255, GREEN_SLIDER);
	pPL->AddRange("Blue BkColor", m_rgbBkColor.rgbBlue, 0, 255, BLUE_SLIDER);
	pPL->AddRGBColor("RGB BkColor", m_rgbBkColor);
	if(m_pMusic) m_pMusic->GetSoundFileName(szFileName, sizeof(szFileName));
	pPL->AddString("Music", szFileName, false);

	int defXSize = m_rcPosition.Width()*m_pWorld->m_szMapSize.cx;
	int defYSize = m_rcPosition.Height()*m_pWorld->m_szMapSize.cy;
	for(int i=0; i<MAX_LAYERS; i++) {
		if(*g_szLayerNames[i] == '\0') break;
		CSize Size;
		CBString sCategory;
		sCategory.Format("Layer %s", g_szLayerNames[i]);
		pPL->AddCategory(sCategory);
		GetChild(i)->GetSize(Size);
		pPL->AddValue("Width", (Size.cx!=-1)?Size.cx:defXSize);
		pPL->AddValue("Height", (Size.cy!=-1)?Size.cy:defYSize);
//		pPL->AddBoolean("", false);
	}

	return true;
}
bool CMapGroup::SetProperties(SPropertyList &PL)
{
	bool bChanged = false;;
	SProperty* pP;

	pP = PL.FindProperty("Name", "Map Group", SProperty::ptString);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(strcmp(GetMapGroupID(), pP->szString)) {
			SetMapGroupID(pP->szString);
			bChanged = true;
		}
	}

	pP = PL.FindProperty("Red BkColor", "Map Group", SProperty::ptRangeValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(m_rgbBkColor.rgbRed != pP->nValue) {
			m_rgbBkColor.rgbRed = pP->nValue;
			bChanged = true;
		}
	}
	pP = PL.FindProperty("Green BkColor", "Map Group", SProperty::ptRangeValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(m_rgbBkColor.rgbGreen != pP->nValue) {
			m_rgbBkColor.rgbGreen = pP->nValue;
			bChanged = true;
		}
	}
	pP = PL.FindProperty("Blue BkColor", "Map Group", SProperty::ptRangeValue);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(m_rgbBkColor.rgbBlue != pP->nValue) {
			m_rgbBkColor.rgbBlue = pP->nValue;
			bChanged = true;
		}
	}

	pP = PL.FindProperty("RGB BkColor", "Map Group", SProperty::ptRGBColor);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(m_rgbBkColor.dwColor != pP->rgbColor) {
			m_rgbBkColor = pP->rgbColor;
			bChanged = true;
		}
	}

	int defXSize = m_rcPosition.Width()*m_pWorld->m_szMapSize.cx;
	int defYSize = m_rcPosition.Height()*m_pWorld->m_szMapSize.cy;
	for(int i=0; i<MAX_LAYERS; i++) {
		if(*g_szLayerNames[i] == '\0') break;
		CSize Size;
		CBString sCategory;
		sCategory.Format("Layer %s", g_szLayerNames[i]);
		GetChild(i)->GetSize(Size);

		pP = PL.FindProperty("Width", sCategory, SProperty::ptValue);
		if(pP) if(pP->bEnabled && pP->bChanged) {
			if(((Size.cx!=-1)?Size.cx:defXSize) != pP->nValue) {
				Size.cx = (pP->nValue!=defXSize)?pP->nValue:-1;
				bChanged = true;
			}
		}

		pP = PL.FindProperty("Height", sCategory, SProperty::ptValue);
		if(pP) if(pP->bEnabled && pP->bChanged) {
			if(((Size.cy!=-1)?Size.cy:defYSize) != pP->nValue) {
				Size.cy = (pP->nValue!=defYSize)?pP->nValue:-1;
				bChanged = true;
			}
		}
		if(Size.cx != -1 || Size.cy != -1) {
			if(Size.cx == -1) Size.cx = defXSize;
			if(Size.cy == -1) Size.cy = defYSize;
		}
		if(Size.cx == defXSize && Size.cy == defYSize) {
			Size.cx = -1;
			Size.cy = -1;
		}

		GetChild(i)->SetSize(Size);
	}

	return bChanged;
}
void CMapGroup::Commit() const
{
}
void CMapGroup::Cancel()
{
}

bool CMapGroup::_Close(bool bForce)
{
	CONSOLE_PRINTF("Closing map: '%s'...\n", m_sMapID);

	Clean();
	// Build all layers for the map:
	CLayer *pLayer = NULL;
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		pLayer = new CLayer;
		pLayer->SetName(g_szLayerNames[i]);
		pLayer->SetObjSubLayer(i);	// tell the newly created layer what layer it is.
		AddChild(pLayer);			// add the new layer to the map group
	}

	if(m_pBitmap != m_pOriginalBitmap) delete []m_pBitmap;
	m_pBitmap = m_pOriginalBitmap;

	return true;
}

bool CMapGroup::CanMerge(CDrawableObject *object) 
{
	// never merge entities:
	return((static_cast<CSprite*>(object))->GetSpriteType() != tEntity);
}

LPCSTR CMapGroup::GetMapGroupID() const
{
	return m_sMapID;
}
void CMapGroup::SetMapGroupID(LPCSTR szNewID)
{
	m_sMapID = szNewID;
}

void CMapGroup::CalculateParallax(RECT *ViewRect)
{
	CSize MapSize;
	GetMapGroupSize(MapSize);
	MapSize.cx *= m_pWorld->m_szMapSize.cx;
	MapSize.cy *= m_pWorld->m_szMapSize.cy;
	
	for(int i=0; i<MAX_LAYERS; i++) {
		CSize Size;
		GetChild(i)->GetSize(Size);
		if( Size.cx != -1 && Size.cy!= -1 && ViewRect != NULL) {
			float fX = (float)ViewRect->left / (float)(MapSize.cx - (ViewRect->right - ViewRect->left));
			float fY = (float)ViewRect->top / (float)(MapSize.cy - (ViewRect->bottom - ViewRect->top));
			GetChild(i)->MoveTo(
				ViewRect->left - (int)((float)(Size.cx - (ViewRect->right - ViewRect->left)) * fX + 0.5f), 
				ViewRect->top - (int)((float)(Size.cy - (ViewRect->bottom - ViewRect->top)) * fY + 0.5f));
 		} else GetChild(i)->MoveTo(0, 0);
	}
}

void CMapGroup::ShowLayer(int nLayer, bool bShow)
{
	ASSERT(nLayer>=0 && nLayer<=MAX_LAYERS);
	CLayer *pLayer = static_cast<CLayer *>(GetChild(nLayer));
	if(pLayer) pLayer->ShowContext(bShow);
}
bool CMapGroup::isVisible(int nLayer)
{
	ASSERT(nLayer>=0 && nLayer<=MAX_LAYERS);
	CLayer *pLayer = static_cast<CLayer *>(GetChild(nLayer));
	if(pLayer) return pLayer->isVisible();
	return false;
}

BITMAP* CMapGroup::GetThumbnail(RECT *pRect, int x, int y) const
{
	ASSERT(pRect);
	if(!m_pBitmap) return NULL;
	if(!m_rcPosition.PtInRect(CPoint(x,y))) return NULL;
	int nSizeMapX = m_pBitmap->bmWidth / m_rcPosition.Width();
	int nSizeMapY = m_pBitmap->bmHeight / m_rcPosition.Height();
	pRect->left = (x-m_rcPosition.left) * nSizeMapX;
	pRect->right = pRect->left + nSizeMapX;
	pRect->bottom = (m_rcPosition.bottom - y) * nSizeMapY;
	pRect->top = pRect->bottom - nSizeMapY;
	return m_pBitmap;
}
BITMAP* CMapGroup::GetThumbnail(RECT *pRect) const
{
	ASSERT(pRect);
	if(!m_pBitmap) return NULL;
	pRect->top = 0;
	pRect->left = 0;
	pRect->bottom = m_pBitmap->bmHeight;
	pRect->right = m_pBitmap->bmWidth;
	return m_pBitmap;
}
void CMapGroup::GetMapGroupSize(CSize &MapGroupSize) const
{
	MapGroupSize.SetSize(m_rcPosition.Width(), m_rcPosition.Height());
}

void CMapGroup::GetMapGroupRect(CRect &MapGroupRect) const
{
	MapGroupRect = m_rcPosition;
}

void CMapGroup::SetMusic(ISound *pSound)
{
	m_pMusic = pSound;
}
ISound* CMapGroup::GetMusic() const
{
	return m_pMusic;
}

void CMapGroup::SetMapGroupSize(const CSize &MapGroupSize)
{
	m_rcPosition.right = m_rcPosition.left + MapGroupSize.cx;
	m_rcPosition.bottom = m_rcPosition.top + MapGroupSize.cy;
	// now, set the size (in bytes) of the drawable context.
	CDrawableContext::SetSize(
		m_rcPosition.Width() * m_pWorld->m_szMapSize.cx, 
		m_rcPosition.Height() * m_pWorld->m_szMapSize.cy
	);
}
void CMapGroup::OffsetMapGroup(int x, int y)
{
	CPoint AdjOffset(0,0);
	m_rcPosition.OffsetRect(x, y);
	if(m_rcPosition.top < 0) AdjOffset.y = -m_rcPosition.top;
	if(m_rcPosition.left < 0) AdjOffset.x = -m_rcPosition.left;
	if(m_rcPosition.bottom >= m_pWorld->m_szWorldSize.cx) AdjOffset.y = m_pWorld->m_szWorldSize.cx - m_rcPosition.bottom;
	if(m_rcPosition.right >= m_pWorld->m_szWorldSize.cy) AdjOffset.x = m_pWorld->m_szWorldSize.cy - m_rcPosition.right;
	m_rcPosition.OffsetRect(AdjOffset);
}
void CMapGroup::MoveMapGroupTo(int x, int y)
{
	m_rcPosition.OffsetRect(CPoint(x,y) - m_rcPosition.TopLeft());
}

bool CMapGroup::isMapGroupAt(int x, int y) const
{
	return m_rcPosition.PtInRect(CPoint(x,y));
}
bool CMapGroup::isMapGroupHead(int x, int y) const
{
	return (m_rcPosition.left==x && m_rcPosition.top==y);
}

CWorld *CMapPos::ms_pWorld = NULL;

CMapPos::CMapPos() : 
	m_nLayer(-1), m_nSubLayer(-1) 
{ 
	ASSERT(ms_pWorld); 
}

CMapPos::CMapPos(CWorld *pWorld) : 
	m_nLayer(-1), m_nSubLayer(-1) 
{ 
	// Warning C4355: THIS CONSTRUCTOR MUST NEVER MAKE ACCESS ANY MEMBERS OF pWorld
	ms_pWorld = pWorld;
	ASSERT(ms_pWorld); 
	// Warning C4355: THIS CONSTRUCTOR MUST NEVER MAKE ACCESS ANY MEMBERS OF pWorld
}

int CMapPos::GetAbsPosition(CPoint &_Point) const 
{
	if(m_sMapID.IsEmpty()) return -1;
	int x = m_LocalPoint.x / ms_pWorld->m_szMapSize.cx;
	int y = m_LocalPoint.y / ms_pWorld->m_szMapSize.cy;
	CMapGroup *pMapGroup = ms_pWorld->FindMapGroup(x, y);
	if(!pMapGroup) return -1;
	CRect rcMap;
	pMapGroup->GetMapGroupRect(rcMap);
	_Point.x = m_LocalPoint.x + rcMap.left * ms_pWorld->m_szMapSize.cx;
	_Point.y = m_LocalPoint.y + rcMap.top * ms_pWorld->m_szMapSize.cy;
	return m_nLayer;
}

int CMapPos::SetAbsPosition(const CPoint &_Point, int _nLayer, int _nSubLayer) 
{
	CPoint LocalPoint;
	m_sMapID.Empty();
	int x = _Point.x / ms_pWorld->m_szMapSize.cx;
	int y = _Point.y / ms_pWorld->m_szMapSize.cy;
	CMapGroup *pMapGroup = ms_pWorld->FindMapGroup(x, y);
	if(!pMapGroup) return -1;
	CRect rcMap;
	pMapGroup->GetMapGroupRect(rcMap);

	LocalPoint.x = _Point.x - rcMap.left * ms_pWorld->m_szMapSize.cx;
	LocalPoint.y = _Point.y - rcMap.top * ms_pWorld->m_szMapSize.cy;
	if(LocalPoint.x < 0) return -1;
	if(LocalPoint.y < 0) return -1;
	if(LocalPoint.x > rcMap.Width() * ms_pWorld->m_szMapSize.cx) return -1;
	if(LocalPoint.y > rcMap.Height() * ms_pWorld->m_szMapSize.cy) return -1;

	if(_nLayer == -1) _nLayer = (m_nLayer==-1)?DEFAULT_LAYER:m_nLayer;
	if(_nSubLayer == -1) _nSubLayer = (m_nSubLayer==-1)?DEFAULT_SUBLAYER:m_nSubLayer;
	if(_nLayer < 0 || _nLayer > MAX_LAYERS) return -1;
	if(_nSubLayer < 0 || _nSubLayer > MAX_SUBLAYERS) return -1;

	m_nLayer = _nLayer;
	m_nSubLayer = _nSubLayer;
	m_LocalPoint = LocalPoint;
	m_sMapID = pMapGroup->GetMapGroupID();
	return m_nLayer;
}

int CMapPos::GetPosition(CPoint &_Point) const 
{
	if(m_sMapID.IsEmpty()) return -1;
	CMapGroup *pMapGroup = ms_pWorld->FindMapGroup(m_sMapID);
	if(!pMapGroup) {
		m_sMapID.Empty();
		return -1;
	}
	CRect rcMap;
	pMapGroup->GetMapGroupRect(rcMap);

	if(m_LocalPoint.x < 0) m_LocalPoint.x = 0;
	if(m_LocalPoint.y < 0) m_LocalPoint.y = 0;
	if(m_LocalPoint.x > rcMap.Width() * ms_pWorld->m_szMapSize.cx)
		m_LocalPoint.x = rcMap.Width() * ms_pWorld->m_szMapSize.cx;
	if(m_LocalPoint.y > rcMap.Height() * ms_pWorld->m_szMapSize.cy)
		m_LocalPoint.y = rcMap.Height() * ms_pWorld->m_szMapSize.cy;
	_Point = m_LocalPoint;
	return m_nLayer;
}

int CMapPos::GetMapGroup(CMapGroup **_ppMapGroup) const 
{
	if(m_sMapID.IsEmpty()) return -1;
	CMapGroup *pMapGroup = ms_pWorld->FindMapGroup(m_sMapID);
	if(!pMapGroup) {
		m_sMapID.Empty();
		return -1;
	}
	*_ppMapGroup = pMapGroup;
	return m_nLayer;
}

int CMapPos::GetLayer() const 
{
	if(m_sMapID.IsEmpty()) return -1;
	CMapGroup *pMapGroup = ms_pWorld->FindMapGroup(m_sMapID);
	if(!pMapGroup) {
		m_sMapID.Empty();
		return -1;
	}
	return m_nLayer;
}

int CMapPos::GetSubLayer() const 
{
	if(m_sMapID.IsEmpty()) return -1;
	CMapGroup *pMapGroup = ms_pWorld->FindMapGroup(m_sMapID);
	if(!pMapGroup) {
		m_sMapID.Empty();
		return -1;
	}
	return m_nSubLayer;
}

int CMapPos::SetPosition(const CMapGroup *_pMapGroup, const CPoint &_Point, int _nLayer, int _nSubLayer) 
{
	int tmp_nLayer = m_nLayer;
	int tmp_nSubLayer = m_nSubLayer;
	CPoint tmp_LocalPoint = m_LocalPoint;
	CBString tmp_sMapID = m_sMapID;

	int bError = false;
	if(SetMapGroup(_pMapGroup) == -1) bError = true;
	else if(SetPosition(_Point) == -1)  bError = true;
	else if(SetLayer(_nLayer) == -1)  bError = true;
	else if(SetSubLayer(_nSubLayer) == -1)  bError = true;
	if(bError) {
		m_nLayer = tmp_nLayer;
		m_nSubLayer = tmp_nSubLayer;
		m_LocalPoint = tmp_LocalPoint;
		m_sMapID = tmp_sMapID;
		return -1;
	}
	return m_nLayer;
}

int CMapPos::SetMapGroup(const CMapGroup *_pMapGroup) 
{
	if(!_pMapGroup) return -1;
	m_sMapID = _pMapGroup->GetMapGroupID();
	return GetLayer();
}

int CMapPos::SetPosition(const CPoint &_Point) 
{
	if(m_sMapID.IsEmpty()) return -1;
	CMapGroup *pMapGroup = ms_pWorld->FindMapGroup(m_sMapID);
	if(!pMapGroup) {
		m_sMapID.Empty();
		return -1;
	}
	CRect rcMap;
	pMapGroup->GetMapGroupRect(rcMap);

	if(_Point.x < 0) return -1;
	if(_Point.y < 0) return -1;
	if(_Point.x > rcMap.Width() * ms_pWorld->m_szMapSize.cx) return -1;
	if(_Point.y > rcMap.Height() * ms_pWorld->m_szMapSize.cy) return -1;

	m_LocalPoint = _Point;
	return m_nLayer;
}

int CMapPos::SetLayer(int _nLayer) 
{
	if(_nLayer == -1) return m_nLayer;
	if(m_sMapID.IsEmpty()) return -1;
	if(_nLayer < 0 || _nLayer > MAX_LAYERS) return -1;
	m_nLayer = _nLayer;
	return GetLayer();
}

int CMapPos::SetSubLayer(int _nSubLayer) 
{
	if(_nSubLayer == -1) return m_nSubLayer;
	if(m_sMapID.IsEmpty()) return -1;
	if(_nSubLayer < 0 || _nSubLayer > MAX_SUBLAYERS) return -1;
	m_nSubLayer = _nSubLayer;
	return GetSubLayer();
}

#pragma warning (push)
#pragma warning(disable : 4355) // ignore the C4355 warning
CWorld::CWorld(LPCSTR szName) : 
 	CNamedObj(szName),
	CDocumentObject(),
	m_szMapSize(DEF_MAPSIZEX, DEF_MAPSIZEY),
	m_szWorldSize(DEF_MAXMAPSX, DEF_MAXMAPSY),
	m_StartPosition(this) // Warning C4355: CMapPos constructor isn't calling any members of 'this')
{
	m_ArchiveIn = new CWorldTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
#pragma warning (pop)

CWorld::~CWorld()
{
	Close(true);
}

bool CWorld::_Close(bool bForce)
{
	if(m_MapGroups.size()) CONSOLE_PRINTF("Closing World...\n");
	for(UINT i=0; i<m_MapGroups.size(); i++) {
		delete m_MapGroups[i];
		m_MapGroups[i] = NULL;
	}
	m_MapGroups.clear();
	return true;
}
CMapGroup* CWorld::FindMapGroup(int x, int y) const
{
	std::vector<CMapGroup*>::const_iterator Iterator = m_MapGroups.begin();
	while(Iterator != m_MapGroups.end()) {
		if((*Iterator)->isMapGroupAt(x,y)) return *Iterator;
		Iterator++;
	}

	return NULL;
}

CMapGroup* CWorld::FindMapGroup(LPCSTR szMapID) const
{
	std::vector<CMapGroup*>::const_iterator Iterator = m_MapGroups.begin();
	while(Iterator != m_MapGroups.end()) {
		if(!strcmp(szMapID, (*Iterator)->GetMapGroupID())) return *Iterator;
		Iterator++;
	}

	return NULL;
}

CMapGroup* CWorld::BuildMapGroup(int x, int y, int width, int height)
{
	CMapGroup *retmap = new CMapGroup;

	retmap->SetWorld(this);
	retmap->SetMapGroupSize(CSize(width, height));
	retmap->MoveMapGroupTo(x, y);
//	retmap->Load();

	m_MapGroups.push_back(retmap);

	return retmap;
}

int CWorld::ForEachMapGroup(SIMPLEPROC ForEach, LPARAM lParam)
{
	int cnt = 0;
	std::vector<CMapGroup*>::iterator Iterator = m_MapGroups.begin();
	while(Iterator != m_MapGroups.end()) {
		ASSERT(*Iterator);
		int aux = ForEach((LPVOID)(*Iterator), lParam);
		if(aux < 0) return aux-cnt;
		cnt += aux;
		Iterator++;
	}
	return cnt;
}
