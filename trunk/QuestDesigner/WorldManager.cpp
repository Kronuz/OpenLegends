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

CLayer::CLayer() :
	m_ptLoadPoint(0,0)
{
	m_ArchiveIn = new CMapTxtArch(this);
	m_ArchiveOut = m_ArchiveIn;

}
CLayer::~CLayer()
{
	delete m_ArchiveIn;
}
inline void CLayer::SetLoadPoint(int x, int y)
{
	m_ptLoadPoint.SetPoint(x,y);
}
inline void CLayer::SetLoadPoint(const POINT &point_)
{
	m_ptLoadPoint = point_;
}
bool CLayer::AddSpriteContext(CSpriteContext *sprite)
{
	CPoint Point;
	sprite->GetPosition(Point);
	Point += m_ptLoadPoint;
	sprite->MoveTo(Point);
	return AddChild(sprite);
}

CMapGroup::CMapGroup()
{

	CLayer *layer = new CLayer;

/*	char file[] = "C:\\qd\\Quest Designer 2.0.4\\questdata\\kakariko\\screens\\0-0.lnd";
	layer->Load(file); /**/

	char file[] = "C:\\qd\\Quest Designer 2.0.4\\questdata\\kakariko\\screens\\3-2.lnd";
	for(int i=0;i<4;i++) {
		file[54]=i+1+'0';
		file[56]='2';
		layer->SetLoadPoint(320*i, 0);
		layer->Load(file);

		file[56]='3';
		layer->SetLoadPoint(320*i, 240);
		layer->Load(file);
	}/**/
	AddChild(layer);

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
