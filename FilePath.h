/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germ�n M�ndez Bravo)
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
				September 24, 2003: Zip files greatly support improved

	This file implements the CVFile to handle filenames and paths. Also
	contains the path to the home directory of the game files.
*/

#pragma once

#include <zip.h>
#include <unzip.h>

#include "Misc.h"

#define DIR_SEP '\\'
#define VBUFFER_SIZE (16*1024)
#define MAXFILENAME (256)

extern CBString g_sHomeDir;

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CVFile;

// Virtual file system
class CVFile 
{
	// -1 = not existent, 0 = ??, FILE_ATTRIBUTE_NORMAL = file, FILE_ATTRIBUTE_DIRECTORY = directory:
	mutable DWORD m_dwFileAttributes; 
	mutable FILETIME m_FileDate;

	// virtual buffer:
	mutable LPBYTE m_pBuffer;
	mutable LPSTR m_BuffStart;
	mutable long m_BuffLen;
	mutable long m_BuffOffset;
	mutable long m_BuffSize;

	mutable unzFile m_vFile;	// compressed file (if virtual)
	mutable zipFile m_vzFile;
	mutable FILE *m_File;		// regular file

	mutable bool m_bOpenFile;
	mutable char m_szMode[7];
	mutable bool m_bChanges;

	bool m_bRelative;
	bool m_bVirtual;

	// HomeFile is not used if the file is not virtual (i.e. compressed)
	CBString m_sHomeFile;		// initial home directory (for compressed files)

	CBString m_sPath;			// path inside home (without the file name)
	CBString m_sTitle;			// filename (without extension)
	CBString m_sExt;			// file name extension

	CBString m_sDesc;			// file description is the file tile until changed.

	FILETIME* GetFileDate(LPCSTR lpFileName) const;
	bool FileExists(LPCSTR lpFileName) const;
	bool FindVirtual();

	bool OpenVirtual(LPCSTR mode) const;
	bool CloseVirtual(bool bDeleteTemps) const;
	int ReadVirtual(LPVOID buffer, long buffsize);

	int RenameVirtual(CVFile &vFile);
	int DeleteVirtual(bool bKeep = false) const;
	int WriteVirtual(LPCVOID buffer, long buffsize);
	int SeekVirtual(long offset, int origin);
	long TellVirtual() const;
	LPSTR GetLineVirtual(LPSTR string, int n);
	long GetVirtualFileSize() const;
	FILETIME* GetVirtualFileDate() const;
	int ForEachVirtualFile(FILESPROC ForEach, LPARAM lParam);

	mutable bool m_bRawMode;
	mutable long m_RawCRC;
	mutable long m_RawSize;
	mutable int m_nMethod;
	mutable int m_nLevel;
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

	bool IsDirectory() const;
	bool FileExists() const;

// Operations with the files:
	bool Flush() const;
	bool Open(LPCSTR mode) const;
	bool Close(bool bFreeBuffer = true, bool bDeleteTemps = true) const;

	int ForEachFile(FILESPROC ForEach, LPARAM lParam);
	// reads the complete file to a buffer or an internal buffer
	LPCVOID ReadFile();

	int Delete();
	int Rename(CVFile &vFile);

	int Read(LPVOID buffer, long buffsize);
	int Write(LPCVOID buffer, long buffsize);
	int Seek(long offset, int origin);
	long Tell() const;
	LPSTR GetLine(LPSTR string, int n);
	FILETIME* GetFileDate() const;
	long GetFileSize() const;
};
inline FILETIME* CVFile::GetFileDate(LPCSTR lpFileName) const
{
	m_dwFileAttributes = 0;
	WIN32_FIND_DATA FindData;
	HANDLE FindHandle = FindFirstFile(lpFileName, &FindData);
	if(INVALID_HANDLE_VALUE != FindHandle) {
		m_dwFileAttributes = FindData.dwFileAttributes;
		m_FileDate = FindData.ftLastWriteTime;
		return &m_FileDate;
	}
	return NULL;
}
inline bool CVFile::FileExists(LPCSTR lpFileName) const 
{
	if(m_dwFileAttributes == -1) return false;
	if(m_dwFileAttributes == 0) {
		if(GetFileDate(lpFileName)==NULL) return false;
	}
	if((m_dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!= FILE_ATTRIBUTE_DIRECTORY) return true;

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
			bExists = (FileExists(g_sHomeDir + szPath) != NULL);
		} else {
			bExists = (FileExists(szPath) != NULL);
		}
		if(bExists) {
			// split the path and make the file virtual.
			m_bVirtual = true;
			m_dwFileAttributes = 0;
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

	if(m_bOpenFile) Close(true, true);
	ASSERT(!m_vFile);
	ASSERT(!m_vzFile);
	ASSERT(!m_File);

	char szDrive[_MAX_DRIVE];
	char szPath[_MAX_DIR];
	char szTitle[_MAX_FNAME];
	char szExt[_MAX_EXT];

	_splitpath(szNewName, szDrive, szPath, szTitle, szExt);
	CBString sPath = szDrive;
	sPath += szPath;

	if(bGlobalize) {
		ASSERT(g_sHomeDir.IsEmpty());
		g_sHomeDir = sPath;
	}

	if(*szNewName != DIR_SEP && *(szNewName+1) != ':') {
		m_sPath = sPath;
		m_bRelative = true;
	} else if(sPath.Find(g_sHomeDir)!=-1 && !g_sHomeDir.IsEmpty()) {
		m_sPath = sPath.Mid(g_sHomeDir.GetLength());
		m_bRelative = true;
	} else {
		m_sPath = sPath;
		m_bRelative = false;
	}

	m_sDesc = szTitle;
	m_sTitle = szTitle;
	m_sExt = szExt;

	m_dwFileAttributes = 0;
	if(!FileExists()) FindVirtual();
}
inline void CVFile::SetFileTitle(LPCSTR szNewName) 
{
	ASSERT(szNewName);
	if(m_bOpenFile) Close(true, true);
	ASSERT(!m_vFile);
	ASSERT(!m_vzFile);
	ASSERT(!m_File);

	m_sTitle = szNewName;
	m_dwFileAttributes = 0;
}
inline void CVFile::SetFileExt(LPCSTR szNewExt) 
{
	ASSERT(szNewExt);
	if(m_bOpenFile) Close(true, true);
	ASSERT(!m_vFile);
	ASSERT(!m_vzFile);
	ASSERT(!m_File);

	m_sExt = szNewExt;
	m_dwFileAttributes = 0;
}
// Virtual Files:
inline FILETIME* CVFile::GetVirtualFileDate() const
{
	CBString sHomeFile = GetHomeFile();
	if(sHomeFile.IsEmpty()) return NULL;

	CBString sFilePath = GetFilePath();
	if(sFilePath.IsEmpty()) return NULL;

	unzFile file = unzOpen(sHomeFile);
	if(file==NULL) return NULL;

	char filename_inzip[MAXFILENAME] = {0};
	unz_file_info file_info;

	if(unzLocateFile(file, sFilePath, 0) != UNZ_OK) {
		unzClose(file);
		return NULL;
	}

	int err = unzGetCurrentFileInfo(file, &file_info, 
		filename_inzip, sizeof(filename_inzip)-1,
		NULL,0,NULL,0);
	if(m_bRawMode) m_RawCRC = file_info.crc;

	unzClose(file);
	
	if(err==UNZ_OK) {
		char cLastChar = filename_inzip[strlen(filename_inzip)-1];
		if(cLastChar == '\\' || cLastChar == '/') m_dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
		else m_dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
		DosDateTimeToFileTime(HIWORD(file_info.dosDate), LOWORD(file_info.dosDate), &m_FileDate);
		return &m_FileDate;
	}
	return NULL;
}

inline int CVFile::DeleteVirtual(bool bKeep) const
{
	// make a temporary file to copy data from:
	CBString sFile = GetHomeFile();
	if(sFile.IsEmpty()) return -1;

	CBString sTmpFile = sFile + "~";

	if(GetFileDate(sTmpFile) && !m_bChanges) {
		unlink(sFile);
		if(rename(sTmpFile, sFile) != 0) return -1;
		return 0;
	} else unlink(sTmpFile);

	// open source and destiny files:
	unzFile ufile = unzOpen(sFile);
	if(ufile == NULL) return -1;

	zipFile zfile = zipOpen(sTmpFile, 0);
	if(zfile == NULL) {
		unzClose(ufile);
		return -1;
	}

	LPBYTE pBuffer = new BYTE[VBUFFER_SIZE];

	// loop through all the existent files:
	int err = unzGoToFirstFile(ufile);

	int method, level;
	unz_file_info file_info;
	zip_fileinfo zfile_info;

	CBString sSkipFile = GetFilePath();

	while(err == UNZ_OK) {
		char szCurrentFileName[MAX_PATH];
		err = unzGetCurrentFileInfo(ufile, &file_info,
							szCurrentFileName, MAX_PATH-1,
							NULL, 0, NULL, 0);
		if(err == UNZ_OK) {
			if(sSkipFile == szCurrentFileName) {
				err = unzGoToNextFile(ufile);
				continue;
			}

			long lSize = file_info.compressed_size;

			zfile_info.external_fa = file_info.external_fa;
			zfile_info.internal_fa = file_info.internal_fa;
			zfile_info.dosDate = file_info.dosDate;
			zfile_info.tmz_date.tm_sec  = file_info.tmu_date.tm_sec; 
			zfile_info.tmz_date.tm_min  = file_info.tmu_date.tm_min; 
			zfile_info.tmz_date.tm_hour = file_info.tmu_date.tm_hour;
			zfile_info.tmz_date.tm_mday = file_info.tmu_date.tm_mday;
			zfile_info.tmz_date.tm_mon  = file_info.tmu_date.tm_mon; 
			zfile_info.tmz_date.tm_year = file_info.tmu_date.tm_year;

			// open the raw files:
			if(unzOpenCurrentFile2(ufile, &method, &level, 1) != UNZ_OK) break;
			if(zipOpenNewFileInZip2(zfile, szCurrentFileName, &zfile_info,
                                 NULL, 0, NULL, 0, NULL,
                                 method,
                                 level, 1) != ZIP_OK) break;

			while(err == ZIP_OK && lSize > 0) {
				long lToRead = min(VBUFFER_SIZE, lSize);
				long lBuffRead = unzReadCurrentFile(ufile, pBuffer, lToRead);
				if(lBuffRead < lToRead) if(unzeof(ufile) == 0) {
					err = Z_ERRNO;
				}
	
				if(lBuffRead > 0 && err == ZIP_OK) {
					if(zipWriteInFileInZip(zfile, pBuffer, lBuffRead) < 0) {
						err = Z_ERRNO;
					}
				}
				lSize -= lBuffRead;
			} 

			unzCloseCurrentFile(ufile);
			zipCloseFileInZipRaw(zfile, file_info.uncompressed_size, file_info.crc);

			if(err != ZIP_OK) break;

			err = unzGoToNextFile(ufile);
		}
	}
	unzClose(ufile);
	zipClose(zfile, NULL);

	delete []pBuffer;

	if(err != UNZ_END_OF_LIST_OF_FILE) {
		unlink(sTmpFile);
		return -1;
	}

	// delete the temporary file:
	if(!bKeep) {
		unlink(sFile);
		if(rename(sTmpFile, sFile) != 0) return -1;
	}

	return 0;
}

inline int CVFile::RenameVirtual(CVFile &vFile)
{
	ASSERT(!*"Rename() is not yet supported on virtual files...");
	return -1;
}
inline int CVFile::ReadVirtual(LPVOID buffer, long buffsize)
{
	if(m_vzFile && m_pBuffer && m_BuffLen!=-1) {
		long size = min(m_BuffOffset+buffsize, m_BuffLen) - m_BuffOffset;
		memcpy(buffer, m_pBuffer+m_BuffOffset, size);
		m_BuffOffset += size; // update the offset
		return size;
	}
	if(m_vFile == NULL) return 0;
	return unzReadCurrentFile(m_vFile, buffer, (long)buffsize);
}
inline int CVFile::WriteVirtual(LPCVOID buffer, long buffsize)
{
	if(m_vzFile == NULL) return 0;

	long offset = buffsize;
	if(m_BuffOffset != -1) {
		ASSERT(m_BuffLen != -1);
		if(strchr(m_szMode, 'a')) offset += m_BuffLen;
		else offset += m_BuffOffset;
	} else {
		offset += GetVirtualFileSize();
	}

	SeekVirtual(offset, SEEK_SET);
	ASSERT(m_BuffLen+buffsize <= m_BuffSize);
	memcpy(m_pBuffer + m_BuffLen, buffer, buffsize);
	m_bChanges = true;
	m_BuffLen = offset;
	return buffsize;
}
inline long CVFile::TellVirtual() const
{
	if(m_vzFile && m_BuffOffset!=-1) return m_BuffOffset;
	if(m_vFile) return (long)unztell(m_vFile);
	return 0;
}
inline int CVFile::SeekVirtual(long offset, int origin)
{
	ASSERT(m_vFile != NULL || m_vzFile != NULL);

	long curr = TellVirtual();
	long filesize = (long)GetVirtualFileSize();

	long newpos = offset;
	if(origin == SEEK_CUR) newpos = curr + offset;
	if(origin == SEEK_END) newpos = filesize - 1 + offset;

	if(m_vzFile) { 
		LPBYTE pTmpBuffer = NULL;
		// find out the new size of the buffer:
		long newsize = max(filesize, newpos + VBUFFER_SIZE);
		if(newpos <= filesize) newsize = filesize;
		if(m_pBuffer) newsize = max(m_BuffLen, newsize);

		// Grow the buffer if necessary:
		if(m_pBuffer && m_BuffSize<newsize) {
			pTmpBuffer = m_pBuffer;
			m_pBuffer = NULL;
		}
		if(!m_pBuffer) {
			m_pBuffer = new BYTE[newsize];
			if(pTmpBuffer && m_BuffLen!=-1) {
				memcpy(m_pBuffer, pTmpBuffer, m_BuffLen);
				delete []pTmpBuffer;
			} else {
				m_BuffOffset = -1;
				m_BuffLen = -1;
			}
			m_BuffSize = newsize;
		}
		if(m_BuffOffset == -1) {
			if(filesize > 0) ReadVirtual(m_pBuffer, filesize);
			m_BuffLen = filesize;
		}
		m_BuffOffset = newpos;
		return 0;
	}

	if(newpos<0) return -1;
	if(newpos>filesize) return -1;

	if(!m_vFile) return -1;

	// is the current position ahead of the wanted position, re-open the file:
	if(curr>newpos) {
		if(unzCloseCurrentFile(m_vFile) != UNZ_OK) return -1;
		if(unzOpenCurrentFile2(m_vFile, &m_nMethod, &m_nLevel, m_bRawMode) != UNZ_OK) return -1;
		curr = 0;
	}

	long diff = newpos-curr;
	int cycles = diff/VBUFFER_SIZE; // read in chunks of VBUFFER_SIZE (m_BuffSize might be too big)
	int mod = diff%VBUFFER_SIZE;
	for(; cycles; cycles--) if(ReadVirtual(m_pBuffer, VBUFFER_SIZE)!= VBUFFER_SIZE) return -1;
	for(; mod; mod--) if(ReadVirtual(m_pBuffer, 1) != 1) return -1;

	return 0;
}
inline bool CVFile::OpenVirtual(LPCSTR mode) const
{
	CBString sFile = GetHomeFile();
	if(sFile.IsEmpty()) return false;

	CBString sTmpFile = sFile + "~";
	unlink(sTmpFile); // delete any temporary file
	m_bChanges = false;

	try {
		ASSERT(m_vFile == NULL);
		ASSERT(m_vzFile == NULL);

		// Open Zip file (for reading and writing if needed):
		if(strchr(mode, 'w') || strchr(mode, 'a')) {
			DeleteVirtual(true); // Create a temporary zip file without the current file.
			m_vzFile = zipOpen(sTmpFile, APPEND_STATUS_ADDINZIP);
			if(!m_vzFile) throw 0;
		}

		if(!FileExists() && !strchr(mode, 'w') && !strchr(mode, 'a')) throw 0;

		m_vFile = unzOpen(sFile);
		if(!m_vFile) throw 0;

		if(FileExists()) { // This call also fills the m_RawCRC if using Raw Mode.
			CBString sFile = GetFilePath();
			if(sFile.IsEmpty()) throw 0;

			// Try to locate and open the wanted file:
			if(unzLocateFile(m_vFile, sFile, 0) != UNZ_OK) throw 0;
			if(unzOpenCurrentFile2(m_vFile, &m_nMethod, &m_nLevel, m_bRawMode) != UNZ_OK) throw 0;
		}

		if(m_vzFile) {
			// Get current time:
			DWORD dwDosTime;
			FILETIME FileTime;
			SYSTEMTIME LocalTime;
			GetLocalTime(&LocalTime);
			SystemTimeToFileTime(&LocalTime, &FileTime);
			FileTimeToDosDateTime(&FileTime, ((LPWORD)&dwDosTime)+1, ((LPWORD)&dwDosTime)+0);

			// Set new file time and flags:
            zip_fileinfo zi;
			zi.tmz_date.tm_sec = LocalTime.wSecond;
			zi.tmz_date.tm_min = LocalTime.wMinute;
			zi.tmz_date.tm_hour = LocalTime.wHour;
			zi.tmz_date.tm_mday = LocalTime.wDay;
			zi.tmz_date.tm_mon = LocalTime.wMonth;
			zi.tmz_date.tm_year = LocalTime.wYear;

            zi.dosDate = dwDosTime;
            zi.internal_fa = 0;
            zi.external_fa = 0;

			// Open new file in zip:
            if(zipOpenNewFileInZip2(m_vzFile, GetFilePath(), &zi, 
				NULL,0,NULL,0,NULL,
				m_nMethod,
				m_nLevel,
				m_bRawMode) != ZIP_OK) throw 1;
		}

		// Initialize the buffer:
		if(!m_pBuffer) {
			m_pBuffer = new BYTE[VBUFFER_SIZE]; ASSERT(m_pBuffer);
			m_BuffSize = VBUFFER_SIZE;
		}
		m_BuffLen = -1;
		m_BuffOffset = -1;

		ASSERT(m_pBuffer);
		if(!m_pBuffer) throw 2;

		m_bOpenFile = true;
		return true;
	} 
	catch(int err) {
		if(err >= 2 && m_vzFile) {
			if(m_bRawMode) zipCloseFileInZipRaw(m_vzFile, m_RawSize, m_RawCRC);
			else zipCloseFileInZip(m_vzFile);
		}
		if(err >= 1) unzCloseCurrentFile(m_vFile);
		if(m_vFile) unzClose(m_vFile); m_vFile = NULL;
		if(m_vzFile) zipClose(m_vzFile, NULL); m_vzFile = NULL;
	}
	return false;
}
inline bool CVFile::CloseVirtual(bool bDeleteTemps) const
{
	if(m_vzFile) {
		if(zipWriteInFileInZip(m_vzFile, m_pBuffer, m_BuffLen) < 0) return false;

		if(m_bRawMode) zipCloseFileInZipRaw(m_vzFile, m_RawSize, m_RawCRC);
		else zipCloseFileInZip(m_vzFile);
		if(zipClose(m_vzFile, NULL) != ZIP_OK) return false;
		m_vzFile = NULL;
	}
	if(m_vFile) {
		unzCloseCurrentFile(m_vFile);
		if(unzClose(m_vFile) != UNZ_OK) return false;
		m_vFile = NULL;
	}

	m_bOpenFile = false;
	m_bRawMode = false;
	m_nMethod = Z_DEFLATED;
	m_nLevel = Z_DEFAULT_COMPRESSION;

	CBString sFile = GetHomeFile();
	ASSERT(!sFile.IsEmpty());
	if(m_bChanges) {
		m_bChanges = false;
		unlink(sFile);
		if(rename(sFile + "~", sFile) != 0) return false;
	} else if(bDeleteTemps) unlink(sFile + "~");
	return true;
}
inline LPSTR CVFile::GetLineVirtual(LPSTR string, int n)
{
	int total = 0;
	int left = n - 1;
	LPSTR stringAux = string;
	LPSTR bufferAux = NULL;
	while(left>0 && !bufferAux) {
		if(m_BuffOffset == m_BuffLen) {
			m_BuffOffset = 0;
			m_BuffStart = (LPSTR)m_pBuffer;
			if((m_BuffLen = ReadVirtual(m_pBuffer, m_BuffSize-1)) == 0) 
				break;
			m_pBuffer[m_BuffSize-1] = '\0';
		}

		bufferAux = strchr(m_BuffStart, '\n');

		int toRead = min(left, m_BuffLen - m_BuffOffset);
		if(bufferAux) toRead = min(toRead, bufferAux - m_BuffStart);

		strncpy(stringAux, m_BuffStart, toRead);
		total += toRead;

		stringAux += toRead;
		left -= toRead;
		if(bufferAux) toRead++; // skip the '\n'
		m_BuffStart += toRead;
		m_BuffOffset += toRead;

	}
	*stringAux = '\0';

	if(total) return string;
	return NULL;
}
inline long CVFile::GetVirtualFileSize() const
{
	char filename_inzip[MAXFILENAME] = {0};
	unz_file_info file_info;

	CBString sFile = GetFilePath();
	if(sFile.IsEmpty()) return 0;

	if(unzLocateFile(m_vFile, sFile, 0) != UNZ_OK) {
		return 0;
	}
	int err = unzGetCurrentFileInfo(m_vFile, &file_info, 
		filename_inzip, sizeof(filename_inzip)-1,
		NULL,0,NULL,0);

	if(err==UNZ_OK) {
		if(m_bRawMode) return file_info.compressed_size;
		else return file_info.uncompressed_size;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////
// Real interface:
inline bool CVFile::Open(LPCSTR mode) const
{
	if(m_bOpenFile) return true;
	if(!FileExists() && !strchr(mode, 'w') && !strchr(mode, 'a')) return false;

	strncpy(m_szMode, mode, 4);
	m_szMode[4] = '\0';
	if(FileExists() && !strchr(m_szMode, '+')) strcat(m_szMode, "+");
	if(!strchr(m_szMode, 'b')) strcat(m_szMode, "b");

	if(m_bVirtual) return OpenVirtual(m_szMode);

	m_bRawMode = false; // regular files can't use raw mode
	CBString sFile = GetHomeFile();
	if(sFile.IsEmpty()) return false;

	ASSERT(m_File == NULL);

	if(strchr(m_szMode, 'w') && strchr(m_szMode, '+')) *strchr(m_szMode, 'w') = 'r';

	m_File = fopen(sFile, m_szMode);
	if(m_File) {
		m_bOpenFile = true;
		return true;
	}
	return false;
}
inline bool CVFile::Flush() const
{
	if(!m_bOpenFile) return false;
	if(m_bVirtual) return true;

	ASSERT(m_File);
	if(fflush(m_File) == 0) return true;

	return false;
}
inline bool CVFile::Close(bool bFreeBuffer, bool bDeleteTemps) const
{
	bool ret = false;
	if(m_bOpenFile) {
		if(m_bVirtual) {
			ret = CloseVirtual(bDeleteTemps);
		} else {
			ASSERT(m_File);
			if(fclose(m_File) == 0) {
				m_File = NULL;
				m_bOpenFile = false;
				ret = true;
			}
		}
	}

	if(bFreeBuffer) {
		delete []m_pBuffer;
		m_pBuffer = NULL;
	}

	return ret;
}
inline int CVFile::Read(LPVOID buffer, long buffsize)
{
	bool wasOpen = m_bOpenFile;
	if(!m_bOpenFile) if(!Open("rb")) return -1;
	if(!Flush()) return -1;

	int ret = -1;

	if(m_bVirtual) {
		ret = ReadVirtual(buffer, buffsize);
	} else {
		ASSERT(m_File);
		ret = (int)fread(buffer, 1, buffsize, m_File);
	}

	if(!wasOpen) Close(false, false);
	return ret;
}

// only stream writes are going to be supported in a near future:
inline int CVFile::Write(LPCVOID buffer, long buffsize)
{
	bool wasOpen = m_bOpenFile;
	if(!m_bOpenFile) if(!Open("wb")) return -1;
	if(!Flush()) return -1;

	int ret = -1;

	if(m_bVirtual) {
		ret = WriteVirtual(buffer, buffsize);
	} else {
		ASSERT(m_File);
		ret = (int)fwrite(buffer, 1, buffsize, m_File);
	}

	if(!wasOpen) Close(false, false);
	return ret;
}
inline int CVFile::Delete()
{
	// If the file does not exists, error:
	if(!FileExists()) return -1;

	// close the open file (if any)
	if(m_bOpenFile) Close(true, false);

	if(m_bVirtual) return DeleteVirtual();

	CBString sFile = GetHomeFile();
	if(sFile.IsEmpty()) return -1;

	return (unlink(sFile)==0)?0:-1;
}

inline int CVFile::Rename(CVFile &vFile)
{
	if(vFile.FileExists()) return -1;

	if(m_bOpenFile) if(!Close(true, true)) return -1;
	if(vFile.m_bOpenFile) if(!vFile.Close(true, true)) return -1;

	int ret = -1;

	// both files are not virtual:
	if(!m_bVirtual && !vFile.m_bVirtual) {
		ret = ((rename(GetFilePath(), vFile.GetFilePath())==0)?0:-1);
		if(ret != -1) {
			*this = vFile;
			m_dwFileAttributes = 0;
		}
		return ret;
	}

	m_bRawMode = true;
	if(!Open("rb")) return -1;
	long size = GetFileSize();
	LPBYTE pBuffer = new BYTE[size];
	if(!Read(pBuffer, size)) {
		delete []pBuffer;
		pBuffer = NULL;
	}
	// Save the needed info from the read data (before closing):
	if(m_bRawMode) {
		vFile.m_bRawMode = m_bRawMode;
		vFile.m_nMethod = m_nMethod;
		vFile.m_nLevel = m_nLevel;
		vFile.m_RawCRC = m_RawCRC;
	}
	// Close the file:
	Close(true, true);

	// Get the size of the uncompressed file (if compressed)
	vFile.m_RawSize = GetFileSize();

	if(pBuffer) {
		Delete();
		if(!vFile.Open("ab")) return -1;
		if(vFile.Write(pBuffer, size) == size) ret = 0;
		vFile.Close(true, true);
	}

	// Ensure the Raw mode does not stay active (in case of errors).
	vFile.m_bRawMode = false;

	if(ret == -1) { // probably it's a directory, so try renaming:
		if(m_bVirtual || vFile.m_bVirtual) ret = RenameVirtual(vFile);
		else ret = (rename(GetHomeFile(), vFile.GetHomeFile())==0)?0:-1;
	}

	delete []pBuffer;

	if(ret != -1) {
		*this = vFile;
		m_dwFileAttributes = 0;
	}
	return ret;
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
inline long CVFile::GetFileSize() const
{
	bool wasOpen = m_bOpenFile;
	if(!m_bOpenFile) if(!Open("r")) return -1;
	long ret = -1;

	if(m_bVirtual) {
		ret = GetVirtualFileSize();
	} else {
		ASSERT(m_File);
		long curr = ftell(m_File);
		fseek(m_File, 0, SEEK_END);
		ret = ftell(m_File);
		fseek(m_File, curr, SEEK_SET);
	}

	if(!wasOpen) Close(false, false);
	return ret;
}
inline LPCVOID CVFile::ReadFile()
{
	bool wasOpen = m_bOpenFile;
	if(!m_bOpenFile) {
		if(!Open("r")) return NULL;
	} else {
		// go to the start of the file:
		Seek(0, SEEK_SET); 
	}

	// the minimum buffer size is always VBUFFER_SIZE
	long tmpsize = GetFileSize();
	long size = max(tmpsize, VBUFFER_SIZE);

	delete []m_pBuffer;
	m_pBuffer = new BYTE[size];
	ASSERT(m_pBuffer);

	if(m_pBuffer) {
		Read(m_pBuffer, size);
		m_BuffSize = size;
		m_BuffLen = 0;
		m_BuffOffset = 0;
	}

	if(!wasOpen) Close(false, false);

	return m_pBuffer;
}
inline FILETIME* CVFile::GetFileDate() const
{
	if(m_bVirtual) return GetVirtualFileDate();

	CBString sFile = GetFilePath();
	if(sFile.IsEmpty()) return NULL;
	return GetFileDate(sFile);
}
inline bool CVFile::IsDirectory() const
{
	if(m_dwFileAttributes == -1) return false;
	if(m_dwFileAttributes == 0) {
		if(GetFileDate()==NULL) return false;
	}
	if((m_dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)== FILE_ATTRIBUTE_DIRECTORY) return true;

	return false;
}
inline bool CVFile::FileExists() const 
{
	if(m_dwFileAttributes == -1) return false;
	if(m_dwFileAttributes == 0) {
		if(GetFileDate()==NULL) return false;
	}
	if((m_dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!= FILE_ATTRIBUTE_DIRECTORY) return true;

	return false;
}

