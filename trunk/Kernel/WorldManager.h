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
/*! \file		WorldManager.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the CWorld and CMap classes.
	\date		April 28, 2003

	This file defines the classes that manage a world in the game,
	this includes the methods to create a new world, maps for that
	world, and being living on it.
	
	\todo Much is missing on this module :)

*/

#pragma once

#include <vector>

#include "SpriteManager.h"
#include "ArchiveText.h"

#define DEF_MAXMAPSX 256
#define DEF_MAXMAPSY 256
#define DEF_MAPSIZEX 640
#define DEF_MAPSIZEY 480

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
interface ISound;
class CWorld;

const char g_szLayerNames[MAX_LAYERS][30] = {
	"0 - Background",
	"1 - Underground",
	"2 - Underground",
	"3 - Ground",
	"4 - First Level",
	"5 - Second Level",
	"6 - Third Level",
	"7 - Top Layer",
	"",		// HUD
	""		// reserved
};
const char g_szSubLayerNames[MAX_SUBLAYERS][30] = {
	"0 - Background",
	"1 - Sprites",
	"2 - Entities",
	"3 - Top Background",
	"4 - Top Sprites",
	"5 - Top Entities",
	"",		// reserved
	"",		// reserved
	"",		// reserved
	""		// reserved
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CMapPos
	\brief		CMapPos class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		July 12, 2005

	This class keeps map positions and returns either absolute or 
	local positions for maps.
	Members return -1 when on error, or the current MapPos layer otherwise.
*/
class CMapPos {
	static CWorld *ms_pWorld;
	int m_nLayer;
	int m_nSubLayer;
	mutable CPoint m_LocalPoint;
	mutable CBString m_sMapID;
protected:
	friend CWorld;
	CMapPos(CWorld *pWorld);
public:
	CMapPos();
	int GetAbsPosition(CPoint &_Point) const;
	int SetAbsPosition(const CPoint &_Point, int _nLayer = -1, int _nSubLayer = -1);
	int GetPosition(CPoint &_Point) const;
	int GetMapGroup(CMapGroup **_ppMapGroup) const;
	int GetLayer() const;
	int GetSubLayer() const;
	int SetPosition(const CMapGroup *_pMapGroup, const CPoint &_Point, int _nLayer = -1, int _nSubLayer = -1);
	int SetMapGroup(const CMapGroup *_pMapGroup);
	int SetPosition(const CPoint &_Point);
	int SetLayer(int _nLayer);
	int SetSubLayer(int _nSubLayer);
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CLayer
	\brief		CLayer class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.2
	\date		May 31, 2003
				June 3, 2003

	A Layer contains a list of all the sprites it has, so its 
	children are Sprites.
*/
class CLayer :
	public CDrawableContext,	// Layers can be painted on the screen.
	public CDocumentObject		// Maps can be loaded from a file into a layer.
{
protected:
//-------------------------------------
// TO KEEP THE MEMENTO:
	struct StateLayer : 
		public CDrawableContext::StateDrawableContext
	{
		virtual bool operator==(const StateData& state) const {
			const StateLayer *curr = static_cast<const StateLayer*>(&state);
			return (
				StateDrawableContext::operator ==(state) &&
				curr->ptLoadPoint == ptLoadPoint
			);
		}

		CPoint ptLoadPoint;
	};
// DATA TO KEEP:
	CPoint m_ptLoadPoint;
//-------------------------------------
protected:

public:
	bool _Close(bool bForce) { Clean(); return true; }

	CLayer();
	~CLayer();

	void LoadMore() { m_bLoaded = false; }
	void Loaded() { m_bLoaded = true; }

	void SetLoadPoint(int x, int y);
	void SetLoadPoint(const CPoint &point_);
	bool AddSpriteContext(CSpriteContext *pSpriteContext);

	virtual CDrawableContext* MakeGroup(LPCSTR szGroupName);

	// Memento interface
	virtual void ReadState(StateData *data);
	virtual void WriteState(StateData *data);
	virtual int _SaveState(UINT checkpoint);
	virtual int _RestoreState(UINT checkpoint);
	static int CALLBACK DestroyCheckpoint(LPVOID Interface, LPARAM lParam);
};
inline void CLayer::SetLoadPoint(int x, int y)
{
	m_ptLoadPoint.SetPoint(x,y);
}
inline void CLayer::SetLoadPoint(const CPoint &point_)
{
	m_ptLoadPoint = point_;
}
inline bool CLayer::AddSpriteContext(CSpriteContext *sprite)
{
	CPoint Point;
	sprite->GetPosition(Point);
	Point += m_ptLoadPoint;
	sprite->MoveTo(Point);
	return AddChild(sprite);
}

/////////////////////////////////////////////////////////////////////////////
/*! \class		CMapGroup
	\brief		CMapGroup class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		May 31, 2003
				July 11, 2005:
						+ Added CWorld* GetWorld() to CMapGroup.

	MapGroup children are Maps.
*/
class CMapGroup :
	public CDrawableContext,
	public CDocumentObject
{

protected:
//-------------------------------------
// TO KEEP THE MEMENTO:
	struct StateMapGroup : 
		public CDrawableContext::StateDrawableContext
	{
		virtual bool operator==(const StateData& state) const {
			const StateMapGroup *curr = static_cast<const StateMapGroup*>(&state);
			return (
				StateDrawableContext::operator ==(state) &&
				curr->bFlagged == bFlagged &&
				curr->sMapID == sMapID &&
				curr->rcPosition == rcPosition
			);
		}

		bool bFlagged;

		CBString sMapID;
		CRect rcPosition;
//		const CWorld *pWorld;
//		BITMAP *pOriginalBitmap;
//		BITMAP *pBitmap;
//		ISound *pMusic;
	};
// DATA TO KEEP:
	bool m_bFlagged;

	CBString m_sMapID;
	CRect m_rcPosition;
	const CWorld *m_pWorld;
	BITMAP *m_pOriginalBitmap;
	BITMAP *m_pBitmap;
	ISound *m_pMusic;
//-------------------------------------

protected:
	virtual bool CanMerge(CDrawableObject *object);

public:
	bool _Close(bool bForce);

	CMapGroup();
	~CMapGroup();

	void SetWorld(const CWorld *pWorld) {
		m_pWorld = pWorld;
	}

	const CWorld* GetWorld() const { return m_pWorld; }

	virtual bool isFlagged();
	virtual void Flag(bool bFlag = true);
	virtual bool GetInfo(SInfo *pI) const;
	virtual bool GetProperties(SPropertyList *pPL) const;
	virtual bool SetProperties(SPropertyList &PL);
	virtual void Commit() const;
	virtual void Cancel();

	// Interface:
	virtual void CalculateParallax(RECT *ViewRect);
	virtual void ShowLayer(int nLayer, bool bShow = true);
	virtual bool isVisible(int nLayer);

	virtual void GetSize(CSize &_Size) const { CDrawableContext::GetSize(_Size); };

	virtual bool isMapGroupAt(int x, int y) const;
	virtual bool isMapGroupHead(int x, int y) const;

	virtual LPCSTR GetMapGroupID() const;
	virtual void SetMapGroupID(LPCSTR szNewID);
	virtual void GetMapGroupRect(CRect &MapGroupRect) const;
	virtual void GetMapGroupSize(CSize &MapGroupSize) const;
	virtual void SetMapGroupSize(const CSize &MapGroupSize);

	virtual void MoveMapGroupTo(int x, int y);
	virtual void OffsetMapGroup(int x, int y);

	virtual void SettleOriginalBitmap() {
		if(m_pBitmap != m_pOriginalBitmap) delete []m_pOriginalBitmap;
		m_pOriginalBitmap = m_pBitmap;
	}
	virtual void SetThumbnail(BITMAP *pBitmap) { 
		if(m_pBitmap != m_pOriginalBitmap) delete []m_pBitmap;
		m_pBitmap = pBitmap; 
	}
	virtual BITMAP* GetThumbnail(RECT *pRect) const;
	virtual BITMAP* GetThumbnail(RECT *pRect, int x, int y) const;

	virtual void SetMusic(ISound *pSound);
	virtual ISound* GetMusic() const;

	// Memento interface
	virtual void ReadState(StateData *data);
	virtual void WriteState(StateData *data);
	virtual int _SaveState(UINT checkpoint);
	virtual int _RestoreState(UINT checkpoint);
	static int CALLBACK DestroyCheckpoint(LPVOID Interface, LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CWorld
	\brief		CWorld class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		April 28, 2003:
						* Creation date.
				July 07, 2005 by Littlebuddy:
						+ Added ForEachMapGroup(SIMPLEPROC ForEach, LPARAM lParam).

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
	bool _Close(bool bForce);

	CMapPos m_StartPosition;
	CMapPos m_CurrentPosition;
	CSize m_szWorldSize;
	CSize m_szMapSize;

	CWorld(LPCSTR szName);
	~CWorld();

	CMapGroup* FindMapGroup(LPCSTR szMapID) const;
	CMapGroup* FindMapGroup(int x, int y) const;
	CMapGroup* BuildMapGroup(int x, int y, int width, int height);
	int ForEachMapGroup(SIMPLEPROC ForEach, LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CThumbnails
	\brief		CThumbnails class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		Oct 9, 2003

	CThumbnails is a Drawable context that contains a single layer and
	a bunch of sprites to be painted as thumbnails in the QD.
*/
class CThumbnails :
	public CDrawableContext
{
protected:
//-------------------------------------
// TO KEEP THE MEMENTO:
	struct StateThumbnails : 
		public CDrawableContext::StateDrawableContext
	{
		virtual bool operator==(const StateData& state) const {
			const StateThumbnails *curr = static_cast<const StateThumbnails*>(&state);
			return (
				StateDrawableContext::operator ==(state)
			);
		}

		// no data
	};
// DATA TO KEEP:
	// no data
//-------------------------------------
protected:

public:
	CThumbnails();
	virtual void CleanThumbnails();

	// Memento interface
	virtual void ReadState(StateData *data) {};
	virtual void WriteState(StateData *data) {};
	virtual int _SaveState(UINT checkpoint) { return 0; };
	virtual int _RestoreState(UINT checkpoint) { return 0; };
	static int CALLBACK DestroyCheckpoint(LPVOID Interface, LPARAM lParam);
};
