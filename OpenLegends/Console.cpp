/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germán Méndez Bravo (Kronuz)
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
/*! \file		Console.cpp
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the Console classes.
	\date		April 23, 2003

	This file implements all the classes that handles all output messages,
	to the user.
*/

#include "stdafx.h"
#include "Console.h"

////////////////////////////////////////////////
// Externals
CConsole* CConsole::_instance = NULL;
char CConsole::ms_szLogFile[MAX_PATH] = "";

////////////////////////////////////////////////

CConsole *CConsole::Instance() {
	if(_instance == NULL) {
		_instance = new CConsole;

		// Find out where the program file is located:
		GetModuleFileName(NULL, ms_szLogFile, MAX_PATH);

		// Build a logfile name on the same path:
		LPSTR aux = strchr(ms_szLogFile, '\0');
		while(aux != ms_szLogFile && *aux != '\\') aux--;
		if(*aux == '\\') aux++;
		*aux = '\0';
		strcat(ms_szLogFile, "OpenLegends.log");

		// Create and clean the logfile:
		FILE *arch = ::fopen(ms_szLogFile, "w");
		if(arch) {
			::fprintf(arch, " --- Log started for Open Legends --- \n");
			::fclose(arch);
		}
	}
	return _instance;
}

/*!
	\param format Format specification.
	\param argptr Pointer to list of arguments.
	\return returns the number of characters written, 
		not including the terminating null character, 
		or a negative value if an output error occurs
	\remarks This function takes a pointer to an argument list, and then 
		formats and writes a log file.

	Called for general purpose "console" output. This function prints general
	purpose messages. The function is modelled after vprintf().
*/
int CConsole::vprintf(const char *format, va_list argptr) { 
	// This creates a log (for debugging purposes only):
	FILE *arch = ::fopen(ms_szLogFile, "at");
	if(arch) {
		::vfprintf(arch, format, argptr);
		::fclose(arch);
	}
	return 0; 
}
/*!
	\param format Format specification.
	\param ... argument Optional arguments.
	\return returns the number of characters written, 
		not including the terminating null character, 
		or a negative value if an output error occurs

	Called for general purpose "console" output. This function prints general
	purpose messages. The function is modelled after printf().
*/
int CConsole::printf(const char *format, ...) { 
	va_list argptr;
	va_start(argptr, format);
	int ret = vprintf(format, argptr);
	va_end(argptr);
	return ret;
}


int CConsole::vfprintf(const char *format, va_list argptr) { 
	// This creates a log (for debugging purposes only):
	FILE *arch = ::fopen(ms_szLogFile, "at");
	if(arch) {
		::vfprintf(arch, format, argptr);
		::fclose(arch);
	}
	return 0; 
}
int CConsole::fprintf(const char *format, ...) { 
	va_list argptr;
	va_start(argptr, format);
	int ret = vfprintf(format, argptr);
	va_end(argptr);
	return ret;
}
