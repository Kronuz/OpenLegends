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

CMap::~CMap()
{
}

CWorld::~CWorld()
{
	if(m_Maps.GetSize()) printf("\tFreeing World Maps...\n");
	for(int i=0; i<m_Maps.GetSize(); i++) {
		delete m_Maps[i];
		m_Maps[i] = NULL;
	}
}

bool CWorld::Load(LPCSTR szFile)
{
	return false;
}

