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
/*! \file		ArchiveBinary.h
	\brief		Archive binary mode operations classes.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	ArchiveText.h provides the interfaces for reading and writing objects
	in text mode. Every object loaded or saved through this
	class is saved in the disk using a plain binary mode to accelerate
	saving/loading operations and save space.
	\sa Archiver.h, ArchiveText.h.
	\todo Write the implementation of this class, also write a new class to
		manage compressed binary mode, saving on-disk space and reducing
		the number of needed files to one (probably zipped).
*/
#pragma once

#include "Archiver.h"

class CSpriteSheetBinArch :
	public CSpriteSheetArch
{
	int ReadSprite(FILE *fInFile);
public:
	bool ReadObject(LPCSTR szFile);
	bool WriteObject(LPCSTR szFile);
};

class CProjectBinArch :
	public CProjectArch
{
public:
	CProjectBinArch(CProjectManager *pProjectManager) : CProjectArch(pProjectManager) {}
	
private:
public:
	bool ReadObject(LPCSTR szFile);
	bool WriteObject(LPCSTR szFile);
};
