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

#include "lapsus_mixer.h"
#include "lapsus.h"

LapsusMixer::LapsusMixer(const char *prefix):
	LapsusModule(prefix)
{
}

LapsusMixer::~LapsusMixer()
{
}

uint LapsusMixer::mixerGetVolume()
{
	return getVolume();
}

bool LapsusMixer::mixerSetVolume(uint val, bool hardwareTrig)
{
	bool ret = setVolume(val);

	if (hardwareTrig && ret && _dbus)
	{
		QStringList args;
		
		args.append(QString::number(val));
		
		if (mixerIsMuted())
		{
			args.append(LAPSUS_FEAT_MUTE);
		}
		
		dbusSignalFeatureNotif(LAPSUS_FEAT_VOLUME_ID, args);
	}
	
	return ret;
}

bool LapsusMixer::mixerSetMuted(bool mState, bool hardwareTrig)
{
	bool muted = mixerIsMuted();
	bool ret = setMuted(mState);

	if (hardwareTrig && muted != mixerIsMuted())
	{
		// Reveresed, muted is old value
		if (muted)
		{
			dbusSignalFeatureNotif(LAPSUS_FEAT_VOLUME_ID,
				LAPSUS_FEAT_UNMUTE);
		}
		else
		{
			dbusSignalFeatureNotif(LAPSUS_FEAT_VOLUME_ID,
				LAPSUS_FEAT_MUTE);
		}
	}
	
	return ret;
}

bool LapsusMixer::mixerToggleMuted(bool hardwareTrig)
{
	bool muted = mixerIsMuted();
	bool ret = toggleMuted();

	if (hardwareTrig && muted != mixerIsMuted())
	{
		// Reveresed, muted is old value
		if (muted)
		{
			dbusSignalFeatureNotif(LAPSUS_FEAT_VOLUME_ID,
				LAPSUS_FEAT_UNMUTE);
		}
		else
		{
			dbusSignalFeatureNotif(LAPSUS_FEAT_VOLUME_ID,
				LAPSUS_FEAT_MUTE);
		}
	}
	
	return ret;
}

bool LapsusMixer::mixerVolumeUp(bool hardwareTrig)
{
	uint vol = mixerGetVolume() + 10;

	if (vol > 100) vol = 100;

	if (mixerIsMuted()) mixerToggleMuted();

	bool ret = mixerSetVolume(vol);

	if (hardwareTrig && _dbus)
	{
		QStringList args;
		
		args.append(QString::number(vol));
		
		dbusSignalFeatureNotif(LAPSUS_FEAT_VOLUME_ID, args);
	}

	return ret;
}

bool LapsusMixer::mixerVolumeDown(bool hardwareTrig)
{
	int vol = mixerGetVolume() - 10;

	if (vol < 0) vol = 0;

	bool ret = mixerSetVolume(vol);

	if (hardwareTrig && _dbus)
	{
		QStringList args;
		
		args.append(QString::number(vol));
		
		if (mixerIsMuted())
			args.append(LAPSUS_FEAT_MUTE);

		dbusSignalFeatureNotif(LAPSUS_FEAT_VOLUME_ID, args);
	}

	return ret;
}

void LapsusMixer::volumeChanged(int val)
{
	if (_dbus)
	{
		QStringList args;
		
		args.append(QString::number(val));
		
		if (mixerIsMuted())
			args.append(LAPSUS_FEAT_MUTE);
		
		dbusSignalFeatureUpdate(LAPSUS_FEAT_VOLUME_ID, args);
	}
}

void LapsusMixer::muteChanged(bool muted)
{
	dbusSignalFeatureUpdate(LAPSUS_FEAT_VOLUME_ID,
			muted?LAPSUS_FEAT_MUTE:LAPSUS_FEAT_UNMUTE);
}

QString LapsusMixer::featureRead(const QString &id)
{
	if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		if (mixerIsMuted())
		{
			return QString("%1,%2")
				.arg(QString::number(mixerGetVolume()))
				.arg(LAPSUS_FEAT_MUTE);
		}
		
		return QString::number(mixerGetVolume());
	}
	
	return "";
}

bool LapsusMixer::featureWrite(const QString &id, const QString &nVal)
{
	bool ret = false;

	if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		QStringList list = QStringList::split(",", nVal);

		for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
		{
			QString val = *it;

			if (val == LAPSUS_FEAT_MUTE)
			{
				ret |= mixerSetMuted(true);
			}
			else if (val == LAPSUS_FEAT_UNMUTE)
			{
				ret |= mixerSetMuted(false);
			}
			else
			{
				bool res = false;
		
				uint iVal = val.toUInt(&res);
		
				if (res) ret |= mixerSetVolume(iVal);
			}
		}
	}

	return ret;
}

QStringList LapsusMixer::featureArgs(const QString &id)
{
	QStringList ret;

	if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		ret.append(QString("0:100"));
		ret.append(LAPSUS_FEAT_MUTE);
		ret.append(LAPSUS_FEAT_UNMUTE);
	}

	return ret;
}

QStringList LapsusMixer::featureList()
{
	QStringList ret;

	ret.append(LAPSUS_FEAT_VOLUME_ID);

	return ret;
}

#ifdef HAVE_ALSA
#include "alsa_mixer.cpp"
#endif
