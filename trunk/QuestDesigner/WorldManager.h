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
	
	\todo Everything is missing on this module :)

*/

#pragma once

#include "Console.h"
#include "interfaces.h"

#include "SpriteManager.h"

class CLayer :
	public CNamedObj,
	public CConsole
{
	CSimpleArray<CSpriteContext*> m_Sprites;
public:
	CLayer(LPCSTR szName);
};

class CMap :
	public CConsole,
	public IDocumentObject
{
	CSimpleArray<CLayer*> m_Layers;
public:
	CMap();
	~CMap();
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CWorldManager
	\brief		CWorldManager class.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	This class Is the one that manages everything in the project,
	from the sprite sheets list and the sounds list, to the world and
	map of the game.
*/
class CWorld :
	public CNamedObj,
	public CConsole,
	public IDocumentObject
{
	CSimpleArray<CMap*> m_Maps;
public:
	CWorld();
	~CWorld();

	// Loading/Saving methods:
	bool Load(LPCSTR szFile); //!< Loads the world from a file
	bool Save(LPCSTR szFile) { return false; } //!< Saves the world to a file
};
