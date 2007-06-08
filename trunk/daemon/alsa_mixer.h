/***************************************************************************
 *   Copyright (C) 2007 by Jakub Schmidtke                                 *
 *   sjakub@users.berlios.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#ifndef LAPSUS_ALSA_MIXER_H
#define LAPSUS_ALSA_MIXER_H

#include <qobject.h>
#include <qsocketnotifier.h>

#include <alsa/asoundlib.h>
#include "sys_backend.h"
#include "lapsus_mixer.h"

class SIDInfo
{
	public:
		long min;
		long max;
		long globalMax;
		bool hasMute;
		bool hasVolume;

		SIDInfo(snd_mixer_t* handle, snd_mixer_selem_id_t* sid = 0);
		~SIDInfo();

		void setGlobalMax(long gm);
		long getVolume();
		bool setVolume(long val);
		bool getMute();
		bool setMute(bool mute);
		bool setEmulMute(bool mute);

	private:
		bool _isEmulMuted;
		long _lastVol;
		snd_mixer_t* _handle;
		snd_mixer_selem_id_t* _sid;

		bool setRealVolume(long val);
		long getRealVolume();
};

/**
 * Simple Alsa mixer. It is possible to use ALSA to control sound volume instead
 * of using laptop's hardware features. For example IBM thinkpads have hardware
 * volume control so they don't need to use this. But Asus laptops don't,
 * so backend will try to use this.
 * It will try to detect available channels and will pick the best one -
 * master or front channel, depending which one exists and provides more
 * functionality - volume control and mute switch. It will also use
 * headphones channel if found.
 */
class LapsusAlsaMixer : public LapsusMixer
{
	Q_OBJECT

	public:
		LapsusAlsaMixer();
		~LapsusAlsaMixer();

		bool hardwareDetected();
		bool mixerIsMuted();
		
	protected:
		bool toggleMuted();
		bool setMuted(bool mState);
		
		int getVolume();
		bool setVolume(int vol);
		int getMaxVolume();
		int getMinVolume();
		
		bool hasVolume();
		bool hasMute();
		
	protected slots:
		void alsaEvent();

	private:
		enum sidType {ID_M=0, ID_F, ID_HP, ID_LAST};
		SIDInfo* sids[ID_LAST];
		snd_mixer_t *_handle;
		bool _isValid;
		int _count;
		struct pollfd *_fds;
		QSocketNotifier **_sns;
		long _curVolume;
		bool _curMute;
		long _globalMax;

		bool init();
};

#endif
