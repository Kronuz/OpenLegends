
#pragma once

#define FEF_NOEXT		0x01
#define FEF_NODIR		0x02

#define FEF_LOWERCASE	0x10
#define FEF_BEAUTIFY	0x14
#define FEF_UPPERCASE	0x20

typedef int CALLBACK FILESPROC(LPCTSTR lpFile, LPARAM lParam);
int ForEachFile(LPCTSTR lpFileName, FILESPROC ForEach, LPARAM lParam, int flags = FEF_BEAUTIFY);
