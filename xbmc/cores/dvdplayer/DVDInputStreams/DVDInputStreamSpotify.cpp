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

#include "DVDInputStreamSpotify.h"
#include "playlists/PlayList.h"
#include "PlayListPlayer.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "music/spotyXBMC/session/Session.h"
#include "music/spotyXBMC/radio/RadioHandler.h"
#include "music/spotyXBMC/player/PlayerHandler.h"
#include "utils/RingBuffer.h"

#define XMIN(a,b) ((a)<(b)?(a):(b))

using namespace addon_music_spotify;
using namespace PLAYLIST;

CDVDInputStreamSpotify::CDVDInputStreamSpotify() :
		CDVDInputStream(DVDSTREAM_TYPE_SPOTIFY) {
	m_buffer.Destroy();
	m_currentTrack = 0;
	m_isPlayerLoaded = false;
	m_endOfTrack = false;
	m_totalTime = 0;
}

CDVDInputStreamSpotify::~CDVDInputStreamSpotify() {
	Close();
}

bool CDVDInputStreamSpotify::Open(const char* strFile,
		const std::string& content) {
	m_buffer.Create(2048 * sizeof(int16_t) * 50);

	CStdString uri = URIUtils::GetFileName(strFile);
	CStdString extension = uri.Right(uri.GetLength() - uri.Find('.') - 1);
	if (extension.Left(12) == "spotifyradio") {
		//if its a radiotrack the radionumber and tracknumber is secretly
		// encoded at the end of the extension
		CStdString trackStr = extension.Right(
				extension.GetLength() - extension.ReverseFind('#') - 1);
		CStdString radioNumber = extension.Left(uri.Find('#'));
		radioNumber = radioNumber.Right(
				radioNumber.GetLength() - radioNumber.Find('#') - 1);
		RadioHandler::getInstance()->pushToTrack(atoi(radioNumber),
				atoi(trackStr));
	}

	//we have a non legit extension so remove it manually
	uri = uri.Left(uri.Find('.'));

	Logger::printOut("trying to load track:");
	Logger::printOut(uri);
	sp_link *spLink = sp_link_create_from_string(uri);
	m_currentTrack = sp_link_as_track(spLink);
	sp_track_add_ref(m_currentTrack);
	sp_link_release(spLink);
	m_endOfTrack = false;
	m_isPlayerLoaded = false;
	m_totalTime = sp_track_duration(m_currentTrack);

	// Prefetch the next track!

	CPlayList & playlist = g_playlistPlayer.GetPlaylist(PLAYLIST_MUSIC);
	int nextSong = g_playlistPlayer.GetNextSong();

	if (nextSong >= 0 && nextSong < playlist.size()) {
		CFileItemPtr song = playlist[nextSong];
		if (song != NULL) {
			CStdString uri = song->GetPath();
			if (uri.Left(7).Equals("spotify")) {
				uri = uri.Left(uri.Find('.'));
				Logger::printOut("prefetching track:");
				Logger::printOut(uri);
				sp_link *spLink = sp_link_create_from_string(uri);
				sp_track* track = sp_link_as_track(spLink);
				sp_session_player_prefetch(getSession(), track);
				sp_link_release(spLink);
			}
		}
	}
	return true;
}

void CDVDInputStreamSpotify::Close() {
	unloadPlayer();
	m_buffer.Destroy();
}

int CDVDInputStreamSpotify::Read(BYTE* pBuffer, int size) {
	if (!m_isPlayerLoaded)
		loadPlayer();

	// Wait at most 3 seconds to fill buffer
	int maxLoops = 60;
	while (!m_endOfTrack && m_buffer.getMaxReadSize() <= 0) {
		if (--maxLoops <= 0) {
			Logger::printOut("Empty buffer");
			return 0;
		}
		Sleep(50);
	}

	unsigned int want = (unsigned int)
			XMIN(m_buffer.getMaxReadSize(), (unsigned int)size);

	if (m_buffer.ReadData((char *) pBuffer, want))
		return want;

	return 0;
}

bool CDVDInputStreamSpotify::SeekTime(int iTimeInMsec) {
	Logger::printOut("trying to seek");
	//if (sp_session_player_seek(getSession(), iTimeInMsec) != SP_ERROR_OK)
	//	return false;
	sp_session_player_seek(getSession(), iTimeInMsec);
	return true;
}

int64_t CDVDInputStreamSpotify::Seek(int64_t offset, int whence) {

	if (whence == SEEK_POSSIBLE)
		return 0;
	return -1;
}

bool CDVDInputStreamSpotify::IsEOF() {
// TODO Semaphore
	return m_endOfTrack;
}

int64_t CDVDInputStreamSpotify::GetLength() {
	return m_totalTime;
}

bool CDVDInputStreamSpotify::Pause(double dTime) {
//CSingleLock lock(m_RTMPSection);
	return true;
}

bool CDVDInputStreamSpotify::loadPlayer() {
	Logger::printOut("load player");
	if (!m_isPlayerLoaded) {
		//do we have a track at all?
		if (m_currentTrack) {
			CStdString name;
			Logger::printOut("load player 2");
			if (sp_track_is_loaded(m_currentTrack)) {
				sp_error error = sp_session_player_load(getSession(),
						m_currentTrack);
				CStdString message;
				Logger::printOut("load player 3");
				message.Format("%s", sp_error_message(error));
				Logger::printOut(message);
				Logger::printOut("load player 4");
				if (SP_ERROR_OK == error) {
					PlayerHandler::getInstance()->attachPlayer(this);
					sp_session_player_play(getSession(), true);
					m_isPlayerLoaded = true;
					Logger::printOut("load player 5");
					return true;
				}
			}
		} else
			return false;
	}
	return true;
}

bool CDVDInputStreamSpotify::unloadPlayer() {
//make sure there is no music_delivery while we are removing the codec
	while (!Session::getInstance()->lock()) {
	}
	if (m_isPlayerLoaded) {
		sp_session_player_play(getSession(), false);
		sp_session_player_unload(getSession());
		if (m_currentTrack != NULL) {
			sp_track_release(m_currentTrack);
		}
		PlayerHandler::getInstance()->detachPlayer(this);
	}

	m_currentTrack = NULL;
	m_isPlayerLoaded = false;
	m_endOfTrack = true;
	Session::getInstance()->unlock();
	return true;
}

sp_session* CDVDInputStreamSpotify::getSession() {
	return Session::getInstance()->getSpSession();
}

int CDVDInputStreamSpotify::sp_musicDelivery(const sp_audioformat *format,
		const void *frames, int num_frames) {
	if (num_frames == 0) {
		Logger::printOut("musicDelivery Discontinuity");
		// A discontinuity has occurred, flush buffers and return
		m_buffer.Clear();
		return 0;
	}

	int consumedFrames = 0;
	unsigned int size = num_frames * (int) sizeof(int16_t) * format->channels;

	if (m_buffer.getMaxWriteSize() > size) {
		if (m_buffer.WriteData((char *) frames, size))
			consumedFrames = size / ((int) sizeof(int16_t) * format->channels);
	}
	return consumedFrames;
}

void CDVDInputStreamSpotify::sp_endOfTrack() {
	m_endOfTrack = true;
}
