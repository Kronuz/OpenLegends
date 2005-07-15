/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germ�n M�ndez Bravo (Kronuz)
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
	\author		Germ�n M�ndez Bravo (Kronuz)
	\brief		Implementation of the Console classes.
	\date		April 23, 2003:
						* Creation date.

	This file implements all the classes that handles all output messages,
	to the user.
*/

#include "stdafx.h"
#include "Console.h"

////////////////////////////////////////////////
// Externals
HWND CConsole::ms_hWnd = NULL;
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
		strcat(ms_szLogFile, "QuestDesigner.log");

		// Create and clean the logfile:
		FILE *arch = ::fopen(ms_szLogFile, "w");
		if(arch) {
			::fprintf(arch, " --- Log started for "QD_NAME" --- \n");
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
		formats and sends the given data to the window ms_hWnd
		associated with the compiler class in the form of a 
		WMQD_MESSAGE message.

	Called for general purpose "console" output. This function prints general
	purpose messages; errors go through sc_error(). The function is modelled
	after vprintf().
*/
int CConsole::vprintf(const char *format, va_list argptr) 
{
	InfoStruct info;
	info.type = t_printf;
	info.message = format;
	info.argptr = argptr;
	if(!::IsWindow(ms_hWnd)) return -1;

	// This creates a log (for debugging purposes only):
	FILE *arch = ::fopen(ms_szLogFile, "at");
	if(arch) {
		::vfprintf(arch, format, argptr);
		::fclose(arch);
	}

	SendMessage(ms_hWnd, WMQD_MESSAGE, 0, (LPARAM)&info);
	return 1;
}

/*!
	\param format Format specification.
	\param ... argument Optional arguments.
	\return returns the number of characters written, 
		not including the terminating null character, 
		or a negative value if an output error occurs

	Called for general purpose "console" output. This function prints general
	purpose messages; errors go through sc_error(). The function is modelled
	after vprintf().
*/
int CConsole::printf(const char *format, ...) 
{
	va_list argptr;
	va_start(argptr, format);
	int ret = vprintf(format, argptr);
	va_end(argptr);
	return ret;
}

int CConsole::vfprintf(const char *format, va_list argptr) 
{
	InfoStruct info;
	info.type = t_printf;
	info.message = format;
	info.argptr = argptr;

	// This creates a log (for debugging purposes only):
	FILE *arch = ::fopen(ms_szLogFile, "at");
	if(arch) {
		::vfprintf(arch, format, argptr);
		::fclose(arch);
	}
	return 1;
}
int CConsole::fprintf(const char *format, ...) 
{
	va_list argptr;
	va_start(argptr, format);
	int ret = vfprintf(format, argptr);
	va_end(argptr);
	return ret;
}

/*! 
     \param number The error number (as documented in the manual)
     \param message A string describing the error with embedded %d and %s tokens
     \param filename The name of the file currently being parsed
     \param firstline The line number at which the expression started on which
			the error was found, or -1 if there is no "starting line"
     \param lastline Tthe line number at which the error was detected
     \param argptr A pointer to the first of a series of arguments (for macro "va_arg")
     \return If the function returns 0, the parser attempts to continue compilation.
		On a non-zero return value, the parser aborts.

	 Called for producing error output.
*/

int CConsole::error(int number, char *message, char *filename, int firstline, int lastline, va_list argptr)
{
	InfoStruct info;
	info.type = t_error;
	info.number = number;
	info.message = message;
	info.filename = filename;
	info.firstline = firstline;
	info.lastline = lastline;
	info.argptr = argptr;

	if(!::IsWindow(ms_hWnd)) return -1;
	SendMessage(ms_hWnd, WMQD_MESSAGE, 0, (LPARAM)&info);

	return 0;
}