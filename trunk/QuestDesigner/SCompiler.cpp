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

#include "stdafx.h"
#include "SCompiler.h"

#define NO_MAIN
#define SC_FUNC    static
#define SC_VDEFINE static
#define SC_SKIP_VDECL   // skip variable "forward declaration"
#define SC_LIGHT
#include "sc.h"

#include "scvars.c"
#include "sc1.c"
#include "sc2.c"
#include "sc3.c"
#include "sc4.c"
#include "sc5.c"
#include "sc6.c"
#include "sc7.c"
#include "sclist.c"
#include "scexpand.c"

////////////////////////////////////////////////
// Externals
SCompiler* SCompiler::_instance = NULL;

////////////////////////////////////////////////

int sc_printf(const char *message,...)
{
  int ret;
  va_list argptr;

  va_start(argptr,message);
  ret = CConsole::Instance()->print(message, argptr);
  va_end(argptr);

  return ret;
	
}
int sc_error(int number,char *message,char *filename,int firstline,int lastline,va_list argptr)
{
	return CConsole::Instance()->error(number,message,filename,firstline,lastline,argptr);
}
void *sc_opensrc(char *filename)
{
	return SCompiler::Instance()->sc_opensrc(filename);
}
void sc_closesrc(void *handle)
{
	SCompiler::Instance()->sc_closesrc(handle);
}
void sc_resetsrc(void *handle,void *position)
{
	SCompiler::Instance()->sc_resetsrc(handle,position);
}
char *sc_readsrc(void *handle,char *target,int maxchars)
{
	return SCompiler::Instance()->sc_readsrc(handle,target,maxchars);
}
void *sc_getpossrc(void *handle)
{
	return SCompiler::Instance()->sc_getpossrc(handle);
}
int sc_eofsrc(void *handle)
{
	return SCompiler::Instance()->sc_eofsrc(handle);
}
void *sc_openasm(char *filename)
{
	return SCompiler::Instance()->sc_openasm(filename);
}
void sc_closeasm(void *handle, int deletefile)
{
	SCompiler::Instance()->sc_closeasm(handle, deletefile);
}
void sc_resetasm(void *handle)
{
	SCompiler::Instance()->sc_resetasm(handle);
}
int sc_writeasm(void *handle,char *st)
{
	return SCompiler::Instance()->sc_writeasm(handle,st);
}
char *sc_readasm(void *handle, char *target, int maxchars)
{
	return SCompiler::Instance()->sc_readasm(handle, target, maxchars);
}
void *sc_openbin(char *filename)
{
	return SCompiler::Instance()->sc_openbin(filename);
}
void sc_closebin(void *handle,int deletefile)
{
	SCompiler::Instance()->sc_closebin(handle,deletefile);
}
void sc_resetbin(void *handle)
{
	SCompiler::Instance()->sc_resetbin(handle);
}
int sc_writebin(void *handle,void *buffer,int size)
{
	return SCompiler::Instance()->sc_writebin(handle,buffer,size);
}
long sc_lengthbin(void *handle)
{
	return SCompiler::Instance()->sc_lengthbin(handle);
}

/* sc_opensrc
 * Opens a source file (or include file) for reading. The "file" does not have
 * to be a physical file, one might compile from memory.
 *    filename    the name of the "file" to read from
 * Return:
 *    The function must return a pointer, which is used as a "magic cookie" to
 *    all I/O functions. When failing to open the file for reading, the
 *    function must return NULL.
 */
void *SCompiler::sc_opensrc(char *filename)
{
  return fopen(filename,"rt");
}

/* sc_closesrc
 * Closes a source file (or include file). The "handle" parameter has the
 * value that sc_opensrc() returned in an earlier call.
 */
void SCompiler::sc_closesrc(void *handle)
{
  assert(handle!=NULL);
  fclose((FILE*)handle);
}

/* sc_resetsrc
 * "position" may only hold a pointer that was previously obtained from
 * sc_getpossrc() */
void SCompiler::sc_resetsrc(void *handle,void *position)
{
  assert(handle!=NULL);
  fsetpos((FILE*)handle,(fpos_t *)position);
}

char *SCompiler::sc_readsrc(void *handle,char *target,int maxchars)
{
  return fgets(target,maxchars,(FILE*)handle);
}

void *SCompiler::sc_getpossrc(void *handle)
{
  static fpos_t lastpos;

  fgetpos((FILE*)handle,&lastpos);
  return &lastpos;
}

int SCompiler::sc_eofsrc(void *handle)
{
  return feof((FILE*)handle);
}

/* should return a pointer, which is used as a "magic cookie" to all I/O
 * functions; return NULL for failure
 */
void *SCompiler::sc_openasm(char *filename)
{
  return fopen(filename,"w+t");
}

void SCompiler::sc_closeasm(void *handle, int deletefile)
{
  fclose((FILE*)handle);
  if (deletefile)
    unlink(outfname);
}

void SCompiler::sc_resetasm(void *handle)
{
  fflush((FILE*)handle);
  fseek((FILE*)handle,0,SEEK_SET);
}

int SCompiler::sc_writeasm(void *handle,char *st)
{
  return fputs(st,(FILE*)handle) >= 0;
}

char *SCompiler::sc_readasm(void *handle, char *target, int maxchars)
{
  return fgets(target,maxchars,(FILE*)handle);
}

/* Should return a pointer, which is used as a "magic cookie" to all I/O
 * functions; return NULL for failure.
 */
void *SCompiler::sc_openbin(char *filename)
{
  return fopen(filename,"wb");
}

void SCompiler::sc_closebin(void *handle,int deletefile)
{
  fclose((FILE*)handle);
  if (deletefile)
    unlink(binfname);
}

void SCompiler::sc_resetbin(void *handle)
{
  fflush((FILE*)handle);
  fseek((FILE*)handle,0,SEEK_SET);
}

int SCompiler::sc_writebin(void *handle,void *buffer,int size)
{
  return fwrite(buffer,1,size,(FILE*)handle) == size;
}

long SCompiler::sc_lengthbin(void *handle)
{
  return ftell((FILE*)handle);
}

SCompiler* SCompiler::Instance()
{
	if(_instance == NULL) {
		_instance = new SCompiler;
	}
	return _instance;
}

SCompiler::SCompiler():
m_bInUse(false),
m_iTabSize(3)
{
	m_Mutex = CreateMutex( NULL, FALSE, NULL );
}
SCompiler::~SCompiler()
{
	CloseHandle(m_Mutex);
}

struct _thrdParam {
	int argc;
	char **argv;
};
DWORD WINAPI ThreadProc( LPVOID lpParameter )
{
	_thrdParam *pTPT = (_thrdParam *)lpParameter;
	SCompiler *pCompiler = SCompiler::Instance();

	WaitForSingleObject(pCompiler->m_Mutex, INFINITE);
	pCompiler->m_bInUse = true;
	::PostMessage(CConsole::ms_hWnd, WMQD_STEPBEGIN, 0, 0);
	sc_compile(pTPT->argc, pTPT->argv);
	pCompiler->m_bInUse = false;
	::PostMessage(CConsole::ms_hWnd, WMQD_STEPEND, 0, 0);
	ReleaseMutex(pCompiler->m_Mutex);

	for(int i=0; i<pTPT->argc; i++) 
		delete []pTPT->argv[i];
	delete []pTPT->argv;
	delete pTPT;
	return 0;
}

void SCompiler::StartThread(int argc, char *argv[])
{
	_thrdParam *pTPT = new _thrdParam;
	pTPT->argc = argc;
	pTPT->argv = argv;

	CreateThread(NULL, 0, (PTHREAD_START_ROUTINE)(void*)ThreadProc, (void*)pTPT, 0, NULL);
}

int SCompiler::Compile(LPCSTR szInludeDir, LPCSTR szSrcFile, LPCSTR szDestFile)
{
	int argc = 0;
	char **argv;
	char buff[_MAX_PATH];
	buff[0] = '-';

	if(m_bInUse) return 1;

	// Printing a message
	char *aux = strrchr(szSrcFile, '\\');
	if(!aux) aux = (char *)szSrcFile;
	else aux++;
	::sc_printf("%s\n", aux);

	argv = new char*[10];
	argv[argc] = new char[strlen("QuestDesigner")+1];
	strcpy(argv[argc++], "QuestDesigner");

	argv[argc] = new char[strlen(szSrcFile)+1];
	strcpy(argv[argc++], szSrcFile);

	if(szDestFile) {
		buff[1] = 'o'; // Output file option
		strcpy(&buff[2], szDestFile);
		argv[argc] = new char[strlen(buff)+1];
		strcpy(argv[argc++], buff);
	}

	if(szInludeDir) {
		buff[1] = 'i'; // Includes directory option
		strcpy(&buff[2], szInludeDir);
		argv[argc] = new char[strlen(buff)+1];
		strcpy(argv[argc++], buff);
	}

	if(m_iTabSize != 8) {
		buff[1] = 't'; // Includes directory option
		sprintf(&buff[2], "%d", m_iTabSize);
		argv[argc] = new char[strlen(buff)+1];
		strcpy(argv[argc++], buff);
	}

	StartThread(argc, argv);

	return 0;
}


