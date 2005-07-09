/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		ArchiveText.h
	\brief		Archive text mode operations classes.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	ArchiveText.h provides the interfaces for reading and writing objects
	in text mode. Every object loaded or saved through this
	class is saved in the disk using a plain text mode. (This class uses
	the same saving format that Greg Denness used)
	\sa ArchiveBinary.h.
	\todo Complete the implementation of this class.
*/

#pragma once

#include <IArchiver.h>
#include <IConsole.h>

class CLayer;
class CGameManager;
class CSpriteSheet;
class CMapGroup;
class CWorld;

class CSpriteSheetTxtArch :
	public IArchive
{
public:
	CSpriteSheetTxtArch(CSpriteSheet *pSpriteSheet) : m_pSpriteSheet(pSpriteSheet) {}
private:
	CSpriteSheet *m_pSpriteSheet;
	int m_nLines;
	int m_nCatalog;

	int ReadSprite(CVFile &vfFile);
public:
	bool ReadObject(CVFile &vfFile);
	bool WriteObject(CVFile &vfFile);
};

class CProjectTxtArch :
	public IArchive
{
public:
	CProjectTxtArch(CGameManager *pGameManager) : m_pGameManager(pGameManager) {}
private:
	CGameManager *m_pGameManager;
public:
	bool ReadObject(CVFile &vfFile);
	bool WriteObject(CVFile &vfFile);
};

class CMapTxtArch :
	public IArchive
{
public:
	CMapTxtArch(CLayer *pLayer) : m_pLayer(pLayer) {}
private:
	int m_nLines;
	CLayer *m_pLayer;

	int ReadSprite(CVFile &vfFile);
	int ReadTile(CVFile &vfFile);
public:
	bool ReadObject(CVFile &vfFile);
	bool WriteObject(CVFile &vfFile);
};

class CWorldTxtArch :
	public IArchive
{
public:
	CWorldTxtArch(CWorld *pWorld) : m_pWorld(pWorld) {}
private:
	int m_nLines;
	CWorld *m_pWorld;

	bool ReadMaps(CVFile &vfFile);
	bool ReadMapGroups(CVFile &vfFile);
	bool ReadProperties(CVFile &vfFile);
	bool LoadThumbnail(CMapGroup *pMapGroup);
public:
	bool ReadObject(CVFile &vfFile);
	bool WriteObject(CVFile &vfFile);
};

