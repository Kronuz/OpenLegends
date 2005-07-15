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
/*! \file		Console.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the Console class.
	\date		April 23, 2003

	\todo		Convert this class to a singleton to eliminate its derived classes.
*/

#pragma once

#include <IConsole.h>

class CConsole : public IConsole
{
	static char ms_szLogFile[MAX_PATH];
	static CConsole *_instance;
public:

//////////////////////////////////////////////////////
// Interface:
	int vprintf(const char *format, va_list argptr);
	int printf(const char *format, ...);

	int vfprintf(const char *format, va_list argptr);
	int fprintf(const char *format, ...);

	int getch() { return 0; }
	int putch(int c) { printf("%c", c); return 1; }
	int gets(char *buffer, int buffsize) { return 0; }

	int error(int number, char *message, char *filename, int firstline, int lastline, va_list argptr) { return 0; }

	static CConsole* Instance();
};
