
#pragma once

#include "IArchiver.h"
#include "IDraw.h"

#include "Kernel/DrawManager.h"
#include "Kernel/SpriteManager.h"
#include "Kernel/WorldManager.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
struct GameInfo;

#define BUFFSIZE 200
#define MAX_PROPS 50

typedef enum tagCase { UpperCase, LowerCase } CASE;

#define SIMPLE_SLIDER	0x40C0C0C0
#define RED_SLIDER		0x400000FF
#define GREEN_SLIDER	0x4000FF00
#define BLUE_SLIDER		0x40FF0000

struct SInfo
{
	InfoType eType;
	char szScope[30];
	char szName[30];
	IPropertyEnabled *pPropObject;
};
struct SProperty
{
	enum _prop_type { ptUnknown, ptCategory, ptValue, ptRangeValue, ptBoolean, ptString, ptUCString, ptLCString, ptList } eType;
	char Buffer[BUFFSIZE];
	LPCSTR szPropName;
	bool bEnabled;
	bool bChanged;
	bool bMultivalue;
	UINT uMDL;	// Maximum data length
	union {
		LPSTR szString;		// ptString, ptUCString, ptLCString
		bool bBoolean;		// ptBoolean
		struct {			// ptValue, ptRangeValue
			int nValue;			
			int nLowerRange;
			int nHigherRange;
			DWORD dwSlider;
		};
		struct {			// ptList
			int nIndex;
			LPCSTR *List;
		};
	};
};
struct SPropertyList
{
	SInfo Information;
	int nProperties;
	SProperty aProperties[MAX_PROPS];

	SProperty* FindProperty(LPCSTR _szName, SProperty::_prop_type _eType = SProperty::ptUnknown)	{
		for(int i=0; i<nProperties; i++) {
			aProperties[i].szPropName = aProperties[i].Buffer;
			if( !stricmp(aProperties[i].Buffer, _szName) &&
				(_eType == SProperty::ptUnknown || _eType == aProperties[i].eType) ) 
				return &aProperties[i];
		}
		return NULL;
	}

	void Touch() {
		for(int i=0; i<nProperties; i++) {
			aProperties[i].bChanged = false;
		}
	}
	bool Merge(const SPropertyList *pPL) {
		for(int i=0; i<pPL->nProperties; i++) {
			SProperty *pP = FindProperty(pPL->aProperties[i].szPropName);
			if(pP) {
				if(pPL->aProperties[i].eType != SProperty::ptCategory && !pP->bMultivalue) {
					// if the new property has a different value from the existent,
					// we set the property to be multivalue.
					if(pP->eType == SProperty::ptString) {
						if(strcmp(pP->szString, pPL->aProperties[i].szString))
							pP->bMultivalue = true;
					} else if(pP->eType == SProperty::ptValue) {
						if(pP->nValue != pPL->aProperties[i].nValue)
							pP->bMultivalue = true;
					} else if(pP->eType == SProperty::ptBoolean) {
						if(pP->bBoolean != pPL->aProperties[i].bBoolean)
							pP->bMultivalue = true;
					} else if(pP->eType == SProperty::ptList) {
						if(pP->nIndex != pPL->aProperties[i].nIndex)
							pP->bMultivalue = true;
					}
					// just disables allowed, no enables are wanted here:
					if(pPL->aProperties[i].bEnabled == false) pP->bEnabled = false;
				}
			} else {
				if(!DupProperty(&pPL->aProperties[i])) return false;
			}
		}
		return true;
	}

	bool DupProperty(const SProperty *pP) {
		if(nProperties>=MAX_PROPS) return false;
		aProperties[nProperties] = *pP;
		aProperties[nProperties].szPropName = aProperties[nProperties].Buffer;
		if(pP->eType == SProperty::ptList) {
			LPSTR eob = aProperties[nProperties].Buffer + BUFFSIZE;
			LPSTR list = eob - aProperties[nProperties].uMDL;

			aProperties[nProperties].List = (LPCSTR *)list;
			for(int i=0; i<=10; i++) {
				LPCSTR tmp = aProperties[nProperties].List[i];
				if(tmp) {
					aProperties[nProperties].List[i] = 
						aProperties[nProperties].Buffer + (int)(tmp - pP->Buffer);
				}
			}
		} else if(pP->eType == SProperty::ptString) {
			LPSTR aux = aProperties[nProperties].Buffer + BUFFSIZE - aProperties[nProperties].uMDL;
			aProperties[nProperties].szString = aux;
		}

		nProperties++;
		return true;
	}
	bool SetName(LPCSTR _szName) {
		if(nProperties>=MAX_PROPS) return false;
		aProperties[nProperties].szPropName = aProperties[nProperties].Buffer;
		strncpy(aProperties[nProperties].Buffer, _szName, 29);
		aProperties[nProperties].uMDL = BUFFSIZE - strlen(aProperties[nProperties].szPropName) - 1;
		return true;
	}
	bool AddCategory(LPCSTR _szName, bool _bEnabled = true) {
		if(nProperties>=MAX_PROPS) return false;
		SetName(_szName);
		aProperties[nProperties].bEnabled = _bEnabled;
		aProperties[nProperties].bChanged = false;
		aProperties[nProperties].eType = SProperty::ptCategory;
		nProperties++;
		return true;
	}
	bool AddString(LPCSTR _szName, LPCSTR _szString, bool _bEnabled = true) {
		if(nProperties>=MAX_PROPS) return false;
		SetName(_szName);
		aProperties[nProperties].bEnabled = _bEnabled;
		aProperties[nProperties].bChanged = false;

		LPSTR aux = aProperties[nProperties].Buffer + BUFFSIZE - aProperties[nProperties].uMDL;

		aProperties[nProperties].szString = aux;
		strncpy(aux, _szString, aProperties[nProperties].uMDL-1);
		aProperties[nProperties].eType = SProperty::ptString;
		nProperties++;
		return true;
	}
	bool AddCaseString(LPCSTR _szName, LPCSTR _szString, CASE eCase, bool _bEnabled = true) {
		if(nProperties>=MAX_PROPS) return false;
		SetName(_szName);
		aProperties[nProperties].bEnabled = _bEnabled;
		aProperties[nProperties].bChanged = false;

		LPSTR aux = aProperties[nProperties].Buffer + BUFFSIZE - aProperties[nProperties].uMDL;

		aProperties[nProperties].szString = aux;
		strncpy(aux, _szString, aProperties[nProperties].uMDL-1);
		if(eCase == UpperCase) {
			for(;*aux; aux++) *aux = toupper(*aux);
			aProperties[nProperties].eType = SProperty::ptUCString;
		} else {
			for(;*aux; aux++) *aux = tolower(*aux);
			aProperties[nProperties].eType = SProperty::ptLCString;
		}
		nProperties++;
		return true;
	}
	bool AddList(LPCSTR _szName, int _nIndex, LPCSTR _szList, bool _bEnabled = true) {
		if(nProperties>=MAX_PROPS) return false;
		SetName(_szName);
		aProperties[nProperties].bEnabled = _bEnabled;
		aProperties[nProperties].bChanged = false;
		aProperties[nProperties].nIndex = _nIndex;
		LPSTR eob = aProperties[nProperties].Buffer + BUFFSIZE;
		LPSTR list = eob - aProperties[nProperties].uMDL;

		aProperties[nProperties].List = (LPCSTR *)list;
		LPSTR buffer = list + 11*sizeof(LPSTR);
		LPCSTR tok = _szList;

		// tokenize the list:
		for(int i=0; tok && i<=10; i++) {
			while(*tok==' ' || *tok==',' || *tok=='\t') tok++;
			LPCSTR aux = strchr(tok, ',');
			int len;
			if(aux) len = aux-tok;
			else len = strlen(tok);
			
			if(len > 29) len = 29;
			if(eob - buffer < len + 1) break;

			aProperties[nProperties].List[i] = buffer;
			strncpy(buffer, tok, len);
			buffer+=len;
			buffer++;
			tok = aux;
		}
		aProperties[nProperties].eType = SProperty::ptList;
		nProperties++;
		return true;
	}
	bool AddValue(LPCSTR _szName, int _nValue, bool _bEnabled = true) {
		if(nProperties>=MAX_PROPS) return false;
		SetName(_szName);
		aProperties[nProperties].bEnabled = _bEnabled;
		aProperties[nProperties].bChanged = false;
		aProperties[nProperties].nValue = _nValue;
		aProperties[nProperties].eType = SProperty::ptValue;
		nProperties++;
		return true;
	}
	bool AddRange(LPCSTR _szName, int _nValue, int _nLower, int _nHigher, DWORD _dwSlider, bool _bEnabled = true) {
		if(nProperties>=MAX_PROPS) return false;
		SetName(_szName);
		aProperties[nProperties].bEnabled = _bEnabled;
		aProperties[nProperties].bChanged = false;
		aProperties[nProperties].nValue = _nValue;
		aProperties[nProperties].nLowerRange = _nLower;
		aProperties[nProperties].nHigherRange = _nHigher;
		aProperties[nProperties].dwSlider = _dwSlider;
		aProperties[nProperties].eType = SProperty::ptRangeValue;
		nProperties++;
		return true;
	}
	bool AddBoolean(LPCSTR _szName, bool _bBoolean, bool _bEnabled = true)	{
		if(nProperties>=MAX_PROPS) return false;
		SetName(_szName);
		aProperties[nProperties].bEnabled = _bEnabled;
		aProperties[nProperties].bChanged = false;
		aProperties[nProperties].bBoolean = _bBoolean;
		aProperties[nProperties].eType = SProperty::ptBoolean;
		nProperties++;
		return true;
	}
};

interface ISound 
{
	virtual DWORD Play(bool _bForever = false) = 0;
	virtual void Loop(int _repeat = -1) = 0;
	virtual void Stop(DWORD ID) = 0;
	virtual void Pause(DWORD ID) = 0;

	virtual bool IsPlaying(DWORD ID) = 0;

	virtual void SetVolume(DWORD ID, int _volume) = 0;
	virtual int GetVolume(DWORD ID) = 0;

	virtual int GetLoopBack() const = 0;
	virtual void SetLoopBack(int _loop) = 0;

	virtual void SetCurrentPosition(DWORD ID, int _pos) = 0;

	virtual LPCSTR GetSoundFilePath(LPSTR szPath, size_t buffsize) = 0;
};

interface ISoundManager
{
	virtual void DoMusic() = 0;
	virtual void SwitchMusic(ISound *_pSound, int _loopback, bool _fade = true) = 0;
	virtual void SetMusicVolume(int _volume) = 0;
	virtual int GetMusicVolume() const = 0;
	virtual void SetMusicFadeSpeed(int _speed) = 0;
	virtual int GetMusicFadeSpeed() const = 0;
};

interface IScript
{
	virtual bool NeedToCompile() const = 0;
	virtual LPCSTR GetScriptFilePath(LPSTR szPath, size_t buffsize) const = 0;
	virtual LPCSTR GetCompiledFilePath(LPSTR szPath, size_t buffsize) const = 0;
};

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
	virtual void WaitScripts() = 0;

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

	virtual LPCSTR GetProjectName() const = 0;
	virtual int CountScripts() const = 0;
	virtual const IScript* GetScript(int idx) const = 0;
	virtual ISoundManager* GetSoundManager() const = 0;
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
	ISound *pInterface;
};
struct ScriptInfo
{
	InfoReason eInfoReason;
	LPCSTR lpszString;
	IScript *pInterface;
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
