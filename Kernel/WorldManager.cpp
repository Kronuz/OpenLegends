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
/*! \file		WorldManager.cpp
	\brief		Implementation of the CGameManager class.
	\date		April 28, 2003

	This file implements the classes that manage a world in the game,
	this includes the methods to create a new world, maps for that
	world, and being living on it.

*/

#include "stdafx.h"
#include "WorldManager.h"
#include "GameManager.h"
#include "SoundManager.h"

CLayer::CLayer() :
	CDocumentObject(),
	CDrawableContext(),
	m_ptLoadPoint(0,0)
{
	m_ArchiveIn = new CMapTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
CLayer::~CLayer()
{
	delete m_ArchiveIn;
}
CMapGroup::CMapGroup() :
	CDrawableContext(),
	m_pWorld(NULL),
	m_pBitmap(NULL),
	m_rcPosition(0,0,0,0),
	m_sMapID("New Map Group"),
	m_bLoaded(false),
	m_bFlagged(false),
	m_pMusic(NULL)
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
CMapGroup::~CMapGroup()
{
	delete []m_pBitmap;
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

bool CMapGroup::Load()
{
	if(m_bLoaded) return true;	// loaded map groups can not be loaded again.

	// For now, there is only one layer availible in the worlds the default layer:
	CLayer *pLayer = static_cast<CLayer *>(GetChild(DEFAULT_LAYER));
	ASSERT(pLayer);
	
	CVFile vfFile;
	CBString sFile;
	CBString sPath = "questdata\\" + m_pWorld->GetFile().GetFileTitle(); // relative by default

	// for each map in the group, we load it in the ground layer:
	for(int j=0; j<m_rcPosition.Height(); j++) {
		for(int i=0; i<m_rcPosition.Width(); i++) {
			sFile.Format("\\screens\\%d-%d.lnd", m_rcPosition.left + i, m_rcPosition.top + j);
			vfFile.SetFilePath(sPath + sFile);

			pLayer->SetLoadPoint(m_pWorld->m_szMapSize.cx*i, m_pWorld->m_szMapSize.cy*j);
			pLayer->Load(vfFile);
		}
	}

	m_bLoaded = true;			// Map group loaded.
	return true;
}

bool CMapGroup::Save()
{
	return false;
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

CWorld::CWorld(LPCSTR szName) : 
 	CNamedObj(szName),
	CDocumentObject(),
	m_szMapSize(DEF_MAPSIZEX, DEF_MAPSIZEY),
	m_szWorldSize(DEF_MAXMAPSX, DEF_MAXMAPSY)
{
	m_ArchiveIn = new CWorldTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
CWorld::~CWorld()
{
	if(m_MapGroups.size()) CONSOLE_PRINTF("\tFreeing World Maps...\n");
	for(UINT i=0; i<m_MapGroups.size(); i++) {
		delete m_MapGroups[i];
		m_MapGroups[i] = NULL;
	}
	m_MapGroups.clear();
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
