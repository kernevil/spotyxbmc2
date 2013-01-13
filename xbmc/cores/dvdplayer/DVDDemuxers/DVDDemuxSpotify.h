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

#include "DVDDemux.h"

class CDVDDemuxSpotify: public CDVDDemux {
public:
	CDVDDemuxSpotify();
	virtual ~CDVDDemuxSpotify();

	bool Open(CDVDInputStream* pInput);
	void Dispose();
	void Reset();
	void Abort();
	void Flush();
	DemuxPacket* Read();

	bool SeekTime(int time, bool backwords = false, double* startpts = NULL);
	void SetSpeed(int iSpeed);
	int GetStreamLength();
	CDemuxStream* GetStream(int iStreamId);
	int GetNrOfStreams();
	std::string GetFileName();
	virtual void GetStreamCodecName(int iStreamId, CStdString &strName);
private:
	friend class CDemuxStreamAudioSpotify;
	CDVDInputStream *m_pInput;
	CDemuxStreamAudio *m_stream;
	double m_pts;
};

