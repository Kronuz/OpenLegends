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

enum _ftype { t_printf, t_error };
struct InfoStruct {
	_ftype type;
	const char *message;
	va_list argptr;
	int number;
	char *filename;
	int firstline;
	int lastline;
};

/*! 
	\class		CConsole
	\brief		Inormation window interface for output.
	\author		Kronuz
	\version	1.0
	\date		April 23, 2003
*/
class CConsole :
	public IConsole
{
	static CConsole *_instance;
public:
	static HWND ms_hWnd; //! Window handler to send the information

	char *gets(char *buffer) { return NULL; }
	int print(const char *format, va_list argptr);
	int printf(const char *format, ...);
	int putch(int c) { printf("%c", c); return 1; }

	int gets(char *buffer, int buffsize) { return 0; }
	int getch() { return 0; }

	int error(int number, char *message, char *filename, int firstline, int lastline, va_list argptr);

	static CConsole *Instance();
};
