
#pragma once

// Flags for the sprites and their transformations (higher byte of the status reserved):
#define SNORMAL				GFX_NORMAL	
#define SMIRRORED			GFX_MIRRORED
#define SFLIPPED			GFX_FLIPPED	
#define _SPT_TRANSFORM		0
#define SPT_TRANSFORM		((SNORMAL<<_SPT_TRANSFORM) | (SMIRRORED<<_SPT_TRANSFORM) | (SFLIPPED<<_SPT_TRANSFORM))

#define SNTILED				0x04
#define _SPT_INFO			0
#define SPT_INFO			(SNTILED<<_SPT_INFO)

#define SROTATE_0			GFX_ROTATE_0	
#define SROTATE_90			GFX_ROTATE_90
#define SROTATE_180			GFX_ROTATE_180
#define SROTATE_270			GFX_ROTATE_270
#define _SPT_ROT			4
#define SPT_ROT				((SROTATE_0<<_SPT_ROT) | (SROTATE_90<<_SPT_ROT) | (SROTATE_180<<_SPT_ROT) | (SROTATE_270<<_SPT_ROT))

#define SPT_ALPHA			0x00ff00
#define _SPT_ALPHA			8

// Flags for the drawable objects and their transformations:
#define DVISIBLE			0x01
#define _DRW_SHFT			24
#define DROTATE				((SROTATE_90<<_SPT_ROT))

enum CURSOR {
	eIDC_HAND			= 0x0001,	// cePanning
	eIDC_ARROW			= 0x0002,	// ceToSelect
	eIDC_CROSS			= 0x0004,
	eIDC_NO				= 0x0008,
	eIDC_SIZEALL		= 0x0010,	// ceMoving
	eIDC_SIZENESW		= 0x0020,	// ceResizing
	eIDC_SIZENS			= 0x0040,	// ceResizing
	eIDC_SIZENWSE		= 0x0080,	// ceResizing
	eIDC_SIZEWE			= 0x0100,	// ceResizing
	eIDC_ARROWADD		= 0x0200,	// ceToSelect
	eIDC_ARROWDEL		= 0x0400,	// ceToSelect
	eIDC_ARROWSELECT	= 0x0800,	// ceToSelect

	cePanning			= 0x0001,
	ceToMove			= 0x0010,
	ceToResize			= 0x01E0,
	ceToSelect			= 0x0E02
};
/////////////////////////////////////////////////////////////////////////////
// Forward declarations
struct SPropertyList;
struct SInfo;

/////////////////////////////////////////////////////////////////////////////
/*! \class	CNamedObj
	\brief		Class to manage the name of objects.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	Many classes are needed to maintain its name, and common ways to
	get and manage it in a simple way. This is a self contained class
	and no implementation is required in any derived class, but a
	call to its constructor in the base class own constructor passing 
	the name (or perhaps class) of the object being created.
*/
class CNamedObj 
{
	CBString m_sName;
protected:
	CNamedObj(LPCSTR szName) : m_sName(szName) { }
public:
	const CBString& GetName() const { return m_sName; }
	void SetName(LPCSTR szName)  { m_sName = szName; }
};

enum InfoType { itUnknown, itWorld, itMapGroup, itMap, itSpriteSheet, itSprite, itBackground, itMask, itEntity, itSpriteContext, itSound, itScript };
interface IPropertyEnabled
{
	virtual bool isFlagged() = 0;
	virtual void Flag(bool bFlag = true) = 0;
	virtual bool GetInfo(SInfo *pI) const = 0;
	virtual bool GetProperties(SPropertyList *pPL) const = 0;
	virtual bool SetProperties(SPropertyList &PL) = 0;
};

struct GameInfo;
typedef int CALLBACK FOREACHPROC(LPVOID Interface, LPARAM lParam);
typedef int CALLBACK STATUSCHANGEDPROC(GameInfo *NewStatus, LPARAM lParam);
