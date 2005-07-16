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
/////////////////////////////////////////////////////////////////////////////
/*! \file		ArchiveBinary.h
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Archive binary mode operations classes.
	\version	1.0
	\date		April 28, 2003:
						* Creation date.
				July 11, 2005 by Kronuz:
						+ Added the CMapGroupBinArch class

	ArchiveText.h provides the interfaces for reading and writing objects
	in text mode. Every object loaded or saved through this
	class is saved in the disk using a plain binary mode to accelerate
	saving/loading operations and save space.
	\sa ArchiveText.h.
	\todo Write the implementation of this class, also write a new class to
		manage compressed binary mode, saving on-disk space and reducing
		the number of needed files to one (probably zipped).
*/
#pragma once

#include <IArchiver.h>
#include <IConsole.h>

class CLayer;
class CMapGroup;
class CGameManager;
class CSpriteSheet;
class CWorld;

class CSpriteSheetBinArch :
	public IArchive
{
public:
	CSpriteSheetBinArch(CSpriteSheet *pSpriteSheet) : m_pSpriteSheet(pSpriteSheet) {}
private:
	CSpriteSheet *m_pSpriteSheet;
public:
	bool ReadObject(CVFile &vfFile);
	bool WriteObject(CVFile &vfFile);
	bool CloseObject(CVFile &vfFile, bool bForce);
};

class CProjectBinArch :
	public IArchive
{
public:
	CProjectBinArch(CGameManager *pGameManager) : m_pGameManager(pGameManager) {}
private:
	CGameManager *m_pGameManager;
public:
	bool ReadObject(CVFile &vfFile);
	bool WriteObject(CVFile &vfFile);
	bool CloseObject(CVFile &vfFile, bool bForce);
};

class CMapGroupBinArch :
	public IArchive
{
public:
	CMapGroupBinArch(CMapGroup *pMapGroup) : m_pMapGroup(pMapGroup) {}
private:
	int m_nLines;
	CMapGroup *m_pMapGroup;
public:
	bool ReadObject(CVFile &vfFile);
	bool WriteObject(CVFile &vfFile);
	bool CloseObject(CVFile &vfFile, bool bForce);
};

class CMapBinArch :
	public IArchive
{
public:
	CMapBinArch(CLayer *pLayer) : m_pLayer(pLayer) {}
private:
	CLayer *m_pLayer;
public:
	bool ReadObject(CVFile &vfFile);
	bool WriteObject(CVFile &vfFile);
	bool CloseObject(CVFile &vfFile, bool bForce);
};

class CWorldBinArch :
	public IArchive
{
public:
	CWorldBinArch(CWorld *pWorld) : m_pWorld(pWorld) {}
private:
	CWorld *m_pWorld;
public:
	bool ReadObject(CVFile &vfFile);
	bool WriteObject(CVFile &vfFile);
	bool CloseObject(CVFile &vfFile, bool bForce);
};

