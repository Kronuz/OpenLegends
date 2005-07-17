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
/*! \file		Debugger.h
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the debugging server.
	\date		October 31, 2003
*/

#pragma once

#include <amx.h>

#include <map>
#include <list>
#include <vector>
#include <string>

#include <Net.h>

struct CDebugFile;
///////////////////////////////////////////////////////////////////////////////////
// Debugging stuff:

#define NAMEMAX         32      // length of variable/function names
#define MAX_DIMS        2       // number of array dimensions
#define MAXFILES        32

enum _vartype {
	DISP_DEFAULT,
	DISP_STRING,
	DISP_BIN,
	DISP_HEX,
	DISP_FIXED,
	DISP_FLOAT,
	///////////
	DISP_TYPES
};

enum __commands {
	NONE,
	QUIT,
	STOP,
	GO,
	STEP_OVER,
	STEP_IN,
	STEP_OUT
};

enum _bptype {
	BP_NONE,
	BP_CODE,
	BP_DATA,
	/* --- */
	BP_TYPES
};

typedef struct __symbol {
	std::string name;
	ucell addr;
	int file;             // file number that a function appears in
	int line;             // line number that a function starts on
	int vclass;           // variable class (global, local, static)
	int type;             // symbol type (local var/global var/function
	int calllevel;        // nesting level (visibility) of the variable
	int length[MAX_DIMS]; // for arrays
	int dims;             // for arrays
	int disptype;         // display type
	__symbol() : addr(NULL), file(-1), line(-1) {}
} SYMBOL;

typedef struct __breakpoint {
	CDebugFile *file;
	_bptype type;		// one of the BP_xxx types
	ucell addr;			// line number, or previous value
	int index;
	SYMBOL *sym;
	__breakpoint() : addr(0), type(BP_NONE), index(-1), sym(NULL) {}
} BREAKPOINT;

struct CDebugFile {
	std::string m_sName;
	static std::vector<std::string> ms_Watches;
	static std::vector<BREAKPOINT> ms_Breakpoints;

	CDebugFile(LPCSTR szName) : m_sName(szName) {}
};

class CDebugScript {
protected:
	// Sockets and debugging (common to all debugged scripts):
	static HANDLE ms_hLines;
	static std::list<std::string> ms_LinesToSend; // Lines to send to the client.
	static SOCKET ms_Socket;
	static int ms_nScripts;
	static CRITICAL_SECTION ms_SendCritical;

public:
	static HANDLE ms_hCommand;
	static CRITICAL_SECTION ms_DebugCritical;
	static CDebugScript *ms_DebuggingScript;
	static volatile __commands ms_eCommand;
	static volatile bool ms_bBreakRequest;	// immediate break
	static volatile bool ms_bInterBreak;

	static bool BeginSend();
	static bool Send(LPCSTR format, ...); // send to the client
	static bool EndSend();

protected:
	static bool __Send(LPCSTR format, va_list argptr);
	static void Dispatch(LPCSTR szCommand);
	static void ListCommands(LPCSTR szCommand = NULL);

	// CDebugScript stuff:
	typedef std::pair<std::string, CDebugFile*> pairDebugFiles;
	typedef std::map<std::string, CDebugFile*> mapDebugFiles;
	static mapDebugFiles ms_DebugFiles; // debugged files (share breakpoints and stuff)

	typedef std::pair<int, CDebugFile*> pairFiles;
	typedef std::map<int, CDebugFile*> mapFiles;
	mapFiles m_Files;	// index of files for the current script/context.

	CDebugScript *m_pDebugCreator;

	std::vector<SYMBOL *> m_Functions;		// table of functions.
	std::vector<SYMBOL *> m_Variables;		// table of variables.
public:
	static int CALLBACK Dispatch(SOCKET s);
	static int CALLBACK Request(SOCKET s);

	inline CDebugFile* GetFile(int num) {
		CDebugFile *pFile = NULL;
		mapFiles::iterator Iterator = m_Files.find(num);
		if(Iterator != m_Files.end()) pFile = Iterator->second;
		else { // if not found, look in the parent:
			Iterator = m_pDebugCreator->m_Files.find(num);
			if(Iterator != m_pDebugCreator->m_Files.end()) pFile = Iterator->second;
		}
		return pFile;
	}

	std::string m_sName; // script name (or ID)
	HANDLE m_hSemaphore;
	bool m_bDebugging;
	DWORD m_dwStartDebug;
	DWORD m_dwDebugTime;
	std::vector<ucell> m_CallStack;			// calling stack is kept here.

	// Current status is kept here:
	int m_nCurrentFile;
	int m_nCurrentLine;
	SYMBOL *m_pCurrentFunction;
	SYMBOL *m_pCurrentVariable;

	// Other Breakpoints:
	int m_nTraceLevel;		// trace level on which to break
	int m_nStopLine;		// "Go to line" breakpoint

	CDebugScript(CDebugScript *pDebugCreator, HANDLE hSemaphore);
	~CDebugScript();
	int BreakCheck(AMX *amx, int line, int file);

	SYMBOL* AddSymbol(std::vector<SYMBOL *> &table, LPCSTR name, int type, ucell addr, int vclass, int level);
	SYMBOL* FindSymbol(std::vector<SYMBOL *> &table, LPCSTR name, int level) {return NULL;}
	bool DeleteSymbol(std::vector<SYMBOL *> &table, ucell addr, int level);

	bool AddFile(int num, LPCSTR name);

	inline bool AddFunction(LPCSTR name, ucell addr) {
		m_pCurrentFunction = AddSymbol(m_Functions, name, 9, addr, 0, -1);
		return (m_pCurrentFunction!=NULL);
	}
	inline bool AddVariable(LPCSTR name, int type, ucell addr, int vclass, int level) {
		m_pCurrentVariable = AddSymbol(m_Variables, name, type, addr, vclass, level);
		return (m_pCurrentVariable!=NULL);
	}
	inline SYMBOL* FindFunction(LPCSTR name, int level) {
		return FindSymbol(m_Functions, name, level);
	}
	inline SYMBOL* FindVariable(LPCSTR name, int level) {
		return FindSymbol(m_Variables, name, level);
	}
	inline bool DeleteVariable(ucell addr) {
		return DeleteSymbol(m_Variables, addr, m_CallStack.size());
	}
};

int AMXAPI amx_InternalDebugProc(AMX *amx);
