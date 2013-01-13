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

#include "DVDDemuxSpotify.h"
#include "DVDInputStreams/DVDInputStream.h"
#include "DVDDemuxUtils.h"
#include "music/spotyXBMC/SxSettings.h"
#include "utils/log.h"
#include "../DVDClock.h"

using namespace std;

class CDemuxStreamAudioSpotify: public CDemuxStreamAudio {
	CDVDDemuxSpotify *m_parent;
	string m_codec;
public:
	CDemuxStreamAudioSpotify(CDVDDemuxSpotify *parent, const string& codec) :
			m_parent(parent), m_codec(codec)

	{
	}
	void GetStreamInfo(string& strInfo) {
		CStdString info;
		info.Format("%s", m_codec.c_str());
		strInfo = info;
	}
};

CDVDDemuxSpotify::CDVDDemuxSpotify() :
		CDVDDemux() {
	m_pInput = NULL;
	m_stream = NULL;
	m_pts = 0;
}

CDVDDemuxSpotify::~CDVDDemuxSpotify() {
	Dispose();
}

bool CDVDDemuxSpotify::Open(CDVDInputStream* pInput) {
	if (!pInput || !pInput->IsStreamType(DVDSTREAM_TYPE_SPOTIFY))
		return false;

	m_pInput = pInput;
	m_stream = new CDemuxStreamAudioSpotify(this, "Spotify");

	m_stream->iSampleRate = 44100;
	m_stream->iBitsPerSample = 16;
	m_stream->iBitRate =
			addon_music_spotify::Settings::getInstance()->useHighBitrate() ?
					320000 : 160000;
	m_stream->iChannels = 2;
	m_stream->type = STREAM_AUDIO;
	m_stream->codec = CODEC_ID_PCM_S16LE;

	return true;
}

void CDVDDemuxSpotify::Dispose() {
	delete m_stream;
	m_stream = NULL;

	m_pInput = NULL;
	m_pts = 0;
}

void CDVDDemuxSpotify::Reset() {
	CDVDInputStream* pInputStream = m_pInput;
	Dispose();
	Open(pInputStream);
}

void CDVDDemuxSpotify::Abort() {
	if (m_pInput)
		return m_pInput->Abort();
}

#define SPOTIFY_READ_SIZE (2048 * sizeof(int16_t))
DemuxPacket* CDVDDemuxSpotify::Read() {
	if (!m_pInput)
		return NULL;

	DemuxPacket* pPacket = CDVDDemuxUtils::AllocateDemuxPacket(
			SPOTIFY_READ_SIZE);

	if (!pPacket) {
		if (m_pInput)
			m_pInput->Close();
		return NULL;
	}

	pPacket->iStreamId = 0;
	pPacket->iSize = m_pInput->Read(pPacket->pData, SPOTIFY_READ_SIZE);
	if (pPacket->iSize < 1) {
		delete pPacket;
		pPacket = NULL;
	} else {
		int n = (m_stream->iChannels * m_stream->iBitsPerSample
				* m_stream->iSampleRate) >> 3;
		if (n > 0) {
			m_pts += ((double) pPacket->iSize * DVD_TIME_BASE) / n;
			pPacket->dts = m_pts;
			pPacket->pts = m_pts;
		} else {
			pPacket->dts = DVD_NOPTS_VALUE;
			pPacket->pts = DVD_NOPTS_VALUE;
		}
	}

	return pPacket;
}

void CDVDDemuxSpotify::Flush() {

}

bool CDVDDemuxSpotify::SeekTime(int time, bool backwords, double *startpts) {
	if (time < 0)
		time = 0;

	CDVDInputStream::ISeekTime* ist =
			dynamic_cast<CDVDInputStream::ISeekTime*>(m_pInput);
	if (ist) {
		if (!ist->SeekTime(time))
			return false;
		m_pts = (int64_t) time * (AV_TIME_BASE / 1000);
		if (startpts)
			*startpts = DVD_NOPTS_VALUE;
		return true;
	}
	return false;
}

void CDVDDemuxSpotify::SetSpeed(int iSpeed) {

}

CDemuxStream* CDVDDemuxSpotify::GetStream(int iStreamId) {
	if (iStreamId != 0)
		return NULL;

	return m_stream;
}

int CDVDDemuxSpotify::GetStreamLength() {
	if (m_pInput)
		return m_pInput->GetLength();
	return 0;
}

int CDVDDemuxSpotify::GetNrOfStreams() {
	return (m_stream == NULL ? 0 : 1);
}

std::string CDVDDemuxSpotify::GetFileName() {
	if (m_pInput)
		return m_pInput->GetFileName();
	else
		return "";
}

void CDVDDemuxSpotify::GetStreamCodecName(int iStreamId, CStdString &strName) {
	if (m_stream && iStreamId == 0)
		strName = "BXA";
}
