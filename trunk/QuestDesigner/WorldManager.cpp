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
	\brief		Implementation of the CProjectManager class.
	\date		April 28, 2003

	This file implements the classes that manage a world in the game,
	this includes the methods to create a new world, maps for that
	world, and being living on it.

*/

#include "stdafx.h"
#include "WorldManager.h"
#include "ProjectManager.h"

CLayer::CLayer(LPCSTR szName) :
	CNamedObj(szName)
{
	m_ArchiveIn = new CLayerTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;

/*
	CSpriteContext *sc;

	sc = new CSpriteContext("");
	sc->SetDrawableObj(CProjectManager::Instance()->FindSprite("_guardb"));
	sc->Alpha(255);
	sc->MoveTo(100,70);
	AddChild(sc);

	sc = new CSpriteContext("");
	sc->SetDrawableObj(CProjectManager::Instance()->FindSprite("_crabenemy"));
	sc->Alpha(255);
	sc->MoveTo(100,50); // OK
	sc->Rotate(SROTATE_0);
	AddChild(sc);

	sc = new CSpriteContext("");
	sc->SetDrawableObj(CProjectManager::Instance()->FindSprite("_crabenemy"));
	sc->Alpha(255);
	sc->MoveTo(78,70);
	sc->Rotate(SROTATE_90);
	AddChild(sc);

	sc = new CSpriteContext("");
	sc->SetDrawableObj(CProjectManager::Instance()->FindSprite("_crabenemy"));
	sc->Alpha(255);
	sc->MoveTo(100,100);
	sc->Rotate(SROTATE_180);
	AddChild(sc);

	sc = new CSpriteContext("");
	sc->SetDrawableObj(CProjectManager::Instance()->FindSprite("_crabenemy"));
	sc->Alpha(255);
	sc->MoveTo(130,70);
	sc->Rotate(SROTATE_270);
	AddChild(sc);
/**/
/*

	sc = new CSpriteContext("");
	sc->SetDrawableObj(CProjectManager::Instance()->FindSprite("_townfill2"));
	sc->Alpha(255);
	sc->MoveTo(20,20);
	sc->SetSize(250,250);
	sc->Tile();
	AddChild(sc);
/* _townfill2 */
}
CLayer::~CLayer()
{
}
void CLayer::AddSpriteContext(CSpriteContext *sprite)
{
	AddChild(sprite);
}

CMap::CMap()
{
}
CMap::~CMap()
{
}

CMapGroup::CMapGroup()
{
	CLayer *l;
	char file[] = "C:\\qd\\Quest Designer 2.0.4\\questdata\\kakariko\\screens\\3-2.lnd";
	int i=0;
	for(int i=0;i<4;i++) {
		file[54]=i+1+'0';
		file[56]='2';
		l = new CLayer("TOP");
		l->Load(file);
		l->MoveTo(320*i,0);
		AddChild(l);

		file[56]='3';
		l = new CLayer("TOP");
		l->Load(file);
		l->MoveTo(320*i,240);
		AddChild(l);
	}
/**/

}
CMapGroup::~CMapGroup()
{
}

CWorld::CWorld(LPCSTR szName) : 
	CNamedObj(szName)
{
}
CWorld::~CWorld()
{
	/*
	if(m_MapGroups.GetSize()) printf("\tFreeing World Maps...\n");
	for(int i=0; i<m_MapGroups.GetSize(); i++) {
		delete m_MapGroups[i];
		m_MapGroups[i] = NULL;
	}*/
}
