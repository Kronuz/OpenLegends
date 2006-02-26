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
/*! \file		GameManager.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the CGameManager class.
	\date		April 15, 2003 //Check GameManager.cpp for updates.

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

#include <Interfaces.h>

#include "SpriteManager.h"
#include "SoundManager.h"
#include "WorldManager.h"

#include <map>
#include <math.h>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CDebugScript;

struct SUndefSprite {
	CSprite *pSprite;
	CBString sFile;
	int nLine;
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CGameManager
	\brief		CGameManager class.
	\author		Germán Méndez Bravo (Kronuz)
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
	public IGame,
	public CDocumentObject	// Projects can be loaded from a file.
{
	static CGameManager *_instance;
	CDebugScript *m_pDummyDebug;
	CBString m_sProjectName;
	CSoundManager *m_pSoundManager;

	static float ms_fDelta;
	static DWORD ms_dwLastTick;

	static IGraphics **ms_ppGraphicsI;

protected:
	bool m_bPlaying;

	//Active group
	CMapGroup **m_ppActiveMapGroup;
	
	//Wipe data
	CMapGroup *m_pWipeTarget;
	
	//This offsets the group without moving it.
	CPoint m_StaticWipeOffset;

	//We need two offsets because we may have to move in a ¨|_ motion to align with the entity.
	CPoint m_WipeOffset;	
	CPoint m_WipeOffset2;
	

	double m_fWipeOffX;
	double m_fWipeOffY;
	double m_fWipeOff2X;
	double m_fWipeOff2Y;
	int m_wipeSpeed;
	bool m_bWiping;
	bool m_bWiping2;
	int m_wipeDir;

	CWorld m_World;

	//Keyboard/Mouse input.
	int m_QueuedInput[256];			//WM_*KEY* is 0-FF, so 256 chars should be enough.
	int m_TranslatedInput[MAXKEYS];	//This input is active for the scripts and will have an enum for keys. (LEFT, RIGHT, UP, DOWN, ACT1, ACT2, ACT3, ACT4, etc.)
	int m_QueuedMouse[3];			//X, Y and Key (LEFT, MIDDLE, RIGHT mouse button)
	int m_ActiveMouse[3];
	char m_KeyMap[MAXKEYS];			//This is the active key map for key translations.

	std::vector<CSpriteSheet*> m_SpriteSheets;
	typedef std::pair<CSpriteContext *, int> BufferPair;
	std::vector<BufferPair> m_SpriteBuffer;

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
	bool _Close(bool bForce); //!< Frees all allocated memory and cleans the object.

	static int CALLBACK LoadSheet(LPCTSTR szFile, DWORD dwFileAttributes, LPARAM lParam);
	void CleanUndefs();

	// Construction/Destruction:
	CGameManager();
	~CGameManager();
	
	void TheSecretsOfDebugging(); //The secret function! =o

	const CBString CreateEntity(LPCSTR szName, LPCSTR szScript);
	
	//These two accept input from mouse and chars
	void MapInput(int chrCode, LPARAM keydata, bool down);
	void MapInput(int xCoord, int yCoord, WPARAM virtKey);

	//This updates input once the scripts have finished running for the current frame.
	void UpdateInput();	

	inline int GetInput(int Key){
		if(Key >= MAXKEYS) return -1;
		return m_TranslatedInput[Key];
	}
	
	inline CPoint* GetMousePos(){
		ASSERT(ms_ppGraphicsI);
		ASSERT(*ms_ppGraphicsI);
		if(!(*ms_ppGraphicsI)) return NULL;
		CPoint pt;
		(*ms_ppGraphicsI)->GetWorldPosition(&pt);
		return new CPoint(m_ActiveMouse[0]+pt.x, m_ActiveMouse[1]+pt.y);
	}
	inline int GetMouseKey(){
		return m_ActiveMouse[2];
	}

	void SetupKeyMap(int KeyMap[MAXKEYS]);

	inline bool LoadStart(CMapGroup **ppMapGroup){
		m_bPlaying = true;

		m_ppActiveMapGroup = ppMapGroup; //This position auto-updates in Open Legends thanks to this connection. =)
		
		m_World.m_StartPosition.GetMapGroup(m_ppActiveMapGroup);
		CPoint StartPos; m_World.m_StartPosition.GetPosition(StartPos);

		m_World.m_CurrentPosition.SetPosition(*m_ppActiveMapGroup,StartPos, 3, 2);
		if(!(*m_ppActiveMapGroup)->Load()) return false;

		return true;
	}

	CMapGroup* Wiping();
	void BeginWipe(CDrawableContext *entity, int edgex, int edgey);
	inline CPoint* GetWipeOffset(){
		return new CPoint((int)m_fWipeOffX + (int)m_fWipeOff2X - m_StaticWipeOffset.x, (int)m_fWipeOffY + (int)m_fWipeOff2Y - m_StaticWipeOffset.y);
	}
	inline CPoint* GetCurrentWipeOffset(){
		return new CPoint((int)m_fWipeOffX + (int)m_fWipeOff2X, (int)m_fWipeOffY + (int)m_fWipeOff2Y);
		
	}
	bool Wipe(int dir, CDrawableContext *context, int x, int y);
	inline static int GetPauseLevel() { return 0; } // ACA
	inline static float GetFPSDelta() { return ms_fDelta; }
	inline static DWORD GetLastTick() { return ms_dwLastTick; }
	inline void UpdateWorldCo(int x, int y) {
		ASSERT(ms_ppGraphicsI);
		ASSERT(*ms_ppGraphicsI);
		if(!(*ms_ppGraphicsI)) return;

		CPoint Point(x,y);

		m_World.m_CurrentPosition.SetPosition(*m_ppActiveMapGroup, Point);

		CRect rcVisible, rcWorld;
		(*ms_ppGraphicsI)->GetVisibleRect(&rcVisible);
		(*ms_ppGraphicsI)->GetWorldRect(&rcWorld);

		Point.Offset(-rcVisible.Width()/2, -rcVisible.Height()/2);
		if(Point.x < 0) Point.x = 0;
		else if(Point.x > rcWorld.right-rcVisible.Width()) Point.x = rcWorld.right-rcVisible.Width();
		if(Point.y < 0) Point.y = 0;
		else if(Point.y > rcWorld.bottom-rcVisible.Height()) Point.y = rcWorld.bottom-rcVisible.Height();

		(*ms_ppGraphicsI)->SetWorldPosition(&Point);
	}
	inline CPoint *GetWorldCo(){
		ASSERT(ms_ppGraphicsI);
		ASSERT(*ms_ppGraphicsI);
		if(!(*ms_ppGraphicsI)) return NULL;

		CPoint *pt = new CPoint;
		m_World.m_CurrentPosition.GetPosition(*pt);
		return pt;
	}
	inline static bool SetFilter(GpxFilters eFilter, void *vParam) {
		ASSERT(ms_ppGraphicsI);
		ASSERT(*ms_ppGraphicsI);
		if(!(*ms_ppGraphicsI)) return false;

		return (*ms_ppGraphicsI)->SetFilter(eFilter, vParam);
	}
	inline bool DrawSprite(LPCSTR spriteName, int coordType, int x, int y, int subLayer, int layer, LPCSTR rgba, float scale, int rot){
			CSprite *pSprite = FindSprite(spriteName);
			if(!pSprite) {
				return false;
			}
			CSpriteContext *pSpriteContext = new CSpriteContext("");
			pSpriteContext->SetDrawableObj(pSprite);
			//if(pSprite->GetSpriteType() == tBackground) pSpriteContext->Tile();	
			pSpriteContext->SetObjSubLayer(subLayer);
			pSpriteContext->SetTemp();
			if(coordType==1) {	//TODO: Apply different coordinate types.
				CPoint pt(0,0);
				ASSERT(ms_ppGraphicsI);
				ASSERT(*ms_ppGraphicsI);
				if((*ms_ppGraphicsI)) (*ms_ppGraphicsI)->GetWorldPosition(&pt);
				pSpriteContext->MoveTo(x+pt.x, y+pt.y);
			} else pSpriteContext->MoveTo(x, y);
			pSpriteContext->Rotate(rot);
			pSpriteContext->ARGB(HEX2ARGB(rgba));
			BufferPair Pair;
			Pair.first = pSpriteContext;
			Pair.second = layer;
			m_SpriteBuffer.push_back(Pair); //used to add objects
			
			return true;
	}
	inline CMapGroup* GetActiveMapGroup(){
		return *m_ppActiveMapGroup;
	}
	
	void QueueFull();
	bool QueueAccepting();

	void FlushSprites();
	inline void ClearSpriteBuffer(){
		m_SpriteBuffer.clear();
	}

	virtual bool Configure(IGraphics **ppGraphicsI, bool bDebug);

	inline bool isPlaying(){
		return m_bPlaying;
	}

	void SetProjectName(LPCSTR szName);
	LPCSTR GetProjectName() const;
	int CountScripts() const;
	const IScript* GetScript(int idx) const;
	const IScript* GetScript(CSprite *pSprite) const;
	ISoundManager* GetSoundManager() const;
	//IGraphics* GetGraphicsI() const; //This function may be useful for scripts in the future, it needs implementation!

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

	void StopWaiting();
	bool isDebugging();
	float UpdateFPS(float fpsLock = -1.0f);

	// World Loading/Saving methods:
	bool LoadProject(LPCSTR szFile) { return Load(CVFile(szFile, true)); }
	bool SaveProject(LPCSTR szFile) { return Save(szFile); }
	bool SaveProject() { return Save(); }
	bool CloseProject(bool bForce) { return Close(bForce); }

	// World Loading/Saving methods:
	bool LoadWorld(LPCSTR szFile) { return m_World.Load(szFile); }
	bool SaveWorld(LPCSTR szFile) { return m_World.Save(szFile); }
	bool SaveWorld() { return m_World.Save(); }
	bool CloseWorld(bool bForce) { return m_World.Close(bForce); }

	CSpriteSelection* CreateSpriteSelection(CDrawableContext **ppDrawableContext_);
	void DeleteSpriteSelection(CSpriteSelection *pSelection);

	void GetMapSize(CSize &mapSize) const;
	void SetMapSize(const CSize &mapSize);

	void GetWorldSize(CSize &worldSize) const;
	void SetWorldSize(const CSize &worldSize);

	// Methods to find stuff in the world:
	CSprite *FindSprite(LPCSTR szName); //!< Finds a sprite using its name.
	CSound *FindSound(LPCSTR szName); //!< Finds a sound using its name.
	CScript *FindScript(LPCSTR szName); //!< Finds a script using its name.

	CSprite *FindSprite(CPoint MapPoint, LPCSTR Layer="Any"); //!< Finds the sprite at a specific location.

	CMapGroup* FindMapGroup(LPCSTR szMapID) const;
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

	int ForEachSound(SIMPLEPROC ForEach, LPARAM lParam);
	int ForEachSprite(SIMPLEPROC ForEach, LPARAM lParam);
	int ForEachScript(SIMPLEPROC ForEach, LPARAM lParam);
	int ForEachSpriteSheet(SIMPLEPROC ForEach, LPARAM lParam);
	int ForEachMap(SIMPLEPROC ForEach, LPARAM lParam);
	int ForEachMapGroup(SIMPLEPROC ForEach, LPARAM lParam);

	DWORD GetModuleID() const { return MODULE_ID; }
	void SetConsole(IConsole *pConsole) { g_pConsole = pConsole; }
};
