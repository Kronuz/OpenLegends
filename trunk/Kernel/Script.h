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
/*! \file		Script.h 
	\author		Germ�n M�ndez Bravo (Kronuz)
	\brief		Main Open Legends core natives for the scripts
	\date		August 10, 2003
				February 12, 2006:
					* Added a few helper functions for retrival of sprite data.
				Feb 13, 2006:
					+ Optimized retrieval of contexts (avoids searching full database, indexes used names.)
*/

#pragma once

#include "WorldManager.h"
#include "ScriptManager.h"

#include <IArchiver.h>
#include <IGame.h>
#include <amx.h>

struct EntParamData{
	LPCSTR szName;
	CDrawableContext *context;
};

#define SPECIALENTITYWORLD "_world"
#define SPECIALENTITYGROUP "_group"

void RegisterNatives(AMX *amx);

void TestSave();

namespace Scripts{
	bool GetStringParam(AMX *amx, cell sParam, char* szString);	
	CSpriteContext* GetContext(AMX *amx, cell param, bool extensive = true, bool converted = false);
	void InitializeSpecialEntities(LPCSTR Groupname);
	int CALLBACK FindNamedEntity(LPVOID lpVoid, LPARAM ret);

	inline HSCRIPT GetThis(AMX *amx){							//Abstract entities can never use this.
		HSCRIPT *pScript;
		amx_GetUserData(amx, HSCRIPTPOINTER, (void **)&pScript);
		return *pScript;
	}

	inline CEntityData* GetEntityData(HSCRIPT hScript){			//Abstract entities can never use this.
		return static_cast<CSpriteContext *>((CDrawableContext *)(hScript->ID))->m_pEntityData;
	}
	inline CSpriteContext* GetSpriteContextByHandle(HSCRIPT hScript){	//Abstract entities can never use this.
		return static_cast<CSpriteContext *>((CDrawableContext *)(hScript->ID));
	}
	inline bool GetStringParam(AMX *amx, cell sParam, char* szString){
		cell *n;

		amx_GetAddr(amx, sParam, &n);
		amx_GetString(szString, n);	

		return true;
	}

	inline CEntityData* GetRelevantEntityData(AMX *amx, cell param, bool converted = false){
		//Note to self: The below function hasn't been tested with live variables yet, should be done asap.
		return GetContext(amx, param, true, converted)->m_pEntityData;
	}
}