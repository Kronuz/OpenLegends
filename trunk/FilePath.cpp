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
/*! \file		FilePath.cpp
	\brief		Implementation of the file manager classes.
	\date		April 28, 2003

	This file implements the CVFile to handle filenames, paths, and the virtual 
	file system. Also contains the path to the home directory of the game files.
*/

#include "stdafx.h"
#include "FilePath.h"

CBString g_sHomeDir;

CVFile::CVFile() : 
	m_vFile(NULL), 
	m_File(NULL), 
	m_nExists(0), 
	m_bOpenFile(false), 
	m_bRelative(false), 
	m_bVirtual(false) 
{
}
CVFile::CVFile(LPCSTR szNewName) : 
	m_vFile(NULL), 
	m_File(NULL), 
	m_nExists(0), 
	m_bOpenFile(false), 
	m_bRelative(false), 
	m_bVirtual(false) 
{
	SetFilePath(szNewName);
}
CVFile::~CVFile() 
{ 
	if(m_vFile) unzClose(m_vFile); m_vFile = NULL; 
	if(m_File) fclose(m_File); m_File = NULL;
}
