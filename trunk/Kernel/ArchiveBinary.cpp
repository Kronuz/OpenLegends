/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germ�n M�ndez Bravo)
   Copyright (C) 2001-2003. Open Zelda's Project
 
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

#include "stdafx.h"
#include "ArchiveBinary.h"

#include "GameManager.h"
#include "SpriteManager.h"
#include "WorldManager.h"

bool CSpriteSheetBinArch::ReadObject(CVFile &vfFile)
{
	return false;
}
bool CSpriteSheetBinArch::WriteObject(CVFile &vfFile)
{
	return false;
}

bool CProjectBinArch::ReadObject(CVFile &vfFile)
{
	return false;
}
bool CProjectBinArch::WriteObject(CVFile &vfFile)
{
	return false;
}
