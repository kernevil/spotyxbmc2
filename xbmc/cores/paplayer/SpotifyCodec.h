#pragma once
/*
 spotyxbmc2 - A project to integrate Spotify into XBMC
 Copyright (C) 2011  David Erenger
               2013  Samuel Cabrero <samuelcabrero@gmail.com>

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

 For contact with the author:
 david.erenger@gmail.com
 */

#include "cores/paplayer/CachingCodec.h"
#include "music/spotyXBMC/player/ISpotifyPlayer.h"
#include <libspotify/api.h>

using namespace addon_music_spotify;
class SpotifyCodec: public CachingCodec, public ISpotifyPlayer {
public:
	SpotifyCodec();
	virtual ~SpotifyCodec();

	bool Init(const CStdString &strFile, unsigned int filecache);
	void DeInit();
	bool CanSeek();
	int64_t Seek(int64_t iSeekTime);
	int ReadPCM(BYTE *pBuffer, int size, int *actualsize);
	bool CanInit();
	CAEChannelInfo GetChannelInfo();

	int sp_musicDelivery(const sp_audioformat *format, const void *frames,
			int num_frames);
	void sp_endOfTrack();

private:
	bool loadPlayer();
	bool unloadPlayer();

	sp_session * getSession();
	sp_track *m_currentTrack;
	bool m_startStream;
	bool m_isPlayerLoaded;
	bool m_endOfTrack;
	int m_bufferSize;
	char *m_buffer;
	int m_bufferPos;
};
