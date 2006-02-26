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
/*! \if	FullDocs
	\file		Script.cpp
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Main Open Legends core natives for the scripts
	\date		August 10, 2003
		- 10/11/05 Modified comments to match the new functions listed in OLScripts/functions.rtf
				February 12, 2006:
					~ Some work with the debug function, it's printing some debug-data to a text file for now (SLOW!)
					* New functions have been implemented for context-based retrieval of entities ("this", CDrawableContext* or "name")
				Feb 13, 2006:
					+ Optimized retrieval of contexts (avoids searching full database, indexes used names.)
				Feb 14, 2006:
					+ The file has been re-ordered a bit for readability.
					* The DrawSprite() function has been added and tested. some features are yet to be implemented in it however. (noted.)
	Improvements in the scripting engine since last Open Legends version:
	[xx/02/06] - Rewritten script threading, should be a lot faster now. - Littlebuddy
	[08/10/03] - The Abstract Virtual Machine now uses assembler code, so 
				 now the entities run about five times faster than they used
				 to run in Open Legends 6.2.4.
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
		Considering the system listed in OLScripts/functions.rtf, do we really
		need SetFilter? This should logically be stored inside a local Get/Set 
		variable that refers to the current group.

	\todo
		Is the console group obsolete? It'd be more logical to implement it 
		inside the core group, if so, it should be removed.

	\todo
		The new groups need implementation. These are:
			core, float, general, entity, data and drawing (and mplayer).
		Core and float handles the same thing as in OpenZelda, that is core 
		small functions and mathematical operations.
		General will handle key input, saving/loading, sounds, 
		draw-location and Wipes.
		Entity will handle collision detection, entity positioning, 
		inter-entity messaging and general entity handling.
		Data will handle the data storage functions for entities, animations, 
		sprites and the world and maps.
		Drawing handles animations, sprites, text and images.
		(Mplayer is a sketch-group and will handle network-interaction 
		in the future.)


*/
#include "stdafx.h"
#include <amx.h>

#include "Script.h"
#include <Interfaces.h>

#include "GameManager.h"
#include "ScriptManager.h"
#include "SpriteManager.h"

using namespace Scripts;

extern cell ConvertFloatToCell(float fValue);
extern float fConvertCellToFloat(cell cellValue);

/*!
	\defgroup openlegends Open Legends API functions
	This is the API for Open Legends scripts.
*/
int CALLBACK Scripts::FindNamedEntity(LPVOID lpVoid, LPARAM ret){	//Callback for extensive entity search.
	CMapGroup *pMapGroup = (CMapGroup *)lpVoid;
	for(int i=0;pMapGroup->GetChild(i) != NULL; i++){
		//Search each sprite context in each layer in each group. YUCK!
		((EntParamData *)ret)->context = pMapGroup->GetChild(i)->GetChild(((EntParamData *)ret)->szName);
		if( ((EntParamData *)ret)->context != NULL ) return -1; //Not an error, we're done tho.
	}
	return 0;
}

CSpriteContext* Scripts::GetContext(AMX *amx, cell param, bool extensive, bool converted){
		char szName[256];
		
		if(converted) strcpy(szName, (char *)param);
		else GetStringParam(amx, param, szName);

		if(!strcmp("this", szName)) return GetSpriteContextByHandle(GetThis(amx));

		CDrawableContext *context = CEntityData::FindContext(szName);
		if(context != NULL) return static_cast<CSpriteContext *>(context);
		
	
		if(!extensive) return NULL;
		/*Find the entity through an extended search - THIS IS NOT RECOMMENDED! Avoid! Avoid! Avoid!*/
		//I doubt this location will be reached anymore.
		EntParamData foreachdata;
		foreachdata.context = NULL;
		foreachdata.szName = szName;
		CGameManager::Instance()->ForEachMapGroup(FindNamedEntity, (LPARAM)&foreachdata);
		CEntityData::InsertContext(szName, foreachdata.context);	//Any previously unused context is inserted for faster searching later on.
		return static_cast<CSpriteContext *>(foreachdata.context);

}

void Scripts::InitializeSpecialEntities(LPCSTR Groupname){	//Called per frame when a new queue is created to make sure they exist.
	if(GetContext(NULL, (cell)SPECIALENTITYWORLD, false, true) == NULL){//It's added to CEntityData on creation, so do a quicksearch.
		CGameManager::Instance()->CreateEntity(SPECIALENTITYWORLD, SPECIALENTITYWORLD);
		if(GetContext(NULL, (cell)SPECIALENTITYWORLD, false, true) == NULL){
			CONSOLE_PRINTF("Scrip Error: The _world entity script could not be created!");
		}
	}
	if(GetContext(NULL, (cell)SPECIALENTITYGROUP, false, true) == NULL){
		CGameManager::Instance()->CreateEntity(SPECIALENTITYGROUP, Groupname);
		if(GetContext(NULL, (cell)SPECIALENTITYGROUP, false, true) == NULL){
			CONSOLE_PRINTF("Script Warning: The group script for Group %s could not be created.", Groupname);
		}
		
	}	
}
/*!
	\defgroup core Small's Core Functions
	\ingroup openlegends
	Core native functions for the Small language.
*/

extern AMX_NATIVE_INFO core_Natives[] = {
	{NULL, NULL}
};
/*! 
	\defgroup float Floating-point related Functions
	\ingroup openlegends
	Float tag type native functions.
*/
extern AMX_NATIVE_INFO float_Natives[] = {
	{NULL, NULL}
};

/*!
	\defgroup general General Open Legends Functions
	\ingroup openlegends
	Main Open Legends core functions.
*/
static cell AMX_NATIVE_CALL GetGroupWidth(AMX *amx, cell *params){
	CMapGroup *group = CGameManager::Instance()->GetActiveMapGroup();
	CSize size;
	group->GetSize(size);
	return size.cx;
}
static cell AMX_NATIVE_CALL GetGroupHeight(AMX *amx, cell *params){
	CMapGroup *group = CGameManager::Instance()->GetActiveMapGroup();
	CSize size;
	group->GetSize(size);
	return size.cy;
}
static cell AMX_NATIVE_CALL GetInput(AMX *amx, cell *params){
	return CGameManager::Instance()->GetInput(params[1]);
}
static cell AMX_NATIVE_CALL GetMouseX(AMX *amx, cell *params){
	return CGameManager::Instance()->GetMousePos()->x;
}
static cell AMX_NATIVE_CALL GetMouseY(AMX *amx, cell *params){
	return CGameManager::Instance()->GetMousePos()->y;
}
static cell AMX_NATIVE_CALL GetMouseKey(AMX *amx, cell *params){
	return CGameManager::Instance()->GetMouseKey();
}
static cell AMX_NATIVE_CALL GetTimeDelta(AMX *amx, cell *params)
{
	return ConvertFloatToCell(CGameManager::GetFPSDelta());
}

static cell AMX_NATIVE_CALL UpdateWorldCo(AMX *amx, cell *params)
{
	CGameManager::Instance()->UpdateWorldCo(params[1], params[2]);
	
	//Special case entity, it will always exist.
	char World[] = SPECIALENTITYWORLD;
	CEntityData *world = Scripts::GetRelevantEntityData(NULL, (cell)World, true);
	world->SetValue("_x", params[1]);
	world->SetValue("_y", params[2]);
	
	return 0;
}
static cell AMX_NATIVE_CALL SetFilter(AMX *amx, cell *params)
{
	GpxFilters eFilter = (GpxFilters)params[1];
	void *vParam = NULL;
	ARGBCOLOR rgbColor;
	int nAux;
	float fAux;

	cell *args[10];
	// get the number of parameters:
	int num = min((int)(params[0]/sizeof(cell))-1, 10);
	for(int i=0; i<num; i++) {
		amx_GetAddr(amx, params[i+2], &args[i]);
	}

	switch(eFilter) {
		case EnableFilters:
		case ClearFilters:
			if(num != 0) return 1;
			break;
		case Pixelate:
			if(num != 1) return 1;
			fAux = fConvertCellToFloat(*args[0]);
			vParam = &fAux;
			break;
		case Alpha:
			if(num != 3) return 1;
			rgbColor = COLOR_RGB((int)*args[1], (int)*args[2], (int)*args[3]);
			vParam = &rgbColor;
			break;
		case Colorize:
			if(num != 4) return 1;
			rgbColor = COLOR_ARGB((int)*args[4], (int)*args[1], (int)*args[2], (int)*args[3]);
			vParam = &rgbColor;
			break;
		case VertMove:
		case HorzMove:
			if(num != 1) return 1;
			nAux = (int)*args[0];
			vParam = &nAux;
			break;
		default: return 1;
	}

	CGameManager::SetFilter(eFilter, vParam);
	return 0;
}

static cell AMX_NATIVE_CALL FirstRun(AMX *amx, cell *params)
{
	if(amx) return amx->firstrun;
	
	return 0;	
}
static cell AMX_NATIVE_CALL Wipe(AMX *amx, cell *params){
	int dir = params[1];
	return CGameManager::Instance()->Wipe(dir, GetContext(amx, params[2]), params[3], params[4]);
}
static cell AMX_NATIVE_CALL DebuggingStuff(AMX *amx, cell *params){
	CEntityData *d = GetRelevantEntityData(amx, (cell)"this", true);
	int x = d->GetValue("_x");
	int y = d->GetValue("_y");
	if(CGameManager::Instance()->GetInput(KEY_LEFT)){
		d->SetValue("_x", x-3);
		x = d->GetValue("_x");
	}
	if(CGameManager::Instance()->GetInput(KEY_RIGHT)){
		d->SetValue("_x", x+3);
		x = d->GetValue("_x");
	}
	if(CGameManager::Instance()->GetInput(KEY_UP)){
		d->SetValue("_y", y-3);
		y = d->GetValue("_y");
	}
	if(CGameManager::Instance()->GetInput(KEY_DOWN)){
		d->SetValue("_y", y+3);
		y = d->GetValue("_y");
	}
	
	CDrawableContext *transfer = GetContext(amx, (cell)"this", true, true);
	if(x > GetGroupWidth(NULL, NULL)) CGameManager::Instance()->Wipe(3, transfer, 64, 64);
	else if(x < 0) CGameManager::Instance()->Wipe(1, transfer, 64, 64);
	else if(y > GetGroupHeight(NULL, NULL)) CGameManager::Instance()->Wipe(2, transfer, 64, 64);
	else if(y < 0) CGameManager::Instance()->Wipe(0, transfer, 64, 64);
	char sprite[] = "__pstn1";
	char color[] = "FF808080";
	if(CGameManager::Instance()->GetMouseKey()){
		CGameManager::Instance()->DrawSprite(sprite, 0, (int)GetMouseX(NULL,NULL), (int)GetMouseY(NULL,NULL), 2, 4, color, 1.0f, 0);
	}
	CGameManager::Instance()->DrawSprite(sprite, 0, x, y, 2, 4, color, 1.0f, 0);
	cell t[3];
	t[1] = x;
	t[2] = y;
	//d = GetRelevantEntityData(amx, (cell)SPECIALENTITYWORLD, true);
	//d->SetValue("_x", x);
	//d->SetValue("_y", y);
	UpdateWorldCo(NULL, t);

	return 0;
}

extern AMX_NATIVE_INFO general_Natives[] = {
	{ "GetGroupHeight"	, GetGroupHeight },
	{ "GetGroupWidth"	, GetGroupWidth },
	{ "UpdateWorldCo"	, UpdateWorldCo },
	{ "GetTimeDelta"	, GetTimeDelta },
	{ "SetFilter"		, SetFilter },
	{ "FirstRun"		, FirstRun },
	{ "DebuggingStuff"	, DebuggingStuff },
	{ "Wipe"			, Wipe },
	{ "GetMouseKey"		, GetMouseKey },
	{ "GetMouseX"		, GetMouseX },
	{ "GetMouseY"		, GetMouseY },
	{ "GetInput"		, GetInput },
	{ NULL				, NULL }        /* terminator */
};

/*!
	\defgroup entity Open Legends Entity Handling Functions
	\ingroup openlegends
	Open Legends entity handling functions.
*/
static cell AMX_NATIVE_CALL CreateEntity(AMX *amx, cell *params){
	char szName[256];
	char szScript[256];
	GetStringParam(amx, params[1], &szScript[0]);
	GetStringParam(amx, params[2], &szName[0]);
	CBString string = CGameManager::Instance()->CreateEntity(szName, szScript);
	if(strlen(string) < 1) return 0;
	strcpy(szName, string);
	amx_SetString(&params[2], szName, false);
	return 1;
}
extern AMX_NATIVE_INFO entity_Natives[] = {
	{"CreateEntity", CreateEntity},
	{NULL, NULL}
};
/*!
	\defgroup data Open Legends Data Storage Functions
	\ingroup openlegends
	Methods for storing and retrieving entity/sprite data.
*/
static cell AMX_NATIVE_CALL sGetString(AMX *amx, cell *params){
	//bool sGetString(szName, szID, szOutput);
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return 0; //indicate failure.
	
	char szValue[256]; GetStringParam(amx, params[2], &szValue[0]);
	CBString ret = data->GetString(szValue);
	char szSet[1024]; strcpy(szSet, ret);
	
	amx_SetString(&params[3], szSet, false);
	
	return 1; //Boolean true.
}
static cell AMX_NATIVE_CALL vGetString(AMX *amx, cell *params){
	//bool vGetString(szName, ID, szOutput);
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return 0; //indicate failure.
	
	CBString ret = data->GetString(params[2]);
	char szSet[1024]; strcpy(szSet, ret);
	
	amx_SetString(&params[3], szSet, false);
	
	return 1;
}

static cell AMX_NATIVE_CALL sSetString(AMX *amx, cell *params){
	//bool sSetString(szName, szID, szText);
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return 0; //indicate failure.
	
	char szID[256]; GetStringParam(amx, params[2], &szID[0]);
	char szText[1024]; GetStringParam(amx, params[3], &szText[0]);
	
	data->SetString(szID, szText);

	return 1;
}
static cell AMX_NATIVE_CALL vSetString(AMX *amx, cell *params){
	//bool sSetString(szName, ID, szText);
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return 0; //indicate failure.
	
	char szText[1024]; GetStringParam(amx, params[3], &szText[0]);
	
	data->SetString(params[2], szText);

	return 1;
}

static cell AMX_NATIVE_CALL sGetValue(AMX *amx, cell *params){
	//int sGetValue(szName, szID); //-1 May indicate failure!
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return -1; //indicate failure.
	
	char szID[256]; GetStringParam(amx, params[2], &szID[0]);
	
	return data->GetValue(szID);
}
static cell AMX_NATIVE_CALL vGetValue(AMX *amx, cell *params){
	//int vGetValue(szName, ID); //-1 May indicate failure!
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return -1; //indicate failure.
	
	return data->GetValue(params[2]);
}

static cell AMX_NATIVE_CALL sSetValue(AMX *amx, cell *params){
	//bool sSetValue(szName, szID, Value);
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return 0; //indicate failure.
	
	char szID[256]; GetStringParam(amx, params[2], &szID[0]);
	
	data->SetValue(szID, params[3]);
	
	return 1;
}
static cell AMX_NATIVE_CALL vSetValue(AMX *amx, cell *params){
	//bool vSetValue(szName, ID, Value);
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return 0; //indicate failure.
	
	data->SetValue(params[2], params[3]);
	
	return 1;
}

static cell AMX_NATIVE_CALL sGetFlag(AMX *amx, cell *params){
	//int sGetFlag(szName, szID); //-1 indicates failure.
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return -1; //indicate failure.
	
	char szID[256]; GetStringParam(amx, params[2], &szID[0]);
	
	return (int)data->GetFlag(szID);
}
static cell AMX_NATIVE_CALL vGetFlag(AMX *amx, cell *params){
	//int vGetFlag(szName, ID); //-1 indicates failure.
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return -1; //indicate failure.
	
	return (int)data->GetFlag(params[2]);
}

static cell AMX_NATIVE_CALL sSetFlag(AMX *amx, cell *params){
	//bool sSetFlag(szName, szID, bVal); //0 indicates failure.
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return 0; //indicate failure.
	
	char szID[256]; GetStringParam(amx, params[2], &szID[0]);
	data->SetFlag(szID, (params[3]>0)?true:false);
	
	return 1;
}
static cell AMX_NATIVE_CALL vSetFlag(AMX *amx, cell *params){
	//bool vSetFlag(szName, ID, bVal); //0 indicates failure.
	CEntityData *data = GetRelevantEntityData(amx, params[1]);
	if(!data) return 0; //indicate failure.
	
	data->SetFlag(params[2], (params[3]>0)?true:false);
	
	return 1;
}

extern AMX_NATIVE_INFO data_Natives[] = {
	{ "sGetString", sGetString },
	{ "vGetString", vGetString },
	{ "sSetString", sSetString },
	{ "vSetString", vSetString },
	{ "sGetValue", sGetValue },
	{ "vGetValue", vGetValue },
	{ "sSetValue", sSetValue },
	{ "vSetValue", vSetValue },
	{ "sGetFlag", sGetFlag },
	{ "vGetFlag", vGetFlag },
	{ "sSetFlag", sSetFlag },
	{ "vSetFlag", vSetFlag },
	{NULL, NULL}
};
/*!
	\defgroup drawing Open Legends Drawing Functions
	\ingroup openlegends
	Drawing functions for sprites, animations, text and images.
*/
static cell AMX_NATIVE_CALL DrawSprite(AMX *amx, cell *params){
	//(string)Spritename, (enum)CoordType, x, y, subLayer=2, Layer=3, rgba="FFFFFF", scale=1.0, rotation=0.0)
	char szSprite[256];
	GetStringParam(amx, params[1], &szSprite[0]);
	int coordType = params[2];
	int x = params[3];
	int y = params[4];
	int subLayer = params[5];
	int Layer = params[6];
	char szRgba[256];
	GetStringParam(amx, params[7], &szRgba[0]);
	float fScale = fConvertCellToFloat(params[8]);
	int Rot = params[9];
	return CGameManager::Instance()->DrawSprite(szSprite, coordType, x, y, subLayer, Layer, szRgba, fScale, Rot);
}
extern AMX_NATIVE_INFO drawing_Natives[] = {
	{"DrawSprite", DrawSprite},
	{NULL, NULL}
};
/*!
	\defgroup mplayer Multiplayer Server/Client functions
	\ingroup openlegends
	Handles functions for multiplayer Server/Client interaction.
*/

extern AMX_NATIVE_INFO mplayer_Natives[] = {
	{NULL, NULL}
};
//-----------------------------------------------------------------------------
// Name: RegisterNatives()
// Desc: 
// 
//-----------------------------------------------------------------------------
void RegisterNatives(AMX *amx)
{
	// Register all the native functions
	amx_Register(amx, core_Natives, -1);
	amx_Register(amx, float_Natives, -1);
	amx_Register(amx, general_Natives, -1);
	amx_Register(amx, entity_Natives, -1);
	amx_Register(amx, data_Natives, -1);
	amx_Register(amx, drawing_Natives, -1);
	amx_Register(amx, mplayer_Natives, -1);
	
	
//	amx_Register(amx, console_Natives, -1);
	
	
}
//-----------------------------------------------------------------------------
#if defined OLDoc 

Limitations/*!<
	\ingroup general
	\brief Description of limitations.

	\remarks Entity/Sprite name length: 255 characters. Should be sufficient for anything.
*/
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
bool Wipe(int dir, char entity="this", int x=16, int y=16); /*!<
	\ingroup general
	\brief This function allows you to wipe between groups.

	\return returns false if failed, true if successful or already wiping.

	\remarks _direction is an enum found in the general include file /TODO: ADD IT!/ which relates to
		the different directions you can wipe to. The actual wipe creates a wipe-effect from one group
		to a different one, where dir indicates in which direction you are wiping. Example:
		if(GetValue("this", "_y") < 2) Wipe(_north); 
		The entity term can be used to move a different entity than the one you are using. 
		x and y can modify the minimum location from the edge that the entity will be moved to. 
		(the new group coordinates will be relative to the x and y pos delta between the two groups wiped between.)
*/
int DrawText(char text, int x, int y, int r=255, int g=255, int b=255, int a=255);/*!<
	\ingroup drawing
	\brief This function draws a string of text on-screen using a default font.

	\return Always returns 0.

	\remarks It accepts an onscreen x & y position which is relative to the top left corner of the game area
		as well as the text to write and any other necessary variables.
		TODO: Function doesn't exist yet!

		Only the drawing.inc file will be necessary for this include.
*/
bool DrawSprite(char spriteName, int coordType, int x, int y, int subLayer=2, int Layer=3, char szRgba="80808080", float fScale=1.0, int Rot=0.0);/*!<
	\ingroup drawing
	\brief This function draws a sprite named spritename on-screen with a few settings available.

	\return Returns true if sprite was found, false if not.

	\remarks It accepts spriteName as a string, coordType as either _worldcoords, _groupcoords or _screencoords, x,y as location.
	subLayer is the layer you draw on(higher is above), Layer is the layer you draw on (standard 3, higher on top), szRgba is a 
	string modding the color with which the text is drawn, fScale mods objects size(where 1.0 is normal size) and 
	fRot rotates the sprite counting in degrees.

	TODO: fScale implementation. coordType implementation.
*/
CDrawableContext *CreateEntity(char szScript, char szName);/*!<
	\ingroup entity
	\brief Creates entity using script szScript and names it szName.

	\return Returns name of entity in szName, if empty a name is created. 0 on fail, 1 on success.

	\remarks This function is used to create an instance of a script named szScript. This script may or may not
	exist as an entity linked to a sprite in the project. This means that it's possible to create an "unscripted" entity
	acting as an abstract storage object.
*/
int SetFilter();/*!< todo: WRITE DOCUMENTATION.

*/
bool FirstRun();/*!<
	\ingroup entity
	\brief Returns true if it's the first time you run the script

	\return boolean (integer)

	\remarks The return value is true if it's the first time you run the script, thus you can use it multiple
		times in the same script and still have it return "true" (1) if it hasn't been run before.
*/
int GetGroupWidth();/*!<
	\ingroup general
	\brief Returns width of active group.

	\return Returns the width of the currently active group, matches coordinates.

	\remarks
*/
int GetGroupHeight();/*!<
	\ingroup general
	\brief Returns height of active group.

	\return Returns the height of the currently active group, matches coordinates.

	\remarks
*/
int GetInput(int Key);/*!<
	\ingroup general
	\brief Returns 1 or higher if a key has been pushed.

	\return Returned value increases by one for each frame the key has been held.

	\remarks
*/
int GetMouseX();/*!<
	\ingroup general
	\brief Returns group coordinates for where mouse is.

	\return Returned value is the group coordinate for X.

	\remarks
*/
int GetMouseY();/*!<
	\ingroup general
	\brief Returns group coordinates for where mouse is.

	\return Returned value is the group coordinate for Y.

	\remarks
*/
int GetMouseKey();/*!<
	\ingroup general
	\brief Returns an enumeration indicating which mouse button is pushed.

	\return Returns an indicator as to which key is being pushed. Match with an enumerator.

	\remarks
*/
/* THESE FUNCTIONS NEED DOCUMENTATION.
vGetValue()
vSetValue()
sGetValue()
sSetValue()

vGetString()
vSetString()
sGetString()
sSetString()

vGetFlag()
vSetFlag()
sGetFlag()
sSetFlag()
*/
#endif