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
/*! \file		FilePath.h 
	\brief		Interface of the classes that maintain filenames and paths.
	\date		April 28, 2003

	This file implements the CVFile to handle filenames and paths. Also
	contains the path to the home directory of the game files.
*/

#pragma once

#include <zip.h>
#include <unzip.h>

#include "Misc.h"

#define DIR_SEP '\\'
#define VBUFFER_SIZE (16*1024)

extern CBString g_sHomeDir;

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CVFile;

// Virtual file system
class CVFile {

	mutable FILETIME m_FileDate;

	// virtual buffer:
	LPBYTE m_pBuffer;
	LPSTR m_BuffStart;
	int m_BuffLen;
	int m_BuffRead;

	mutable int m_nExists;	// -1 = not existent, 0 = ??, 1 = file exists
	bool m_bRelative;
	bool m_bVirtual;
	bool m_bOpenFile;

	unzFile m_vFile;	// compressed file (if virtual)
	FILE *m_File;		// regular file

	// HomeFile is not used if the file is not virtual (i.e. compressed)
	CBString m_sHomeFile;	// initial home directory (for compressed files)

	CBString m_sPath;		// path inside home (without the file name)
	CBString m_sTitle;		// filename (without extension)
	CBString m_sExt;			// file name extension

	CBString m_sDesc;		// file description is the file tile until changed.

	FILETIME* GetFileTime(LPCSTR lpFileName) const;
	bool FileExists(LPCSTR szFile) const;
	bool FindVirtual();
	bool VirtualFileExists() const;

	bool OpenVirtual();
	bool CloseVirtual();
	int ReadVirtual(LPVOID buffer, size_t buffsize);
	int WriteVirtual(LPVOID buffer, size_t buffsize);
	int SeekVirtual(long offset, int origin);
	long TellVirtual() const;
	LPSTR GetLineVirtual(LPSTR string, int n);
	size_t GetVirtualFileSize() const;
	FILETIME* GetVirtualFileDate() const;
	int ForEachVirtualFile(FILESPROC ForEach, LPARAM lParam);
public:
	CVFile();
	CVFile(LPCSTR szNewName, bool bGlobalize = false);
	~CVFile();

	CBString GetFileTitle() const;
	CBString GetFileExt() const;
	CBString GetFileName() const;
	CBString GetPath() const;
	CBString GetHomeFile() const;
	CBString GetFilePath() const;
	CBString GetFileDesc() const;

	void SetFileDesc(LPCSTR szNewDesc);
	void SetFilePath(LPCSTR szNewName, bool bGlobalize = false);
	void SetFileTitle(LPCSTR szNewName);
	void SetFileExt(LPCSTR szNewExt);

	bool FileExists() const;

// Operations with the files:
	bool Open();
	bool Close();

	int ForEachFile(FILESPROC ForEach, LPARAM lParam);
	// reads the complete file to a buffer or an internal buffer
	LPCVOID ReadFile(LPVOID buffer = NULL, size_t buffsize = 0);

	int Read(LPVOID buffer, size_t buffsize);
	int Write(LPVOID buffer, size_t buffsize);
	int Seek(long offset, int origin);
	long Tell() const;
	LPSTR GetLine(LPSTR string, int n);
	FILETIME* GetFileDate() const;
	size_t GetFileSize() const;
};
inline FILETIME* CVFile::GetFileTime(LPCSTR lpFileName) const
{
	WIN32_FIND_DATA FindData;
	HANDLE FindHandle = FindFirstFile(lpFileName, &FindData);
	if(INVALID_HANDLE_VALUE != FindHandle) {
		if(FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) return NULL;
		m_FileDate = FindData.ftLastWriteTime;
		return &m_FileDate;
	}
	return NULL;
}
inline bool CVFile::FileExists(LPCSTR lpFileName) const
{
	return (GetFileTime(lpFileName)!=NULL);
}
inline bool CVFile::FindVirtual() 
{
	char szPath[_MAX_DIR];
	strcpy(szPath, m_sPath);
	char *tmp;
	char *aux = strrchr(szPath, DIR_SEP);
	while(aux) {
		tmp = aux;
		*tmp = '\0';

		bool bExists = false;
		// try to open path as a compressed file
		if(m_bRelative) {
			bExists = FileExists(g_sHomeDir + szPath);
		} else {
			bExists = FileExists(szPath);
		}
		if(bExists) {
			// split the path and make the file virtual.
			m_bVirtual = true;
			m_nExists = 0;
			m_sHomeFile = szPath;
			m_sPath = tmp+1;
			return true;
		}

		aux = strrchr(szPath, DIR_SEP);
		*tmp = '/'; // inside compressed files directory separator it's always '/'
	}
	return false;
}
inline CBString CVFile::GetFileTitle() const 
{ 
	return m_sTitle; 
}
inline CBString CVFile::GetFileExt() const 
{ 
	return m_sExt; 
}
inline CBString CVFile::GetFileName() const 
{ 
	return m_sTitle + m_sExt; 
}
inline CBString CVFile::GetHomeFile() const
{
	if(m_bVirtual) {
		if(m_bRelative)
			return g_sHomeDir + m_sHomeFile; 
		return m_sHomeFile;
	}
	return GetFilePath();
}
inline CBString CVFile::GetPath() const 
{ 
	if(!m_bVirtual && m_bRelative)
		return g_sHomeDir + m_sPath; 

	return m_sPath;
}
inline CBString CVFile::GetFilePath() const 
{ 
	return GetPath() + GetFileName();
}
inline CBString CVFile::GetFileDesc() const 
{
	return m_sDesc;
}
inline void CVFile::SetFileDesc(LPCSTR szNewDesc) 
{
	m_sDesc = szNewDesc;
}
inline void CVFile::SetFilePath(LPCSTR szNewName, bool bGlobalize) 
{
	ASSERT(szNewName);
	if(!*szNewName) return;

	if(m_bOpenFile) Close();
	ASSERT(!m_vFile);
	ASSERT(!m_File);

	char szDrive[_MAX_DRIVE];
	char szPath[_MAX_DIR];
	char szTitle[_MAX_FNAME];
	char szExt[_MAX_EXT];

	_splitpath(szNewName, szDrive, szPath, szTitle, szExt);
	CBString sPath = szDrive;
	sPath += szPath;

	if(bGlobalize) {
		ASSERT(g_sHomeDir == "");
		g_sHomeDir = sPath;
	}

	if(*szNewName != DIR_SEP && *(szNewName+1) != ':') {
		m_sPath = sPath;
		m_bRelative = true;
	} else if(sPath.Find(g_sHomeDir)!=-1) {
		ASSERT(g_sHomeDir != "");
		m_sPath = sPath.Mid(g_sHomeDir.GetLength());
		m_bRelative = true;
	} else {
		m_sPath = sPath;
		m_bRelative = false;
	}

	m_sDesc = szTitle;
	m_sTitle = szTitle;
	m_sExt = szExt;

	if(!FileExists()) FindVirtual();
}
inline void CVFile::SetFileTitle(LPCSTR szNewName) 
{
	if(m_bOpenFile) Close();
	m_sTitle = szNewName;
	m_nExists = 0;
}
inline void CVFile::SetFileExt(LPCSTR szNewExt) 
{
	if(m_bOpenFile) Close();
	m_sExt = szNewExt;
	m_nExists = 0;
}

inline bool CVFile::FileExists() const
{
	if(m_nExists==0) {
		if(m_bVirtual) {
			if(VirtualFileExists()) m_nExists = 1;
			else m_nExists = -1;
		} else {
			CBString sFile = GetFilePath();
			if(sFile=="") return false;
			if(FileExists(sFile)) m_nExists = 1;
			else m_nExists = -1;
		}
	}
	if(m_nExists>0) return true;
	return false;
}
inline bool CVFile::VirtualFileExists() const 
{
	CBString sFile = GetHomeFile();
	if(sFile=="") return false;

	unzFile file = unzOpen(sFile);
	if(file==NULL) return false;

	sFile = GetFilePath();
	if(sFile=="") {
		unzClose(file);
		return false;
	}
	if(unzLocateFile(file, sFile, 0) == UNZ_OK) {
		unzClose(file);
		return true;
	}
	unzClose(file);
	return false;
}

inline int CVFile::ReadVirtual(LPVOID buffer, size_t buffsize)
{
	ASSERT(m_vFile != NULL);
	return unzReadCurrentFile(m_vFile, buffer, (long)buffsize);
}

inline int CVFile::WriteVirtual(LPVOID buffer, size_t buffsize)
{
	ASSERT(m_vFile != NULL);
	ASSERT(!*"No writes allowed on virtual files, yet...");
	return -1;
}
inline long CVFile::TellVirtual() const
{
	ASSERT(m_vFile != NULL);
	return (long)unztell(m_vFile);
}
inline int CVFile::SeekVirtual(long offset, int origin)
{
	ASSERT(m_vFile != NULL);
	long curr = (long)unztell(m_vFile);
	long lastbyte = (long)GetVirtualFileSize() - 1;
	if(lastbyte<0) return -1;

	long newpos = offset;
	if(origin == SEEK_CUR) newpos = curr + offset;
	if(origin == SEEK_END) newpos = lastbyte + offset;
	if(newpos<0) return -1;
	if(newpos>lastbyte) return -1;

	// is the current position ahead of the wanted position, re-open the file:
	if(curr>newpos) {
		if(unzCloseCurrentFile(m_vFile) != UNZ_OK) return -1;
		if(unzOpenCurrentFile(m_vFile) != UNZ_OK) return -1;
		curr = 0;
	}

	long diff = newpos-curr;
	int cycles = diff/VBUFFER_SIZE;
	int mod = diff%VBUFFER_SIZE;
	for(; cycles; cycles--) if(ReadVirtual(m_pBuffer, VBUFFER_SIZE)!= VBUFFER_SIZE) return -1;
	for(; mod; mod--) if(ReadVirtual(m_pBuffer, 1) != 1) return -1;

	m_BuffStart = (LPSTR)m_pBuffer;
	m_BuffLen = 0;
	m_BuffRead = 0;

	return 0;
}
inline bool CVFile::OpenVirtual()
{
	if(m_vFile == NULL) {
		CBString sFile = GetHomeFile();
		if(sFile=="") return false;
		m_vFile = unzOpen(sFile);
	}
	if(m_vFile) {
		CBString sFile = GetFilePath();
		if(sFile=="") {
			unzClose(m_vFile);
			return false;
		}
		if(unzLocateFile(m_vFile, sFile, 0) == UNZ_OK) {
			if(unzOpenCurrentFile(m_vFile) != UNZ_OK) return false;
			m_bOpenFile = true;

			// Initialize the buffer:
			if(!m_pBuffer) m_pBuffer = new BYTE[VBUFFER_SIZE+1];
			m_pBuffer[VBUFFER_SIZE] = '\0';
			m_BuffStart = (LPSTR)m_pBuffer;
			m_BuffLen = 0;
			m_BuffRead = 0;

			return true;
		}
	}
	return false;
}
inline bool CVFile::CloseVirtual()
{
	ASSERT(m_vFile != NULL);
	if(unzCloseCurrentFile(m_vFile) == UNZ_OK) {
		if(unzClose(m_vFile) == UNZ_OK) {
			m_vFile = NULL;
			m_bOpenFile = false;
			return true;
		}
	}
	return false;
}
inline LPSTR CVFile::GetLineVirtual(LPSTR string, int n)
{
	int total = 0;
	int left = n - 1;
	LPSTR stringAux = string;
	LPSTR bufferAux = NULL;
	while(left>0 && !bufferAux) {
		if(m_BuffRead == m_BuffLen) {
			m_BuffRead = 0;
			m_BuffStart = (LPSTR)m_pBuffer;
			if((m_BuffLen = ReadVirtual(m_pBuffer, VBUFFER_SIZE)) == 0) 
				break;
		}

		bufferAux = strchr(m_BuffStart, '\n');

		int toRead = min(left, m_BuffLen - m_BuffRead);
		if(bufferAux) toRead = min(toRead, bufferAux - m_BuffStart);

		strncpy(stringAux, m_BuffStart, toRead);
		total += toRead;

		stringAux += toRead;
		left -= toRead;
		if(bufferAux) toRead++; // skip the '\n'
		m_BuffStart += toRead;
		m_BuffRead += toRead;

	}
	*stringAux = '\0';

	if(total) return string;
	return NULL;
}
inline bool CVFile::Open()
{
	if(m_bOpenFile) return true;
	if(m_bVirtual) return OpenVirtual();

	CBString sFile = GetHomeFile();
	if(sFile=="") return false;
	if(m_File == NULL) m_File = fopen(sFile, "r+b");
	if(m_File) {
		m_bOpenFile = true;
		return true;
	}
	return false;
}
inline bool CVFile::Close()
{
	if(!m_bOpenFile) return false;

	delete []m_pBuffer;
	m_pBuffer = NULL;

	if(m_bVirtual) return CloseVirtual();

	ASSERT(m_File);
	if(fclose(m_File) == 0) {
		m_File = NULL;
		m_bOpenFile = false;
		return true;
	}
	return false;
}
inline int CVFile::Read(LPVOID buffer, size_t buffsize)
{
	if(!m_bOpenFile) return 0;
	if(m_bVirtual) return ReadVirtual(buffer, buffsize);

	ASSERT(m_File);
	return (int)fread(buffer, 1, buffsize, m_File);
}
inline int CVFile::Write(LPVOID buffer, size_t buffsize)
{
	if(!m_bOpenFile) return 0;
	if(m_bVirtual) return WriteVirtual(buffer, buffsize);

	ASSERT(m_File);
	return (int)fwrite(buffer, 1, buffsize, m_File);
}
inline long CVFile::Tell() const
{
	if(!m_bOpenFile) return -1;
	if(m_bVirtual) return TellVirtual();

	ASSERT(m_File);
	return ftell(m_File);
}
inline int CVFile::Seek(long offset, int origin)
{
	if(!m_bOpenFile) return -1;
	if(m_bVirtual) return SeekVirtual(offset, origin);

	ASSERT(m_File);
	return (int)fseek(m_File, offset, origin);
}

inline LPSTR CVFile::GetLine(LPSTR string, int n)
{
	if(!m_bOpenFile) return NULL;
	if(m_bVirtual) return GetLineVirtual(string, n);

	ASSERT(m_File);
	return fgets(string, n, m_File);
}
inline size_t CVFile::GetFileSize() const
{
	if(!m_bOpenFile) return -1;
	if(m_bVirtual) return GetVirtualFileSize();

	ASSERT(m_File);
	long curr = ftell(m_File);
	fseek(m_File, 0, SEEK_END);
	long ret = ftell(m_File);
	fseek(m_File, curr, SEEK_SET);
	return ret;
}
inline size_t CVFile::GetVirtualFileSize() const
{
	char filename_inzip[256];
	unz_file_info file_info;

	int err = unzGetCurrentFileInfo(m_vFile, &file_info, 
		filename_inzip, sizeof(filename_inzip)-1,
		NULL,0,NULL,0);

	if(err==UNZ_OK) {
		return file_info.uncompressed_size;
	}
	return 0;
}
inline LPCVOID CVFile::ReadFile(LPVOID buffer, size_t buffsize)
{
	if(!m_bOpenFile) return NULL;

	// go to the start of the file:
	Seek(0, SEEK_SET);

	size_t size = GetFileSize();
	if(buffer) {
		if(size>buffsize) size = buffsize;
	} else {
		size_t tmpsize = size;
		// the minimum buffer size is always VBUFFER_SIZE+1
		if(tmpsize<VBUFFER_SIZE+1) tmpsize = VBUFFER_SIZE+1;

		delete []m_pBuffer;
		m_pBuffer = new BYTE[tmpsize];
		buffer = m_pBuffer;
	}
	Read(buffer, size);

	return buffer;
}
inline FILETIME* CVFile::GetFileDate() const
{
	if(m_bVirtual) return GetVirtualFileDate();

	CBString sFile = GetFilePath();
	if(sFile=="") return NULL;
	return GetFileTime(sFile);
}
inline FILETIME* CVFile::GetVirtualFileDate() const
{
	return NULL;
}
