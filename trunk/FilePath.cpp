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
/*! \file		FilePath.cpp
	\brief		Implementation of the file manager classes.
	\date		April 28, 2003

	This file implements the CVFile to handle filenames, paths, and the virtual 
	file system. Also contains the path to the home directory of the game files.
*/

//#include "stdafx.h"
#include "FilePath.h"

CBString g_sHomeDir;

CVFile::CVFile() : 
	m_vFile(NULL), 
	m_File(NULL), 
	m_pBuffer(NULL),
	m_nExists(0), 
	m_bOpenFile(false), 
	m_bRelative(false), 
	m_bVirtual(false)
{
	m_BuffStart = NULL;
	m_BuffLen = 0;
	m_BuffRead = 0;
}
CVFile::CVFile(LPCSTR szNewName, bool bGlobalize) : 
	m_vFile(NULL), 
	m_File(NULL), 
	m_pBuffer(NULL),
	m_nExists(0), 
	m_bOpenFile(false), 
	m_bRelative(false), 
	m_bVirtual(false) 
{
	SetFilePath(szNewName, bGlobalize);
}
CVFile::~CVFile() 
{ 
	delete []m_pBuffer;
	if(m_vFile) unzClose(m_vFile); m_vFile = NULL; 
	if(m_File) fclose(m_File); m_File = NULL;
}
inline int wildcardcmp(const char *a, const char *b)
{
	int c = 0;
	while(*a && *b) {
		if(*a=='*' && *(a+1)==*b) a++;
		if(*a!=*b && *a!='*' && *a!='?') return *a-*b;
		if(*a!='*') a++;
		b++;
	}
	return *a-*b;
}
int CVFile::ForEachVirtualFile(FILESPROC ForEach, LPARAM lParam)
{
	int files=0;

	CBString sInFile = m_sPath + m_sTitle + m_sExt;
	CBString sFile = GetHomeFile();
	if(sFile=="") return 0;

	unzFile file = unzOpen(sFile);
	if(file==NULL) return 0;

	int err = unzGoToFirstFile(file);

	while(err == UNZ_OK) {
		char szCurrentFileName[MAX_PATH];
		err = unzGetCurrentFileInfo(file, NULL,
							szCurrentFileName, MAX_PATH-1,
							NULL, 0, NULL, 0);
		if(err == UNZ_OK) {
			if(wildcardcmp(sInFile, szCurrentFileName) == 0) {
				int cnt = ForEach(sFile + '\\' + szCurrentFileName, lParam);
				if(cnt < 0) {
					files = cnt-files;
					break;
				}
				files += cnt;

			}
			err = unzGoToNextFile(file);
		}
	}

	unzClose(file);
	return files;
}
int CVFile::ForEachFile(FILESPROC ForEach, LPARAM lParam)
{
	if(m_bVirtual) return ForEachVirtualFile(ForEach, lParam);

	int files=0;

	CBString sFileName = GetFilePath();

	char filename[_MAX_PATH];
	lstrcpy(filename, sFileName);
	LPTSTR fin = strchr(filename, '\0');

	while(fin!=filename && *fin!='\\') fin = CharPrev(filename, fin);
	if(*fin == '\\') fin = CharNext(fin);

	WIN32_FIND_DATA FindData;
	HANDLE FindHandle = FindFirstFile(sFileName, &FindData);
	if(INVALID_HANDLE_VALUE != FindHandle) {
		do {
			if (0 == lstrcmp(FindData.cFileName, "..") ||
				0 == lstrcmp(FindData.cFileName, ".")) continue;
			if(FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) continue;

			*fin = '\0';
			lstrcat(filename, FindData.cFileName);

			int cnt = ForEach(filename, lParam);
			if(cnt < 0) {
				files = cnt-files;
				break;
			}
			files += cnt;
		} while(FindNextFile(FindHandle, &FindData));
		FindClose(FindHandle);
	}

	return files;
}
