/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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

#pragma once

interface IConsole
{
	virtual int print(const char *format, va_list argptr) = 0;
	virtual int error(int number, char *message, char *filename, int firstline, int lastline, va_list argptr) = 0;

	virtual int printf(const char *format, ...) = 0;
	virtual int getch() = 0;
	virtual int putch(int c) = 0;
	virtual int gets(char *buffer, int buffsize) = 0;
};
