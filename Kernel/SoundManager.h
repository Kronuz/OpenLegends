/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germán Méndez Bravo)
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
/////////////////////////////////////////////////////////////////////////////
/*! \file		SoundManager.h 
	\brief		Interface of the CSound and CMusic classes.
	\date		April 28, 2003

	Classes that handle sounds and music in the game.
	
	\todo Everything is missing on this module :)

*/

#pragma once

#include "../IGame.h"

#include "libs/fmod/fmod.h"

// each sound or music is a CSound
class CSound :
	public ISound
{
	int m_nLength;
	LPBYTE m_pData;
	CVFile m_vFile;
	enum SoundType { stUnknown, stStream, stSample, stMusic } eType;
	union {
		FSOUND_STREAM *m_pStream;
		FSOUND_SAMPLE *m_pSample;
		FMUSIC_MODULE *m_pMusic;
	};

public:
	CSound();
	~CSound();

	bool LoadFile(CVFile &vFile);
	bool UnloadFile();

	virtual void Play() const;
	virtual void Loop(int _repeat = -1);
	virtual void Stop() const;
	virtual void FadeOut(bool _fade = true);
	virtual void FadeIn(bool _fade = true);

	virtual int SetFadeSpeed(int _speed);
	virtual int SetVolume(int _volume);

	virtual int GetFadeSpeed() const;
	virtual int GetVolume() const;

};

// this is the singleton sound manager.
class CSoundManager {
	static CSoundManager *_instance;
public:
	static bool ms_bSound;

	CSoundManager();
	~CSoundManager();
	bool InitSound();

	void DoMusic();
	void SwitchMusic(ISound *pSound_);
	void FadeSound();
	int SetVolume(int _volume);
	int GetVolume() const;

	static CSoundManager* Instance() {
		if(!_instance) {
			_instance = new CSoundManager;
		}
		return _instance;
	}
};
