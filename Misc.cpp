
#include "stdafx.h"
#include "Misc.h"

int ForEachFile(LPCSTR lpFileName, FILESPROC ForEach, LPARAM lParam, int flags)
{
	int files=0;
	char filename[_MAX_PATH];
	lstrcpy(filename, lpFileName);
	LPTSTR fin = strchr(filename, '\0');

	while(fin!=filename && *fin!='\\') fin = CharPrev(filename, fin);
	if(*fin == '\\') fin = CharNext(fin);

	WIN32_FIND_DATA FindData;
	HANDLE FindHandle = FindFirstFile(lpFileName, &FindData);
	if(INVALID_HANDLE_VALUE != FindHandle) {
		do {
			if (0 == lstrcmp(FindData.cFileName, "..") ||
				0 == lstrcmp(FindData.cFileName, ".")) continue;
			if(FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) continue;

			*fin = '\0';
			lstrcat(filename, FindData.cFileName);

			LPTSTR aux = strchr(filename, '\0');
			if(flags & FEF_NOEXT) {
				while(aux!=filename && *aux!='\\' && *aux!='.') aux = CharPrev(filename, aux);
				if(*aux=='.') { *aux = '\0'; aux = CharPrev(filename, aux); }
			}
			if(flags & FEF_NODIR) {
				while(aux!=filename && *aux!='\\') aux = CharPrev(filename, aux);
				if(*aux=='\\') aux = CharNext(aux);
			} else aux = filename;
			if(*aux != '\0') {
				files++;
				if((flags & FEF_LOWERCASE) == FEF_LOWERCASE) CharLower(aux);
				else if((flags & FEF_UPPERCASE) == FEF_UPPERCASE) CharUpper(aux);
				if((flags & FEF_BEAUTIFY) == FEF_BEAUTIFY) *aux = toupper(*aux);
				if(!ForEach(aux, lParam)) break;
			}
		} while(FindNextFile(FindHandle, &FindData));
		FindClose(FindHandle);
	}
	return files;
}
