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
/*! \file		SpriteManager.cpp
	\brief		Implementation of the classes that maintain and run scripts.
	\date		August 04, 2003
*/

#include "stdafx.h"
#include "ScriptManager.h"

#include "Script.h"

const char* AMXAPI amx_StrError(int error)
{
	static const char *errors[] = {
		"no error",
		"forced exit",
		"assertion failed",
		"stack/heap collision",
		"index out of bounds",
		"invalid memory access",
		"invalid instruction",
		"stack underflow",
		"heap underflow",
		"no callback, or invalid callback",
		"native function failed",
		"divide by zero",
		"go into sleepmode - code can be restarted",
		"reserved",
		"reserved",
		"reserved",
		"out of memory",
		"invalid file format",
		"file is for a newer version of the AMX",
		"function not found",
		"invalid index parameter (bad entry point)",
		"debugger cannot run",
		"AMX not initialized (or doubly initialized)",
		"unable to set user data field (table full)",
		"cannot initialize the JIT",
		"parameter error"
	};

	return errors[error];
}

int AMXAPI amx_ListUnresolved(AMX *amx, int number)
{
	AMX_FUNCSTUB *func;
	AMX_HEADER *hdr;
	int i,numnatives,unresolved;

	unresolved = 0;
	hdr=(AMX_HEADER *)amx->base;
	assert(hdr!=NULL);
	assert(hdr->natives<=hdr->libraries);
	numnatives=NUMENTRIES(*hdr,natives,libraries);

	func=(AMX_FUNCSTUB *)(amx->base+(int)hdr->natives);
	for(i=0; i<numnatives; i++) {
		if(func->address==0) {
			CONSOLE_PRINTF("Script error in %s: Unresolved native symbol '%s'.\n", amx->szFileName, func->name);
			/* this function is not yet located */
			unresolved++;
		} /* if */
		func++;
	} /* for */
	return unresolved;
}

// All thread resources should be initialized to be unavailible.
HANDLE CScript::Resources = CreateSemaphore( NULL, 0, MAX_THREADS, NULL );
CScriptThread CScript::Threads[MAX_THREADS];

CScriptThread::CScriptThread() : 
	m_bRun(false), 
	m_bThreadUp(false), 
	m_bRunning(false), 
	m_hThread(NULL), 
	m_hScript(NULL)
{ 
	InitializeCriticalSection(&XCritical);
	ScriptReady = CreateSemaphore( NULL, 0, 1, NULL );
}

CScriptThread::~CScriptThread() 
{
	KillThread(1000);
	DeleteCriticalSection(&XCritical);
	CloseHandle(ScriptReady);
}

DWORD WINAPI CScriptThread::ExecScript(LPVOID lpParameter) 
{
	CScriptThread *THIS = reinterpret_cast<CScriptThread *>(lpParameter);

	bool bRun = true;

	EnterCriticalSection(&THIS->XCritical);
	THIS->m_bThreadUp = true;
	LeaveCriticalSection(&THIS->XCritical);

	while(bRun) {
		ReleaseSemaphore(CScript::Resources, 1, NULL);	// make this resource availible
		WaitForSingleObject(THIS->ScriptReady, INFINITE); // wait for a script.
		
		EnterCriticalSection(&THIS->XCritical);
		THIS->m_bRunning = true;
		THIS->m_dwStartTime = GetTickCount();
		LeaveCriticalSection(&THIS->XCritical);
		
		CONSOLE_DEBUG("Running script '%s' (%x)...\n", THIS->m_hScript->amx.szFileName, THIS->m_hScript->ID);

		// Run the script here.
		cell ret = 0;
		int err = amx_Exec(&THIS->m_hScript->amx, &ret, AMX_EXEC_MAIN, 0);
		if(err == AMX_ERR_EXIT) {
			THIS->m_hScript->nKilled++;
			CONSOLE_PRINTF("Script error in '%s' (%x): Script took too long to finish and was forced to exit (%d ms)\n", THIS->m_hScript->amx.szFileName, THIS->m_hScript->ID, GetTickCount()-THIS->m_dwStartTime);
		} else if(err != AMX_ERR_NONE) {
			if(err != THIS->m_hScript->error) 
				CONSOLE_PRINTF("Script error in '%s' (%x): Run time error %d on line %ld: %s\n", THIS->m_hScript->amx.szFileName, THIS->m_hScript->ID, err, THIS->m_hScript->amx.curline, amx_StrError(err));
			ret = err;
		} else {
			(THIS->m_hScript->amx).firstrun = 0;
			if(ret != 0 && ret != THIS->m_hScript->error)
				CONSOLE_PRINTF("Script warning in '%s' (%x): Abnormal script termination: %d (0x%x)\n", THIS->m_hScript->amx.szFileName, THIS->m_hScript->ID, (int)ret, (int)ret);
		}
		THIS->m_hScript->error = ret; // save the termination error

		//Sleep(rand()%800 + 50);  // simulate a buggy script. (just for testing)

		CONSOLE_DEBUG("Script '%s' (%x) finished: took %d milliseconds to end\n", THIS->m_hScript->amx.szFileName, THIS->m_hScript->ID, GetTickCount()-THIS->m_dwStartTime);

		EnterCriticalSection(&THIS->XCritical);
		THIS->m_bRunning = false;
		bRun = THIS->m_bRun; // shall we continue running the thread?
		LeaveCriticalSection(&THIS->XCritical);
	}
	return 0;
}

bool CScriptThread::Ready() 
{
	if(m_hThread == NULL) CreateThread();

	DWORD dwDelta = 0;
	EnterCriticalSection(&XCritical);
	bool bReady = (m_bThreadUp && !m_bRunning);
	if(m_bThreadUp && m_bRunning) dwDelta = GetTickCount() - m_dwStartTime;
	LeaveCriticalSection(&XCritical);

	if(dwDelta >= 500) { // the thread seems to be locked, kill the thread.
		KillThread();
		if(m_hScript) m_hScript->nKilled++;
		CONSOLE_PRINTF("Script error in '%s' (%x): Script took too long to finish and thread was killed (%d ms)\n", m_hScript->amx.szFileName, m_hScript->ID, dwDelta);
		CreateThread(); // Recover the thread for future uses
	} else if(dwDelta >= 200) { // the script took too long to finish, try to abort.
		m_hScript->amx.flags |= AMX_FLAG_ABORT;
	} else if(dwDelta >= 10) { // taking too long....
		if(dwDelta > m_hScript->dwTooLong) {
			CONSOLE_PRINTF("Script warning in '%s' (%x): Script still running (it's taking too long to finish: %d ms)\n", m_hScript->amx.szFileName, m_hScript->ID, dwDelta);
			m_hScript->dwTooLong = dwDelta;
		}
	}
	return bReady;
}

void CScriptThread::RunScript(HSCRIPT hScript) 
{
	ASSERT(hScript);
	m_hScript = hScript;
	ReleaseSemaphore(ScriptReady, 1, NULL); // tell the thread the script is ready to run.
}

bool CScriptThread::CreateThread() 
{
	if(m_hThread == NULL) {
		m_bRun = true; // no thread, so mutex is not needed.
		m_bThreadUp = false;
		m_bRunning = false;
		DWORD dwThreadID = 0;
		m_hThread = ::CreateThread(NULL, 0, CScriptThread::ExecScript, (void*)this, 0, &dwThreadID);
	}
	return (m_hThread != NULL);
}

void CScriptThread::KillThread(DWORD dwMilliseconds) 
{
	if(m_hThread != NULL) {
		EnterCriticalSection(&XCritical);
		m_bRun = false;		
		m_bThreadUp = false;
		LeaveCriticalSection(&XCritical);

		bool bTerminateThread = false;
		if(dwMilliseconds) {
			if(WaitForSingleObject(m_hThread, dwMilliseconds) == WAIT_TIMEOUT) {
				bTerminateThread = true;
			}
		} else {
			bTerminateThread = true;
		}

		if(bTerminateThread) TerminateThread(m_hThread, 1);

		ASSERT(WaitForSingleObject(m_hThread, 2000) != WAIT_TIMEOUT);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

CScript::CScript() :
	m_pProgram(NULL), 
	m_bInitialized(false), 
	m_bCompiledFilePath(false),
	m_nErrorLevel(S_SCRIPT_OK),
	m_nKilled(0)
{
}
CScript::~CScript() 
{
	Scripts::iterator Iterator = m_Scripts.begin();
	while(Iterator!=m_Scripts.end()) {
		DeleteScript(Iterator->second);
	}
	m_Scripts.clear();
	delete []m_pProgram;
}

bool CScript::InitScript()
{
	if(m_bInitialized) return true;
	
	ASSERT(m_pProgram == NULL); 

	AMX_HEADER hdr;
	memset(&hdr, 0, sizeof(AMX_HEADER));

	MakeCompiledFile();

	if(!m_fnCompiledFile.Open()) return false;
	m_fnCompiledFile.Read(&hdr, sizeof(AMX_HEADER));
	m_fnCompiledFile.Seek(0, SEEK_SET);

	if(hdr.magic!=AMX_MAGIC) {
		m_fnCompiledFile.Close();
		return false; // invalid AMX file
	}
	m_pProgram = new BYTE[hdr.stp];
	m_fnCompiledFile.Read(m_pProgram, hdr.size);
	m_fnCompiledFile.Close();
	
	memset(&amx, 0, sizeof(AMX));
	if(amx_Init(&amx, m_pProgram) !=AMX_ERR_NONE) {
		delete []m_pProgram; m_pProgram = NULL;
		return false;
	}
	
	strncpy(amx.szFileName, m_fnCompiledFile.GetFileName(), sizeof(amx.szFileName)-1);

	RegisterNatives(&amx);

	// find out if there are any missing functions:
	if(amx_ListUnresolved(&amx, -1)) {
		delete []m_pProgram; m_pProgram = NULL;
		return false;
	}/**/
	m_bInitialized = true;
	return true;
}

HSCRIPT CScript::CloneScript()
{
	if(!m_bInitialized) return NULL;
	HSCRIPT hScript = new HSCRIPT__;
	long datasize=0, stackheap=0;
	amx_MemInfo(&amx, NULL, &datasize, &stackheap);
	hScript->m_pData = new BYTE[datasize + stackheap];
	amx_Clone(&hScript->amx, &amx, hScript->m_pData);

	return hScript;
}
void CScript::DeleteScript(HSCRIPT hScript)
{
	delete hScript;
}
bool CScript::RunScript(const CDrawableContext &context, RUNACTION action)
{
	if(m_nErrorLevel < 0) return false;

	if(!m_bInitialized) {
		if(!InitScript()) {
			if(m_nErrorLevel != E_SCRIPT_NOT_FOUND) {
				CONSOLE_PRINTF("Script fatal error in '%s': Unable to run script\n", m_fnCompiledFile.GetFileName());
				m_nErrorLevel = E_SCRIPT_NOT_FOUND;
			}
			return false;
		} else {
			CONSOLE_PRINTF("Script '%s' ready to run.\n", m_fnCompiledFile.GetFileName());
		}
	}

	// action.m_Ptr should have a pointer to the parent Map Group when not counting.
	CMapGroup *pMapGroupI = reinterpret_cast<CMapGroup*>(action.m_Ptr);

	HSCRIPT hScript = (HSCRIPT)context.m_pPtr;
	if(!hScript) {
		Scripts::iterator Iterator = m_Scripts.find((DWORD)&context);
		if(Iterator==m_Scripts.end()) {
			HSCRIPT hScript = CloneScript();
			if(hScript) {
				hScript->ID = (DWORD)&context;
				m_Scripts.insert(pairScripts(hScript->ID, hScript));
				context.m_pPtr = (LPVOID)hScript;
			}
		} else {
			hScript = Iterator->second;
		}
	}
	
	if(hScript) {
		if(hScript->nKilled) {
			if(hScript->nKilled==1) m_nKilled++;
			if(m_nKilled > 3 || hScript->nKilled > 5) {
				CONSOLE_PRINTF("Script fatal error in '%s': There must be something really buggy on this script.\n", m_fnCompiledFile.GetFileName());
				m_nErrorLevel = E_SCRIPT_BUGGY;
				return false;
			}
		}
		WaitForSingleObject(Resources, 300); // wait for a resource to free
		ReleaseSemaphore(Resources, 1, NULL);
	}
	while(hScript) {
		for(int i=0; i<MAX_THREADS; i++) {
			if(Threads[i].Ready()) {
				Threads[i].RunScript(hScript);
				hScript = NULL;
				break;
			}
		}
		if(hScript) Sleep(10);
	}

	m_nErrorLevel = S_SCRIPT_OK;
	return false;
}
void CScript::WaitScripts()
{
	WaitForSingleObject(Resources, 300); // wait for a resource to free
	ReleaseSemaphore(Resources, 1, NULL);

	int nReady;
	while(true) {
		nReady = 0;
		for(int i=0; i<MAX_THREADS; i++) {
			if(Threads[i].Ready()) nReady++;
		}
		if(nReady == MAX_THREADS) break;
		Sleep(10);
	}
}

bool CScript::NeedToCompile() const
{
	MakeCompiledFile();
	FILETIME *script = m_fnScriptFile.GetFileDate();
	FILETIME *compiled = m_fnCompiledFile.GetFileDate();
	if(!compiled) return true;
	ASSERT(script);
	long comp = CompareFileTime(script, compiled);
	return (comp>0);
}
LPCSTR CScript::GetScriptFilePath(LPSTR szPath, size_t buffsize) const
{
	strncpy(szPath, (LPCSTR)m_fnScriptFile.GetFilePath(), buffsize);
	return szPath;
}
LPCSTR CScript::GetCompiledFilePath(LPSTR szPath, size_t buffsize) const
{
	MakeCompiledFile();
	strncpy(szPath, (LPCSTR)m_fnCompiledFile.GetFilePath(), buffsize);
	return szPath;
}
