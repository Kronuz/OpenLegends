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
/*! \file		Interfaces.h 
	\brief		Interfaces and abstract classes used by the core of OZ.
	\date		April 16, 2003

	This file defines the interfaces needed to manage objects with names,
	flyweight drawable objects, document management (file saving and loading), 
	and scriptable objects.
*/

#pragma once

#include "../IGraphics.h"
#include "../IGame.h"
#include "../IConsole.h"
#include "../IArchiver.h"

#include "../Core.h"
#include "../Misc.h"

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IScriptableObject
	\brief		Interface for scriptable objects.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	IScriptableObject allows the object to be driven by by a script. It provides a
	default implementation for the ExecuteScript member which logs the 
	unexecuted script directly to a logging file.
*/
interface IScriptableObject 
{
	/*! \brief Enumaration of all sprite types.
	
		Override this and handle object specific scripts here. If your class
		handless the script then return true. else delegate the script to
		the base class, returning the result.
	*/
	virtual bool ExecuteScript(CBString sCommand);
};
