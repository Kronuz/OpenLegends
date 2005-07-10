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

#include <Misc.h>

int ForEachFile(LPCSTR lpFileName, FILESPROC ForEach, LPARAM lParam, int flags)
{
	int files=0;
	char filename[_MAX_PATH];
	lstrcpy(filename, lpFileName);
	LPSTR fin = strchr(filename, '\0');

	while(fin!=filename && *fin!='\\') fin = CharPrev(filename, fin);
	if(*fin == '\\') fin = CharNext(fin);

	WIN32_FIND_DATA FindData;
	HANDLE FindHandle = FindFirstFile(lpFileName, &FindData);
	if(INVALID_HANDLE_VALUE != FindHandle) {
		do {
			if (0 == lstrcmp(FindData.cFileName, "..") ||
				0 == lstrcmp(FindData.cFileName, ".")) continue;

			*fin = '\0';
			lstrcat(filename, FindData.cFileName);

			LPSTR aux = strchr(filename, '\0');
			if(flags & FEF_NOEXT) {
				while(aux!=filename && *aux!='\\' && *aux!='.') aux = CharPrev(filename, aux);
				if(*aux=='.') { *aux = '\0'; aux = CharPrev(filename, aux); }
			}
			if(flags & FEF_NODIR) {
				while(aux!=filename && *aux!='\\') aux = CharPrev(filename, aux);
				if(*aux=='\\') aux = CharNext(aux);
			} else aux = filename;
			if(*aux != '\0') {
				if((flags & FEF_LOWERCASE) == FEF_LOWERCASE) CharLower(aux);
				else if((flags & FEF_UPPERCASE) == FEF_UPPERCASE) CharUpper(aux);
				if((flags & FEF_BEAUTIFY) == FEF_BEAUTIFY) *aux = toupper(*aux);
				int cnt = ForEach(aux, FindData.dwFileAttributes, lParam);
				if(cnt < 0) {
					files = cnt-files;
					break;
				}
				files += cnt;
			}
		} while(FindNextFile(FindHandle, &FindData));
		FindClose(FindHandle);
	}
	return files;
}
// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY GetAppRegistryKey( LPCSTR pszRegistryKey, LPCSTR pszProfileName  )
{
	ASSERT( pszRegistryKey != NULL  );
	ASSERT( pszProfileName != NULL  );

	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	if ( RegOpenKeyEx( HKEY_CURRENT_USER, "Software", 0, KEY_WRITE|KEY_READ,
		&hSoftKey ) == ERROR_SUCCESS )
	{
		DWORD dw;
		if ( RegCreateKeyEx( hSoftKey, pszRegistryKey, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw ) == ERROR_SUCCESS )
		{
			RegCreateKeyEx( hCompanyKey, pszProfileName, 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw );
		}
	}
	if ( hSoftKey != NULL )
		RegCloseKey( hSoftKey );
	if ( hCompanyKey != NULL )
		RegCloseKey( hCompanyKey );

	return hAppKey;
}

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\pszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY GetSectionKey( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection  )
{
	ASSERT( pszSection != NULL );

	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetAppRegistryKey( pszRegistryKey, pszProfileName  );
	if ( hAppKey == NULL )
		return NULL;

	DWORD dw;
	RegCreateKeyEx( hAppKey, pszSection, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw );
	RegCloseKey( hAppKey );
	return hSectionKey;
}

UINT RegGetProfileInt( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, int nDefault )
{
	ASSERT( pszSection != NULL );
	ASSERT( pszEntry != NULL );
	HKEY hSecKey = GetSectionKey( pszRegistryKey, pszProfileName, pszSection );
	if ( hSecKey == NULL )
	{
		return nDefault;
	}
	else
	{
		DWORD dwValue;
		DWORD dwType;
		DWORD dwCount = sizeof( DWORD );
		LONG lResult = RegQueryValueEx( hSecKey, ( LPSTR )pszEntry, NULL, &dwType,
		( LPBYTE )&dwValue, &dwCount );
		RegCloseKey( hSecKey );
		if ( lResult == ERROR_SUCCESS )
		{
			ASSERT( dwType == REG_DWORD );
			ASSERT( dwCount == sizeof( dwValue ) );
			return ( UINT )dwValue;
		}
	}
	return nDefault;
}

// buffer pointed to by pszText must be 1024 bytes long.
LPCSTR RegGetProfileString( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, LPSTR pszValue, LPCSTR pszDefault )
{
	ASSERT( pszSection != NULL );
	ASSERT( pszEntry != NULL );
	HKEY hSecKey = GetSectionKey( pszRegistryKey, pszProfileName, pszSection );
	if ( hSecKey == NULL )
		return pszDefault;
	DWORD dwType, dwCount;
	LONG lResult = RegQueryValueEx( hSecKey, ( LPSTR )pszEntry, NULL, &dwType,
		NULL, &dwCount );
	if ( lResult == ERROR_SUCCESS )
	{
		ASSERT( dwType == REG_SZ );
		LPSTR pszOut = new char[ dwCount + 1 ];
		lResult = RegQueryValueEx( hSecKey, ( LPSTR )pszEntry, NULL, &dwType,
			( LPBYTE )pszOut, &dwCount );
		strncpy( pszValue, pszOut, min( 1024, dwCount ) );
		pszValue[ 1024 ] = '\0';
		delete [] pszOut;
	}
	RegCloseKey( hSecKey );
	if ( lResult == ERROR_SUCCESS )
	{
		ASSERT( dwType == REG_SZ );
		return pszValue;
	}
	return pszDefault;
}

BOOL RegGetProfileBinary( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry,	BYTE** ppData, UINT* pBytes )
{
	ASSERT( pszSection != NULL );
	ASSERT( pszEntry != NULL );
	ASSERT( ppData != NULL );
	ASSERT( pBytes != NULL );
	*ppData = NULL;
	*pBytes = 0;
	HKEY hSecKey = GetSectionKey( pszRegistryKey, pszProfileName, pszSection );
	if ( hSecKey == NULL )
		return FALSE;

	DWORD dwType, dwCount;
	LONG lResult = RegQueryValueEx( hSecKey, ( LPSTR )pszEntry, NULL, &dwType,
		NULL, &dwCount );
	*pBytes = dwCount;
	if ( lResult == ERROR_SUCCESS )
	{
		ASSERT( dwType == REG_BINARY );
		*ppData = new BYTE[*pBytes];
		lResult = RegQueryValueEx( hSecKey, ( LPSTR )pszEntry, NULL, &dwType,
			*ppData, &dwCount );
	}
	RegCloseKey( hSecKey );
	if ( lResult == ERROR_SUCCESS )
	{
		ASSERT( dwType == REG_BINARY );
		return TRUE;
	}
	else
	{
		delete [] *ppData;
		*ppData = NULL;
	}
	return FALSE;
}

BOOL RegWriteProfileInt( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, int nValue )
{
	ASSERT( pszSection != NULL );
	ASSERT( pszEntry != NULL );
	HKEY hSecKey = GetSectionKey( pszRegistryKey, pszProfileName, pszSection );
	if ( hSecKey == NULL )
		return FALSE;
	LONG lResult = RegSetValueEx( hSecKey, pszEntry, NULL, REG_DWORD,
		( LPBYTE )&nValue, sizeof( nValue ) );
	RegCloseKey( hSecKey );
	return lResult == ERROR_SUCCESS;
}

BOOL RegWriteProfileString( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, LPCSTR pszValue )
{
	ASSERT( pszSection != NULL );
	LONG lResult;
	if ( pszEntry == NULL ) //delete whole section
	{
		HKEY hAppKey = GetAppRegistryKey( pszRegistryKey, pszProfileName );
		if ( hAppKey == NULL )
			return FALSE;
		lResult = RegDeleteKey( hAppKey, pszSection );
		RegCloseKey( hAppKey );
	}
	else if ( pszValue == NULL )
	{
		HKEY hSecKey = GetSectionKey( pszRegistryKey, pszProfileName, pszSection );
		if ( hSecKey == NULL )
			return FALSE;
		// necessary to cast away const below
		lResult = RegDeleteValue( hSecKey, ( LPSTR )pszEntry );
		RegCloseKey( hSecKey );
	}
	else
	{
		HKEY hSecKey = GetSectionKey( pszRegistryKey, pszProfileName, pszSection );
		if ( hSecKey == NULL )
			return FALSE;
		lResult = RegSetValueEx( hSecKey, pszEntry, NULL, REG_SZ,
			( LPBYTE )pszValue, ( strlen( pszValue )+1 )*sizeof( char ) );
		RegCloseKey( hSecKey );
	}
	return lResult == ERROR_SUCCESS;
}

BOOL RegWriteProfileBinary( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, LPBYTE pData, UINT nBytes )
{
	ASSERT( pszSection != NULL );
	LONG lResult;
	HKEY hSecKey = GetSectionKey( pszRegistryKey, pszProfileName, pszSection );
	if ( hSecKey == NULL )
		return FALSE;
	lResult = RegSetValueEx( hSecKey, pszEntry, NULL, REG_BINARY,
		pData, nBytes );
	RegCloseKey( hSecKey );
	return lResult == ERROR_SUCCESS;
}

