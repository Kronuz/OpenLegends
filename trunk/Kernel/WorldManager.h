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
/*! \file		WorldManager.h 
	\brief		Interface of the CWorld and CMap classes.
	\date		April 28, 2003

	This file defines the classes that manage a world in the game,
	this includes the methods to create a new world, maps for that
	world, and being living on it.
	
	\todo Much is missing on this module :)

*/

#pragma once

#include <vector>

#define DEF_MAXMAPSX 256
#define DEF_MAXMAPSY 256
#define DEF_MAPSIZEX 640
#define DEF_MAPSIZEY 480

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CWorld;

#include "SpriteManager.h"
#include "ArchiveText.h"

/////////////////////////////////////////////////////////////////////////////
/*! \class		CLayer
	\brief		CLayer class.
	\author		Kronuz
	\version	1.2
	\date		May 31, 2003
				June 3, 2003

	A Layer contains a list of all the sprites it has, so its 
	children are Sprites.
*/
class CLayer :
	public CDocumentObject,		// Maps can be loaded from a file into a layer.
	public CDrawableContext		// Layers can be painted on the screen.
{
	CPoint m_ptLoadPoint;

public:
	void SetLoadPoint(int x, int y);
	void SetLoadPoint(const CPoint &point_);
	CLayer();
	~CLayer();

	bool AddSpriteContext(CSpriteContext *pSpriteContext, bool bAllowDups_ = true);
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CMapGroup
	\brief		CMapGroup class.
	\author		Kronuz
	\version	1.0
	\date		May 31, 2003

	MapGroup children are Maps.
*/
class CMapGroup :
	public CDrawableContext
{
	CBString m_sMapID;
	CRect m_rcPosition;
	const CWorld *m_pWorld;
	BITMAP *m_pBitmap;
	bool m_bLoaded;

public:
	CMapGroup();
	~CMapGroup();

	void SetWorld(const CWorld *pWorld) {
		m_pWorld = pWorld;
	}

	// Interface:
	virtual bool Load();
	virtual bool Save();

	virtual void GetSize(CSize &_Size) const { CDrawableContext::GetSize(_Size); };

	virtual bool isMapGroupAt(int x, int y) const;
	virtual bool isMapGroupHead(int x, int y) const;

	virtual LPCSTR GetMapGroupID() const;
	virtual void GetMapGroupRect(CRect &MapGroupRect) const;
	virtual void GetMapGroupSize(CSize &MapGroupSize) const;
	virtual void SetMapGroupSize(const CSize &MapGroupSize);

	virtual void MoveMapGroupTo(int x, int y);
	virtual void OffsetMapGroup(int x, int y);

	virtual void SetThumbnail(BITMAP *pBitmap) { 
		delete []m_pBitmap;
		m_pBitmap = pBitmap; 
	}
	virtual BITMAP* GetThumbnail(RECT *pRect) const;
	virtual BITMAP* GetThumbnail(RECT *pRect, int x, int y) const;
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CWorld
	\brief		CWorld class.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	This class Is the one that manages everything in the project,
	from the sprite sheets list and the sounds list, to the world and
	map of the game.
	A world contains a list of all map groups it represents, and World 
	children are MapGroups.
*/
class CWorld :
	public CNamedObj,			// Worlds can have name.
	public CDocumentObject		// Worlds can be loaded from a file.
{
	typedef std::vector<CMapGroup*>::iterator iterMapGroup;
	std::vector<CMapGroup*> m_MapGroups;

public:
	CSize m_szWorldSize;
	CSize m_szMapSize;

	CWorld(LPCSTR szName);
	~CWorld();

	CMapGroup* FindMapGroup(int x, int y) const;
	CMapGroup* BuildMapGroup(int x, int y, int width, int height);
};


inline void CLayer::SetLoadPoint(int x, int y)
{
	m_ptLoadPoint.SetPoint(x,y);
}
inline void CLayer::SetLoadPoint(const CPoint &point_)
{
	m_ptLoadPoint = point_;
}
inline bool CLayer::AddSpriteContext(CSpriteContext *sprite, bool bAllowDups_)
{
	CPoint Point;
	sprite->GetPosition(Point);
	Point += m_ptLoadPoint;
	sprite->MoveTo(Point);
	return AddChild(sprite, bAllowDups_);
}
