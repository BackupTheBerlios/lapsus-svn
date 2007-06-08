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

#ifndef LAPSUS_MIXER_H
#define LAPSUS_MIXER_H

#include "lapsus_module.h"

#include "../config.h"

class LapsusMixer : public LapsusModule
{
	Q_OBJECT

	public:
		LapsusMixer(const char *prefix);
		virtual ~LapsusMixer();

		virtual uint mixerGetNormVolume();
		virtual bool mixerSetNormVolume(uint val, bool hardwareTrig = false);
		
		virtual bool mixerIsMuted() = 0;
		virtual bool mixerSetMuted(bool mState, bool hardwareTrig = false);
		virtual bool mixerToggleMuted(bool hardwareTrig = false);

		virtual bool mixerVolumeUp(bool hardwareTrig = false);
		virtual bool mixerVolumeDown(bool hardwareTrig = false);
		
		virtual QStringList featureList();
		virtual QStringList featureArgs(const QString &id);
		
		virtual QString featureRead(const QString &id);
		virtual bool featureWrite(const QString &id, const QString &nVal, bool testWrite = false);
	
	protected:
		virtual bool toggleMuted() = 0;
		virtual bool setMuted(bool mState) = 0;
		
		virtual int getVolume() = 0;
		virtual bool setVolume(int vol) = 0;
		virtual int getMaxVolume() = 0;
		virtual int getMinVolume() = 0;
		
		virtual void volumeChanged(int val);
		virtual void muteChanged(bool muted);
	
	private:
		int _minVol;
		int _maxVol;
		bool _doSetup;
		
		void setupLimits();
		uint toNorm(int vol);
		int fromNorm(uint vol);
};

#ifdef HAVE_ALSA
#include "alsa_mixer.h"
#endif

#endif
