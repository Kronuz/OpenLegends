/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003 Kronuz
   Copyright (C) 2001/2003 Open Zelda's Project
 
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

#define DIR_SEP '\\'

extern CBString g_sHomeDir;

// Virtual file system
class CVFile {

	// virtual buffer:
	CHAR m_Buffer[16385];
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

	bool FileExists(LPCSTR szFile) const;
	bool FindVirtual();
	bool VirtualFileExists() const;

	bool OpenVirtual();
	bool CloseVirtual();
	int ReadVirtual(LPVOID buffer, size_t size);
	int WriteVirtual(LPVOID buffer, size_t size);
	int SeekVirtual(long offset, int origin);
	LPSTR GetLineVirtual(LPSTR string, int n);
	long GetVirtualFileSize();

public:
	CVFile();
	CVFile(LPCSTR szNewName);
	~CVFile();

	CBString GetFileTitle() const;
	CBString GetFileName() const;
	CBString GetPath() const;
	CBString GetHomeFile() const;
	CBString GetFilePath() const;
	CBString GetFileDesc() const;

	void SetFileDesc(LPCSTR szNewDesc);
	void SetFilePath(LPCSTR szNewName);
	void SetFileTitle(LPCSTR szNewName);

	bool FileExists() const;

	bool Open();
	bool Close();
	int Read(LPVOID buffer, size_t size);
	int Write(LPVOID buffer, size_t size);
	int Seek(long offset, int origin);
	LPSTR GetLine(LPSTR string, int n);
	long GetFileSize();
};
inline bool CVFile::FileExists(LPCSTR szFile) const
{
	FILE *f = fopen(szFile, "r");
	if(f) {
		fclose(f);
		return true;
	}
	return false;
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
inline void CVFile::SetFilePath(LPCSTR szNewName) 
{
	if(!*szNewName) return;

	char szDrive[_MAX_DRIVE];
	char szPath[_MAX_DIR];
	char szTitle[_MAX_FNAME];
	char szExt[_MAX_EXT];

	if(m_vFile) unzClose(m_vFile); m_vFile = NULL;
	if(m_File) fclose(m_File); m_File = NULL;

	_splitpath(szNewName, szDrive, szPath, szTitle, szExt);
	CBString sPath = szDrive;
	sPath += szPath;

	if(*szNewName != DIR_SEP && *(szNewName+1) != ':') {
		m_sPath = sPath;
		m_bRelative = true;
	} else if(sPath.Find(g_sHomeDir)) {
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
	m_sTitle = szNewName;
	m_nExists = 0;
}

inline bool CVFile::FileExists() const
{
	if(m_nExists==0) {
		if(m_bVirtual) {
			if(VirtualFileExists()) m_nExists = 1;
			else m_nExists = -1;
		} else {
			if(FileExists(GetFilePath())) m_nExists = 1;
			else m_nExists = -1;
		}
	}
	if(m_nExists>0) return true;
	return false;
}
inline bool CVFile::VirtualFileExists() const 
{
	unzFile file = unzOpen(GetHomeFile());
	if(file==NULL) return false;
	if(unzLocateFile(file, GetFilePath(), 0) == UNZ_OK) {
		unzClose(file);
		return true;
	}
	unzClose(file);
	return false;
}

inline bool CVFile::OpenVirtual()
{
	if(m_vFile == NULL) m_vFile = unzOpen(GetHomeFile());
	if(m_vFile) {
		if(unzLocateFile(m_vFile, GetFilePath(), 0) == UNZ_OK) {
			unzOpenCurrentFile(m_vFile);
			m_bOpenFile = true;

			// Initialize the buffer:
			m_Buffer[sizeof(m_Buffer)-1] = '\0';
			m_BuffStart = m_Buffer;
			m_BuffLen = 0;
			m_BuffRead = 0;

			return true;
		}
	}
	return false;
}
inline bool CVFile::CloseVirtual()
{
	if(unzCloseCurrentFile(m_vFile) == UNZ_OK) {
		if(unzClose(m_vFile) == UNZ_OK) {
			m_vFile = NULL;
			m_bOpenFile = false;
			return true;
		}
	}
	return false;
}
inline int CVFile::ReadVirtual(LPVOID buffer, size_t size)
{
	ASSERT(m_vFile != NULL);
	return unzReadCurrentFile(m_vFile, buffer, (long)size);
}

inline int CVFile::WriteVirtual(LPVOID buffer, size_t size)
{
	ASSERT(m_vFile != NULL);
	return -1;
}
inline int CVFile::SeekVirtual(long offset, int origin)
{
	return -1;
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
			m_BuffStart = m_Buffer;
			if((m_BuffLen = ReadVirtual(m_Buffer, sizeof(m_Buffer)-1)) == 0) 
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

	if(m_File == NULL) m_File = fopen(GetHomeFile(), "r+b");
	if(m_File) {
		m_bOpenFile = true;
		return true;
	}
	return false;
}
inline bool CVFile::Close()
{
	if(!m_bOpenFile) return false;
	if(m_bVirtual) return CloseVirtual();

	ASSERT(m_File);
	if(fclose(m_File) == 0) {
		m_File = NULL;
		m_bOpenFile = false;
		return true;
	}
	return false;
}
inline int CVFile::Read(LPVOID buffer, size_t size)
{
	if(!m_bOpenFile) return 0;
	if(m_bVirtual) return ReadVirtual(buffer, size);

	ASSERT(m_File);
	return (int)fread(buffer, 1, size, m_File);
}
inline int CVFile::Write(LPVOID buffer, size_t size)
{
	if(!m_bOpenFile) return 0;
	if(m_bVirtual) return WriteVirtual(buffer, size);

	ASSERT(m_File);
	return (int)fwrite(buffer, 1, size, m_File);
}
inline int CVFile::Seek(long offset, int origin)
{
	if(!m_bOpenFile) return 0;
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
inline long CVFile::GetFileSize()
{
	if(!m_bOpenFile) return NULL;
	if(m_bVirtual) return GetVirtualFileSize();

	ASSERT(m_File);
	long curr = ftell(m_File);
	fseek(m_File, 0, SEEK_END);
	long ret = ftell(m_File);
	fseek(m_File, curr, SEEK_SET);
	return ret;
}
inline long CVFile::GetVirtualFileSize()
{
	char filename_inzip[256];
	unz_file_info file_info;

	int err = unzGetCurrentFileInfo(m_vFile, &file_info, 
		filename_inzip, sizeof(filename_inzip)-1,
		NULL,0,NULL,0);

	if (err!=UNZ_OK) {
		return file_info.uncompressed_size;
	}
	return 0;
}
