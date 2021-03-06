/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germ�n M�ndez Bravo (Kronuz)
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
/*! \file		IdleHandlerPump.h 
	\author		Germ�n M�ndez Bravo (Kronuz)
	\brief		Implementation of the CIdleHandlerPump class.
	\date		April 15, 2003
*/

#pragma once

class CIdleHandlerPump :
	public CIdleHandler
{
public:
	void PumpIdleMessages() {
		// pump idle messages to update window
		CMessageLoop *pLoop = _Module.GetMessageLoop();
		ATLASSERT(NULL!=pLoop);
		pLoop->OnIdle(0);
	}
};
