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
/*! \file		interfaces.h 
	\brief		Interfaces and abstract classes used by the core of OZ.
	\date		April 16, 2003

	This file defines the interfaces needed to manage objects with names,
	flyweight drawable objects, document management (file saving and loading), 
	and scriptable objects.
*/

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
interface IDrawableObject;

/////////////////////////////////////////////////////////////////////////////
/*! \class	CNamedObj
	\brief		Interface to manage the name of the objects.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	Many classes are needed to maintain its name, and common ways to
	get and manage it in a simple way. This is an self contained class
	and no implementation is required in any derivated class, but a
	call to its constructor in the base class own constructor passing 
	the name (or perhaps class) of the object being created.
*/
class CNamedObj 
{
	CString m_sName;
protected:
	CNamedObj(LPCSTR szName) : m_sName(szName) { }
public:
	CString& GetName() { return m_sName; }
	void SetName(LPCSTR szName)  { m_sName = szName; }
};

/////////////////////////////////////////////////////////////////////////////
/*! \class	CDrawableContext
	\brief		Flyweight drawable-objects context class.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	This class maintains the extrinsic status of flyweight drawable-objects. 
	It is an abstract class that must be implemented in any derivated class.

	\sa IDrawableObject
*/
class CDrawableContext
{
protected:
	CPoint m_Position;
	IDrawableObject *m_pDrawableObj;

	CDrawableContext() : m_pDrawableObj(NULL) {}
public:
	virtual ~CDrawableContext();

	virtual void MoveTo(POINT NewPos) = 0; //!< Moves the sprite
	virtual void SetStatus(DWORD Flags) = 0;

	void SetDrawableObj(IDrawableObject *pDrawableObj) { m_pDrawableObj = pDrawableObj; }
	IDrawableObject* GetDrawableObj() { return m_pDrawableObj; }

};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IDrawableObject
	\brief		Interface for drawable objects.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	IDrawableObject is used to provide an interface that allows a flyweight
	object to be drawn. It is an abstract class that must be implemented in any
	derivated class, and must receive a drawing context containing the extrinsic
	data to the flyweight object.

	\sa SDrawContext
*/
interface IDrawableObject 
{
	/*! \brief Draw the object on the screen.
	
		Draw the object to the screen in its current state
		returns true if the object was drawn, or false if it
		was not drawn (this is not necessarily an error - the
		object may have determinated that it was obscured by others)
	*/
	virtual bool Draw(CDrawableContext&) = 0;

	/*! \brief Determinates if an object needs to be drawn.
	
		Returns false if the object does not need to be drawn (i.e. off screen)
	*/
	virtual bool NeedToDraw(CDrawableContext&) = 0;

	virtual bool IsAt(RECT, CDrawableContext&) = 0;
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IScriptableObject
	\brief		Interface for scriptable objects.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	IScriptableObject allows the object to be driven by by a script. It provides a
	default implementation for the ExecuteScript member which logs the 
	unexecuted script directly to a logging file.
*/
interface IScriptableObject 
{
	/*! \brief Enumaration of all sprite types.
	
		Override this and handle object specific scripts here. If your class
		handless the script then return true. else delegate the script to
		the base class, returning the result.
	*/
	virtual bool ExecuteScript(CString sCommand);
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IArchive
	\brief		Interface for concrete implementation disk IO objects.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	IArchive Interface to implement disk read/write operations of specific
	objects. It is an abstract class that must be implemented in any
	derivated class.
*/
interface IArchive
{
	/*! \brief Reads the object from the disk.
	
		Reads the object from the disk or input device. This method fills all necesary
		information and attributes from the object in a file
		\sa Save()
	*/
	virtual bool ReadObject(LPCSTR szFile) = 0;

	/*! \brief Writes the object to the disk.
	
		Writes the object to the disk or output device. This method takes all necesary
		information and attributes from the object and puts them in the file
		for latter load and use of the object.
		\sa Load()
	*/
	virtual bool WriteObject(LPCSTR szFile) = 0;

};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IDocumentObject
	\brief		Interface for objects with on-disk read write operations.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	IDocumentObject allows the object to be saved or loaded from the 
	Archive interface. It is an abstract class that must be 
	implemented in any derivated class.
*/
interface IDocumentObject 
{
	/*! \brief Reads the object from the IArchive.
	
		Loads or reads the object using an Archive interface derivated object. 
		\sa Save()
	*/
	virtual bool Load(LPCSTR szFile) {
		ATLASSERT(m_ArchiveIn);
		return m_ArchiveIn->ReadObject(szFile);
	}

	/*! \brief Saves the object to the IArchive.
	
		Saves or writes the object using an Archive interface derivated object. 
		\sa Load()
	*/
	virtual bool Save(LPCSTR szFile) {
		ATLASSERT(m_ArchiveOut);
		return m_ArchiveOut->WriteObject(szFile);
	}

protected:
	IDocumentObject() : m_ArchiveIn(NULL), m_ArchiveOut(NULL) {}
	IArchive *m_ArchiveIn;
	IArchive *m_ArchiveOut;
};
