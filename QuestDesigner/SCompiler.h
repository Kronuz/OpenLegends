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
/*! \file		SCompiler.h 
	\brief		Interface of the SCCompiler class.
	\date		April 15, 2003
*/

#pragma once
#include "Console.h"

/*! \class		SCompiler
	\brief		Compiler class.
	\author		Kronuz
	\version	1.0
	\date		April 15, 2003

	This singleton class works as an interface between the Small
	Compiler and the rest of the application.\n
	In order of this class to work properly, the public member
	SCompiler::ms_hWnd must be set with a valid window handle.
*/
class SCompiler {
	friend DWORD WINAPI ThreadProc( LPVOID lpParameter );

	static int ms_nDebug;
	static SCompiler *_instance;
	HANDLE m_Mutex;
	bool m_bInUse;

protected:
	int m_nTabSize;

	SCompiler();
public:
	static SCompiler* Instance();
	~SCompiler();

	int sc_printf(const char *format, va_list argptr); //!< Sends formatted output using a pointer to a list of arguments to the current handler window.
	int sc_error(int number,char *message,char *filename,int firstline,int lastline,va_list argptr);
	void *sc_opensrc(char *filename);
	void sc_closesrc(void *handle);
	void sc_resetsrc(void *handle,void *position);
	char *sc_readsrc(void *handle,char *target,int maxchars);
	void *sc_getpossrc(void *handle);
	int sc_eofsrc(void *handle);
	void *sc_openasm(char *filename);
	void sc_closeasm(void *handle, int deletefile);
	void sc_resetasm(void *handle);
	int sc_writeasm(void *handle,char *st);
	char *sc_readasm(void *handle, char *target, int maxchars);
	void *sc_openbin(char *filename);
	void sc_closebin(void *handle,int deletefile);
	void sc_resetbin(void *handle);
	int sc_writebin(void *handle,void *buffer,int size);
	long sc_lengthbin(void *handle);

	int Compile(LPCSTR szInludeDir, LPCSTR szSrcFile, LPCSTR szDestFile);
	void StartThread(int argc, char*argv[]);

};
