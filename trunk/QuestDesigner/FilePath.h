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
/*! \file		FilePath.h 
	\brief		Interface of the classes that maintain filenames and paths.
	\date		April 28, 2003

	This file implements the CFileName to handle filenames and paths. Also
	contains the path to the home directory of the game files.
*/

#pragma once
#include "interfaces.h"

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
