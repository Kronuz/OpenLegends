/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		FilePath.cpp
	\brief		Implementation of the file manager classes.
	\date		April 28, 2003

	This file implements the CVFile to handle filenames, paths, and the virtual 
	file system. Also contains the path to the home directory of the game files.
*/

#include <FilePath.h>

CBString g_sHomeDir;

CVFile::CVFile() : 
	m_vFile(NULL), 
	m_vzFile(NULL),
	m_File(NULL), 
	m_pBuffer(NULL),
	m_bOpenFile(false), 
	m_bRelative(false), 
	m_bVirtual(false),
	m_bRawMode(false),
	m_nMethod(Z_DEFLATED),
	m_nLevel(Z_DEFAULT_COMPRESSION),
	m_dwFileAttributes(0)
{
	m_BuffStart = NULL;
	m_BuffLen = -1;
	m_BuffOffset = -1;
}
CVFile::CVFile(LPCSTR szNewName, bool bGlobalize) : 
	m_vFile(NULL), 
	m_vzFile(NULL),
	m_File(NULL), 
	m_pBuffer(NULL),
	m_bOpenFile(false), 
	m_bRelative(false), 
	m_bVirtual(false),
	m_bRawMode(false),
	m_nMethod(Z_DEFLATED),
	m_nLevel(Z_DEFAULT_COMPRESSION),
	m_dwFileAttributes(0)
{
	SetFilePath(szNewName, bGlobalize);
}
CVFile::~CVFile() 
{ 
	delete []m_pBuffer;	m_pBuffer = NULL;
	if(m_vFile) unzClose(m_vFile); m_vFile = NULL; 
	if(m_vzFile) zipClose(m_vzFile, NULL); m_vzFile = NULL; 
	if(m_File) fclose(m_File); m_File = NULL;
}
inline int wildcardcmp(const char *a, const char *b)
{
	int c = 0;
	char ca = tolower(*a);
	char cb = tolower(*b);
	while(ca && cb) {
		if(ca=='*' && *(a+1)==cb) ca = tolower(*(++a));
		if(ca!=cb && ca!='*' && ca!='?') return ca-cb;
		if(ca!=cb && (ca=='\\' || ca=='/' || cb=='\\' || cb=='/')) return ca-cb;
		if(ca!='*') ca = tolower(*(++a));
		cb = tolower(*(++b));
	}
	return ca-cb;
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
		unz_file_info file_info;
		char szCurrentFileName[MAX_PATH];
		err = unzGetCurrentFileInfo(file, &file_info,
							szCurrentFileName, MAX_PATH-1,
							NULL, 0, NULL, 0);
		
		if(err == UNZ_OK) {
			char *pcLastChar = &(szCurrentFileName[strlen(szCurrentFileName)-1]);
			
			if(*pcLastChar == '\\' || *pcLastChar == '/') {
				*pcLastChar = '\0';
				file_info.external_fa |= FILE_ATTRIBUTE_DIRECTORY;
			} else if(file_info.external_fa == 0) file_info.external_fa |= FILE_ATTRIBUTE_NORMAL;

			if( /*(file_info.external_fa & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY ||*/
				wildcardcmp(sInFile, szCurrentFileName) == 0 ) {
				int cnt = ForEach(sFile + '\\' + szCurrentFileName, file_info.external_fa, lParam);
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

			*fin = '\0';
			lstrcat(filename, FindData.cFileName);

			int cnt = ForEach(filename, FindData.dwFileAttributes, lParam);
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
