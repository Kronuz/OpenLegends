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
/*! \file		Console.h 
	\brief		Interface of the Console class.
	\date		April 23, 2003

	\todo		Convert this class to a singleton to eliminate its derived classes.
*/

#pragma once

#include "../IConsole.h"

class CConsole : public IConsole
{
	static CConsole *_instance;
	FILE *file;
public:
	CConsole() : file(NULL) {}
	~CConsole() { if(file) fclose(file); }

	char *gets(char *buffer) { return NULL; }
	int print(const char *format, va_list argptr) { 
		if(!file) file = fopen("c:\\log.txt", "wt");
		vfprintf(file, format, argptr);
		fflush(file);
		return 0; 
	}
	int printf(const char *format, ...) { 
		va_list argptr;
		va_start(argptr, format);
		int ret = print(format, argptr);
		va_end(argptr);
		return ret;
	}
	int error(int number, char *message, char *filename, int firstline, int lastline, va_list argptr) { return 0; }

	static CConsole* Instance() {
		if(_instance) return _instance;
		_instance = new CConsole;
		return _instance;
	}
};
