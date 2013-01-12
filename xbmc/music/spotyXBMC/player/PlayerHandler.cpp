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

#include "PlayerHandler.h"
#include "ISpotifyPlayer.h"
#include "../session/Session.h"

namespace addon_music_spotify {

PlayerHandler* PlayerHandler::m_instance = 0;
ISpotifyPlayer* PlayerHandler::m_player = 0;
PlayerHandler *PlayerHandler::getInstance() {
	return m_instance ? m_instance : (m_instance = new PlayerHandler());
}
ISpotifyPlayer* PlayerHandler::getPlayer() {
	return m_player;
}

PlayerHandler::PlayerHandler() {
	m_instance = 0;
}

PlayerHandler::~PlayerHandler() {
	if (!m_instance)
		return;
	m_instance->detachPlayer(m_player);
}

void PlayerHandler::deInit() {
	if (!m_instance)
		return;
	m_instance->detachPlayer(m_player);
}

int PlayerHandler::cb_musicDelivery(sp_session *session,
		const sp_audioformat *format, const void *frames, int num_frames) {
	if (m_instance) {
		ISpotifyPlayer *player = m_instance->getPlayer();
		if (player)
			return player->sp_musicDelivery(format, frames, num_frames);
	}
	return 0;
}

void PlayerHandler::cb_endOfTrack(sp_session *session) {
	if (m_instance) {
		ISpotifyPlayer *player = m_instance->getPlayer();
		if (player)
			player->sp_endOfTrack();
	}
}

void PlayerHandler::attachPlayer(ISpotifyPlayer *player) {
	m_player = player;
}
void PlayerHandler::detachPlayer(ISpotifyPlayer *player) {
	m_player = 0;
}
}/* namespace addon_music_spotify */
