/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germán Méndez Bravo)
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
/*! \if	FullDocs
	\file		Script.cpp
	\brief		Main Open Zelda core natives for the scripts
	\date		August 10, 2003

	Improvements in the scripting engine since last Open Zelda version:
	[08/10/03] - The Abstract Virtual Machine now uses assembler code, so 
				 now the entities run about five times faster than they used
				 to run in Open Zelda 6.2.4.
	[08/10/03] - The core natives now include the stricmp() function to compare
				 case insensitive strings.
	[08/09/03] - The floating point native functions now have full support
				 for all trigonometric functions, pow(), abs(), fmod(), etc.
	 
	\todo 
		The abstract machine should be thread safe, and each entity run
		in a separate thread for a maximum amount of time before killing it and
		issuing a warning (defend against infinite loops and stuff)

	\endif

	\todo
		Check the strcpy(), strcat() functions in the core natives,
		LittleBuddy has said these have some problems some times.
*/

/*!
	\defgroup openzelda Open Zelda API functions
	This is the API for Open Zelda scripts.
*/
/*!
	\defgroup core Small's Core Functions
	\ingroup openzelda
	Core native functions for the Small language.
*/
/*! 
	\defgroup float Floating-point related Functions
	\ingroup openzelda
	Float tag type native functions.
*/
/*!
	\defgroup general General Open Zelda Functions
	\ingroup openzelda
	Main Open Zelda core functions.
*/

#include "stdafx.h"

#include "Script.h"

#include "GameManager.h"

extern "C" AMX_NATIVE_INFO console_Natives[];
extern "C" AMX_NATIVE_INFO core_Natives[];
extern "C" AMX_NATIVE_INFO float_Natives[];

extern cell ConvertFloatToCell(float fValue);
extern float fConvertCellToFloat(cell cellValue);



#if defined OZDoc
float GetTimeDelta(); /*!< 
	\ingroup general
	\brief This function obtains the current delta time for the frame being rendered.

	\return Delta time passed since last frame (in seconds)

	\remarks Each frame a delta time is calculated to do all the needed operations.
		This delta is the lapse of time that has been passed since the last
		frame was rendered. Delta is calculated as follows: MillisecondsSinceLastFrame / 1000
		Although this is not always entirely true, since when the delta time comes to be
		higher than 0.05 (i.e. more than 50 milliseconds since have been past since last frame) 
		the delta time remains at 0.05 thus slowing the whole game down, instead of dropping frames.

		To use this function you should include both general and float header files.

	<b>Example:</b>
	\code
	...
	new float: timer;
	timer += GetTimeDelta();          
	...
	\endcode
*/
#endif
static cell AMX_NATIVE_CALL GetTimeDelta(AMX *amx, cell *params)
{
	return ConvertFloatToCell(CGameManager::GetFPSDelta());
}
//-----------------------------------------------------------------------------
// Name: FirstRun()
//-----------------------------------------------------------------------------
static cell AMX_NATIVE_CALL FirstRun(AMX *amx, cell *params)
{
	if(amx) return amx->firstrun;
	
	return 0;	
}

// Define a List of native General functions (Open Zelda core natives)
extern AMX_NATIVE_INFO general_Natives[] = {
	{ "GetTimeDelta",  GetTimeDelta },
	{ "FirstRun",  FirstRun},
	{ NULL, NULL }        /* terminator */
};

//-----------------------------------------------------------------------------
// Name: RegisterNatives()
// Desc: 
// 
//-----------------------------------------------------------------------------
void RegisterNatives(AMX *amx)
{
	// Register all the native functions
//	amx_Register(amx, entity_Natives, -1);
	amx_Register(amx, general_Natives, -1);
	amx_Register(amx, core_Natives, -1);
	amx_Register(amx, console_Natives, -1);
	amx_Register(amx, float_Natives, -1);
//	amx_Register(amx, animation_Natives, -1);
//	amx_Register(amx, counter_Natives, -1);
}

