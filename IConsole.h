
#pragma once

interface IConsole
{
	virtual char *gets(char *buffer) = 0;
	virtual int print(const char *format, va_list argptr) = 0;
	virtual int printf(const char *format, ...) = 0;
	virtual int error(int number, char *message, char *filename, int firstline, int lastline, va_list argptr) = 0;
};
