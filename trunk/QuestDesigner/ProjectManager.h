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
/*! \file		ProjectManager.h 
	\brief		Interface of the CProjectManager class.
	\date		April 15, 2003

	This file defines the classes needed to handle the game project.
	The project manager is in charge of pretty much everything that has
	to do with the game. This includes the sounds, the sprites, the scripts,
	the saved files, the maps and the entities and its properties.

	The project manager makes use of several classes defined in four main
	modules: the File Manager, which is in charge of saving and loading the
	needed files for the project; the Sprite Manager, with the job of
	maintaining the sprites: entities, masks and backgrounds of the game, along
	with their properties; the Sound Manager, which maintains the full list of
	sounds used by the game; and a World Manager, which provides the necessary
	methods and structures to build the whole quest.
	
	CProjectManager is perhaps the most important module of all, since it 
	unites all other four modules into one working engine. Its functionality
	is implemented as an object factory, giving birth and controling life of 
	each object related to the game.
*/

#pragma once

#include "SCompiler.h"

#include "Console.h"
#include "interfaces.h"

#include "SpriteManager.h"
#include "SoundManager.h"
#include "WorldManager.h"

/////////////////////////////////////////////////////////////////////////////
/*! \class		CProjectManager
	\brief		CProjectManager class.
	\author		Kronuz
	\version	1.3
	\date		April 15, 2003
				April 28, 2003
				June 3, 2003

	This class Is the one that manages everything in the project,
	from the sprite sheets list and the sounds list, to the world and
	map of the game.
*/
class CProjectManager :
	public CConsole,
	public IDocumentObject
{
	static CProjectManager *_instance;
	CString m_sProjectName;

protected:

	// Current state of the object:
	int m_iStep, m_iCnt1, m_iCnt2;

	CWorld *m_World;
	CSimpleArray<CSpriteSheet*> m_SpriteSheets;
	CSimpleMap<CString, CSprite*> m_UndefSprites;
	CSimpleMap<CString, CScript*> m_Scripts;
	CSimpleMap<CString, CSound*> m_Sounds;

	// Helper methods:
	CSprite *CreateSprite(_spt_type sptType, LPCSTR szName);

	void DeleteScript(int nIndex);
	void DeleteSpriteSheet(int nIndex);
	void DeleteMap(int nIndex);

public:
	static int CALLBACK LoadSheet(LPCTSTR szFile, LPARAM lParam);
	CString& GetProjectName() { return m_sProjectName; }

	// Construction/Destruction:
	CProjectManager();
	~CProjectManager();

	void Clean(); //!< Frees all allocated memory and cleans the object.

	// Building/Linking methods:
	LRESULT StartBuild(); //!< Starts the building process
	LRESULT BuildNextStep(WPARAM wParam, LPARAM lParam); //!< Continues the building process

	// Loading/Saving methods:
	bool Load(LPCSTR szFile); //!< Loads the project from a file
	bool Save(LPCSTR szFile) { return false; } //!< Saves the project to a file

	// Object factory methods:
	CSprite *ReferSprite(LPCSTR szName, _spt_type sptType);  //!< Gets or makes a reference to a sprite.
	CScript *ReferScript(LPCSTR szName);  //!< Gets or makes a reference to a script

	CSprite *MakeSprite(LPCSTR szName, _spt_type sptType, CSpriteSheet *pSpriteSheet);
	CScript *MakeScript(LPCSTR szName);

	void DeleteScript(LPCSTR szName); //!< Deletes a script via its name.
	void DeleteSpriteSheet(LPCSTR szName); //!< Deletes a sprite sheet via its name.
	void DeleteMap(LPCSTR szName); //!< Deletes a map via its name.
	void DeleteSprite(LPCSTR szName); //!< Deletes a sprite via its name.

	// Methods to find stuff in the world:
	CSprite *FindSprite(LPCSTR szName); //!< Finds a sprite using its name.
	CSound *FindSound(LPCSTR szName); //!< Finds a sound using its name.
	CScript *FindScript(LPCSTR szName); //!< Finds a script using its name.

	CSprite *FindSprite(POINT MapPoint, LPCSTR Layer="Any"); //!< Finds the sprite at a specific location.
	
	static CProjectManager* Instance() {
		if(_instance == NULL) {
			_instance = new CProjectManager;
		}
		return _instance;
	}
};
