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
/*! \file		GameManager.h 
	\brief		Interface of the CGameManager class.
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
	
	CGameManager is perhaps the most important module of all, since it 
	unites all other four modules into one working engine. Its functionality
	is implemented as an object factory, giving birth and controling life of 
	each object related to the game.
*/

#pragma once

#include "Kernel.h"

#include "../Interfaces.h"

#include "SpriteManager.h"
#include "SoundManager.h"
#include "WorldManager.h"

#include <map>
#include <vector>

struct SUndefSprite {
	CSprite *pSprite;
	CBString sFile;
	int nLine;
};
/////////////////////////////////////////////////////////////////////////////
/*! \class		CGameManager
	\brief		CGameManager class.
	\author		Kronuz
	\version	1.4
	\date		April 15, 2003
				April 28, 2003
				June 3, 2003
				June 25, 2003

	This class Is the one that manages everything in the project,
	from the sprite sheets list and the sounds list, to the world and
	map of the game. It acts as a singleton object factory, building 
	sprites, sounds, scripts, etc. for the project.
*/
class CGameManager :
	public IGame
{
	static CGameManager *_instance;
	CBString m_sProjectName;
	CSoundManager *m_pSoundManager;

	static float ms_fDelta;
	static DWORD ms_dwLastTick;

	static IGraphics **ms_ppGraphicsI;

protected:
	CWorld m_World;
	std::vector<CSpriteSheet*> m_SpriteSheets;

	typedef std::pair<CBString, SUndefSprite> pairUndefSprite;
	std::map<CBString, SUndefSprite> m_UndefSprites;

	typedef std::pair<CBString, CScript*> pairScript;
	std::map<CBString, CScript*> m_Scripts;

	typedef std::pair<CBString, CSound*> pairSound;
	std::map<CBString, CSound*> m_Sounds;

	// Helper methods:
	CSprite *CreateSprite(_spt_type sptType, LPCSTR szName);

	void DeleteScript(int nIndex);
	void DeleteSpriteSheet(int nIndex);
	void DeleteMap(int nIndex);
	void DeleteSound(int nIndex);

public:
	static int CALLBACK LoadSheet(LPCTSTR szFile, DWORD dwFileAttributes, LPARAM lParam);

	// Construction/Destruction:
	CGameManager();
	~CGameManager();

	void Clean(); //!< Frees all allocated memory and cleans the object.

	inline static int GetPauseLevel() { return 0; } // ACA
	inline static float GetFPSDelta() { return ms_fDelta; }
	inline static DWORD GetLastTick() { return ms_dwLastTick; }
	inline static void UpdateWorldCo(int x, int y) {
		ASSERT(ms_ppGraphicsI);
		ASSERT(*ms_ppGraphicsI);
		if(!(*ms_ppGraphicsI)) return;

		CPoint Point(x,y);
		CRect rcVisible, rcWorld;
		(*ms_ppGraphicsI)->GetVisibleRect(&rcVisible);
		(*ms_ppGraphicsI)->GetWorldRect(&rcWorld);

		Point.Offset(-rcVisible.Width()/2, -rcVisible.Height()/2);
		if(Point.x < 0) Point.x = 0;
		else if(Point.x > rcWorld.right-rcVisible.Width()) Point.x = rcWorld.right-rcVisible.Width();
		if(Point.y < 0) Point.y = 0;
		else if(Point.y > rcWorld.bottom-rcVisible.Height()) Point.y = rcWorld.bottom-rcVisible.Height();

		(*ms_ppGraphicsI)->SetWorldPosition(Point);
	}

	virtual bool UsingGraphics(IGraphics **ppGraphicsI) { ms_ppGraphicsI = ppGraphicsI; return true; }

	LPCSTR GetProjectName() const;
	int CountScripts() const;
	const IScript* GetScript(int idx) const;
	ISoundManager* GetSoundManager() const;

	// Object factory methods:
	CSprite *ReferSprite(LPCSTR szName, _spt_type sptType, LPCSTR szFile_, int nLine_);  //!< Gets or makes a reference to a sprite.
	CScript *ReferScript(LPCSTR szName);  //!< Gets or makes a reference to a script

	CSprite *MakeSprite(LPCSTR szName, _spt_type sptType, CSpriteSheet *pSpriteSheet);
	CScript *MakeScript(LPCSTR szName);
	CSound *MakeSound(LPCSTR szName);
	CSpriteSheet *MakeSpriteSheet(CSpriteSheet *pSpriteSheet);

	void DeleteSound(LPCSTR szName);
	void DeleteScript(LPCSTR szName); //!< Deletes a script using its name.
	void DeleteSpriteSheet(LPCSTR szName); //!< Deletes a sprite sheet using its name.
	void DeleteMap(LPCSTR szName); //!< Deletes a map using its name.
	void DeleteSprite(LPCSTR szName); //!< Deletes a sprite using its name.

	// Methods to find stuff in the world:
	CSprite *FindSprite(LPCSTR szName); //!< Finds a sprite using its name.
	CSound *FindSound(LPCSTR szName); //!< Finds a sound using its name.
	CScript *FindScript(LPCSTR szName); //!< Finds a script using its name.

	CSprite *FindSprite(CPoint MapPoint, LPCSTR Layer="Any"); //!< Finds the sprite at a specific location.

	static CGameManager* Instance() {
		if(_instance == NULL) {
			_instance = new CGameManager;
		}
		return _instance;
	}

///////////////////////////////////////////////////////////////////////
// Module Interface:
protected:
	struct Callbacks {
		STATUSCHANGEDPROC *Callback;
		LPARAM lParam;
		Callbacks() : Callback(NULL), lParam(NULL) {}
	} 	Sound,
		Sprite,
		Script,
		SpriteSheet,
		Map,
		MapGroup;

public:

	void WaitScripts();
	float UpdateFPS(float fpsLock = -1.0f);

	// Loading/Saving methods:
	bool Load(CVFile &vfFile); //!< Loads the project from a file
	bool Save(CVFile &vfFile) { return false; } //!< Saves the project to a file
	bool Save() { return false; }
	bool Close(bool bForce) { return(m_sProjectName==""); } //!< Closes the project

	bool Load(LPCSTR szFile) { return Load(CVFile(szFile, true)); }
	bool Save(LPCSTR szFile) { return Save(CVFile(szFile)); }


	bool LoadWorld(CVFile &vfFile) { return m_World.Load(vfFile); }
	bool SaveWorld(CVFile &vfFile) { return m_World.Save(vfFile); }
	bool SaveWorld() { return false; }
	bool CloseWorld(bool bForce) { return m_World.Close(bForce); }

	bool LoadWorld(LPCSTR szFile) { return LoadWorld(CVFile(szFile)); }
	bool SaveWorld(LPCSTR szFile) { return SaveWorld(CVFile(szFile)); }

	CSpriteSelection* CreateSpriteSelection(CDrawableContext **ppDrawableContext_);
	void DeleteSpriteSelection(CSpriteSelection *pSelection);

	void GetMapSize(CSize &mapSize) const;
	void SetMapSize(const CSize &mapSize);

	void GetWorldSize(CSize &worldSize) const;
	void SetWorldSize(const CSize &worldSize);

	CMapGroup *FindMapGroup(int x, int y) const; //!< Get the map group at location (x,y)
	CMapGroup* BuildMapGroup(int x, int y, int width, int height);
	CThumbnails* GetThumbnails() {
		static CThumbnails *s_Thumbnails;
		if(!s_Thumbnails) s_Thumbnails = new CThumbnails;
		return s_Thumbnails;
	}

	void SetSoundCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam);
	void SetSpriteCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam);
	void SetScriptCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam);
	void SetSpriteSheetCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam);
	void SetMapCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam);
	void SetMapGroupCallback(STATUSCHANGEDPROC *StatusChanged, LPARAM lParam);

	int ForEachSound(FOREACHPROC ForEach, LPARAM lParam);
	int ForEachSprite(FOREACHPROC ForEach, LPARAM lParam);
	int ForEachScript(FOREACHPROC ForEach, LPARAM lParam);
	int ForEachSpriteSheet(FOREACHPROC ForEach, LPARAM lParam);
	int ForEachMap(FOREACHPROC ForEach, LPARAM lParam);
	int ForEachMapGroup(FOREACHPROC ForEach, LPARAM lParam);

	DWORD GetModuleID() const { return MODULE_ID; }
	void SetConsole(IConsole *pConsole) { g_pConsole = pConsole; }
};
