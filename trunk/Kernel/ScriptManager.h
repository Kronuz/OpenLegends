/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		ScriptManager.h 
	\brief		Interface of the classes that maintain and run scripts.
	\date		August 04, 2003
*/

#pragma once

#include <vector>

#include <amx.h>

#include "WorldManager.h"

#include "../IArchiver.h"
#include "../IGame.h"

#define S_SCRIPT_OK			0
#define E_SCRIPT_NOT_FOUND	-1
#define E_SCRIPT_BUGGY		-2

// just one extra thread for now... I don't want to mess with multiple exlusion,
// plus I don't think it's worth the time.
#define MAX_THREADS 1	

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CDebugScript;

typedef struct HSCRIPT__ {
	AMX amx;				// cloned amx
	BYTE *m_pData;			// data and stack segments
	CDebugScript *m_pDebug;	// debug information
	int nKilled;			// times the script has been killed
	int error;				// script termination error level
	DWORD ID;				// Script's ID or context
	DWORD dwTooLong;

	HSCRIPT__();
	~HSCRIPT__();
} *HSCRIPT;

class CScriptThread
{
	DWORD m_dwStartTime;	// Time when it started running
	bool m_bRun;			// Must the thread be running?
	bool m_bRunning;		// Is the script running
	bool m_bThreadUp;

	HANDLE m_hThread;
	HSCRIPT m_hNextScript;
	HSCRIPT m_hScript;

	HANDLE ScriptReady;
	HANDLE ScriptRunning;

	CRITICAL_SECTION XCritical;

	static DWORD WINAPI ExecScript(LPVOID lpParameter);
public:
	bool Ready();
	void RunScript(HSCRIPT hScript);
	bool CreateThread();
	void KillThread(DWORD dwMilliseconds = 0);
	CScriptThread();
	~CScriptThread();

};
/////////////////////////////////////////////////////////////////////////////
/*! \class		CScript
	\brief		The Scripts base class.
	\author		Kronuz
	\version	1.2
	\date		April 16, 2003
				August 04, 2003

	\todo 
*/
class CScript :
	public IScript
{
	// Static to all scripts:
	static CScriptThread Threads[MAX_THREADS];

	// Script dependent:
	AMX amx;
	BYTE *m_pProgram;
	CDebugScript *m_pDebug;	// debug information

	int m_nErrorLevel;

	bool m_bInitialized;
	int m_nKilled;

	mutable bool m_bCompiledFilePath;
	mutable CVFile m_fnCompiledFile;

	CVFile m_fnScriptFile;
	void MakeCompiledFile() const;

	typedef std::map<DWORD, HSCRIPT> Scripts;
	typedef std::pair<DWORD, HSCRIPT> pairScripts;
	Scripts m_Scripts; // Keeps a list of all the current script's clones

public:
	static volatile bool ms_StopWaiting;
	static bool ms_bDebug;
	static HANDLE Resources; // Thread resources availible
	static bool WaitScripts();
	static void StopWaiting();
	static bool isDebugging();

	CScript();
	~CScript();

	void SetSource(CBString sSource);
	void SetTitle(CBString sTitle);
	CBString GetTitle();

	bool SourceExists();
	bool CompiledExists();

	bool NeedToCompile() const;
	LPCSTR GetScriptFilePath(LPSTR szPath, size_t buffsize) const;
	LPCSTR GetCompiledFilePath(LPSTR szPath, size_t buffsize) const;

	HSCRIPT CloneScript();
	void DeleteScript(HSCRIPT hScript);

	bool RunScript(const CDrawableContext &context, RUNACTION action);
	bool InitScript();
};

inline void CScript::SetTitle(CBString sTitle)
{
	m_fnScriptFile.SetFileTitle(sTitle);
	if(m_bCompiledFilePath)
		m_fnCompiledFile.SetFileTitle(sTitle);
}
inline CBString CScript::GetTitle()
{
	return m_fnScriptFile.GetFileTitle();
}
inline void CScript::SetSource(CBString sSource)
{
	m_fnScriptFile.SetFilePath(sSource);
	m_bCompiledFilePath = false;
}
inline void CScript::MakeCompiledFile() const
{
	if(m_bCompiledFilePath == false) {
		m_fnCompiledFile.SetFilePath(m_fnScriptFile.GetPath() + "amx\\" + m_fnScriptFile.GetFileTitle() + ".amx");
		m_bCompiledFilePath = true;
	}
}
inline bool CScript::SourceExists()
{
	return m_fnScriptFile.FileExists();
}
inline bool CScript::CompiledExists()
{
	MakeCompiledFile();
	return m_fnCompiledFile.FileExists();
}
