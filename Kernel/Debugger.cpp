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
/*! \file		Debugger.cpp
	\brief		Implementation of the debugging server.
	\date		October 31, 2003
*/

#include "stdafx.h"

#include "Debugger.h"
#include "ScriptManager.h"

CDebugScript DummyDebug(NULL, NULL);

static cell get_symbolvalue(AMX *amx, SYMBOL *sym, int index)
{
	cell *value;
	cell base;

	if(sym->type==2 || sym->type==4) {   // a reference
		amx_GetAddr(amx, sym->addr, &value);
		base = *value;
	} else {
		base = sym->addr;
	}
	amx_GetAddr(amx, (cell)(base + index*sizeof(cell)), &value);
	return *value;
}

vector<string> CDebugFile::ms_Watches;
vector<BREAKPOINT> CDebugFile::ms_Breakpoints;

list<string> CDebugScript::ms_LinesToSend;
SOCKET CDebugScript::ms_Socket = INVALID_SOCKET;
int CDebugScript::ms_nScripts = 0;
bool CDebugScript::ms_bBreakRequest = false;
bool CDebugScript::ms_bInterBreak = false;
bool CDebugScript::ms_bSending = false;
CRITICAL_SECTION CDebugScript::ms_SendCritical;
CRITICAL_SECTION CDebugScript::ms_DebugCritical;
CDebugScript *CDebugScript::ms_DebuggingScript = NULL;
__commands CDebugScript::ms_eCommand;
HANDLE CDebugScript::ms_hLines = NULL;
HANDLE CDebugScript::ms_hCommand = NULL;

CDebugScript::mapDebugFiles CDebugScript::ms_DebugFiles;

CDebugScript::CDebugScript(CDebugScript *pDebugCreator, HANDLE hSemaphore) :
	m_hSemaphore(hSemaphore),
	m_pDebugCreator(pDebugCreator),
	m_nCurrentFile(-1),
	m_nCurrentLine(-1),
	m_pCurrentFunction(NULL),
	m_pCurrentVariable(NULL),
	m_nTraceLevel(-1),
	m_nStopLine(-1),
	m_bDebugging(false),
	m_dwStartDebug(0),
	m_dwDebugTime(0)
{
	if(ms_nScripts++ == 0) {
		InitializeCriticalSection(&ms_SendCritical);
		InitializeCriticalSection(&ms_DebugCritical);
		ms_hLines = CreateSemaphore(NULL, 0, 500, NULL); // create a semaphore of up to 500 items
		ms_hCommand = CreateSemaphore(NULL, 1, 1, NULL); // create a semaphore for commands (1 item)
	}
	if(ms_Socket == INVALID_SOCKET) {
		ms_Socket = InitServer(7683, CDebugScript::Dispatch, CDebugScript::Request);
		if(ms_Socket == INVALID_SOCKET) {
			CONSOLE_PRINTF("Debug error: Couldn't Initialize debug server!\n");
		} 
	}
}
CDebugScript::~CDebugScript()
{
	if(--ms_nScripts == 0) {
		EndServer();
		DeleteCriticalSection(&ms_SendCritical);
		DeleteCriticalSection(&ms_DebugCritical);
		CloseHandle(ms_hLines);
		CloseHandle(ms_hCommand);
		ms_hCommand = NULL;
		ms_hLines = NULL;
		ms_Socket = INVALID_SOCKET;
	}
	for_each(m_Functions.begin(), m_Functions.end(), ptr_delete());
	for_each(m_Variables.begin(), m_Variables.end(), ptr_delete());
}

bool CDebugScript::BeginSend()
{
	if(ms_bSending) return false;
	EnterCriticalSection(&ms_SendCritical);
	ms_bSending = true;
	return true;
}
bool CDebugScript::EndSend()
{
	if(!ms_bSending) return false;
	ms_bSending = false;
	LeaveCriticalSection(&ms_SendCritical);
	return true;
}
bool CDebugScript::__Send(LPCSTR format, va_list argptr)
{
	char buff[2000];
	vsprintf(buff, format, argptr);
	ms_LinesToSend.push_back(buff);
	ReleaseSemaphore(ms_hLines, 1, NULL);
	return true;
}
bool CDebugScript::Send(LPCSTR format, ...)
{
	if(!ms_bSending) return false;

	va_list argptr;
	va_start(argptr, format);
	bool ret = __Send(format, argptr);
	va_end(argptr);
	return ret;
}
int CALLBACK CDebugScript::Request(SOCKET s)
{
	CDebugScript::ms_eCommand = NONE;
	ms_bBreakRequest = true;

	BeginSend();
	Send("100 Welcome to the Open Zelda debugger v1.0.\r\n");
	Send("200 Type HELP for help.\r\n\r\n");
	EndSend();

	while(Connected()) {
		if(WaitForSingleObject(ms_hLines, 500) != WAIT_TIMEOUT) {
			EnterCriticalSection(&ms_SendCritical);
			string sLine = ms_LinesToSend.front();
			::Send(s, sLine.c_str(), sLine.length());
			ms_LinesToSend.pop_front();
			LeaveCriticalSection(&ms_SendCritical);
		}
	}
	return 0;
}

void CDebugScript::ListCommands(LPCSTR command)
{
	BeginSend();

	if(command == NULL)
		command = "";

	Send("201\r\n"); // start multiline information.
	if(stricmp(command,"break")==0) {
		Send(
			"\tBREAK\t\tlist all breakpoints\r\n"
			"\tBREAK n\t\tset a breakpoint at line \"n\"\r\n"
			"\tBREAK func\tset a breakpoint at function with name \"func\"\r\n"
			"\tBREAK var\tset a breakpoint at variable \"var\"\r\n"
			"\tBREAK var[i]\tset a breakpoint at array element \"var[i]\"\r\n");
	} else if(stricmp(command,"cbreak")==0) {
		Send(
			"\tCBREAK n\tremove breakpoint number \"n\"\r\n"
			"\tCBREAK *\tremove all breakpoints\r\n");
	} else if(stricmp(command,"cw")==0 || stricmp(command,"cwatch")==0) {
		Send(
			"\tCWATCH may be abbreviated to CW\r\n\r\n"
			"\tCWATCH n\tremove watch number \"n\"\r\n"
			"\tCWATCH *\tremove all watches\r\n");
	} else if(stricmp(command,"d")==0 || stricmp(command,"disp")==0) {
		Send(
			"\tDISP may be abbreviated to D\r\n\r\n"
			"\tDISP\t\tdisplay all variables that are currently in scope\r\n"
			"\tDISP var\tdisplay the value of variable \"var\"\r\n"
			"\tDISP var[i]\tdisplay the value of array element \"var[i]\"\r\n");
	} else if(stricmp(command,"file")==0) {
		Send(
			"\tFILE\t\tlist all files that this program is composed off\r\n"
			"\tFILE name\tset the current file to \"name\"\r\n");
	} else if(stricmp(command,"g")==0 || stricmp(command,"go")==0) {
		Send(
			"\tGO may be abbreviated to G\r\n\r\n"
			"\tGO\t\trun until the next breakpoint or program termination\r\n"
			"\tGO RET\t\trun until the end of the current function\r\n"
			"\tGO n\t\trun until line number \"n\"\r\n");
	} else if(
		stricmp(command,"calls")==0 ||
		stricmp(command,"n")==0 || stricmp(command,"next")==0 ||
		stricmp(command,"quit")==0 ||
		stricmp(command,"s")==0 || stricmp(command,"step")==0 ||
		stricmp(command,"o")==0 || stricmp(command,"out")==0 )
	{
		Send(
			"\tno additional information\r\n");
	} else if(stricmp(command,"type")==0) {
		Send(
			"\tTYPE var STRING\tdisplay \"var\" as string\r\n"
			"\tTYPE var STD\tset default display format\r\n");
	} else if(stricmp(command,"watch")==0 || stricmp(command,"w")==0) {
		Send(
			"\tWATCH may be abbreviated to W\r\n\r\n"
			"\tWATCH var\tset a new watch at variable \"var\"\r\n"
			"\tWATCH n var\tchange watch \"n\" to variable \"var\"\r\n");
	} else {
		Send(
			"\tBREAK\t\tset breakpoint at line number or variable name\r\n"
			"\tCALLS\t\tshow call stack\r\n"
			"\tCBREAK\t\tremove breakpoint\r\n"
			"\tCW(atch)\tremove a \"watchpoint\"\r\n"
			"\tD(isp)\t\tdisplay the value of a variable, list variables\r\n"
			"\tFILE\t\tswitch to a file\r\n"
			"\tG(o)\t\trun program (until breakpoint)\r\n"
			"\tN(ext)\t\tRun until next line, step over functions\r\n"
			"\tQUIT\t\texit debugger, close the connection\r\n"
			"\tS(tep)\t\tsingle step, step into functions\r\n"
			"\tO(ut)\t\tsingle step, step out\r\n"
			"\tTYPE\t\tset the \"display type\" of a symbol\r\n"
			"\tW(atch)\t\tset a \"watchpoint\" on a variable\r\n"
			"\tHELP <command>\tto view more information on a command\r\n");
	}
	Send("\r\n.\r\n"); // end multiline information.
	EndSend();
}

void CDebugScript::Dispatch(LPCSTR szCommand)
{
	int nError = 0;

	BeginSend();
	ms_eCommand = NONE;
	if(!stricmp(szCommand, "break")) {
		if(ms_DebuggingScript == NULL) ms_bBreakRequest = true;
		else nError = 1;
	} 
	else 
	if(!stricmp(szCommand, "go") || !stricmp(szCommand, "g")) {
		if(ms_DebuggingScript == NULL) nError = 0;
		else ms_eCommand = GO;
	} 
	else 
	if(!stricmp(szCommand, "next") || !stricmp(szCommand, "n")) {
		if(ms_DebuggingScript == NULL) ms_bBreakRequest = true;
		else ms_eCommand = STEP_OVER;
	} 
	else 
	if(!stricmp(szCommand, "step") || !stricmp(szCommand, "s")) {
		if(ms_DebuggingScript == NULL) ms_bBreakRequest = true;
		else ms_eCommand = STEP_IN;
	} 
	else 
	if(!stricmp(szCommand, "out") || !stricmp(szCommand, "o")) {
		if(ms_DebuggingScript == NULL) ms_bBreakRequest = true;
		else ms_eCommand = STEP_OUT;
	} 
	else 
	if(!stricmp(szCommand, "quit")) {
		Send("101 Bye.\r\n");
		ms_eCommand = QUIT;
	} 
	else nError = -1;

	// Help commands:
	if(nError == -1) {
		nError = 0;
		if(!stricmp(szCommand, "help")) ListCommands();
		else if(!strnicmp(szCommand, "help ", 5)) ListCommands(&szCommand[5]);
		else if(!stricmp(szCommand, "?")) ListCommands();
		else if(!strnicmp(szCommand, "? ", 2)) ListCommands(&szCommand[2]);
		else nError = -1;
	}

	// Print errorlevel:
	switch(nError) {
		case -1:	Send("400 Unrecognized command: %s\r\n", szCommand); break;
		case 1:		Send("200 Already stopped!\r\n"); break;
	}
	EndSend();
	if(ms_eCommand == QUIT) {
		Sleep(500);
		Disconnect();
		ms_eCommand = NONE;
	}

	// if there's really a command to execute, release the semaphore:
	if(ms_eCommand != NONE) ReleaseSemaphore(ms_hCommand, 1, NULL);
}

int CALLBACK CDebugScript::Dispatch(SOCKET s)
{
	char buffer[1000];
	char *buffaux = buffer;
	int nReceived = 0;
	while(Connected()) {
		int len = ::Recv(s, buffaux, sizeof(buffer) - nReceived - 1);
		if(len != SOCKET_ERROR && len != 0) {
			nReceived += len;
			*(buffaux += len) = '\0';
			char *aux = buffer;
			char *aux2;
			while( (aux2 = strchr(aux, '\n')) ) {
				*aux2 = '\0';

				while(*aux && *aux < ' ') aux++;
				char *aux3 = aux2;
				while(*aux3 < ' ' && aux3 != aux) *aux3-- = '\0';
				if(*aux) Dispatch(aux);

				aux = aux2 + 1;
			}
			if(aux > buffer) {
				buffaux = buffer;
				while(*aux) *buffaux++ = *aux++;
				*buffaux = '\0';
				nReceived = strlen(buffaux);
			}
		}
	}
	// on connection failure, just go
	ms_eCommand = GO;
	ReleaseSemaphore(ms_hCommand, 1, NULL);
	return 0;
}

// Check for an breakpoint at the given file and line 
int CDebugScript::BreakCheck(AMX *amx, int file, int line)
{
	mapFiles::iterator Iterator = m_Files.find(file);
	if(Iterator != m_Files.end()) {
		CDebugFile *pDebugFile = Iterator->second;
		for(int index=0; index<(int)CDebugFile::ms_Breakpoints.size(); index++) {
			if( CDebugFile::ms_Breakpoints[index].file == pDebugFile &&
				CDebugFile::ms_Breakpoints[index].type == BP_CODE && 
				CDebugFile::ms_Breakpoints[index].addr == line ) {
				return index;
			} else if(CDebugFile::ms_Breakpoints[index].type == BP_DATA) {
				int idx = CDebugFile::ms_Breakpoints[index].index;
				SYMBOL *sym = CDebugFile::ms_Breakpoints[index].sym;
				ASSERT(sym != NULL);
				ucell value = get_symbolvalue(amx, sym, (idx>=0)?idx:0);
				if(CDebugFile::ms_Breakpoints[index].addr != value) {
					CDebugFile::ms_Breakpoints[index].addr = value;
					return index;
				}
			}
		}
	}
	return -1;
}

bool CDebugScript::AddFile(int num, LPCSTR name) 
{
	if(m_Files.find(num) != m_Files.end()) return false;
	if(m_pDebugCreator && m_pDebugCreator->m_Files.find(num) != m_pDebugCreator->m_Files.end()) return false;

	CDebugFile *pDebugFile = NULL;
	mapDebugFiles::iterator Iterator = ms_DebugFiles.find(name);
	if(Iterator == ms_DebugFiles.end()) {
		pDebugFile = new CDebugFile(name);
		pair<mapDebugFiles::iterator, bool> pr =
			ms_DebugFiles.insert(pairDebugFiles(name, pDebugFile));
		if(pr.second == false) {
			delete pDebugFile;
			pDebugFile = NULL;
		}
	} else pDebugFile = Iterator->second;

	ASSERT(pDebugFile);
	if(pDebugFile) return m_Files.insert(pairFiles(num, pDebugFile)).second;
	return false;
}

SYMBOL* CDebugScript::AddSymbol(vector<SYMBOL *> &table, LPCSTR name, int type, ucell addr, int vclass, int level) 
{
	SYMBOL *sym = new SYMBOL;
	if(!sym) return NULL;

	sym->name = name;
	sym->addr = addr;
	sym->file = 0;
	sym->line = 0;
	sym->vclass = vclass;
	sym->type = type;
	sym->calllevel = level;
	sym->length[0] = 0;						// indeterminate
	sym->length[1] = 0;						// indeterminate
	sym->disptype = DISP_DEFAULT;
	if(type==3 || type==4) sym->dims = 1;	// for now, assume single dimension
	else sym->dims = 0;						// not an array

	table.push_back(sym);
	return sym;
}
bool CDebugScript::DeleteSymbol(vector<SYMBOL *> &table, ucell addr, int level)
{
	// Delete all local symbols below a certain address (these are local
	// variables from a function/block that one stepped out of). Also
	// remove all symbols of deeper calllevels (removes the static variables
	// at the end of a function).
	vector<SYMBOL *>::iterator Iterator = table.begin();
	while(Iterator != table.end()) {
		if((*Iterator)->calllevel>level || (*Iterator)->vclass==1 && (*Iterator)->addr<addr) {
			delete *Iterator;
			*Iterator = NULL;
		}
		Iterator++;
	}
	Iterator = remove(table.begin(), table.end(), (SYMBOL *)NULL);
	table.erase(Iterator, table.end());
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// Debugging stuff (Debug server):

// will this only work using single threaded scripts??:
int AMXAPI amx_InternalDebugProc(AMX *amx)
{
	int aux;
	int type, vclass;
	unsigned short flags;

	CDebugScript *pDebug = *(CDebugScript**)amx->param;

	ASSERT(pDebug);
	if(!pDebug) return AMX_ERR_NONE;

	pDebug->m_dwStartDebug = GetTickCount();
	pDebug->m_bDebugging = true; // set the debugging flag to true.

	switch (amx->dbgcode) {
		case DBG_INIT:
			ASSERT(amx->flags == AMX_FLAG_BROWSE);
			// check whether we should run
			amx_Flags(amx, &flags);
			if((flags & AMX_FLAG_DEBUG)==0) {
				pDebug->m_bDebugging = false; // set the debugging flag to false.
				return AMX_ERR_DEBUG;     // the debugger cannot run
			}
			break;

		// Called just before loading a needed file:
		case DBG_FILE: // file number in curfile, filename in dbgname
			ASSERT(amx->flags == (AMX_FLAG_DEBUG | AMX_FLAG_BROWSE));
			pDebug->AddFile((int)amx->curfile, amx->dbgname);
			break;

		// Called before executing every line of the code:
		case DBG_LINE: { // line number in curline, file number in curfile
			if((amx->flags & AMX_FLAG_BROWSE)!=0) {
				// check whether there is a function symbol that needs to be adjusted
				if(pDebug->m_pCurrentFunction != NULL) {
					pDebug->m_pCurrentFunction->line = (int)amx->curline;
					pDebug->m_pCurrentFunction->file = (int)amx->curfile;
				}
				pDebug->m_pCurrentFunction = NULL;
				break; // ??? could build a list with valid breakpoints
			}
			pDebug->m_nCurrentLine = (int)amx->curline - 1;

			// check breakpoints:
			CDebugScript::BeginSend();
			if((int)amx->curline == pDebug->m_nStopLine) {
				CDebugScript::Send("500 STOPPED at line %d\r\n", (int)amx->curline);
				pDebug->m_nTraceLevel = pDebug->m_CallStack.size(); // make the debugger stop from now on
			} else if( (aux = pDebug->BreakCheck(amx, (int)amx->curfile, (int)amx->curline)) >= 0) {
				CDebugScript::Send("500 STOPPED at breakpoint %d\r\n", aux);
				pDebug->m_nTraceLevel = pDebug->m_CallStack.size(); // make the debugger stop from now on
			} else if(CDebugScript::ms_bBreakRequest) {
				CDebugScript::Send("500 STOPPED because of break request\r\n");
				pDebug->m_nTraceLevel = pDebug->m_CallStack.size(); // make the debugger stop from now on
			} else if(CDebugScript::ms_bInterBreak) {
				pDebug->m_nTraceLevel = pDebug->m_CallStack.size(); // make the debugger stop from now on
			}
			CDebugScript::EndSend();

			// No breaks needed, continue executing the code...
			if(pDebug->m_nTraceLevel < (int)pDebug->m_CallStack.size()) break;

			// Here, tell the debug client that it needs to show a file in execution,
			// needed file to be shown is that of amx->curfile at amx->curline...

			CDebugFile *pFile = pDebug->GetFile((int)amx->curfile);
			CDebugScript::BeginSend();
			if(pFile) {
				CDebugScript::Send("501 STEP ID: %s\r\n", pDebug->m_sName.c_str());
				CDebugScript::Send("502 STEP FILE: %s\r\n", pFile->m_sName.c_str());
				CDebugScript::Send("503 STEP LINE: %d\r\n", (int)amx->curline);
			} else {
				CDebugScript::Send("410 Open Zelda's debugger internal error!\r\n");
			}
			CDebugScript::EndSend();

			EnterCriticalSection(&CDebugScript::ms_DebugCritical);
			CDebugScript::ms_DebuggingScript = pDebug;

			// Disable some types of breakpoints:
			pDebug->m_nStopLine = -1;
			CDebugScript::ms_bBreakRequest = false;
			CDebugScript::ms_bInterBreak = false;

			// Tell OZ to contine to draw stuff in the screen while debugging:
			if(pDebug->m_hSemaphore) ReleaseSemaphore(pDebug->m_hSemaphore, 1, NULL);

			// Also, update information (variables and stuff) in the client and wait for user input...
			// the client should as for the information as commands (VAR for variables, etc.)

			while(true) {
				WaitForSingleObject(CDebugScript::ms_hCommand, INFINITE);
				__commands cmd = CDebugScript::ms_eCommand;
				switch(cmd) {
					case GO:
						pDebug->m_nTraceLevel = -1;
						break;
					case STEP_OUT:
						pDebug->m_nTraceLevel = pDebug->m_CallStack.size() - 1;
						break;
					case STEP_OVER:
						pDebug->m_nTraceLevel = pDebug->m_CallStack.size();
						break;
					case STEP_IN:
						pDebug->m_nTraceLevel = pDebug->m_CallStack.size() + 1;
						break;
				}
				// Other commands might include some way of changing variables or even the code itself...
				if( cmd == GO ||
					cmd == STEP_OUT ||
					cmd == STEP_OVER ||
					cmd == STEP_IN ) break;
			}

			// Now tell OZ to stop drawing until the next debug or 
			// until all the scripts have been executed:
			if(pDebug->m_hSemaphore) WaitForSingleObject(pDebug->m_hSemaphore, INFINITE);

			CDebugScript::ms_DebuggingScript = NULL;
			LeaveCriticalSection(&CDebugScript::ms_DebugCritical);

			break;
		}
		case DBG_SYMBOL: // address in dbgaddr, class/type in dbgparam, symbol name in dbgname
			vclass = (int)(amx->dbgparam>>8); // class of the symbol
			type = (int)amx->dbgparam & 0xff; // type of the symbol

			if(type == 9) { // the symbol is a function:
				ASSERT(vclass == 0);
				ASSERT(amx->flags == (AMX_FLAG_DEBUG | AMX_FLAG_BROWSE));
				pDebug->AddFunction(amx->dbgname, amx->dbgaddr);
			} else {
				ASSERT((amx->flags & AMX_FLAG_DEBUG) != 0);
				// for local variables, must modify address relative to frame
				if(vclass==1) amx->dbgaddr += amx->frm;
				int lvl = ((amx->flags & AMX_FLAG_BROWSE) != 0) ? -1 : pDebug->m_CallStack.size();
				// if a static variable is declared inside a loop, it was not removed
				// at the end of the loop, so we do not have to declared it again
				if(vclass==2 && pDebug->FindVariable(amx->dbgname, lvl) != NULL) break;
				pDebug->AddVariable(amx->dbgname, type, amx->dbgaddr, vclass, lvl);
			}
			break;

		case DBG_SRANGE:
			// check whether there is a symbol that needs to be adjusted
			if(pDebug->m_pCurrentVariable != NULL) {
				pDebug->m_pCurrentVariable->length[(int)amx->dbgaddr] = (int)amx->dbgparam;
				if(pDebug->m_pCurrentVariable->dims < (int)amx->dbgaddr + 1) 
					pDebug->m_pCurrentVariable->dims = (int)amx->dbgaddr + 1;
				if(amx->dbgaddr == 0) pDebug->m_pCurrentVariable = NULL; // last dimension handled
			}
			break;

		case DBG_CLRSYM: // stk = stack address below which locals should be removed
			ASSERT((amx->flags & (AMX_FLAG_DEBUG | AMX_FLAG_BROWSE)) == AMX_FLAG_DEBUG);
			pDebug->DeleteVariable(amx->stk);
			break;

		case DBG_CALL: // function call
			ASSERT((amx->flags & (AMX_FLAG_DEBUG | AMX_FLAG_BROWSE)) == AMX_FLAG_DEBUG);
			pDebug->m_CallStack.push_back(amx->dbgaddr);
			break;

		case DBG_RETURN: // function returns
			ASSERT((amx->flags & (AMX_FLAG_DEBUG | AMX_FLAG_BROWSE)) == AMX_FLAG_DEBUG);
			__noop("Function returns: %ld\n", (long)amx->dbgparam);
			pDebug->m_CallStack.pop_back();
			pDebug->DeleteVariable(amx->stk);
			if(pDebug->m_nTraceLevel > (int)pDebug->m_CallStack.size()) 
				pDebug->m_nTraceLevel = pDebug->m_CallStack.size();

			if(pDebug->m_nTraceLevel >=0) {
				if((int)pDebug->m_CallStack.size() == 0 && (amx->flags & AMX_FLAG_BROWSE) == 0) {
					CDebugScript::ms_bInterBreak = true;
				}
			}

			break;

		case DBG_TERMINATE: 
			// program ends, either because the "entry point" function returns, or because of a hard exit
			ASSERT((amx->flags & (AMX_FLAG_DEBUG | AMX_FLAG_BROWSE)) == AMX_FLAG_DEBUG);
			if(amx->dbgparam == AMX_ERR_SLEEP) {
				__noop("AMX put to sleep: tag=%ld, value=%ld\n",(long)amx->alt, (long)amx->pri);
			} else {
				pDebug->m_nTraceLevel = -1;
				pDebug->m_CallStack.clear();
			}
			pDebug->m_nCurrentFile = -1;
			// ??? save breakpoints on exit
			// ??? save watches
			// ??? save terminal type
			break;
	}

	pDebug->m_bDebugging = false; // set the debugging flag to false.
	pDebug->m_dwDebugTime += (GetTickCount() - pDebug->m_dwStartDebug);

	return AMX_ERR_NONE;
}


/*
// Misc stuff:
static LPCSTR skipwhitespace(LPCSTR str)
{
	while(*str==' ' || *str=='\t') str++;
	return str;
}

// Symbols stuff:
static SYMBOL *add_symbol(SYMBOL *table, LPCSTR name, int type, ucell addr, int vclass, int level)
{
	SYMBOL *sym = new SYMBOL;
	if(!sym) return NULL;

	sym->name = name;
	sym->addr = addr;
	sym->file = 0;
	sym->line = 0;
	sym->vclass = vclass;
	sym->type = type;
	sym->calllevel = level;
	sym->length[0] = 0;	// indeterminate
	sym->length[1] = 0;	// indeterminate
	if(type==3 || type==4) {
		sym->dims = 1;	// for now, assume single dimension
	} else {
		sym->dims = 0;	// not an array
	}
	sym->disptype = DISP_DEFAULT;

	sym->next = table->next;
	table->next = sym;

	return sym;
}

static SYMBOL *find_symbol(SYMBOL *table, LPCSTR name, int level)
{
	SYMBOL *sym = table->next;

	while(sym != NULL) {
		if(sym->name == name && sym->calllevel == level) return sym;
		sym = sym->next;
	}
	return NULL;
}

static SYMBOL *find_symbol_addr(SYMBOL *table, ucell addr, int level)
{
	SYMBOL *sym = table->next;

	while(sym != NULL) {
		if(sym->addr == addr && sym->calllevel == level) return sym;
		sym = sym->next;
	}
	return NULL;
}

static void delete_symbol(SYMBOL *table, ucell addr, int level)
{
	SYMBOL *prev = table;
	SYMBOL *cur = prev->next;

	// Delete all local symbols below a certain address (these are local
	// variables from a function/block that one stepped out of). Also
	// remove all symbols of deeper calllevels (removes the static variables
	// at the end of a function).
	while(cur!=NULL) {
		if(cur->calllevel>level || cur->vclass==1 && cur->addr<addr) {
			prev->next = cur->next;
			delete cur;
			cur = prev->next;
		} else {
			prev = cur;
			cur = cur->next;
		}
	}
}

static void delete_allsymbols(SYMBOL *table)
{
	SYMBOL *sym = table->next, *next;

	while(sym != NULL) {
		next = sym->next;
		delete sym;
		sym = next;
	}
	table->next = NULL;
}


static LPCSTR get_string(AMX *amx, SYMBOL *sym, LPSTR string, int maxlength)
{
	char *ptr;
	cell *addr;
	cell base;
	int length, num;

	ASSERT(sym->type == 3 || sym->type == 4);
	ASSERT(sym->dims == 1);
	*string = '\0';

	// get the staring address and the length of the string
	base = sym->addr;
	if(sym->type==4) { // a reference
		amx_GetAddr(amx, base, &addr);
		base = *addr;
	}
	amx_GetAddr(amx, base, &addr);
	amx_StrLen(addr, &length);

	// allocate a temporary buffer
	ptr = new char[length + 1];
	if(ptr != NULL) {
		amx_GetString(ptr, addr);
		num = length;
		if(num >= maxlength) {
			num = maxlength - 1;
			if(num > 3) num -= 3; // make space for the ellipsis terminator
		}
		ASSERT(num>=0);
		strncpy(string, ptr, num);
		string[num] = '\0';
		if(num < length && num == maxlength - 3) strcat(string,"...");
		delete []ptr;
	}
	return string;
}

// Breakpoints stuff:

static int break_set(AMX *amx, LPCSTR str, int calllevel)
{
	SYMBOL *sym;

	// find an empty slot
	int size = breakpoints.size();
	for(int index=0; index<size; index++) {
		if(breakpoints[index].type == BP_NONE) break;
	}
	if(index == size) {
		int newsize = (size+1)*2;
		breakpoints.resize(newsize);
		for(int i=size; i<newsize; i++) {
			breakpoints[i].type = BP_NONE;
		}
	}

	ASSERT(breakpoints[index].type == BP_NONE);

	// Initialize the breakpoint:
	breakpoints[index].sym = NULL;
	breakpoints[index].index = -1;

	// find type
	str = skipwhitespace(str);
	if(isdigit(*str)) {
		breakpoints[index].type = BP_CODE;
		breakpoints[index].file = curfileno;
		breakpoints[index].addr = (ucell)atol(str);
	} else if((sym = find_symbol(&functab, str, -1)) != NULL) {
		breakpoints[index].type = BP_CODE;
		breakpoints[index].addr = sym->line;
		breakpoints[index].file = sym->file;
		breakpoints[index].sym = sym;
	} else {
		int idx = -1;
		char *idxptr = strchr(str, '[');
		if(idxptr != NULL) {
			idx = atoi(idxptr + 1);
			*idxptr = '\0';
		}
		if((sym = find_symbol(&vartab, str, calllevel)) != NULL) {
			if(sym->type==3 || sym->type==4) { // is an array:
				if(idxptr == NULL) return -1; // missing index on array
				if(sym->length[0]>0 && idx>=sym->length[0]) return -1; // index out of range
			}
			if(sym->type!=3 && sym->type!=4 && idxptr!=NULL) return -1;
			breakpoints[index].type = BP_DATA;
			breakpoints[index].addr = get_symbolvalue(amx, sym, (idx>=0)?idx:0);
			breakpoints[index].sym = sym;
			breakpoints[index].index = idx;
		} else {
			return -1;
		}
	}
	return index;
}

static int break_check(AMX *amx, int line, int file)
{
	for(int index=0; index<(int)breakpoints.size(); index++) {
		if( breakpoints[index].type == BP_CODE && 
			(int)breakpoints[index].addr == line
			&& breakpoints[index].file == file ) {
			return index;
		} else if (breakpoints[index].type == BP_DATA) {
			int idx = breakpoints[index].index;
			SYMBOL *sym = breakpoints[index].sym;
			ASSERT(sym != NULL);
			ucell value = get_symbolvalue(amx, sym, (idx>=0)?idx:0);
			if(breakpoints[index].addr != value) {
				breakpoints[index].addr = value;
				return index;
			}
		}
	}
	return -1;
}
*/
