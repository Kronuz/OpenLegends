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
/*! \file		FileManager.h 
	\brief		Interface of the classes that maintain file operations.
	\date		April 16, 2003

	Every single object, sound graphic, map, etc. has to go through this module,
	this module is designed to keep an easy reliable, robust and flexible
	way of saving files of all types. 
*/

#pragma once
#include "interfaces.h"

/////////////////////////////////////////////////////////////////////////////
/*! \class		CIOMode
	\brief		IO integrated modes class.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	CIOMode provides the implementation of the input output
	primitives for objects. Every object loaded or saved through this
	class is saved in the disk using a compressed binary mode, to save 
	on-disk space.

	\remarks This class unites the input/output modes and manages
	the current input/output mode. Modes are derivated from the following classes: 
		\li \c CIOTextMode (for plain text input/output)
		\li \c CIOBinaryMode (for plain binary input/output)
		\li \c CIOCompressedMode (for compressed binary input/output)

	\sa CIOTextMode, CIOBinaryMode and CIOCompressedMode.
	\todo Write the implementation of this class.
*/
class CIOMode
{
};
/////////////////////////////////////////////////////////////////////////////
/*! \class		CIOTextMode
	\brief		IO text mode operations class.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	CIOTextMode provides the implementation of the input output
	primitives for objects. Every object loaded or saved through this
	class is saved in the disk using a plain text mode. (This class uses
	the same saving format that Greg used)
	\sa CIOBinaryMode, CIOCompressedMode.
	\todo Write the implementation of this class.
*/
class CIOTextMode
{
public:
	int ReadToken() {

	}
};

/////////////////////////////////////////////////////////////////////////////
/*! \class		CIOBinaryMode
	\brief		IO binary mode operations class.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	CIOBinaryMode provides the implementation of the input output
	primitives for objects. Every object loaded or saved through this
	class is saved in the disk using a plain binary mode.
	\sa CIOTextMode, CIOCompressedMode.
	\todo Write the implementation of this class.
*/
class CIOBinaryMode
{
};
/////////////////////////////////////////////////////////////////////////////
/*! \class		CIOCompressedMode
	\brief		IO binary compressed mode operations class.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	CIOCompressedMode provides the implementation of the input output
	primitives for objects. Every object loaded or saved through this
	class is saved in the disk using a compressed binary mode, to save 
	on-disk space.
	\sa CIOTextMode, CIOBinaryMode.
	\todo Write the implementation of this class.
*/
class CIOCompressedMode
{
};


extern CString g_sHomeDir;
class CFileName {
	bool m_bRelative;
	CString m_sPath;
	CString m_sTitle;
	CString m_sExt;
public:
	CString GetFileTitle() { return m_sTitle; }
	CString GetFileName() { return m_sTitle + m_sExt; }
	CString GetPath() { 
		if(m_bRelative)
			return g_sHomeDir + m_sPath; 
		return m_sPath;
	}
	CString GetFilePath() { 
		if(m_bRelative)
			return g_sHomeDir + m_sPath + m_sTitle + m_sExt;
		return m_sPath + m_sTitle + m_sExt;
	}
	void SetFileTitle(LPCSTR szNewName) {
		m_sTitle = szNewName;
	}
	void SetFilePath(LPCSTR szNewName) {
		char szDrive[_MAX_DRIVE];
		char szPath[_MAX_DIR];
		char szTitle[_MAX_FNAME];
		char szExt[_MAX_EXT];

		_splitpath(szNewName, szDrive, szPath, szTitle, szExt);
		CString sPath = szDrive;
		sPath += szPath;

		if(sPath.Find(g_sHomeDir)) {
			m_sPath = sPath.Mid(g_sHomeDir.GetLength());
			m_bRelative = true;
		} else {
			m_sPath = sPath;
			m_bRelative = false;
		}
		m_sTitle = szTitle;
		m_sExt = szExt;
	}
	bool FileExists()
	{
		return (PathFileExistsA(GetFilePath()))?true:false;
	}
};
