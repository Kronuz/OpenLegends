
#pragma once

#include "IArchiver.h"
#include "IDraw.h"

#include "Kernel/DrawManager.h"
#include "Kernel/SpriteManager.h"
#include "Kernel/WorldManager.h"
#include "Kernel/SoundManager.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
struct GameInfo;

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IGame
	\brief		Interface for the game engine.
	\author		Kronuz
	\version	1.0
	\date		June 27, 2003

*/
interface IGame :
	public CDocumentObject
{
	virtual ~IGame() {} // Virtual destructor. (needed when the objects are deleted indirectly through the interface)

	static const WORD Version;

	virtual float UpdateFPS(float fpsLock = -1.0f) = 0;

	// CDocumentObject override:
	virtual bool Load(LPCSTR szFile) = 0;
	virtual bool Save(LPCSTR szFile) = 0;
	virtual bool Save() = 0;
	virtual bool Close(bool bForce = false) = 0;

	virtual bool LoadWorld(LPCSTR szFile) = 0;
	virtual bool SaveWorld(LPCSTR szFile) = 0;
	virtual bool SaveWorld() = 0;
	virtual bool CloseWorld(bool bForce = false) = 0;

	virtual CSpriteSelection* CreateSpriteSelection(CDrawableContext **ppDrawableContext_) = 0;
	virtual void DeleteSpriteSelection(CSpriteSelection *pSelection) = 0;

	virtual void GetMapSize(CSize &mapSize) const = 0;
	virtual void SetMapSize(const CSize &mapSize) = 0;

	virtual void GetWorldSize(CSize &worldSize) const = 0;
	virtual void SetWorldSize(const CSize &worldSize) = 0;

	virtual CMapGroup *FindMapGroup(int x, int y) const = 0; //!< Get the mapgroup at location (x,y)
	virtual CMapGroup* BuildMapGroup(int x, int y, int width, int height) = 0;

	/*	\brief Sets the callback function for different objects.

		\remarks 
		Callback functions should be set only when they are needed. They are called
		whenever a change has been made in one object, and they can be used to keep 
		separate lists or trees of all existent objects in the game.
	*/
	virtual void SetSoundCallback(STATUSCHANGEDPROC StatusChanged, LPARAM lParam) = 0;
	virtual void SetSpriteCallback(STATUSCHANGEDPROC StatusChanged, LPARAM lParam) = 0;
	virtual void SetScriptCallback(STATUSCHANGEDPROC StatusChanged, LPARAM lParam) = 0;
	virtual void SetSpriteSheetCallback(STATUSCHANGEDPROC StatusChanged, LPARAM lParam) = 0;
	virtual void SetMapCallback(STATUSCHANGEDPROC StatusChanged, LPARAM lParam) = 0;
	virtual void SetMapGroupCallback(STATUSCHANGEDPROC StatusChanged, LPARAM lParam) = 0;

	virtual int ForEachSound(FOREACHPROC ForEach, LPARAM lParam) = 0;
	virtual int ForEachSprite(FOREACHPROC ForEach, LPARAM lParam) = 0;
	virtual int ForEachScript(FOREACHPROC ForEach, LPARAM lParam) = 0;
	virtual int ForEachSpriteSheet(FOREACHPROC ForEach, LPARAM lParam) = 0;
	virtual int ForEachMap(FOREACHPROC ForEach, LPARAM lParam) = 0;
	virtual int ForEachMapGroup(FOREACHPROC ForEach, LPARAM lParam) = 0;

	virtual DWORD GetModuleID() const = 0;
	virtual void SetConsole(IConsole *pConsole) = 0;
};

enum InfoType { itWorld, itMapGroup, itMap, itSpriteSheet, itSprite, itSpriteContext, itSound, itScript };
enum InfoReason { irNoReason, irAdded, irDeleted, irChanged };

struct WorldInfo
{
	InfoReason eInfoReason;
	LPCSTR lpszString;
	CWorld *pInterface;
};
struct MapGroupInfo
{
	InfoReason eInfoReason;
	LPCSTR lpszString;
	CMapGroup *pInterface;
};
struct SpriteSheetInfo
{
	InfoReason eInfoReason;
	LPCSTR lpszString;
	CSpriteSheet *pInterface;
};
struct SpriteInfo
{
	InfoReason eInfoReason;
	LPCSTR lpszString;
	CSprite *pInterface;
};
struct SpriteContextInfo
{
	InfoReason eInfoReason;
	LPCSTR lpszString;
	CSpriteContext *pInterface;
};
struct SoundInfo
{
	InfoReason eInfoReason;
	LPCSTR lpszString;
	CSound *pInterface;
};
struct ScriptInfo
{
	InfoReason eInfoReason;
	LPCSTR lpszString;
	CScript *pInterface;
};
struct GameInfo
{
	InfoType eInfoType;
	union {
		WorldInfo World;
		MapGroupInfo MapGroup;
		SpriteSheetInfo SpriteSheet;
		SpriteInfo Sprite;
		SpriteContextInfo SpriteContext;
		SoundInfo Sound;
		ScriptInfo Script;
	};
};

extern "C"
{
	HRESULT QueryGameInterface(WORD Version, IGame **pInterface, IConsole *Output);
	typedef HRESULT (*QUERYGAMEINTERFACE)(WORD Version, IGame **pInterface, IConsole *Output);

	HRESULT ReleaseGameInterface(IGame **pInterface);
	typedef HRESULT (*RELEASEGAMEINTERFACE)(IGame **pInterface);
}
