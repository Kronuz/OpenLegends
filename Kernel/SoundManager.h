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

	bool m_bLoaded;
	int m_nLoopBack;

	bool LoadFile();
public:
	CSound();
	~CSound();

	void SetSource(CBString sPath);
	bool SourceExists();
	bool UnloadFile();
	CBString GetTitle() { return m_vFile.GetFileName(); }

	virtual DWORD Play(bool _bForever = false);
	virtual void Loop(int _repeat = -1);
	virtual void Stop(DWORD ID);
	virtual void Pause(DWORD ID);

	virtual bool IsPlaying(DWORD ID);

	virtual void SetVolume(DWORD ID, int _volume);
	virtual int GetVolume(DWORD ID);

	virtual int GetLoopBack() const { return m_nLoopBack; }
	virtual void SetLoopBack(int _loop) { m_nLoopBack = _loop; }

	virtual void SetCurrentPosition(DWORD ID, int _pos);

	virtual LPCSTR GetSoundFilePath(LPSTR szPath, size_t buffsize);

	virtual LPCSTR GetSoundFileName(LPSTR szFileName, size_t buffsize);

};

// this is the singleton sound manager.
class CSoundManager :
	public ISoundManager
{
	static CSoundManager *_instance;

	ISound *m_pCurrentMusic;
	ISound *m_pCurrentMusicIn;
	ISound *m_pNextMusic;
	float m_fMusicFadeOut;
	float m_fMusicFadeIn;
	float m_fFadeSpeed;
	float m_fMusicTimeOut;
	int m_nMusicVolume;

	DWORD m_dwMusicID;
	DWORD m_dwMusicInID;

	void FadeOutMusic();
	void FadeInMusic();

public:
	static bool ms_bSound;

	CSoundManager();
	~CSoundManager();
	bool InitSound();

	virtual void DoMusic();
	virtual void SwitchMusic(ISound *_pSound, int _loopback, bool _fade = true);
	virtual void SetMusicVolume(int _volume) { m_nMusicVolume = _volume; }
	virtual int GetMusicVolume() const { return m_nMusicVolume; };
	virtual void SetMusicFadeSpeed(int _speed) { m_fFadeSpeed = (float)_speed; }
	int GetMusicFadeSpeed() const { return (int)m_fFadeSpeed; }

	static CSoundManager* Instance() {
		if(!_instance) {
			_instance = new CSoundManager;
		}
		return _instance;
	}
};
