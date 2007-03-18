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

SysGeneric::SysGeneric()
#ifdef HAVE_ALSA
	: _hasVolume(false), _mix(0)
#endif
{
	detect();
}

SysGeneric::~SysGeneric()
{
#ifdef HAVE_ALSA
	if (_mix) delete _mix;
#endif
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

bool SysGeneric::hardwareDetected()
{
#ifdef HAVE_ALSA
	return _hasVolume;
#else
	return false;
#endif
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

	return ret;
}

void SysGeneric::acpiEvent(const QString &group, const QString &action,
	const QString &device, uint id, uint value)
{
	if (_dbus)
		_dbus->sendACPIEvent(group, action, device, id, value);
}

QString SysGeneric::featureRead(const QString &id)
{
#ifdef HAVE_ALSA
	if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		if (!_mix) return "";

		return QString::number(_mix->getVolume());
	}
#endif

	return "";
}

bool SysGeneric::featureWrite(const QString &id, const QString &nVal)
{
#ifdef HAVE_ALSA
	if (id == LAPSUS_FEAT_VOLUME_ID)
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
