/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. System_Failure 
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
/*! \file		GraphicsOGL.cpp
	\brief		Implementation of the OpenGL 2D Graphics plugin for Open Zelda.
	\date		Nov. 10, 2003: 
					* Initial Release.
				
	\author		System Failure
	\remarks	
				Starting this puppy. Got the framework layed out just gotta fill in the blanks ;)
				

			Known bugs:
				

*/

#include "stdafx.h"
#include "GraphicsOGL.h"

//////////////////////////////////////////////////////////////////////////////
// Interface Version Definition:
const WORD IGraphics::Version = 0x0011;//version 1.1, same for a while. hard to improve a good thing ;)

//////////////////////////////////////////////////////////////////////////////
// Needed Libraries:
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "GLAUX.lib")
#pragma comment(lib, "OPENGL32.lib")

//////////////////////////////////////////////////////////////////////////////
// Exported functions:

HRESULT QueryGfxInterface(WORD Version, IGraphics **pInterface, IConsole *Output)
{
	return S_OK;
}
// Release our Handle to the class
HRESULT ReleaseGfxInterface(IGraphics **pInterface)
{
	return S_OK;
}