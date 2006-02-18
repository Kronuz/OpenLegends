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
/*! \file		SpriteManager.cpp
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the classes that maintain and run scripts.
	\date		August 04, 2003:
					* Initial addition to OL.
				November 01, 2003:
					* Debugger added.
				Jan-Feb, 2006:
					* Extensive modification of script threading.
				February 12, 2006:
					* Added saving of pointer to current script handler in the AMX.
*/

#include "stdafx.h"
#include "ScriptManager.h"

#include "Script.h"
#include "Debugger.h"

HSCRIPT__::HSCRIPT__() 
{
	m_pDebug = NULL;
	m_pData = NULL;
	nKilled = 0;
	dwTooLong = 0;
	memset(&amx, 0, sizeof(AMX));
}

HSCRIPT__::~HSCRIPT__() 
{
	BEGIN_DESTRUCTOR
	delete []m_pData;
	delete m_pDebug;
	END_DESTRUCTOR
}

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
		"file is for an unsupported version of the AMX",
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
	ASSERT(hdr!=NULL);
	ASSERT(hdr->natives<=hdr->libraries);
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

CScriptThread CScript::Threads[MAX_THREADS];
std::vector<HSCRIPT> CScript::ScriptQueue;
bool CScript::m_bNewQueue = true;
HANDLE CScript::m_hHandlerThread = NULL;
int CScript::m_iHandlerUsage = 0;
CRITICAL_SECTION CScript::XCritical;
bool CScript::m_bClearing = true;

bool CScript::ms_bDebug  = false; // debug by default


//Handle the queue
void CScript::NewQueue(){
	m_bNewQueue = true;
}
bool CScript::QueueAccepting(){
	m_bClearing = false;
	return m_bNewQueue;
}
void CScript::QueueFull(){
	m_bNewQueue = false;
	if(m_hHandlerThread == NULL) CScript::Threads[0].CreateThread(); //Create the first thread to initialize the thread handler.
}

void CScript::CleanQueue(){
	if(ScriptQueue.begin() != ScriptQueue.end()) ScriptQueue.clear();
	m_bClearing = true;
}

void CScript::CreateHandler(){
	if(m_hHandlerThread == NULL) m_hHandlerThread = ::CreateThread(NULL, 0, CScript::HandlerThread, NULL, 0, NULL);
	m_iHandlerUsage++;
}

void CScript::KillHandler(){
	if(--m_iHandlerUsage == 0) m_hHandlerThread = NULL; //Handler thread will die on it's own.
}

DWORD WINAPI CScript::HandlerThread(LPVOID lpParameter){
	while(m_iHandlerUsage > 0){
		for(int i=0; i<MAX_THREADS; i++){
			Threads[i].Ready(); //Do a ready-check to find locked threads.
			//The previous engine stored the execution time of each object and printed it, we don't want that, it's slow^2.
		}
		Sleep(10);	//Do the ready checks every 100ms to find any locked threads.
	}
	return 0;
}

CScriptThread::CScriptThread() : 
	m_bRun(false), 
	m_bThreadUp(false),  
	m_bRunning(false),
	m_hThread(NULL), 
	m_hScript(NULL)
{ 
	InitializeCriticalSection(&XCritical);
	CScript::CreateHandler();
}

CScriptThread::~CScriptThread() 
{
	BEGIN_DESTRUCTOR
	KillThread(1000);
	DeleteCriticalSection(&XCritical);
	CScript::KillHandler();
	END_DESTRUCTOR
}

// Running thread of scripts
DWORD WINAPI CScriptThread::ExecScript(LPVOID lpParameter) 
{
	CScriptThread *THIS = reinterpret_cast<CScriptThread *>(lpParameter);

	bool bRun = true;

	EnterCriticalSection(&THIS->XCritical);
		THIS->m_bThreadUp = true;
	LeaveCriticalSection(&THIS->XCritical);

	while(bRun) {

		//When we're out of scripts we should create a new queue and wait for it to fill up.
		while(CScript::ScriptQueue.begin() == CScript::ScriptQueue.end() || CScript::QueueAccepting()){
			if(!CScript::QueueAccepting()) CScript::NewQueue(); else Sleep(1);
		}
		EnterCriticalSection(&CScript::XCritical);					//Critical section, get and erase script from vector.									
			THIS->m_hScript = *CScript::ScriptQueue.begin();
			CScript::ScriptQueue.erase(CScript::ScriptQueue.begin());
		LeaveCriticalSection(&CScript::XCritical);

		EnterCriticalSection(&THIS->XCritical);
			THIS->m_dwStartTime = GetTickCount();
			THIS->m_bRunning = true;
			if(THIS->m_hScript->m_pDebug) THIS->m_hScript->m_pDebug->m_dwDebugTime = 0;
		LeaveCriticalSection(&THIS->XCritical);

		// Run the script here.
		cell ret = 0;
		//Store the pointer to the script in the AMX-data.
		amx_SetUserData(&(THIS->m_hScript->amx), HSCRIPTPOINTER, (void *)&(THIS->m_hScript));
		int err = amx_Exec(&(THIS->m_hScript->amx), &ret, AMX_EXEC_MAIN, 0);
		DWORD dwDelta = GetTickCount() - THIS->m_dwStartTime;
		if(THIS->m_hScript->m_pDebug) {
			dwDelta -= THIS->m_hScript->m_pDebug->m_dwDebugTime;
		}

		if(err == AMX_ERR_EXIT) {
			THIS->m_hScript->nKilled++;
			CONSOLE_PRINTF("Script error in '%s' (%x): Script took too long to finish and was forced to exit (%d ms)\n", THIS->m_hScript->amx.szFileName, THIS->m_hScript->ID, dwDelta);
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

		#ifdef _DEBUG
			//Sleep(rand()%800 + 50);  // simulate a buggy script. (just for testing) -- Damn you, debug output is enough for simulating that! / Littlebuddy
			/*if(THIS->m_hScript->m_pDebug) {
				CONSOLE_DEBUG("Script '%s' (%x) finished: took %d milliseconds to end (%d ms debugging)\n", THIS->m_hScript->amx.szFileName, THIS->m_hScript->ID, dwDelta, THIS->m_hScript->m_pDebug->m_dwDebugTime);
			} else {
				CONSOLE_DEBUG("Script '%s' (%x) finished: took %d milliseconds to end\n", THIS->m_hScript->amx.szFileName, THIS->m_hScript->ID, dwDelta);
			}*/
		#endif

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
	bool bReady = m_bThreadUp;
	if(m_bRunning) {
		dwDelta = GetTickCount();
		if(m_hScript && m_hScript->m_pDebug) {
			if(m_hScript->m_pDebug->m_bDebugging) {
				dwDelta = m_hScript->m_pDebug->m_dwStartDebug;
			} 
			dwDelta -= m_hScript->m_pDebug->m_dwDebugTime;
		} 
		dwDelta -= m_dwStartTime;
	}
	LeaveCriticalSection(&XCritical);

	// dwDelta in milliseconds
	// These suck hard when debugging - Littlebuddy
	if(dwDelta >= 500) { // the thread seems to be locked, kill the thread.
		KillThread();
		if(m_hScript) m_hScript->nKilled++;
		CONSOLE_PRINTF("Script error in '%s' (%x): Script took too long to finish and thread was killed (%d ms)\n", m_hScript->amx.szFileName, m_hScript->ID, dwDelta);
		CreateThread(); // Recover the thread for future uses
	} else if(dwDelta >= 200) { // the script took too long to finish, try to abort.
		CONSOLE_PRINTF("Script warning in '%s' (%x): Trying to abort (it's taking too long to finish: %d ms)\n", m_hScript->amx.szFileName, m_hScript->ID, dwDelta);
		m_hScript->amx.flags |= AMX_FLAG_ABORT; // set the abort flag of the scrip.
	} else if(dwDelta >= 10) { // taking too long....
		if(dwDelta > m_hScript->dwTooLong) {
			CONSOLE_PRINTF("Script warning in '%s' (%x): Script still running (it's taking too long to finish: %d ms)\n", m_hScript->amx.szFileName, m_hScript->ID, dwDelta);
			m_hScript->dwTooLong = dwDelta;
		}
	}
	return bReady;
}

bool CScriptThread::CreateThread() 
{
	if(m_hThread == NULL) {
		m_bRun = true; // no thread, so mutex is not needed.
		m_bThreadUp = false;
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
		m_bRunning = false;
		LeaveCriticalSection(&XCritical);

		bool bTerminateThread = false;
		if(dwMilliseconds) {
			if(WaitForSingleObject(m_hThread, dwMilliseconds) == WAIT_TIMEOUT) {
				bTerminateThread = true;
			}
		} else {
			bTerminateThread = true;
		}

		if(bTerminateThread) {
			MessageBeep((UINT)-1);
			TerminateThread(m_hThread, 1);
			CONSOLE_DEBUG("Thread killed at: %s (%d)", __FILE__, __LINE__);
		}

		ASSERT(WaitForSingleObject(m_hThread, 2000) == WAIT_OBJECT_0);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

CScript::CScript() :
	m_pProgram(NULL), 
	m_pDebug(NULL),
	m_bInitialized(false), 
	m_bCompiledFilePath(false),
	m_nErrorLevel(S_SCRIPT_OK),
	m_nKilled(0),
	m_bRunScript(true)
{
}
CScript::~CScript() 
{
	BEGIN_DESTRUCTOR
	Scripts::iterator Iterator = m_Scripts.begin();
	while(Iterator!=m_Scripts.end()) {
		DeleteScript(Iterator->second);
	}
	m_Scripts.clear();
	delete []m_pProgram;
	delete m_pDebug;
	END_DESTRUCTOR
}

bool CScript::InitScript()
{
	if(m_bInitialized || !m_bRunScript){
		m_bInitialized = true;
		return true;
	}
	
	ASSERT(m_pProgram == NULL && m_pDebug == NULL); 

	AMX_HEADER hdr;
	memset(&hdr, 0, sizeof(AMX_HEADER));

	MakeCompiledFile();

	if(!m_fnCompiledFile.Open("r")) {
		CONSOLE_PRINTF("Script error in '%s': File not found\n", (LPCSTR)m_fnCompiledFile.GetFileName());
		return false;
	}
	m_fnCompiledFile.Read(&hdr, sizeof(AMX_HEADER));
	m_fnCompiledFile.Seek(0, SEEK_SET);

	if(hdr.magic!=AMX_MAGIC) {
		m_fnCompiledFile.Close();
		return false; // invalid AMX file
	}
	m_pProgram = new BYTE[hdr.stp];
	m_fnCompiledFile.Read(m_pProgram, hdr.size);
	m_fnCompiledFile.Close();
	

	int err = AMX_ERR_NONE;
	memset(&amx, 0, sizeof(AMX));
	if(ms_bDebug) {
		m_pDebug = new CDebugScript(NULL, NULL); // create a debug class if needed
		err = amx_SetDebugHook(&amx, amx_InternalDebugProc, (cell)&m_pDebug);
	}
	if(err == AMX_ERR_NONE) err = amx_Init(&amx, m_pProgram);

	if(err != AMX_ERR_NONE) {
		CONSOLE_PRINTF("Script error in '%s': %s (%d)\n", (LPCSTR)m_fnCompiledFile.GetFileName(), amx_StrError(err), err);
		delete []m_pProgram; m_pProgram = NULL;
		delete m_pDebug; m_pDebug = NULL;
		return false;
	}
	
	strncpy(amx.szFileName, m_fnCompiledFile.GetFileName(), sizeof(amx.szFileName)-1);

	RegisterNatives(&amx);

	// find out if there are any missing functions:
	if(amx_ListUnresolved(&amx, -1)) {
		delete []m_pProgram; m_pProgram = NULL;
		delete m_pDebug; m_pDebug = NULL;
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
	hScript->amx.param = (cell)&(hScript->m_pDebug); // set the parent of the AMX
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

	if(!m_bRunScript){
		m_nErrorLevel = S_SCRIPT_OK;
		return false;
	}

	HSCRIPT hScript = (HSCRIPT)context.m_pPtr;
	if(!hScript) {
		Scripts::iterator Iterator = m_Scripts.find((DWORD)&context);
		if(Iterator==m_Scripts.end()) {
			HSCRIPT hScript = CloneScript();
			if(hScript) {
				hScript->ID = (DWORD)&context;
				m_Scripts.insert(pairScripts(hScript->ID, hScript));
				context.m_pPtr = (LPVOID)hScript;
				if(ms_bDebug) {
					hScript->m_pDebug = new CDebugScript(m_pDebug, action.hSemaphore);
					LPCSTR szName = (LPCSTR)context.GetName();
					char buff[20];
					sprintf(buff, "0x%08X", hScript->ID);
					hScript->m_pDebug->m_sName = buff;
					if(*szName && *szName!='*') {
						hScript->m_pDebug->m_sName += " ('";
						hScript->m_pDebug->m_sName += szName;
						hScript->m_pDebug->m_sName += "')";
					}
				}
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
	}


	if(QueueAccepting() && hScript){
		ScriptQueue.push_back(hScript); //Place the script in the queue.
	}

	m_nErrorLevel = S_SCRIPT_OK;
	return false;
}

void CScript::StopWaiting()
{
	Disconnect();
}
bool CScript::isDebugging()
{
	if(!ms_bDebug) return false;
	return !Connected();
}
// check if the script needs to be compiled (based on the modification date of the files)
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
	strncpy(szPath, (LPCSTR)m_fnScriptFile.GetAbsFilePath(), buffsize);
	return szPath;
}

LPCSTR CScript::GetCompiledFilePath(LPSTR szPath, size_t buffsize) const
{
	MakeCompiledFile();
	strncpy(szPath, (LPCSTR)m_fnCompiledFile.GetAbsFilePath(), buffsize);
	return szPath;
}
