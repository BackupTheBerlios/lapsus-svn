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

#include "lapsus.h"
#include "sys_generic.h"

SysGeneric::SysGeneric():
	_hasTouchpad(false),
#ifdef HAVE_ALSA
	_hasVolume(false), _mix(0),
#endif
	_synap(0)
{
	detect();
}

SysGeneric::~SysGeneric()
{
#ifdef HAVE_ALSA
	if (_mix) delete _mix;
#endif
	if (_synap) delete _synap;
}

void SysGeneric::detect()
{
#ifdef HAVE_ALSA
	_mix = new LapsusAlsaMixer();

	if (_mix->isValid())
	{
		_hasVolume = true;

		connect(_mix, SIGNAL(volumeChanged(int)),
			this, SLOT(volumeChanged(int)));

		connect(_mix, SIGNAL(muteChanged(bool)),
			this, SLOT(muteChanged(bool)));
	}
	else
	{
		delete _mix;
		_mix = 0;
	}
#endif
	_synap = new LapsusSynaptics();

	if (_synap->isValid())
	{
		_hasTouchpad = true;

		connect(_synap, SIGNAL(stateChanged(bool)),
			this, SLOT(touchpadChanged(bool)));
	}
	else
	{
		delete _synap;
		_synap = 0;
	}
}

#ifdef HAVE_ALSA
void SysGeneric::volumeChanged(int val)
{
	if (_dbus)
		_dbus->signalFeatureChanged(LAPSUS_FEAT_VOLUME_ID, QString::number(val));
}

void SysGeneric::muteChanged(bool muted)
{
	if (_dbus)
		_dbus->signalFeatureChanged(LAPSUS_FEAT_VOLUME_ID,
				muted?LAPSUS_FEAT_MUTE:LAPSUS_FEAT_UNMUTE);
}
#endif

void SysGeneric::touchpadChanged(bool nState)
{
	if (_dbus)
	{
		_dbus->signalFeatureChanged(LAPSUS_FEAT_TOUCHPAD_ID,
			nState?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);
	}
}

bool SysGeneric::hardwareDetected()
{
	return _hasTouchpad
#ifdef HAVE_ALSA
		|| _hasVolume
#endif
	;
}

QStringList SysGeneric::featureList()
{
	QStringList ret;

#ifdef HAVE_ALSA
	if (_hasVolume)
	{
		ret.append(LAPSUS_FEAT_VOLUME_ID);
	}
#endif
	if (_hasTouchpad)
	{
		ret.append(LAPSUS_FEAT_TOUCHPAD_ID);
	}

	return ret;
}

QStringList SysGeneric::featureArgs(const QString &id)
{
	QStringList ret;

#ifdef HAVE_ALSA
	if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		if (_mix)
		{
			if (_mix->isValid())
			{
				ret.append(QString("0:%1").arg(QString::number(_mix->getMaxVolume())));
				ret.append(LAPSUS_FEAT_MUTE);
				ret.append(LAPSUS_FEAT_UNMUTE);
			}
		}
	}
#endif
	else if (id == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		ret.append(LAPSUS_FEAT_ON);
		ret.append(LAPSUS_FEAT_OFF);
	}

	return ret;
}

QString SysGeneric::featureRead(const QString &id)
{
	if (id == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		if (!_synap) return "";

		if (_synap->getState()) return LAPSUS_FEAT_ON;

		return LAPSUS_FEAT_OFF;
	}
#ifdef HAVE_ALSA
	else if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		if (!_mix) return "";

		return QString::number(_mix->getVolume());
	}
#endif

	return "";
}

bool SysGeneric::featureWrite(const QString &id, const QString &nVal)
{
	if (id == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		if (!_synap) return false;

		if (nVal == LAPSUS_FEAT_ON)
			return _synap->setState(true);

		return _synap->setState(false);
	}
#ifdef HAVE_ALSA
	else if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		if (!_mix) return false;

		if (nVal == LAPSUS_FEAT_MUTE) return _mix->setMuted(true);
		if (nVal == LAPSUS_FEAT_UNMUTE) return _mix->setMuted(false);

		bool res = false;

		int val = nVal.toInt(&res);

		if (!res) return false;

		return _mix->setVolume(val);
	}
#endif

	return false;
}
