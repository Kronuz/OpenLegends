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
/*! \file		QuestDesigner.h 
	\brief		Include file containing all default stuff for the Quest Designer
	\date		April 15, 2003
*/

#pragma once

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////

#include "MainFrm.h"

#include "WorldEditorFrm.h"
#include "MapEditorFrm.h"
#include "SptShtEditorFrm.h"

#include "ScriptEditorFrm.h"

#include "../Net.h"

int CALLBACK Request(SOCKET s);
int CALLBACK Dispatch(SOCKET s);
bool Send(LPCSTR format, ...);
bool BeginSend();
bool EndSend();
