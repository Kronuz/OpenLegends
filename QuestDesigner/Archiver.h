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
/*! \file		Archiver.h
	\brief		Archive interfaces.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	Archiver.h provides the abstract interfaces for reading and writing
	objects in any mode. 

	\sa ArchiveText.h, ArchiveBinary.h.
*/
#pragma once

#include "interfaces.h"

#include "Console.h"

class CMap;
class CProjectManager;
class CSpriteSheet;

//////////////////////////////////////////////////////////////
class CSpriteSheetArch :
	public IArchive,
	public CConsole
{
public:
	CSpriteSheetArch(CSpriteSheet *pSpriteSheet) : m_pSpriteSheet(pSpriteSheet) {}
protected:
	CSpriteSheet *m_pSpriteSheet;
public:
	bool ReadObject(LPCSTR szFile) = 0;
	bool WriteObject(LPCSTR szFile) = 0;
};

//////////////////////////////////////////////////////////////
class CProjectArch:
	public IArchive,
	public CConsole
{
public:
	CProjectArch(CProjectManager *pProjectManager) : m_pProjectManager(pProjectManager) {}
protected:
	CProjectManager *m_pProjectManager;
public:
	bool ReadObject(LPCSTR szFile) = 0;
	bool WriteObject(LPCSTR szFile) = 0;
};

//////////////////////////////////////////////////////////////
class CArchive:
	public IArchive,
	public CConsole
{
public:
	CArchive(void *pCaller) : m_pCaller(pCaller) {}
protected:
	void *m_pCaller;
public:
	bool ReadObject(LPCSTR szFile) = 0;
	bool WriteObject(LPCSTR szFile) = 0;
};
