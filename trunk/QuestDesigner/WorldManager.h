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

#include "Console.h"
#include "interfaces.h"

#include "SpriteManager.h"
#include "ArchiveText.h"

/////////////////////////////////////////////////////////////////////////////
/*! \class		CLayer
	\brief		CLayer class.
	\author		Kronuz
	\version	1.0
	\date		May 31, 2003

	A layer contains a list of all the sprites it has, and Layer 
	children are Sprites.
*/
class CLayer :
	public CConsole,
	public CNamedObj,			// Layers can have a name.
	public IDocumentObject,		// Layers can be loaded from a file.
	public CDrawableContext		// Layers can be painted on the screen.
{
public:
	CLayer(LPCSTR szName);
	~CLayer();

	void AddSpriteContext(CSpriteContext *pSpriteContext);
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CMap
	\brief		CMap class.
	\author		Kronuz
	\version	1.0
	\date		May 31, 2003

	Map children are Layers.
*/
class CMap :
	public CConsole,
	public CDrawableObject		// Maps can be painted on the screen.
{
public:
	CMap();
	~CMap();
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
	public CConsole,
	public CDrawableContext		// Map Groups can be painted on the screen.
{
public:
	CMapGroup();
	~CMapGroup();
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
	A world contains a list of all map groups it represents, and World 
	children are MapGroups.
*/
class CWorld :
	public CConsole,
	public CNamedObj,			// Worlds can have name.
	public IDocumentObject,		// Worlds can be loaded from a file.
	public CDrawableObject		// Worlds can be painted on the screen.
{
public:
	CWorld(LPCSTR szName);
	~CWorld();
};
