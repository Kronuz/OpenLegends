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
/*! \file		GraphicsFactory.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface of the CGraphicsFactory class.
	\date		June 26, 2003
*/

#pragma once
#include <IGraphics.h>

/////////////////////////////////////////////////////////////////////////////
/*! \class		CGraphicsFactory
	\brief		CGraphicsFactory class.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		June 26, 2003

	This class generates the interface to a graphics device through
	the use of plugins in DLL files.
*/
class CGraphicsFactory
{
	static CGraphicsFactory *_instance;
	CString sIName;
   	HMODULE m_hDLL;

public:
	static HRESULT Delete(IGraphics **pGraphics);
	static HRESULT New(IGraphics **pGraphics, LPCSTR lpszIName = NULL);

	CGraphicsFactory();
	~CGraphicsFactory();
};


