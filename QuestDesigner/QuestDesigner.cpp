/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
   Copyright (C) 2001-2003. Open Legends's Project
*/
/*! \mainpage Quest Designer: <br>First Part of Open Legends's Project
	\section license Open Legends's Project License
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	\section intro Concerning Open Legends
	Open Legends is a 2D page scrolling game engine for Win32 that uses scripts or 
	rules, sprites, and a number of sounds to render many types of 2D based games. 
	Its uses, with a little of immagination and creativity, can vary from the 
	classic Legends-style orthogonal views (on which the engine was based) to the 
	popular side view scrolling view, and even fake 3D isometric views!

	The Quest Designer, as his name suggests, is an editor of quests. A quest,
	in Open Legends, is nothing more than the set of sprites, maps, sound and rules
	that make all this types of games possible.


	\section QD Quest Designer

	\subsection motivation Motivation:
	In the dawn of games, Nintendo gave us the SNES with Legends 3 and 
	opened our eyes, but after some great years, SNES sadly expired. It was 
	not over, and not everything was lost, for the emulators came and gave 
	us back some hope.
	
	Soon our hope was diminished when we finally knew by heart the whole game,
	and we just yearned for those long gone days when we first played Legends 3.

	Oh, but then there was Greg Denness, who gave us the light and showed
	us new ways with his magnificent creation: Open Legends and the Quest 
	Designer. Our joy ended soon when he had to stall the project, but it was
	once again restrengthened when he released the source code. Now we are 
	illuminated with all the existent possibilities, and for that we are 
	deeply thankful.

	Greg said when he released Quest Designers' code:

		<em>"This program was created rather hastily, as such it's not the best
		piece of programming in the world =). In fact if you wanted to 
		completly recode it all and make it look nice I	certainly would not 
		take offence."</em>

	This words of wisdom and sincerity opened new doors for us but that was 
	just the beginning...
 
	\subsection goal Goal
	Our goal is to build a fully featured quest editor for Open Legends.
	The intended Quest Designer will have a much robust and user friendly 
	interface along with a better file management for smaller quest files 
	and higher efficiency in their use than the current Open Legends's 
	Quest Designer by Greg Denness.

 	\subsection road The road ahead
	This is my small effort to completely rewrite Quest Designer, although now,
	Quest Designer, will have a slightly more solid foundation that the original version,
	it is still a long way to go until we can have something real useful.

	During the next months we are going to build the strong basis needed to fully develop
	this software, and there are going to be several steps we are going to take in order
	to accomplish our goals.
 
	\subsubsection step1 Step 1: Opening pandora's box
	The first thing we need is to understand Greg's code. And to understand someone else's 
	code is no easy task, but we are digging hard on it. There's not much more we can say 
	about this step, just read, read, read...

	\subsubsection step2 Step 2: Building the foundations
	Start coding the fundamental application, using fast, small, and reliable programming 
	tools and methods.

	It has been decided that the Quest Designer will be programmed in C/C++
	in the Win32 plataform using DirectX libraries. Also, Quest Designer, will not make use 
	of the nice, but slow and heavy, Microsoft Foundation Class (MFC) Library. Instead
	we are going to base the code in the ATL/WTL libraries.

	Desing, architecture and documentation of the software will have an important role 
	on the development of the project, Doxygen has been chosen to documentate the code, 
	CVS control system will take care of the code control, but most important, intensive 
	and meticulous bug and error detection techniques and a rigorous quality control will 
	be applied on every module in every stage of the process. Making of Quest Designer a robust 
	and reliable tool with strong foundations.

	\subsubsection step3 Step 3: Graphics and DirectX
	DirectX modules to handle graphics and classes to manage the sprites and sheets and scripts
	will be created at this point of the project.

*/


/////////////////////////////////////////////////////////////////////////////
// QuestDesigner.cpp : main source file for QuestDesigner.exe
//

#include "stdafx.h"
#include "QuestDesigner.h"
#include "../Net.h"

#include <list>
#include <string>

static std::list<std::string> g_LinesToSend;

HANDLE g_hLines = NULL;
HWND g_hWnd = NULL;
CAppModule _Module;
CRITICAL_SECTION g_DebugCritical;

void ShowHelp(HWND hWnd, LPCSTR szTopic) 
{
	char szPath[MAX_PATH];
	::GetModuleFileNameA(_Module.GetModuleInstance(), szPath, MAX_PATH);
	char *aux = strrchr(szPath, '\\');
	if(aux) *(aux+1) = '\0';
	strcat(szPath, "ozscript.chm");

	HtmlHelp(hWnd, szPath, HH_DISPLAY_TOC, NULL);
	if(szTopic) {
		HH_AKLINK link;
		link.cbStruct =     sizeof(HH_AKLINK) ;
		link.fReserved =    FALSE;
		link.pszKeywords =  szTopic; 
		link.pszUrl =       NULL; 
		link.pszMsgText =   NULL; 
		link.pszMsgTitle =  NULL; 
		link.pszWindow =    NULL;
		link.fIndexOnFail = TRUE;

		HtmlHelp(hWnd, szPath, HH_KEYWORD_LOOKUP, (DWORD)&link);
		HtmlHelp(hWnd, szPath, HH_SYNC, NULL);
	}
}

int StartCodeSense(CodeSenseLibrary *cmaxlib)
{
	CM_LANGUAGE LangZES = { 
	// language style
	CMLS_PROCEDURAL,
	// Is case-sensitive?
	TRUE,
	// keywords
	_T("#assert\n#define\n#else\n#emit\n#endif\n#endinput\n#endscript\n#if\n#include\n#pragma\n")
	//_T("ctrlchar\ndynamic\nlibrary\npack\nrational\nsemicolon\ntabsize")
	_T("assert\nbreak\ncase\ncontinue\ndefault\ndo\nelse\nexit\nfor\ngoto\nif\nreturn\nsleep\nswitch\nwhile\n")
	_T("const\nenum\nforward\nnative\nnew\noperator\npublic\nstatic\nstock\n")
	_T("char\ndefined\nsizeof\n")
	_T("false\ntrue\ncellbits\ncellmax\ncellmin\ncharbits\ncharmax\ncharmin\ndebug\n_Small"),
	// operators
	_T("bool\nfixed\nfloat\n")
	_T("+\n-\n*\n/\n%\n++\n--\n~\n>>\n>>>\n<<\n&\n|\n^\n=\n+=\n-=\n*=\n/=\n%=\n>>=\n>>>=\n<<=\n&=\n|=\n^=\n==\n!=\n<\n<=\n>\n>=\n!\n||\n&&"),
	// Single line comments
	_T("//"),
	// Multi-line comment set
	_T("/*"),
	_T("*/"),
	// Scope words
	_T("{"),
	_T("}"),
	// String literal char
	_T("\"\n'"),
	// Escape Char
	_T('^'),
	// Statement Terminator Char
	_T(';'),
// Tag element names
	_T("black\nblue\ncyan\ngreen\nmagenta\nred\nyellow\nwhite\nchasing\ndead\ndying\nfalling\nfalling2\nhit\njumping\nknocked\n")
	_T("lifting\nstanding\nswimming\nstandLift\nusing\nwalking\nwalkLift\nscared\ndoorType\nenemyType\nitemType\nnpcType\n")
	_T("otherType\nplayerType\nweaponType\neast\nnorth\nnortheast\nnorthwest\nsouth\nsoutheast\nsouthwest\nwest\nwaiting\n")
	_T("stunned\nfrozen\nburning\nsleeping\npushing\nleaping\npulling"),
// Tag attribute names
	_T("AddAnimframe\nAKey\nCreateAnim\nCreateAnimWithID\nclamp\nCreateCounter\nCreateCounterWithID\nCheckMask\n")
	_T("CheckForHoleXY\n\nClearRestartTable\nCalculateAngle\nDeleteAnim\nDrawAnim\nDrawAnimNoInc\ndeleteproperty\n")
	_T("DeleteCounter\nDrawImage\nDownKey\n\nDrawText\ndate\nexistproperty\nEnterKey\nFinishedAnim\nfuncidx\nFirstRun\n")
	_T("FadeTo\n\nFinishedReading\nGetAnimImage\nGetAnimCount\nGetMaxAnim\nGetAnimWidth\nGetAnimHeight\ngetchar\n")
	_T("getstring\ngetvalue\n\ngetarg\ngetproperty\nGetCounterValue\nGetWidth\nGetHeight\nGetWorldX\nGetWorldY\n")
	_T("GetGroupMinX\nGetGroupMinY\n\nGetGroupMaxX\nGetGroupMaxY\nGetPauseLevel\nGetLowerLevelX\nGetLowerLevelY\n")
	_T("GetLastAnswer\nGetMinuteCount\nGetHourCount\nGetDayCount\n\nGetGlobal\nGetLocal\nheapspace\nIncrementAnim\n")
	_T("IncCounterTarget\nIncCounterValue\nInitialPlayerX\nInitialPlayerY\nInitTextBox\n\nLeftKey\nmin\nmax\nnumargs\n")
	_T("NearPoint\nprint\nprintf\nPutSprite\n\nPlaySound\nPlayMusic\nQKey\nrandom\nRightKey\nSetAnimSpeed\nSetAnimCount\n")
	_T("SetAnimDirection\nSetAnimLoop\n\nsetarg\nstrlen\nstrcmp\nstrcpy\nstrpack\nstrunpack\nswapchars\nsetproperty\n")
	_T("SetCounterMin\n\nSetCounterMax\nSetCounterValue\nSetCounterTarget\nSetCounterSpeed\nSKey\nSetQKey\nSetAKey\n")
	_T("SetSKey\nSetWKey\n\nSetEnterKey\nSetUpKey\nSetDownKey\nSetLeftKey\nSetRightKey\nSetWorldX\nSetWorldY\nSetPauseLevel\n")
	_T("SetRestartTable\n\nSetRestartPoint\nSetLowerLevel\nSetTextBoxColor\nSetMinuteCount\nSetHourCount\nSetDayCount\n")
	_T("SetDayLength\nSetGlobal\nSetLocal\n\nSetMusicVolume\nSetMusicFadeSpeed\ntolower\ntoupper\nToString\nToFile\nTextFile\n")
	_T("TestFunc\ntime\n\ntickcount\nUpKey\nUpdateWorldCo\nWKey\nWipe\nGetTimeDelta\nfloatstr\nfloatround\nDrawRectangle\n")
	_T("DrawLine\nSetTextSize\nEnableControls\nDrawBitmapText\n\nSetFloat\nGetFloat\nSetSpriteAlpha"),
// Tag entities
	_T("AngleMove\nAngleCollide\nAllocateStrings\nCreateEntity\nCreateEntityWithID\nChangeDirection\nClearCollisionRect\n")
	_T("Collide\nCollideAll\nCollidePoint\nCheckForHole\nCallFunction\nDeleteEntity\nGetDamage\nGetRespawnValue\n")
	_T("GetLiftLevel\nGetX\nGetY\nGetMoveAngle\nGetDirection\nGetSpeed\nGetSpeedMod\nGetItem\nGetImage\nGetType\nGetString\n")
	_T("GetState\nGetInitialX\nGetInitialY\nGetEntityCount\nGetAnglePointX\nGetAnglePointY\nGetWeight\nGetBounceValue\n")
	_T("GetValue\nGetHealth\nGetMaxHealth\nGetCurrentEntity\nGetNextMessage\nGetGotoNextMessage\nGetNumNextMessages\n")
	_T("isOpen\nisTaken\nisDead\nisVisible\nisActive\nisCuttable\nisLarge\nisPickable\nisPushed\nisInteracting\nMessageMap\n")
	_T("NextEntity\nRespawn\nSetOpenFlag\nSetTakenFlag\nSetDeadFlag\nSetVisibleFlag\nSetActiveFlag\nSetCuttableFlag\n")
	_T("SetLargeFlag\nSetPickableFlag\nSetPushedFlag\nSetInteractingFlag\nSetDamage\nSetRespawnValue\nSetLiftLevel\nSetX\n")
	_T("SetY\nSetPosition\nSetMoveAngle\nSetDirection\nSetSpeed\nSetSpeedMod\nSetAngleFromDir\nSetDirFromAngle\n")
	_T("SetCollisionRect\nSetItem\nSetImage\nSetType\nSetString\nSetActiveDist\nSetState\nSetWeight\nSetBounceValue\n")
	_T("SetValue\nSetHealth\nSetMaxHealth\nStartEntity\nisOwned\nSetOwnedFlag\nGetParent\nGetParam\nSetActiveInGroups"),
	};

	if(!cmaxlib->Initialize()) return 0;
	cmaxlib->UnregisterAllLanguages();
	cmaxlib->RegisterLanguage(CMLANG_ZES, &LangZES );

	return 1;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMainFrame wndMain;
	CMessageLoop theLoop;
	CodeSenseLibrary cmaxlib;

	_Module.AddMessageLoop(&theLoop);
	

	// Initialize the html help system
	DWORD dwCookie;
	HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&dwCookie);

	if(!StartCodeSense(&cmaxlib)) {
		ATLTRACE ( _T ( "CodeSense initialization failed!\n" ) );
		return 0; // bail...
	}

	if(wndMain.CreateEx() == NULL) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}
	wndMain.ShowWindow(SW_MAXIMIZE);
	g_hWnd = wndMain.m_hWnd;

	int nRet = theLoop.Run();

	// Uninitialize the help system
	HtmlHelp(NULL, NULL, HH_UNINITIALIZE, dwCookie);

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
/*
	ATOM gi_atom = 0;
	if(GlobalFindAtomA("QuestDesigner") > 0) {
		return 0;
	}
    gi_atom = GlobalAddAtomA("QuestDesigner");
*/
	HRESULT hRes;
	hRes = ::OleInitialize(NULL);
//	hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	g_hLines = CreateSemaphore(NULL, 0, 500, NULL);
	InitializeCriticalSection(&g_DebugCritical);

	AtlAxWinInit();

	HMODULE hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
	ATLASSERT(hInstRich != NULL);

	int nRet = Run(lpstrCmdLine, nCmdShow);

	::FreeLibrary(hInstRich);

	if(g_hLines) { CloseHandle(g_hLines); g_hLines = NULL; }
	DeleteCriticalSection(&g_DebugCritical);

	_Module.Term();
//	::CoUninitialize();
	::OleUninitialize();

/*
	if(gi_atom) GlobalDeleteAtom(gi_atom);
*/
	return nRet;
}

/////////////////////////////////////////////////////////
// REAL TIME DEBUGGER STUFF:

bool __Send(LPCSTR format, va_list argptr)
{
	char buff[2001];
	vsprintf(buff, format, argptr);
	ASSERT(strlen(buff) < 2000);
	buff[2000] = '\0';
	g_LinesToSend.push_back(buff);
	ReleaseSemaphore(g_hLines, 1, NULL);
	return true;
}

bool BeginSend()
{
	EnterCriticalSection(&g_DebugCritical);
	return true;
}
bool EndSend()
{
	LeaveCriticalSection(&g_DebugCritical);
	return true;
}

bool Send(LPCSTR format, ...)
{
	EnterCriticalSection(&g_DebugCritical);

	va_list argptr;
	va_start(argptr, format);
	bool ret = __Send(format, argptr);
	va_end(argptr);

	LeaveCriticalSection(&g_DebugCritical);
	return ret;
}
int CALLBACK Dispatch(SOCKET s)
{
	char buffer[100];
	// Check for the welcome message from the server:
	if(RecvLine(s, buffer, sizeof(buffer))) {
		if(strncmp(buffer, "100 ", 4))
			Disconnect();
	}
	do {
		if(*buffer) {
			LPSTR szCommand = new char[strlen(buffer) + 1];
			strcpy(szCommand, buffer);
			// Post the command:
			if(::PostMessage(g_hWnd, WM_USER, (WPARAM)s, (LPARAM)szCommand) == 0) {
				delete szCommand;
			}
		}
		if(!RecvLine(s, buffer, sizeof(buffer))) *buffer = '\0';
	} while(Connected());
	return 0;
}
int CALLBACK Request(SOCKET s)
{
	EnterCriticalSection(&g_DebugCritical);
	g_LinesToSend.clear();
	LeaveCriticalSection(&g_DebugCritical);

	while(Connected()) {
		if(WaitForSingleObject(g_hLines, 500) == WAIT_OBJECT_0) {
			EnterCriticalSection(&g_DebugCritical);
			if(!g_LinesToSend.empty()) {
				std::string sLine = g_LinesToSend.front();
				::Send(s, sLine.c_str(), sLine.length());
				g_LinesToSend.pop_front();
			}
			LeaveCriticalSection(&g_DebugCritical);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
