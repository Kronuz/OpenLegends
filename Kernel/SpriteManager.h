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
/*! \file		SpriteManager.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the classes that maintain sprites properties.
	\date		April 15, 2003
				April 17, 2003
				September 3, 2003
				September 16, 2003
				July 15, 2005:
						* Added CMutable Touch() calls
				February 12, 2006:
						* Added CEntityData
						* Implemented CEntityData
				Feb 13, 2006:
						* Added static functions and variables for optimized retrieval of contexts through CEntityData.

	All sprites are almost the same, but to distinguish sprites that
	can handle an advanced interaction (entities) from those which do not 
	(backgrounds), we are going to call Background Sprites to those objects that never move, 
	and that have no other purpose in the game other than to look nice 
	and occupy a space in the world (some times a solid space). Background
	sprites must use Mask maps if they need some sort of collision 
	detection. An example of background sprites would be a flower, or a fence.
	
	Entities, on the other hand, are sprites that can maintain an advanced
	interaction with other entities. This advanced interaction is ruled by
	the entity's script file. An enemy or a chest are classic examples of 
	entities. Entities can have either of two methods for collision 
	detection: the use a mask map (as background sprites) or the use any
	of the collision detection functions inside its associated script.

	A mask map is a special type of sprite that it's attached to backgrounds or
	to some entities. Mask maps are not visible in the game, its functionality
	is more related to the behavior of other entities approaching to its parent
	background or entity. A mask map can be set to solidify objects or to make 
	objects  harder to penetrate. This is done using different colors, in the 
	mask, to map different grades of body solidification, matching the 
	areas of the background or entity sprite that need to be solidified. Using 
	different shades of red, the closer it gets to the red, the easier it's to 
	pass through. Solid red, howeve, only slows down any entity trying to cross 
	the mask, while green presents no obstacule at all, as if no mask map existed 
	in that zone)
	
	\sa CSprite, CBackground, CEntity, and CMaskMap
*/

#pragma once

#include <IArchiver.h>

#include "DrawManager.h"
#include <cmath>

#include <map>
#include <vector>
#include "Misc.h"

#define SPRITE_BOUNDS		0x0001	// show sprite boundaries?
#define SPRITE_MASKS		0x0002	// show the sprite mask, if any?
#define SPRITE_ENTITIES		0x0004	// show entities?
#define GRAPHICS_FILTERS	0x0100	// Activate the filters, mainly to show a frame around the world.

enum _Chain { relative=0, stretch=1, left=3, right=2, up=2, down=3, fixed=4 };

extern bool g_bBounds;
extern bool g_bMasks;
extern bool g_bEntities;


/*! \brief Enumaration of all sprite types.

	Background, Entity and Mask are be mutually exclusive, and all 
	sprites(except for the entities) are tiles.\n
	Generally, all sprites are marked as background, which just means
	that the sprite is visible, but not an entity.\n
	Sprites that are entities, in addition to being visible, also have 
	a script and they represent an actual entity in the world.\n
	Masks have the job of creating collision maps for the entities 
	in the game.\n \n
	Collision maps are generally defined inside the scripts
	for the entities, and masks are most useful for background sprites.
*/
enum _spt_type	{
		tMask,			//!< Indicates a mask map sprite (or unknown)
		tBackground,	//!< Indicates a background sprite
		tEntity,		//!< Indicates an entity sprite
		tSpriteSet		//!< Indicates a group of sprites or a sprite set
	};	

enum _Direction { 
		_d_up=0,		_d_down=1,		_d_left=2,		_d_right=3,
		_d_top=0,		_d_bottom=1,
		_d_north=0,		_d_south=0,		_d_west=2,		_d_east=3
	};

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
interface IScript;

class CGameManager;
class CSpriteSheet;
class CLayer;
class CBackground;
class CScript;

typedef CBackground CMaskMap;

/////////////////////////////////////////////////////////////////////////////
/*! \struct		SSpriteData
	\brief		Data structure of the sprites.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		April 28, 2003

	Structure to contain all the intrinsic information for the flyweight the sprites.

	\sa CSprite
*/
struct SSpriteData {
	// Sprite:
	_spt_type  SptType;		//!< Sprite data type
	int iAnimSpd;			//!< Animation speed of the sprite
	bool bAnimLoop;			//!< Animation in a loop, or play once only mode.
	_Direction eAnimDir;	//!< Animation direction (must be up, or down)
	UCHAR cAlphaValue; 	/*!< \brief Sets the intensity of the entire sprite.

		Although sprites can have an alpha value attatched to each pixel, 
		within the image file (sprite sheet), there is also the possibility to 
		change the overall alpha value of a specific sprite.
	*/
	int nSubLayer; 	/*!< \brief SubLayer of the sprite.
		
		This is it, finally, this member defines the default "sublayer" to which
		the sprite belongs to.\n
		There are two types of layers: Layers and Sublayers. Sublayers are defined
		for	each sprite when it is created, whilst Layers are chosen in the map
		editor at the time the sprite is positioned. Sublayers are made to facilitate
		the use of layers, giving the default Z order of the objects within the 
		current layer. (more on this latter)
	*/
};
struct SBackgroundData : public SSpriteData
{
	SBackgroundData() { SptType = tBackground; }

	// Background:
	CMaskMap *pMaskMap;		/*!< \brief Pointer to the mask map fot the sprite.

		The value can be NULL if no mask is associated with the sprite,
		if a mask map is associated with the sprite, the pointer should be a
		valid pointer to a mask, although the mask can be either just declared
		or declared and defined.
	*/
};
struct SEntityData : public SBackgroundData
{
	SEntityData() { SptType = tEntity; }

	// Entity:
	CScript *pScript; 	/*!< \brief Name of the entity's script.

		Every entity should have a script to represent its behavior, 
		this member keeps a pointer to the corresponfing script.\n
		Note that neither background nor mask sprites can be associated 
		with a script, thus this should be either NULL or not existent
		for non-entity sprites.
	*/
};
/////////////////////////////////////////////////////////////////////////////
/*! \class		CEntityData
	\brief		The entity data container.
	\author		Littlebuddy
	\version	1.0
	\date		February 12, 2006

	The CEntityData class interacts with the CSpriteContext class to store the
	data for each entity in an effective way, allowing us to easily retrieve this
	data. 
	
	\remarks

	\sa 
	\todo Some more testing would be useful.
*/
class CEntityData{
public:
	typedef std::pair<CBString, CDrawableContext*> contextPair;
	static std::vector<contextPair> ms_ContextIndex;
	static CDrawableContext* FindContext(LPCSTR szName);
	static void InsertContext(LPCSTR szName, CDrawableContext *context);
	static void RemoveContext(LPCSTR szName);

protected:
	//Data-storage types:
	//Valuebased items
	typedef std::pair<int,		int>		vvPair;
	typedef std::pair<CBString, int>		svPair;
	//Stringbased items
	typedef std::pair<int,		CBString>	vsPair;
	typedef std::pair<CBString, CBString>	ssPair;
	//Flags (Booleans)
	typedef std::pair<int,		bool>		vfPair;
	typedef std::pair<CBString, bool>		sfPair;

	//Data-storage vectors:
	std::vector<vvPair>	m_vvStorage;	//Value-based value storage.
	std::vector<svPair>	m_svStorage;	//String-based value storage.
	std::vector<vsPair>	m_vsStorage;	//Value-based string storage.
	std::vector<ssPair>	m_ssStorage;	//String-based string storage.
	std::vector<vfPair>	m_vfStorage;	//Value-based flag storage. 
	std::vector<sfPair>	m_sfStorage;	//String-based flag storage.


public:

	//Storage retrieval:
	int GetValue(int Id);	//Entity retrieved from is done elsewhere. Personal note.
	int GetValue(LPCSTR Id);
	bool GetFlag(int Id);
	bool GetFlag(LPCSTR Id);
	CBString GetString(int Id);
	CBString GetString(LPCSTR Id);
	
	//Storage insertion: (Returns previously exists?true:false)
	bool SetValue(	int Id,		int Value = 0);
	bool SetValue(	LPCSTR Id,	int Value = 0);
	bool SetFlag(	int Id,		bool Set = true);
	bool SetFlag(	LPCSTR Id,	bool Set = true);
	bool SetString(	int Id,		LPCSTR Text = "");
	bool SetString(	LPCSTR Id,	LPCSTR Text = "");

	//Storage clearing:
	inline void ClearData(){
		m_vvStorage.clear();
		m_svStorage.clear();
		m_vsStorage.clear();
		m_ssStorage.clear();
		m_vfStorage.clear();
		m_sfStorage.clear();
	}
};
inline void CEntityData::InsertContext(LPCSTR szName, CDrawableContext *context){
	contextPair pair;
	pair.first = CBString(szName);
	pair.second = context;
	ms_ContextIndex.push_back(pair);
}
/////////////////////////////////////////////////////////////////////////////
/*! \class		CSprite
	\brief		The sprites base class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		April 15, 2003

	CSprite class provides the base functionality of all sprites on the game,
	including visible (such as CBackground) and invisible (as CMaskMap) sprites.
	This class maintains all the sprite properties as single sprites, whether
	they are entities, background sprites or mask maps.\n
	This class does NOT give any information about the sprite in the map
	or its actual status. For real sprites on the map, giving information 
	about its state, location and other details, refer to CEntityI or
	CBackgroundI class.

	\remarks
	Every sprite must have a name, and the names can never be the same, 
	they are unique	and represent a sole sprite in the full game.

	\sa CBackground, CEntity and CMaskMap
	\todo Write the implementation of this class.
*/
class CSprite :
	public CDrawableObject
{
public:
	const struct CatalogCompare : 
	public std::binary_function<const CSprite*, const CSprite*, bool> {
		bool operator()(const CSprite *a, const CSprite *b) const {
			return(a->m_nCatalog > b->m_nCatalog);
		}
	};
	friend CatalogCompare;

	int m_nCatalog;
	CSprite(LPCSTR szName);
	virtual ~CSprite();

	static void SetShowOptions(WORD wFlags) {
		if(wFlags&SPRITE_BOUNDS) g_bBounds = true;
		else g_bBounds = false;
		if(wFlags&SPRITE_MASKS) g_bMasks = true;
		else g_bMasks = false;
		if(wFlags&SPRITE_ENTITIES) g_bEntities = true;
		else g_bEntities = false;
	}

protected:
	bool m_bDefined; /*!< \brief Shows if the sprite is a defined sprite.

		A sprite can be either just declared or declared and defined. A
		declared sprite is one that has been referred at some point by anyone,
		while a defined sprite is one that really exist in a file, and one that
		has all its properties and full functionality.
		\sa m_iReferences
	*/

	CSpriteSheet *m_pSpriteSheet;	/*!< \brief Pointer to the sprites sheet that contains this sprite.

		This can never be a NULL, since every animation belongs to a 
		sprite sheet (and to only one).
	*/

	SSpriteData *m_pSpriteData;

	std::vector<CRect> m_Boundaries; 	/*!< \brief List of boundaries boxes for the sprite.

		This list keeps the boundaries boxes of the sprite. If the sprite is an 
		animation, there should be more than one element in the list.
		Each element represents the possition and size of the sprite in the
		sprite sheet. All values must be valid and within the sprite
		sheet image file boundaries.
	*/

	_spt_type m_SptType;	/*!< \brief Current sprite type.
			
			Sprites marked as background or entities are supposed 
			to be visible in the map editor whilst masked sprites (or maps)
			are just noticable during the game, but not visibles.\n
			In a mask, the black color is impenetrable and different shades of
			red slow down the character (the closer it gets to the red, the 
			faster the entities can move, but always slower than the normal speed
			of the entity). Green zones on the mask represent
			no obstacle thus this areas do not slow down nor stop the entity from passing.
		*/

	bool Draw(const CDrawableContext &context, bool bBounds, const ARGBCOLOR *rgbColorModulation, int nBuffer);

public:
	_spt_type GetSpriteType() const;
	void SetSpriteType(_spt_type SptType);
	CSpriteSheet* GetSpriteSheet();
	bool IsDefined();

	void AddRect(RECT rcRect);
	void SetSpriteSheet(CSpriteSheet *pSpriteSheet);

	void SetSpriteData(SSpriteData *pSpriteData);
	SSpriteData* GetSpriteData();

	// CDrawableObject Interface:
	virtual void GetSize(CSize &Size) { 
		Size.SetSize(m_Boundaries[0].Width(), m_Boundaries[0].Height());
	}
	virtual void GetOrigin(CPoint &Point) {
		Point.SetPoint(
			m_Boundaries[0].CenterPoint().x - m_Boundaries[0].left, 
			m_Boundaries[0].Height() );
	}

	// CSprite Interface:
	virtual void GetBaseRect(CRect &Rect) {
		Rect = m_Boundaries[0];
	}
	virtual void SetCatalogOrder(int nCatalog) { m_nCatalog = nCatalog; }
	virtual int GetCatalogOrder() { return m_nCatalog; }

};
/////////////////////////////////////////////////////////////////////////////
// CSprite Implementation:
/////////////////////////////////////////////////////////////////////////////
inline _spt_type CSprite::GetSpriteType() const
{ 
	return m_SptType; 
}
inline void CSprite::SetSpriteType(_spt_type SptType) 
{ 
	// only valid conversion between types is tMask <-> tBackground
	ASSERT(m_SptType == tMask || m_SptType == tBackground);
	ASSERT(SptType == tMask || SptType == tBackground);
	m_SptType = SptType; 
}
inline CSpriteSheet* CSprite::GetSpriteSheet() 
{ 
	if(m_bDefined) return m_pSpriteSheet; 
	return NULL;
}
inline bool CSprite::IsDefined() 
{ 
	return m_bDefined; 
}
inline void CSprite::SetSpriteSheet(CSpriteSheet *pSpriteSheet) 
{
	m_pSpriteSheet = pSpriteSheet;
	if(m_pSpriteSheet) m_bDefined = true;
}
inline void CSprite::AddRect(RECT rcRect) 
{
	m_Boundaries.push_back(rcRect);
}

inline SSpriteData* CSprite::GetSpriteData() {
	return m_pSpriteData;
}

inline void CSprite::SetSpriteData(SSpriteData *pSpriteData) {
	m_pSpriteData = pSpriteData;
}

/////////////////////////////////////////////////////////////////////////////
/*! \class		CBackground
	\brief		Background sprites class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		April 15, 2003

	The CBackground class provides the functionality for all visible objects on the game,
	this includes a base to manage Entities

	\remarks
	This class, and its derivates (i.e. CEntity), are implemented to work as flyweight 
	objects, receiving any extrinsic state as a SDrawContext parameter for the
	CDrawableObject interface.

	\sa CDrawableObject, SDrawContext, CEntity and CMaskMap
	\todo Write the implementation of this class.
*/
class CBackground : 
	public CSprite
{
public:
	CBackground(LPCSTR szName);
protected:
public:
	int GetAlphaValue() {
		return reinterpret_cast<SBackgroundData*>(m_pSpriteData)->cAlphaValue;
	}
	int GetObjSubLayer() {
		return reinterpret_cast<SBackgroundData*>(m_pSpriteData)->nSubLayer;
	}
	bool Draw(const CDrawableContext &context, const ARGBCOLOR *rgbColorModulation);
	bool NeedToDraw(const CDrawableContext &scontext);
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CEntity
	\brief		Entity sprites class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		April 16, 2003

	The CEntity class provides the functionality for entities in the game.
	Every entity has the unique characteristic of owning a script that
	regulates and keeps its behavior and interaction with other entities
	and sprites.

	\sa CBackground and CMaskMap
	\todo Write the implementation of this class.
*/
class CEntity :
//	public IScriptableObject,
	public CBackground
{
public:
	CEntity(LPCSTR szName);
protected:

public:
	const IScript* GetScript() const;
	bool Run(const CDrawableContext &context, RUNACTION action);
	bool Draw(const CDrawableContext &context, const ARGBCOLOR *rgbColorModulation);
	bool NeedToDraw(const CDrawableContext &context) { return CBackground::NeedToDraw(context); }
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CSpriteSheet
	\brief		CSpriteSheet class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		April 15, 2003

	This class keeps a list of pointers to all the sprites and a list of all 
	the animations that constitute the actual sprite sheet.\n
	This class does NOT give any information about the sprites in the map
	or its actual state.

	This class reads, keeps, and maintains a flyweight pool of sprites,
	each sprite shares a common state, but receives a context from the client 
	that needs to use it.

	\sa CSprite
	\todo Write the implementation of this class.
*/
class CSpriteSheet :
	public CNamedObj,
	public CDocumentObject
{
	friend CGameManager;
protected:
	CGameManager *m_pGameManager;

	/*
	typedef std::vector<CSprite *> vectorSprite;
	vectorSprite m_Sprites; //!< Flyweight pool of sprites.
	/*/
	typedef std::vector<CSprite *> vectorSprite;
	vectorSprite m_Catalog; //!< Sprites' catalog.

	typedef std::pair<CBString, CSprite*> pairSprite;
	typedef std::map<CBString, CSprite*> mapSprite;
	mapSprite m_Sprites; //!< Flyweight pool of sprites.
	/**/

	//! No one but the Project Manager is alowed to create Sprite Sheets
	CSpriteSheet(CGameManager *pGameManager);
	~CSpriteSheet();
public:
	bool _Close(bool bForce) { return true; }

	DWORD m_dwLastTry;
	ITexture *m_pTexture; //! cached texture (valid only if it was aquired with the same Device ID)

	const CBString& GetName() const { return CNamedObj::GetName(); }

	int ForEachSprite(SIMPLEPROC ForEach, LPARAM lParam);

	// Interface:
	virtual LPCSTR GetFilePath(LPSTR szPath, size_t buffsize) const
	{
		strncpy(szPath, (LPCSTR)m_fnFile.GetAbsFilePath(), buffsize);
		return szPath;
	}
	virtual LPCSTR GetName(LPSTR szPath, size_t buffsize) const
	{
		strncpy(szPath, (LPCSTR)GetName(), buffsize);
		return szPath;
	}
	virtual CSprite* FindSprite(LPCSTR szSpriteName) const
	{
		/*
		vectorSprite::const_iterator Iterator = 
			find_if(m_Sprites.begin(), m_Sprites.end(), bind2nd(CNamedObj::NameCompare(), szSpriteName));
		if(Iterator == m_Sprites.end()) return NULL;
		return *Iterator;
		/*/
		mapSprite::const_iterator Iterator = m_Sprites.find(szSpriteName);
		if(Iterator == m_Sprites.end()) return NULL;
		return Iterator->second;
		/**/
	}
	// sort by the catalog:
	void BuildCatalog()
	{
		if(m_Catalog.empty()) {
			mapSprite::iterator Iterator = m_Sprites.begin();
			while(Iterator != m_Sprites.end()) {
				m_Catalog.push_back(Iterator->second);
				Iterator++;
			}
			sort(m_Catalog.begin(), m_Catalog.end(), CSprite::CatalogCompare());
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
/*! \class	CSpriteContext
	\brief		Flyweight sprites context class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		April 28, 2003

	This class maintains the extrinsic status of the CSprite's flyweight objects.
	It is a concrete class based on the interface for flyweight drawable
	objects.

	\sa CDrawableObject
*/
class CSpriteContext :
	public CDrawableContext
{
protected:
//-------------------------------------
// TO KEEP THE MEMENTO:
	struct StateSpriteContext : 
		public CDrawableContext::StateDrawableContext
	{
		virtual bool operator==(const StateData& state) const {
			const StateSpriteContext *curr = static_cast<const StateSpriteContext*>(&state);
			return (
				StateDrawableContext::operator ==(state) &&
				curr->rgbColor == rgbColor &&
				curr->eXChain == eXChain &&
				curr->eYChain == eYChain
			);
		}

		ARGBCOLOR rgbColor;
		_Chain eXChain;
		_Chain eYChain;
	};
// DATA TO KEEP:
	ARGBCOLOR m_rgbColor;
	_Chain m_eXChain;
	_Chain m_eYChain;
//-------------------------------------
protected:

public:
	mutable int m_nFrame[CONTEXT_BUFFERS];

	CSpriteContext(LPCSTR szName);
	~CSpriteContext();

	CEntityData *m_pEntityData;
	void Mirror();
	void Flip();
	void Mirror(bool bMirror);	//!< Mirrors the object.
	void Flip(bool bFlip);		//!< Flips the object.
	void Alpha(int alpha);
	void Red(int red);
	void Green(int green);
	void Blue(int blue);
	void Lightness(int lightness);
	void ARGB(ARGBCOLOR rgbColor_);
	void Rotate(int rotate);	//!< Rotates the object (the angle is given in degrees)
	void Tile(bool bTile = true);
	void SetYChain(_Chain eYChain);
	void SetXChain(_Chain eXChain);

	_Chain GetXChain() const;
	_Chain GetYChain() const;
	bool isTiled() const;
	bool isMirrored() const;
	bool isFlipped() const;
	int getAlpha() const;
	int getRed() const;
	int getBlue() const;
	int getGreen() const;
	int getLightness() const;
	ARGBCOLOR getARGB() const;
	ARGBCOLOR getRGB() const;
	int Transformation() const;

	_spt_type GetSpriteType() const;
	
	int Rotation() const;				//!< returns the basic roatation of the object (defined in the map editor)

	float RelRotation() const;			//!< returns the relative roatation of the object in radians (definded during the game)
	float RelScale() const;				//!< returns the relative scale of the object (defined during the game)

	// Interface:
	virtual bool GetInfo(SInfo *pI) const;
	virtual bool GetProperties(SPropertyList *pPL) const;
	virtual bool SetProperties(SPropertyList &PL);
	virtual void Commit() const;
	virtual void Cancel();

	virtual CDrawableContext* MakeGroup(LPCSTR szGroupName);

	// Memento interface
	virtual void ReadState(StateData *data);
	virtual void WriteState(StateData *data);
	virtual int _SaveState(UINT checkpoint);
	virtual int _RestoreState(UINT checkpoint);
	
	static int CALLBACK DestroyCheckpoint(LPVOID Interface, LPARAM lParam);

private:
	// Commit variables (saved in case of abortion):
	mutable ARGBCOLOR Commit_rgbColor;
};
/////////////////////////////////////////////////////////////////////////////
// CSpriteContext Implementation:
/////////////////////////////////////////////////////////////////////////////
inline void CSpriteContext::Mirror() 
{
	if(isMirrored()) Mirror(false);
	else Mirror(true);
	Touch();
}
inline void CSpriteContext::Flip() 
{
	if(isFlipped()) Flip(false);
	else Flip(true);
	Touch();
}
inline void CSpriteContext::Mirror(bool bMirror) 
{
	if(bMirror) m_dwStatus |= (SMIRRORED<<_SPT_TRANSFORM);
	else		m_dwStatus &= ~(SMIRRORED<<_SPT_TRANSFORM);
	Touch();
}
inline void CSpriteContext::Flip(bool bFlip) 
{
	if(bFlip)	m_dwStatus |= (SFLIPPED<<_SPT_TRANSFORM);
	else		m_dwStatus &= ~(SFLIPPED<<_SPT_TRANSFORM);
	Touch();
}
inline void CSpriteContext::Red(int red) 
{
	m_rgbColor.rgbRed = (BYTE)(red / 2 + 128);
	Touch();
}
inline void CSpriteContext::Green(int green) 
{
	m_rgbColor.rgbGreen = (BYTE)(green / 2 + 128);
	Touch();
}
inline void CSpriteContext::Blue(int blue) 
{
	m_rgbColor.rgbBlue = (BYTE)(blue / 2 + 128);
	Touch();
}
inline void CSpriteContext::Alpha(int alpha) 
{
	m_rgbColor.rgbAlpha = (BYTE)alpha;
	Touch();
/* Deprecated, now use full ARGB values (Strider's idea):
	DWORD newAlpha = ((alpha<<_SPT_ALPHA)&SPT_ALPHA);
	if(newAlpha != (m_dwStatus&SPT_ALPHA)) {
		m_dwStatus &= ~SPT_ALPHA;
		m_dwStatus |= newAlpha;
	}
*/
}
inline void CSpriteContext::ARGB(ARGBCOLOR rgbColor_) 
{
	if(m_rgbColor == rgbColor_) return;
	m_rgbColor = rgbColor_;
	Touch();
}
inline void CSpriteContext::Rotate(int rotate) 
{
	m_dwStatus &= ~SPT_ROT;
	m_dwStatus |= ((rotate<<_SPT_ROT)&SPT_ROT);
	Touch();
}
inline void CSpriteContext::Tile(bool bTile) 
{
	if(!bTile)	m_dwStatus |= (SNTILED<<_SPT_INFO);
	else		m_dwStatus &= ~(SNTILED<<_SPT_INFO);
	Touch();
}
inline void CSpriteContext::SetYChain(_Chain eYChain)
{
	m_eYChain = eYChain;
}
inline void CSpriteContext::SetXChain(_Chain eXChain)
{
	m_eXChain = eXChain;
}

inline _Chain CSpriteContext::GetXChain() const
{
	return m_eXChain;
}
inline _Chain CSpriteContext::GetYChain() const
{
	return m_eXChain;
}
inline bool CSpriteContext::isTiled() const
{
	return !((m_dwStatus&(SNTILED<<_SPT_INFO))==(SNTILED<<_SPT_INFO));
}
inline bool CSpriteContext::isMirrored() const
{
	return ((m_dwStatus&(SMIRRORED<<_SPT_TRANSFORM))==(SMIRRORED<<_SPT_TRANSFORM));
}
inline bool CSpriteContext::isFlipped() const
{
	return ((m_dwStatus&(SFLIPPED<<_SPT_TRANSFORM))==(SFLIPPED<<_SPT_TRANSFORM));
}
inline int CSpriteContext::getLightness() const
{
	AHSLCOLOR hslColor = RGB2HSL(m_rgbColor);
	return hslColor.hslLightness;
}

inline void CSpriteContext::Lightness(int lightness)
{
	AHSLCOLOR hslColor = RGB2HSL(m_rgbColor);
	hslColor.hslLightness = lightness;

	if(m_rgbColor == HSL2RGB(hslColor)) return;
	m_rgbColor = HSL2RGB(hslColor);
	Touch();
}
inline int CSpriteContext::getRed() const
{
	int red = ((int)m_rgbColor.rgbRed - 128) * 2;
	return red;
}
inline int CSpriteContext::getGreen() const
{
	int green = ((int)m_rgbColor.rgbGreen - 128) * 2;
	return green;
}
inline int CSpriteContext::getBlue() const
{
	int blue = ((int)m_rgbColor.rgbBlue - 128) * 2;
	return blue;
}
inline int CSpriteContext::getAlpha() const
{
	return m_rgbColor.rgbAlpha;
/* Deprecated, now use full ARGB values (Strider's idea):
	return ((m_dwStatus&SPT_ALPHA)>>_SPT_ALPHA);
*/
}
inline ARGBCOLOR CSpriteContext::getARGB() const
{
	return m_rgbColor;
}
inline ARGBCOLOR CSpriteContext::getRGB() const
{
	ARGBCOLOR rgbColorRet = m_rgbColor;
	rgbColorRet.rgbAlpha = 255;
	return rgbColorRet;
}
inline int CSpriteContext::Transformation() const 
{
	return ((m_dwStatus&SPT_TRANSFORM)>>_SPT_TRANSFORM);
}
inline int CSpriteContext::Rotation() const
{
	return ((m_dwStatus&SPT_ROT)>>_SPT_ROT);
}

inline float CSpriteContext::RelRotation() const
{
	return 0.0f;
}
inline float CSpriteContext::RelScale() const
{
	return 1.0f;
}
inline _spt_type CSpriteContext::GetSpriteType() const
{
	if(!m_pDrawableObj) return tSpriteSet;
	else return static_cast<CSprite*>(m_pDrawableObj)->GetSpriteType();
}
/////////////////////////////////////////////////////////////////////////////
/*! \class	CSpriteSetContext
	\brief		Flyweight sprites context class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		July 15, 2005

	\todo	Class yet to be implemented. 
	This class keeps the state of a sprite set which is nothing more 
	than a group of sprite contexts.

	\sa CDrawableObject, CSpriteContext
*/
class CSpriteSetContext :
	public CSpriteContext
{
protected:
	bool m_bOriginalContext;	//!< Keeps the state of the sprite set against the original saved context

public:
	CSpriteSetContext(LPCSTR szName) : CSpriteContext(szName) { Tile();	}
	~CSpriteSetContext() { m_Children.clear(); }
};
