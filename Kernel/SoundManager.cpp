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
/*! \file		SoundManager.cpp
	\brief		Implementation of the Sound System.
	\date		August 29, 2003
*/

#include "stdafx.h"

#include "SoundManager.h"
#include "GameManager.h"

CSound::CSound() :
	m_pData(NULL),
	eType(stUnknown),
	m_bLoaded(false)
{
	
}
CSound::~CSound()
{
	delete []m_pData;
}

LPCSTR CSound::GetSoundFilePath(LPSTR szPath, size_t buffsize)
{
	strncpy(szPath, (LPCSTR)m_vFile.GetFilePath(), buffsize);
	return szPath;
}

inline bool CSound::LoadFile() 
{
	if(m_bLoaded) return true;

	if(m_vFile.Open()) {
		m_nLength = m_vFile.GetFileSize();
		if(m_nLength > 0) {
			if(eType == stSample && m_nLength > 100*1024)
				eType = stStream;
			delete []m_pData;
			m_pData = new BYTE[m_nLength];
			if(!m_pData) {
				CONSOLE_PRINTF("Kernel Fatal Error: Not enough memory to hold %d bytes!\n", m_vFile.GetFileSize());
			} else m_vFile.ReadFile(m_pData, m_nLength);
		} else eType = stUnknown;
		m_vFile.Close();
	} else eType = stUnknown;

	if(!m_pData) return false;

	switch(eType) {
		case stMusic: {
			m_pMusic = FMUSIC_LoadSongEx(
				(char*)m_pData,
				m_nLength,
				FSOUND_LOADMEMORY | FSOUND_LOOP_NORMAL,
				NULL,
				0);
			if(!m_pMusic) eType = stUnknown;
			delete []m_pData;
			m_pData = NULL;
			m_nLength = 0;
			break;
		}
		case stStream: {
			m_pStream = FSOUND_Stream_OpenFile(
				(char*)m_pData, 
				FSOUND_NORMAL | FSOUND_MPEGACCURATE | FSOUND_LOADMEMORY, 
				m_nLength);
			if(!m_pStream) eType = stUnknown;
			break;
		}
		case stSample: {
			m_pSample = FSOUND_Sample_Load(
				FSOUND_UNMANAGED, 
				(char*)m_pData, 
				FSOUND_NORMAL | FSOUND_MPEGACCURATE | FSOUND_LOADMEMORY, 
				m_nLength);
			if(!m_pSample) eType = stUnknown;
			delete []m_pData;
			m_pData = NULL;
			m_nLength = 0;
			break;
		}
	}
	if(eType == stUnknown) return false;

	return true;
}

void CSound::SetSource(CBString sPath) {

	if(CSoundManager::ms_bSound == false) return;

	m_vFile.SetFilePath(sPath);

	LPCSTR szExt = m_vFile.GetFileExt();
		
		 if(!stricmp(szExt, ".it"))		eType = stMusic;
	else if(!stricmp(szExt, ".mid"))	eType = stMusic;
	else if(!stricmp(szExt, ".mod"))	eType = stMusic;
	else if(!stricmp(szExt, ".wav"))	eType = stSample;
	else if(!stricmp(szExt, ".mp3"))	eType = stSample;
	else if(!stricmp(szExt, ".ogg"))	eType = stSample;
	else if(!stricmp(szExt, ".rmi"))	eType = stMusic;
	else if(!stricmp(szExt, ".s3m"))	eType = stMusic;
	else if(!stricmp(szExt, ".xm"))		eType = stMusic;
	else if(!stricmp(szExt, ".raw"))	eType = stSample;
	else if(!stricmp(szExt, ".mp2"))	eType = stSample;
	else if(!stricmp(szExt, ".wma"))	eType = stSample;
	else if(!stricmp(szExt, ".asf"))	eType = stSample;
	else								eType = stUnknown;
}

bool CSound::SourceExists()
{
	return m_vFile.FileExists();
}

bool CSound::UnloadFile()
{
	if(!m_bLoaded) return false;

	ASSERT(eType != stUnknown);
	if(eType == stUnknown) return true;

	switch(eType) {
		case stMusic: {
			if(m_pMusic) {
				if(FMUSIC_IsPlaying(m_pMusic))
					FMUSIC_StopSong(m_pMusic);

				if(FMUSIC_FreeSong(m_pMusic) == FALSE) return false;
				m_pMusic = NULL;
			}
			break;
		}
		case stStream: {
			if(m_pStream) {
				if(FSOUND_Stream_Close(m_pStream) == FALSE) return false;
				m_pStream = NULL;
			}
			break;
		}
		case stSample: {
			if(m_pSample) {
				FSOUND_Sample_Free(m_pSample);
				m_pSample = NULL;
			}
			break;
		}
	}
	delete []m_pData;
	m_pData = NULL;
	m_nLength = 0;

	eType = stUnknown;
	m_bLoaded = false;
	return true;
}

DWORD CSound::Play(bool _bForever)
{
	if(!LoadFile()) return -1;

	ASSERT(eType != stUnknown);

	switch(eType) {
		case stMusic: {
			FMUSIC_PlaySong(m_pMusic);
			return (DWORD)m_pMusic;
		}
		case stStream: {
			return FSOUND_Stream_Play(FSOUND_FREE, m_pStream);
		}
		case stSample: {
			return FSOUND_PlaySound(FSOUND_FREE, m_pSample);
		}
	}
	if(_bForever) Loop();

	return -1;
}
void CSound::Loop(int _repeat)
{
	int mode = FSOUND_LOOP_OFF;
	if(_repeat) mode = FSOUND_LOOP_NORMAL;

	switch(eType) {
		case stMusic: {
			break;
		}
		case stStream: {
			FSOUND_Stream_SetMode(m_pStream, mode);
			break;
		}
		case stSample: {
			FSOUND_Sample_SetMode(m_pSample, mode);
			break;
		}
	}
}

void CSound::Stop(DWORD ID)
{
	ASSERT(eType != stUnknown);

	if(!IsPlaying(ID)) return;

	switch(eType) {
		case stMusic: {
			FMUSIC_StopSong(m_pMusic);
			break;
		}
		case stStream: {
			FSOUND_Stream_Stop(m_pStream);
			break;
		}
		case stSample: {
			FSOUND_StopSound((int)ID);
			break;
		}
	}
}
void CSound::Pause(DWORD ID)
{
	ASSERT(eType != stUnknown);

	switch(eType) {
		case stMusic: {
			break;
		}
		case stStream: {
			break;
		}
		case stSample: {
			break;
		}
	}
}

bool CSound::IsPlaying(DWORD ID)
{
	ASSERT(eType != stUnknown);

	switch(eType) {
		case stMusic: {
			return (FMUSIC_IsPlaying(m_pMusic) == TRUE);
		}
		case stStream: {
			return (FSOUND_IsPlaying((int)ID) == TRUE);
			break;
		}
		case stSample: {
			return (FSOUND_IsPlaying((int)ID) == TRUE);
		}
	}
	return false;
}
void CSound::SetCurrentPosition(DWORD ID, int _pos)
{
	ASSERT(eType != stUnknown);

	switch(eType) {
		case stMusic: {
			FMUSIC_SetOrder(m_pMusic, _pos);
			break;
		}
		case stStream: {
			FSOUND_SetCurrentPosition((int)ID, _pos);
			break;
		}
		case stSample: {
			FSOUND_SetCurrentPosition((int)ID, _pos);
			break;
		}
	}
}

void CSound::SetVolume(DWORD ID, int _volume)
{
	ASSERT(eType != stUnknown);

	switch(eType) {
		case stMusic: {
			FMUSIC_SetMasterVolume(m_pMusic, _volume);
			break;
		}
		case stStream: {
			FSOUND_SetVolume((int)ID, _volume);
			break;
		}
		case stSample: {
			FSOUND_SetVolume((int)ID, _volume);
			break;
		}
	}
}

int CSound::GetVolume(DWORD ID)
{
	ASSERT(eType != stUnknown);

	switch(eType) {
		case stMusic: {
			return FMUSIC_GetMasterVolume(m_pMusic);
		}
		case stStream: {
			return FSOUND_GetVolume((int)ID);
			break;
		}
		case stSample: {
			return FSOUND_GetVolume((int)ID);
		}
	}
	return 0;
}


CSoundManager *CSoundManager::_instance = NULL;
bool CSoundManager::ms_bSound = false;

CSoundManager::CSoundManager() :
	m_pCurrentMusic(NULL),
	m_pCurrentMusicIn(NULL),
	m_pNextMusic(NULL),
	m_fMusicFadeOut(-1.0f),
	m_fMusicFadeIn(-1.0f),
	m_fFadeSpeed(110.0f),
	m_fMusicTimeOut(0.0f),
	m_nMusicVolume(200)
{
	ms_bSound = InitSound();
}

CSoundManager::~CSoundManager()
{
	// Stop Sound
	FSOUND_Close();
}

bool CSoundManager::InitSound()
{
	// Check for the FMOD DLL
	if(FSOUND_GetVersion() < FMOD_VERSION)
		return false;

	// Init Sound
	if(!FSOUND_Init(44100, 32, 0))
		return false;

	return true;
}

void CSoundManager::SwitchMusic(ISound *_pSound, int _loopback, bool _fade)
{
	if(_fade) {
		// If the music to be switched into is the same as the current being played,
		// recover from fade out in case of fading out:
		if(_pSound == m_pCurrentMusic && m_pCurrentMusicIn==NULL && m_fMusicFadeOut >= 0) {
			m_dwMusicInID = m_dwMusicID;
			m_pCurrentMusicIn = m_pCurrentMusic;
			m_pCurrentMusic = NULL;
			m_fMusicFadeIn = m_fMusicFadeOut;
			m_fMusicFadeOut = -1;
			m_pNextMusic = NULL;
			CONSOLE_DEBUG("Recovering %s...\n", 
				m_pCurrentMusicIn?static_cast<CSound*>(m_pCurrentMusicIn)->GetTitle():"NUL"
			);
			return;
		}

		if(_pSound == m_pCurrentMusic && m_pCurrentMusicIn==NULL && m_pNextMusic == NULL) return;
		if(_pSound) {
			if(_pSound == m_pCurrentMusicIn && m_pNextMusic == NULL) return;
		}
		CONSOLE_DEBUG("Making switch...\n");

		// If there is not a switch already in progress:
		if(m_pNextMusic == NULL && m_pCurrentMusicIn==NULL) {
			// Fade the Music out
			if(m_pCurrentMusic) {
				m_fMusicFadeOut = (float)m_pCurrentMusic->GetVolume(m_dwMusicID);
				CONSOLE_DEBUG("Fading out %s...\n", 
					m_pCurrentMusic?static_cast<CSound*>(m_pCurrentMusic)->GetTitle():"NUL"
				);
			} else {
				m_fMusicFadeOut = -1;
				m_fMusicFadeIn = 0;
			}
		}
		// Point the next music to this new music
		m_pNextMusic = _pSound;
	} else {
		m_pNextMusic = NULL;
		m_fMusicFadeOut = -1;
		m_fMusicFadeOut = (float)m_nMusicVolume;
		if(m_pCurrentMusicIn) {
			CONSOLE_DEBUG("Stopping %s.\n", 
				m_pCurrentMusicIn?static_cast<CSound*>(m_pCurrentMusicIn)->GetTitle():"NUL"
			);
			m_pCurrentMusicIn->Stop(m_dwMusicInID);
			m_pCurrentMusicIn = NULL;
		}
		if(m_pCurrentMusic) {
			CONSOLE_DEBUG("Stopping %s.\n", 
				m_pCurrentMusic?static_cast<CSound*>(m_pCurrentMusic)->GetTitle():"NUL"
			);
			m_pCurrentMusic->Stop(m_dwMusicID);
		}
		m_pCurrentMusic = _pSound;
		if(m_pCurrentMusic) {
			CONSOLE_DEBUG("Playing %s.\n", 
				m_pCurrentMusic?static_cast<CSound*>(m_pCurrentMusic)->GetTitle():"NUL"
			);
			m_dwMusicID = m_pCurrentMusic->Play(true);
			m_pCurrentMusic->SetVolume(m_dwMusicID, m_nMusicVolume);
		}
	}
	if(_pSound) _pSound->SetLoopBack(_loopback);
}

void CSoundManager::FadeOutMusic()
{
	m_fMusicFadeOut -= m_fFadeSpeed * CGameManager::GetFPSDelta();

	// Set the new Volume
	if(m_pCurrentMusic) {
		m_pCurrentMusic->SetVolume(m_dwMusicID, (int)m_fMusicFadeOut);
	}

	// Change the Music (Fade Out/In)
	if(m_pCurrentMusicIn==NULL) {
		if(m_fMusicFadeOut <= m_nMusicVolume/3) {
			m_fMusicFadeIn = 0;
		}
	}

	// Check if the fade has ended yet
	if(m_fMusicFadeOut <= 5) {
		m_fMusicFadeOut = -1;
		if(m_pCurrentMusic) {
			CONSOLE_DEBUG("Stopping %s, faded out.\n", 
				m_pCurrentMusic?static_cast<CSound*>(m_pCurrentMusic)->GetTitle():"NUL"
			);
			m_pCurrentMusic->Stop(m_dwMusicID);
			m_pCurrentMusic = NULL;
		}
	}
}

void CSoundManager::FadeInMusic()
{
	if(m_pCurrentMusicIn == NULL && m_pNextMusic != NULL) {
		m_pCurrentMusicIn = m_pNextMusic;
		m_pNextMusic = NULL;
		// Point the next music to the new comming music
		if(m_pCurrentMusicIn) {
			m_fMusicTimeOut = 5;
				
			// Start the music playing
			m_dwMusicInID = m_pCurrentMusicIn->Play(true);
		}
		CONSOLE_DEBUG("Fading in %s...\n", 
			m_pCurrentMusicIn?static_cast<CSound*>(m_pCurrentMusicIn)->GetTitle():"NUL"
		);
	}

	ISound *pMusic = m_pCurrentMusicIn;
	DWORD dwID = m_dwMusicInID;
		
	m_fMusicFadeIn += m_fFadeSpeed * CGameManager::GetFPSDelta() * 2;
	if(m_fMusicFadeIn >= m_nMusicVolume) {
		m_dwMusicID = m_dwMusicInID;
		m_pCurrentMusic = m_pCurrentMusicIn;
		m_pCurrentMusicIn = NULL;

		m_fMusicFadeIn = (float)m_nMusicVolume;
		pMusic = m_pCurrentMusic;
		dwID = m_dwMusicID;

		if(m_pNextMusic) {
			m_fMusicFadeOut = m_fMusicFadeIn;
		}
		if(m_pCurrentMusic) {
			CONSOLE_DEBUG("%s faded in.\n", 
				static_cast<CSound*>(m_pCurrentMusic)->GetTitle()
			);
		}
	}

	// Set the new Volume
	if(pMusic) {
		pMusic->SetVolume(dwID, (int)m_fMusicFadeIn);
	}

}

void CSoundManager::DoMusic()
{
	// Check if the Music is fading
	if(m_fMusicFadeOut >= 0)
		FadeOutMusic();

	if(m_fMusicFadeIn < m_nMusicVolume)
		FadeInMusic();

	if(!m_pCurrentMusic) return;

	// If the current song stops go back to a specific place
	if(!m_pCurrentMusic->IsPlaying(m_dwMusicID) && m_fMusicTimeOut <= 0) {
		// Song has stopped so play it again
		m_dwMusicID = m_pCurrentMusic->Play(true);
		m_fMusicTimeOut = 5;

		if(m_pCurrentMusic->GetLoopBack() >= 0) {
			m_pCurrentMusic->SetCurrentPosition(m_dwMusicID, m_pCurrentMusic->GetLoopBack());
		}
	}

	if(m_fMusicTimeOut > 0)
		m_fMusicTimeOut -= CGameManager::GetFPSDelta();

}
