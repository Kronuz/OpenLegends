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
/*! \file		SpriteManager.h 
	\brief		Interface of the classes that maintain sprites properties.
	\date		April 15, 2003

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
#include "Console.h"
#include "interfaces.h"
#include "FilePath.h"
#include "Archiver.h"

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
					tUnknown,		//!< Indicates an unknown sprite type
					tBackground,	//!< Indicates a background sprite
					tEntity,		//!< Indicates an entity sprite
					tMask			//!< Indicates a mask sprite (or Mask map)
				};	

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CProjectManager;
class CSpriteSheet;
class CAnimation;

class CMaskMap;

/////////////////////////////////////////////////////////////////////////////
/*! \class		CScript
	\brief		The Scripts base class.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	\todo Write the implementation of this class.
*/
class CScript :
	public CConsole
{
public:
	CFileName m_fnScriptFile;

	bool NeedsToCompile();
	CString GetScriptFile();
	CString GetAmxFile();
};
 
/////////////////////////////////////////////////////////////////////////////
/*! \struct		SSpriteData
	\brief		Data structure of the sprites.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	Structure to contain all the intrinsic information for the flyweight the sprites.

	\sa CSprite
*/
struct SSpriteData {
	// Sprite:
	int iAnimSpd;		//!< Animation speed of the sprite
	bool bVisible;		//!< Is the sprite visible?
	bool bFlipped;		//!< Is the sprite flipped?
	bool bMirrored;		//!< Is the sprite mirrored?
	int iRotation;		//!< Sprite rotation (0, 90 180, or 270)
};
struct SMaskData : public SSpriteData
{
};
struct SBackgroundData : public SSpriteData
{
	// Background:
	UCHAR cAlphaValue; 	/*!< \brief Sets the intensity of the entire sprite.

		Although sprites can have an alpha value attatched to each pixel, 
		within the image file (sprite sheet), there is also the possibility to 
		change the overall alpha value of a specific sprite.
	*/
	CString sSubLayer; 	/*!< \brief Name of the sublayer of the sprite.
		
		This is it, finally, this member defines the default "sublayer" to which
		the sprite belongs to.\n
		There are two types of layers: Layers and Sublayers. Sublayers are defined
		for	each sprite when it is created, whilst Layers are chosen in the map
		editor at the time the sprite is positioned. Sublayers are made to facilitate
		the use of layers, giving the default Z order of the objects within the 
		current layer. (more on this latter)
	*/
	CMaskMap *pMaskMap;		/*!< \brief Pointer to the mask map fot the sprite.

		The value can be NULL if no mask is associated with the sprite,
		if a mask map is associated with the sprite, the pointer should be a
		valid pointer to a mask, although the mask can be either just declared
		or declared and defined.
	*/
};
struct SEntityData : public SBackgroundData
{
	// Entity:
	CScript *pScript; 	/*!< \brief Name of the entity's script.

		Every entity should have a script to represent its behavior, 
		this member keeps a pointer to the corresponfing script.\n
		Note that neither background nor mask sprites can be associated 
		with a script, thus this should be NULL for non-entity sprites.
	*/
};
/////////////////////////////////////////////////////////////////////////////
/*! \class		CSprite
	\brief		The sprites base class.
	\author		Kronuz
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
	public CNamedObj
{
public:
	CSprite(LPCSTR szName);
	virtual ~CSprite();

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

	CSimpleArray<CRect> m_Boundaries; 	/*!< \brief List of boundaries boxes for the sprite.

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

public:
	_spt_type GetSpriteType();
	CSpriteSheet* GetSpriteSheet();
	bool IsDefined();

	void AddRect(RECT rcRect);
	void SetSpriteSheet(CSpriteSheet *pSpriteSheet);
	void SetSpriteData(SSpriteData *pSpriteData);
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CMaskMap
	\brief		Mask maps class.
	\author		Kronuz
	\version	1.0
	\date		April 15, 2003

	This class manages the mask maps for the sprites that need a mask map to
	detect collisions.
	In general, mask maps are most useful for CBackground sprites,
	leaving to the scripts the job to manage entities's collision detection.

	\sa CBackground end CEntity
	\todo Write the implementation of this class.
*/
class CMaskMap :
	public CSprite
{
public:
	CMaskMap(LPCSTR szName);
protected:
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CBackground
	\brief		Background sprites class.
	\author		Kronuz
	\version	1.0
	\date		April 15, 2003

	The CBackground class provides the functionality for all visible objects on the game,
	this includes a base to manage Entities

	\remarks
	This class, and its derivates (i.e. CEntity), are implemented to work as flyweight 
	objects, receiving any extrinsic state as a SDrawContext parameter for the
	IDrawableObject interface.

	\sa IDrawableObject, SDrawContext, CEntity and CMaskMap
	\todo Write the implementation of this class.
*/
class CBackground : 
	//public IDrawableObject,
	public CSprite
{
public:
	CBackground(LPCSTR szName);
protected:
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CEntity
	\brief		Entity sprites class.
	\author		Kronuz
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
};

/////////////////////////////////////////////////////////////////////////////
/*! \class	CSpriteContext
	\brief		Flyweight sprites context class.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	This class maintains the extrinsic status of the CSprite's flyweight objects.
	It is a concrete class based on the interface for flyweight drawable
	objects.

	\sa IDrawableObject
*/
class CSpriteContext :
	public CDrawableContext
{
public:
	void MoveTo(POINT NewPos) = 0; //!< Moves the sprite
	void SetStatus(DWORD Flags) = 0;
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CSpriteSheet
	\brief		CSpriteSheet class.
	\author		Kronuz
	\version	1.0
	\date		April 15, 2003

	This class keeps a list of pointers to all the sprites and a list of all 
	the animations that constitute the actual sprite sheet.\n
	This class does NOT give any information about the sprites in the map
	or its actual state.

	This class reads, keeps, and maintains a flyweight pool of sprites,
	each sprite shares a common state, but receives a context from the client 
	that needs to use it.

	\sa CAnimation and CSprite
	\todo Write the implementation of this class.
*/
class CSpriteSheet :
	public CNamedObj,
	public CConsole,
	public IDocumentObject
{
	friend CProjectManager;
protected:
	CProjectManager *m_pProjectManager;

	CFileName m_fnSheetFile;
	//CString m_sSheetName;

	CSimpleMap<CString, CSprite*> m_Sprites; //!< Flyweight pool of sprites.

	//! No one but the Project Manager is alowed to create Sprite Sheets
	CSpriteSheet(CProjectManager *pProjectManager);
	~CSpriteSheet();
public:
	CFileName& GetFileName() { return m_fnSheetFile; }
	CProjectManager* GetProjectManager() { return m_pProjectManager; }
};
