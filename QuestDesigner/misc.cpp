
#include "stdafx.h"
#include "misc.h"

int ForEachFile(LPCTSTR lpFileName, FILESPROC ForEach, LPARAM lParam, int flags)
{
	int files=0;
	TCHAR filename[_MAX_PATH];
	lstrcpy(filename, lpFileName);
	LPTSTR fin = _tcschr(filename, _T('\0'));

	while(fin!=filename && *fin!=_T('\\')) fin = CharPrev(filename, fin);
	if(*fin == _T('\\')) fin = CharNext(fin);

	WIN32_FIND_DATA FindData;
	HANDLE FindHandle = FindFirstFile(lpFileName, &FindData);
	if(INVALID_HANDLE_VALUE != FindHandle) {
		do {
			if (0 == lstrcmp(FindData.cFileName, _T("..")) ||
				0 == lstrcmp(FindData.cFileName, _T("."))) continue;
			if(FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) continue;

			*fin = _T('\0');
			lstrcat(filename, FindData.cFileName);

			LPTSTR aux = _tcschr(filename, _T('\0'));
			if(flags & FEF_NOEXT) {
				while(aux!=filename && *aux!=_T('\\') && *aux!=_T('.')) aux = CharPrev(filename, aux);
				if(*aux==_T('.')) { *aux = _T('\0'); aux = CharPrev(filename, aux); }
			}
			if(flags & FEF_NODIR) {
				while(aux!=filename && *aux!=_T('\\')) aux = CharPrev(filename, aux);
				if(*aux==_T('\\')) aux = CharNext(aux);
			} else aux = filename;
			if(*aux != _T('\0')) {
				files++;
				if((flags & FEF_LOWERCASE) == FEF_LOWERCASE) CharLower(aux);
				else if((flags & FEF_UPPERCASE) == FEF_UPPERCASE) CharUpper(aux);
				if((flags & FEF_BEAUTIFY) == FEF_BEAUTIFY) *aux = _totupper(*aux);
				if(!ForEach(aux, lParam)) break;
			}
		} while(FindNextFile(FindHandle, &FindData));
		FindClose(FindHandle);
	}
	return files;
}