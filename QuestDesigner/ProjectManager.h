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
/*! \file		ProjectManager.h 
	\brief		Interface of the CProjectManager class.
	\date		April 15, 2003

	This file defines the classes needed to handle the game project.
	The project manager is in charge of pretty much everything that has
	to do with the game. This includes the sounds, the sprites, the scripts,
	the saved files, the maps and the entities and its properties.

	The project manager makes use of several classes defined in four main
	modules: the File Manager, which is in charge of saving and loading the
	needed files for the project; the Sprite Manager, with the job of
	maintaining the sprites: entities, masks and backgrounds of the game, along
	with their properties; the Sound Manager, which maintains the full list of
	sounds used by the game; and a World Manager, which provides the necessary
	methods and structures to build the whole quest.
	
	The project manager is perhaps the most	important module of all, since it 
	unites all other four modules into one working engine.
*/

#pragma once

#include "SCompiler.h"

#include "interfaces.h"
#include "SpriteManager.h"

/////////////////////////////////////////////////////////////////////////////
/*! \class		CProjectManager
	\brief		CProjectManager class.
	\author		Kronuz1
	\version	1.0
	\date		April 15, 2003

	This class Is the one that manages everything in the project,
	from the sprite sheets list and the sounds list, to the world and
	map of the game.
*/
class CProjectManager :
	public CConsole,
	public IIOObject
{
	CString m_sProjectName;
	int m_iStep, m_iCnt1, m_iCnt2;
protected:
	CSimpleMap<CString, CSprite*> m_UndefSprites;
	CSimpleMap<CString, CScript*> m_Scripts;
	CSimpleArray<CSpriteSheet*> m_SpriteSheets;

	CSprite *CreateSprite(_spt_type sptType, LPCSTR sName);

public:
	static int CALLBACK LoadSheet(LPCTSTR szFile, LPARAM lParam);

	CString& GetProjectName() {return m_sProjectName; }

	~CProjectManager();

	LRESULT StartBuild();
	LRESULT BuildNextStep(WPARAM wParam, LPARAM lParam);

	CSprite *ReferSprite(CString &sName, _spt_type sptType);
	CSprite *DefineSprite(CString &sName, _spt_type sptType, CSpriteSheet *pSpriteSheet);
	CScript *DefineScript(CString &sName);

	bool Load(LPCSTR szFile);
	bool Save(LPCSTR szFile) { return false; }

};
