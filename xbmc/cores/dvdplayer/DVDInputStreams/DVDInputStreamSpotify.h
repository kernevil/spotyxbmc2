#pragma once

/*
 spotyxbmc2 - A project to integrate Spotify into XBMC
 Copyright (C) 2013  Samuel Cabrero <samuelcabrero@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DVDInputStream.h"
#include "utils/RingBuffer.h"
#include "music/spotyXBMC/player/ISpotifyPlayer.h"
#include </usr/local/include/libspotify/api.h>

using namespace addon_music_spotify;
class CDVDInputStreamSpotify: public CDVDInputStream,
		public CDVDInputStream::ISeekTime,
		public ISpotifyPlayer {
public:
	CDVDInputStreamSpotify();
	virtual ~CDVDInputStreamSpotify();

	virtual bool Open(const char* strFile, const std::string& content);
	virtual void Close();
	virtual int Read(BYTE* buf, int buf_size);
	bool SeekTime(int iTimeInMsec);
	virtual int64_t Seek(int64_t offset, int whence);
	virtual bool IsEOF();
	virtual int64_t GetLength();
	virtual bool Pause(double dTime);

	int sp_musicDelivery(const sp_audioformat *format, const void *frames,
			int num_frames);
	void sp_endOfTrack();

private:
	bool loadPlayer();
	bool unloadPlayer();
	sp_session * getSession();

	sp_track *m_currentTrack;
	CRingBuffer m_buffer;
	//bool m_startStream;
	bool m_isPlayerLoaded;
	bool m_endOfTrack;
	int64_t m_totalTime; // time in milliseconds
};

