/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germ�n M�ndez Bravo (Kronuz)
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

#include <Core.h>
#include <FilePath.h>

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IArchive
	\brief		Interface for concrete implementation disk IO objects.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003
				July 12, 2005 + Added IsLoaded()

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
	virtual bool ReadObject(CVFile &vfFile) = 0;

	/*! \brief Writes the object to the disk.
	
		Writes the object to the disk or output device. This method takes all necesary
		information and attributes from the object and puts them in the file
		for latter load and use of the object.
		\sa Load()
	*/
	virtual bool WriteObject(CVFile &vfFile) = 0;

};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	CDocumentObject
	\brief		Interface for objects with on-disk read write operations.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	CDocumentObject allows the object to be saved or loaded from the 
	Archive interface. It is an abstract class that must be 
	implemented in any derivated class.
*/
interface CDocumentObject 
{
	/*! \brief Reads the object from the IArchive.
	
		Loads or reads the object using an Archive interface derivated object. 
		\sa Save()
	*/
	virtual bool Load(CVFile &vfFile) {
		bool bRet = true;
		ASSERT(m_ArchiveIn);
		if(m_bLoaded && !vfFile.IsEmpty()) return false;
		m_fnFile = vfFile;
		if((bRet=m_ArchiveIn->ReadObject(vfFile))) m_bLoaded = true;
		return bRet;
	}

	/*! \brief Saves the object to the IArchive.
	
		Saves or writes the object using an Archive interface derivated object. 
		\sa Load()
	*/
	virtual bool Save(CVFile &vfFile) {
		ASSERT(m_ArchiveOut);
		m_fnFile = vfFile;
		if(!m_bLoaded) return true;
		return m_ArchiveOut->WriteObject(vfFile);
	}
	/*! \brief Saves the object to the IArchive.
	
		Saves or writes the object using an Archive interface derivated object, to the 
		same file where it was loaded.

		\sa Load()
	*/
	virtual bool Save() {
		ASSERT(m_ArchiveOut);
		if(!m_bLoaded) return true;
		return m_ArchiveOut->WriteObject(m_fnFile);
	}

	virtual bool Close(bool bForce = false) {
		bool bRet = true;
		if(!m_bLoaded && !bForce) return true;
		if(IsModified() && !bForce) return false;
		if((bRet = _Close(bForce))) m_bLoaded = false;
		return bRet;
	}

	virtual bool Load(LPCSTR szFile = "") { return Load(CVFile(szFile)); }
	virtual bool Save(LPCSTR szFile) { return Save(CVFile(szFile)); }
	virtual bool IsLoaded() {return m_bLoaded;}

protected:
	virtual bool _Close(bool bForce) = 0; //!< Cleans the object when it's closed.

	bool m_bLoaded;
	bool m_hasChanged;
	IArchive *m_ArchiveIn;
	IArchive *m_ArchiveOut;
	CVFile m_fnFile;

	CDocumentObject() : m_ArchiveIn(NULL), m_ArchiveOut(NULL), m_hasChanged(false), m_bLoaded(false) {}
public:
	void Touch() { m_hasChanged = true; }
	bool IsModified() { return m_hasChanged; }
	const CVFile& GetFile() const { return m_fnFile; }
};

