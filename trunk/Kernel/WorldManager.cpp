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
	m_bChanged(true),
	m_bLoaded(false)
{
}
CMapGroup::~CMapGroup()
{
	delete []m_pBitmap;
}
bool CMapGroup::Load()
{
	if(m_bLoaded) return true;	// loaded map groups can not be loaded again.
	CLayer *layer = new CLayer;
	
	CVFile vfFile;
	CBString sFile;
	CBString sPath = "questdata\\" + m_pWorld->m_fnFileLoad.GetFileTitle(); // relative by default

	// for each map in the group, we load it in the new layer
	for(int j=0; j<m_rcPosition.Height(); j++) {
		for(int i=0; i<m_rcPosition.Width(); i++) {
			sFile.Format("\\screens\\%d-%d.lnd", m_rcPosition.left + i, m_rcPosition.top + j);
			vfFile.SetFilePath(sPath + sFile);

			layer->SetLoadPoint(m_pWorld->m_szMapSize.cx*i, m_pWorld->m_szMapSize.cy*j);
			layer->Load(vfFile);
		}
	}
	layer->SetObjSubLayer(0);	// tell the newly created layer what layer it is.
	AddChild(layer);			// add the new layer to the map group

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
	m_szMapSize(320*2, 240*2)
{
	m_ArchiveIn = new CWorldTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;
}
CWorld::~CWorld()
{
	if(m_MapGroups.size()) CONSOLE_OUTPUT("\tFreeing World Maps...\n");
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
