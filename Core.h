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

#pragma once

#include "../Version.h"

#pragma warning(disable : 4250) // ignore the C4355 warning: inheritance via dominance

#include <functional>
#include <vector>

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

// Deprecated, now use full ARGB values (Strider's idea):
//#define SPT_ALPHA			0x00ff00
//#define _SPT_ALPHA			8

// Flags for the drawable objects and their transformations:
#define DVISIBLE			0x01
#define DTEMP				0x02
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

////////////////////////////////////////////////////////////////////
// Files stuff:
#pragma pack(1)
// All Open Legends saved files must have this header:
struct _OpenLegendsFile {
	char ID[160];			// ID, Name and Description Separated by '\n' and ended by '\0'
	DWORD dwSignature;		// Signature of the file. (all OL signatures have 0x7a6f in the low word)
	DWORD dwSize;			// Size of the file (or memory)
	DWORD dwBitmapOffset;	// Where the thumbnail is (Start + Index Size + Data Size). 0 if no thumbnail.
	DWORD dwDataOffset;		// Where the data begins. (Start + Index Size). 0 if no data.
};
#pragma pack()

typedef _OpenLegendsFile OLFILE;
typedef _OpenLegendsFile *LPOLFILE;
typedef const _OpenLegendsFile *LPCOLFILE;

// List of valid file types, their IDs and Signatures:
const char OLF_ID[]			= OL_NAME;
const WORD OLF_SIGNATURE	= 0x4c4f; // OL

#define MAKEOLSIGN(w) (((DWORD)(w&0xffff)<<16) | OLF_SIGNATURE)
#define MAKEOLID(str) OL_NAME " " str

const DWORD OLF_SPRITE_SET_SIGNATURE	= MAKEOLSIGN(0xff01);
const DWORD OLF_MAP_GROUP_SIGNATURE		= MAKEOLSIGN(0xff02);
const DWORD OLF_WORLD_SIGNATURE			= MAKEOLSIGN(0xff03);
const DWORD OLF_SPRITE_SHEET_SIGNATURE	= MAKEOLSIGN(0xff04);

const char OLF_SPRITE_SET_ID[]			= MAKEOLID("Sprite Set");
const char OLF_MAP_GROUP_ID[]			= MAKEOLID("Map Group");
const char OLF_WORLD_ID[]				= MAKEOLID("World");
const char OLF_SPRITE_SHEET_GROUP_ID[]	= MAKEOLID("Sprite Sheet");

// verifies that the pointer passed as pOLFile is a valid Open Legends file in memory
inline bool VerifyOLFile(LPCOLFILE *ppOLFile)
{
	LPCOLFILE pOLFile = *ppOLFile;
	*ppOLFile = NULL;

	if(!pOLFile) return false;

	if(::IsBadReadPtr(pOLFile, sizeof(_OpenLegendsFile))) return false;
	if(strncmp(pOLFile->ID, OLF_ID, sizeof(OLF_ID)-1)) return false;
	if(LOWORD(pOLFile->dwSignature) != OLF_SIGNATURE) return false;

	// Do we have access to all of the memory stated in the file header:
	if(::IsBadReadPtr(pOLFile, pOLFile->dwSize)) return false;

	*ppOLFile = pOLFile;
	return true;
}
inline LPCSTR GetNameFromOLFile(LPCOLFILE pOLFile, LPSTR szBuffer, int nBuffSize)
{
	ASSERT(pOLFile);
	ASSERT(nBuffSize>0);

	if(::IsBadReadPtr(pOLFile, pOLFile->dwSize) || ::IsBadReadPtr(pOLFile, sizeof(_OpenLegendsFile))) {
		strncpy(szBuffer, "Corrupted file!", nBuffSize-1);
		szBuffer[nBuffSize-1] = '\0';
	}

	int nNameLen = 0;
	LPCSTR aux = strchr(pOLFile->ID, '\n');
	if(aux) {
		aux++;
		LPCSTR aux2 = strchr(aux, '\n');
		if(aux2) nNameLen = aux2 - aux;
		else nNameLen = (int)strlen(aux);

		nNameLen = min(nBuffSize-1, nNameLen);

		strncpy(szBuffer, aux, nNameLen);
	}
	szBuffer[nNameLen] = '\0';
	return szBuffer;
}
inline LPCSTR GetDescFromOLFile(LPCOLFILE pOLFile, LPSTR szBuffer, int nBuffSize)
{
	ASSERT(pOLFile);
	ASSERT(nBuffSize>0);

	if(::IsBadStringPtr(szBuffer, nBuffSize) || ::IsBadReadPtr(pOLFile, sizeof(_OpenLegendsFile))) {
		strncpy(szBuffer, "Corrupted file!", nBuffSize-1);
		szBuffer[nBuffSize-1] = '\0';
	}

	int nNameLen = 0;
	LPCSTR aux = strchr(pOLFile->ID, '\n');
	if(aux) {
		aux = strchr(aux+1, '\n');
		if(aux) {
			aux++;
			nNameLen = min(nBuffSize-1, (int)strlen(aux));
			strncpy(szBuffer, aux, nNameLen);
		}
	}
	szBuffer[nNameLen] = '\0';
	return szBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// Functor class to easily delete objects within containers:
struct ptr_delete {
	template<typename Type>
	void operator()(const Type *ptr) const {
		delete ptr;
		ptr = NULL;
	}
};
template<class Type>
struct ptr_equal_to : public std::binary_function<Type, Type, bool> 
{
	bool operator()(const Type& _Left, const Type& _Right) const {
		return(*_Left == *_Right);
	}
};

template <class Pair>
struct select1st : public std::unary_function<Pair, typename Pair::first_type> {
  const typename Pair::first_type& operator()(const Pair& x) const
  {
    return x.first;
  }
};

template <class Pair>
struct select2nd : public std::unary_function<Pair, typename Pair::second_type> {
  const typename Pair::second_type& operator()(const Pair& x) const
  {
    return x.second;
  }
};

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
struct SPropertyList;
struct SInfo;

/////////////////////////////////////////////////////////////////////////////
/*! \class	CNamedObj
	\brief		Class to manage the name of objects.
	\author		Germán Méndez Bravo (Kronuz)
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
protected:
	const struct NameEqual : 
	public std::binary_function<const CNamedObj*, CBString, bool> {
		inline bool operator()(const CNamedObj *a, const CBString &b) const {
			return(a->m_sName == b);
		}
	} m_equalName;

	friend NameEqual;

	CBString m_sName;
	CNamedObj(LPCSTR szName) : m_sName(szName) { }
public:
	const struct NameCompare : 
	public std::binary_function<const CNamedObj*, LPCSTR, bool> {
		bool operator()(const CNamedObj *a, LPCSTR b) const {
			return(a->m_sName == b);
		}
	};
	friend NameCompare;

	inline const CBString& GetName() const { return m_sName; }
	inline void SetName(LPCSTR szName) { m_sName = szName; }
};

template <class T> class CReferredObj
{
	std::vector<T**> m_References;
public:
	~CReferredObj() {
		while(m_References.size()) {
			std::vector<T**>::iterator Iterator = m_References.begin();
			ASSERT(*(*Iterator) == reinterpret_cast<T*>(this));
			*(*Iterator) = NULL;
			m_References.erase(Iterator);
		}/**/
	}
	void Ref(T** ref) {
		m_References.push_back(ref);
	}
	void UnRef(T** ref) {
		std::vector<T**>::iterator Iterator = std::find(m_References.begin(), m_References.end(), ref);
		m_References.erase(Iterator);
	}
};

/////////////////////////////////////////////////////////////////////////////
/*! \class	CMutable
	\brief		Class to manage the objects that change its internal state.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		July 15, 2005

	Many classes modify its internal state. This class provides the methods to
	keep track of the changes that an object has suffered as its life goes on.
*/
interface CMutable
{
	bool m_bChanged;	//!< Set if the object has chenged since the last call to Haschanged();
	bool m_bModified;	//!< Set if the object has been modified since the last call to WasSaved();
public:
	CMutable() : m_bModified(false), m_bChanged(false) {}
	virtual void Touch(bool bChange = true) { m_bModified = true; if(bChange) m_bChanged = true; }
	virtual void WasSaved() { m_bModified = false; }
	// returns whether or not the object has changed since last call to HasChanged()
	virtual bool HasChanged() { if(m_bChanged) { m_bChanged = false; return true; } return false; }
	// returns true if the object has been modified since the last save or its initial state.
	virtual bool IsModified() { return m_bModified; }
};

enum InfoType { itUnknown, itWorld, itMapGroup, itMap, itSpriteSheet, itSprite, itBackground, itMask, itEntity, itSpriteContext, itSound, itScript, itSpriteSet };
interface IPropertyEnabled
{
	// Multipurpose flag:
	virtual bool isFlagged() = 0;
	virtual void Flag(bool bFlag = true) = 0;

	// Get the Item's information:
	virtual bool GetInfo(SInfo *pI) const = 0;

	// Get/Set Item's Properties:
	virtual bool GetProperties(SPropertyList *pPL) const = 0;
	virtual bool SetProperties(SPropertyList &PL) = 0;

	// Commit/Begin changes:
	virtual void Commit() const = 0; // commit the changes

	// Cancel Changes since last call to Commit():
	virtual void Cancel() = 0; // cancel the changes
};

struct GameInfo;
typedef int CALLBACK SIMPLEPROC(LPVOID Interface, LPARAM lParam);
typedef int CALLBACK STATUSCHANGEDPROC(GameInfo *NewStatus, LPARAM lParam);
