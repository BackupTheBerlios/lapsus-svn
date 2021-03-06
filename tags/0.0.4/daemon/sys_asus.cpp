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

#include <qdir.h>
#include <qfile.h>
#include <qstringlist.h>

#include "lapsus.h"
#include "sys_asus.h"

#define qPrintable(str)			(str.ascii())

#define ASUS_DISPLAY_PATH		"/sys/devices/platform/asus-laptop/display"
#define ASUS_GET_BACKLIGHT_PATH		"/sys/class/backlight/asus-laptop/actual_brightness"
#define ASUS_SET_BACKLIGHT_PATH		"/sys/class/backlight/asus-laptop/brightness"
#define ASUS_MAX_BACKLIGHT_PATH		"/sys/class/backlight/asus-laptop/max_brightness"

SysAsus::SysAsus():
	_hasSwitches(false), _hasBacklight(false), _hasDisplay(false),
	_hasTouchpad(false), _maxBacklight(0),
#ifdef HAVE_ALSA
	_hasVolume(false), _mix(0),
#endif
	_synap(0), _notifyTouchpadChange(false)
{
	detect();
}

SysAsus::~SysAsus()
{
#ifdef HAVE_ALSA
	if (_mix) delete _mix;
#endif
	if (_synap) delete _synap;
}

void SysAsus::detect()
{
	QDir *dir;
	QString path;

	dir = new QDir("/sys/class/leds");

	if (dir->exists())
	{
		dir->setSorting(QDir::Name);
		dir->setFilter(QDir::Dirs);

		QStringList list = dir->entryList("asus:*");

		for (unsigned int i = 0; i < list.size(); ++i)
		{
			QString fName = list[i];

			if (fName.length() > 5)
			{
				path = QString("/sys/class/leds/%1/brightness").arg(fName);

				if (QFile::exists(path) && fName.startsWith("asus:") && testR(path))
				{
#if 0
					// Known LED names - only for translation purposes.
					I18N_NOOP("Mail LED");
					I18N_NOOP("Touchpad LED");
					I18N_NOOP("Recording LED");
					I18N_NOOP("Phone LED");
					I18N_NOOP("Gaming LED");
#endif

					QString id = QString(LAPSUS_FEAT_LED_ID_PREFIX "%1").
							arg(fName.mid(5).lower());
					QString name = QString("%1 LED").arg(fName.mid(5));

					name[0] = name[0].upper();

					setFeature(id, path, name);

					_hasSwitches = true;
				}
			}
		}
	}

	delete dir;

	path = "/sys/devices/platform/asus-laptop/bluetooth";

	if (QFile::exists(path) && testR(path))
	{
		setFeature(LAPSUS_FEAT_BLUETOOTH_ID, path);

		_hasSwitches = true;
	}

	path = "/sys/devices/platform/asus-laptop/wlan";

	if (QFile::exists(path) && testR(path))
	{
		setFeature(LAPSUS_FEAT_WIRELESS_ID, path);

		_hasSwitches = true;
	}

	path = ASUS_DISPLAY_PATH;

	if (QFile::exists(path) && testR(path))
	{
		_hasDisplay = true;
	}

	path = ASUS_GET_BACKLIGHT_PATH;

	if (QFile::exists(path) && testR(path))
	{
		path = ASUS_MAX_BACKLIGHT_PATH;

		if (QFile::exists(path) && testR(path))
		{
			_maxBacklight = readPathUInt(path);

			path = ASUS_SET_BACKLIGHT_PATH;

			if (_maxBacklight > 0 && QFile::exists(path))
			{
				_hasBacklight = true;
			}
			else
			{
				_maxBacklight = 0;
			}
		}
	}

	// If we still don't have any hardware detected,
	// don't try to open AlsaMixer. ALSA is available
	// on most systems, so all of them would be detected as
	// Asus laptops only because they have ALSA installed.
	// That is wrong. We want to try to run AlsaMixer only
	// if we alreayd know that we are running on Asus Laptop.

	// We also don't want to detect Synaptics client, as it is
	// probably present on most laptops.

	if (!hardwareDetected()) return;

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
void SysAsus::volumeChanged(int val)
{
	if (_dbus)
		_dbus->signalFeatureChanged(LAPSUS_FEAT_VOLUME_ID, QString::number(val));
}

void SysAsus::muteChanged(bool muted)
{
	if (_dbus)
		_dbus->signalFeatureChanged(LAPSUS_FEAT_VOLUME_ID,
				muted?LAPSUS_FEAT_MUTE:LAPSUS_FEAT_UNMUTE);
}
#endif

void SysAsus::touchpadChanged(bool nState)
{
	if (_dbus)
	{
		_dbus->signalFeatureChanged(LAPSUS_FEAT_TOUCHPAD_ID,
			nState?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);

		if (_notifyTouchpadChange)
		{
			_dbus->signalFeatureNotif(LAPSUS_FEAT_TOUCHPAD_ID,
				nState?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);
		}
	}

	_notifyTouchpadChange = false;
}

bool SysAsus::hardwareDetected()
{
	return _hasSwitches
		|| _hasBacklight
		|| _hasDisplay;
}

QStringList SysAsus::featureList()
{
	QStringList ret;

	if (_hasSwitches)
	{
		ret += getFeatures();
	}

	if (_hasBacklight)
	{
		ret.append(LAPSUS_FEAT_BACKLIGHT_ID);
	}

	if (_hasDisplay)
	{
		ret.append(LAPSUS_FEAT_DISPLAY_ID_PREFIX LAPSUS_FEAT_DISPLAY_LCD);
		ret.append(LAPSUS_FEAT_DISPLAY_ID_PREFIX LAPSUS_FEAT_DISPLAY_CRT);
		ret.append(LAPSUS_FEAT_DISPLAY_ID_PREFIX LAPSUS_FEAT_DISPLAY_TV);
		ret.append(LAPSUS_FEAT_DISPLAY_ID_PREFIX LAPSUS_FEAT_DISPLAY_DVI);
	}

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

QStringList SysAsus::featureArgs(const QString &id)
{
	QStringList ret;

	if (id == LAPSUS_FEAT_BACKLIGHT_ID)
	{
		if (_maxBacklight > 0)
		{
			ret.append(QString("0:%1").arg(QString::number(_maxBacklight)));
		}
	}
#ifdef HAVE_ALSA
	else if (id == LAPSUS_FEAT_VOLUME_ID)
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
	else if (hasFeature(id) || isDisplayFeature(id))
	{
		ret.append(LAPSUS_FEAT_ON);
		ret.append(LAPSUS_FEAT_OFF);
	}

	return ret;
}

QStringList SysAsus::featureParams(const QString &id)
{
	QStringList ret;

	if (id == LAPSUS_FEAT_BACKLIGHT_ID
		|| id == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		ret.append(LAPSUS_FEAT_PARAM_NOTIF);
	}
#ifdef HAVE_ALSA
	else if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		if (_mix)
		{
			if (_mix->isValid())
			{
				ret.append(LAPSUS_FEAT_PARAM_NOTIF);
			}
		}
	}
#endif

	return ret;
}

void SysAsus::acpiEvent(const QString &group, const QString &action,
	const QString &device, uint id, uint value)
{
	if (group != "hotkey" || action != "hotkey" || device != "ATKD")
	{
		if (_dbus)
			_dbus->sendACPIEvent(group, action, device, id, value);
		return;
	}

#ifdef HAVE_ALSA

	if (_hasVolume && _mix)
	{
		if (id == 0x32)
		{
			bool muted = _mix->isMuted();

			_mix->toggleMute();

			if (muted != _mix->isMuted())
			{
				// Reveresed, muted is old value
				if (muted)
				{
					_dbus->signalFeatureNotif(LAPSUS_FEAT_VOLUME_ID,
								LAPSUS_FEAT_UNMUTE);
				}
				else
				{
					_dbus->signalFeatureNotif(LAPSUS_FEAT_VOLUME_ID,
								LAPSUS_FEAT_MUTE);
				}
			}
		}
		else if (id == 0x31 || id == 0x30)
		{
			int val = _mix->getVolume();
			int mVal = _mix->getMaxVolume();

			if (id == 0x30) val += 5;
			else val -= 5;

			if (val < 0) val = 0;
			if (val > mVal) val = mVal;

			if (_mix->setVolume(val) && _dbus)
			{
				_dbus->signalFeatureNotif(LAPSUS_FEAT_VOLUME_ID,
						QString::number(val));
			}

			return;
		}
	}
#endif

	// I'm not sure what should be the ID range if max_backlight != 15...
	if (_hasBacklight && id >= 0x20 && id <= 0x2e)
	{
		int oVal = readPathString(ASUS_GET_BACKLIGHT_PATH).toInt();
		int nVal = id - 0x20;

		if (oVal == 0 && nVal == 0)
			nVal = _maxBacklight;

		if (setBacklight(nVal) && _dbus)
		{
			_dbus->signalFeatureNotif(
				LAPSUS_FEAT_BACKLIGHT_ID,
				QString::number(nVal));
		}

		return;
	}

	if (_hasTouchpad && id == 0x6b && _synap)
	{
		_notifyTouchpadChange = true;

		_synap->toggleState();

		return;
	}

	// Unknown hotkey. Just send is as a normal ACPI event.

	if (_dbus)
		_dbus->sendACPIEvent(group, action, device, id, value);
}

QString SysAsus::featureRead(const QString &id)
{
	if (id == LAPSUS_FEAT_BACKLIGHT_ID)
	{
		return readPathString(ASUS_GET_BACKLIGHT_PATH);
	}
#ifdef HAVE_ALSA
	else if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		if (!_mix) return "";

		return QString::number(_mix->getVolume());
	}
#endif
	else if (id == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		if (!_synap) return "";

		if (_synap->getState()) return LAPSUS_FEAT_ON;

		return LAPSUS_FEAT_OFF;
	}
	else if (hasFeature(id))
	{
		uint val = readIdUInt(id);

		if (val)
			return LAPSUS_FEAT_ON;

		return LAPSUS_FEAT_OFF;
	}

	QString disp;

	if (isDisplayFeature(id, disp))
	{
		uint val = readPathUInt(ASUS_DISPLAY_PATH);
		uint offs = 0;

		if (disp == LAPSUS_FEAT_DISPLAY_LCD) offs = 0;
		else if (disp == LAPSUS_FEAT_DISPLAY_CRT) offs = 1;
		else if (disp == LAPSUS_FEAT_DISPLAY_TV) offs = 2;
		else if (disp == LAPSUS_FEAT_DISPLAY_DVI) offs = 3;
		else return "";

		if (val & (1<<offs) )
		{
			return LAPSUS_FEAT_ON;
		}

		return LAPSUS_FEAT_OFF;
	}

	return "";
}

bool SysAsus::setBacklight(uint uVal)
{
	if (uVal > _maxBacklight)
		uVal = _maxBacklight;

	uint oVal = readPathUInt(ASUS_GET_BACKLIGHT_PATH);

	if (oVal == uVal) return false;

	bool res = writePathUInt(ASUS_SET_BACKLIGHT_PATH, uVal);

	if (res && _dbus)
	{
		_dbus->signalFeatureChanged(LAPSUS_FEAT_BACKLIGHT_ID, QString::number(uVal));
	}

	return res;
}

bool SysAsus::featureWrite(const QString &id, const QString &nVal)
{
	if (id == LAPSUS_FEAT_BACKLIGHT_ID)
	{
		bool res = false;

		uint uVal = nVal.toUInt(&res);

		if (!res) return false;

		return setBacklight(uVal);
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
	else if (id == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		if (!_synap) return false;

		_notifyTouchpadChange = false;

		if (nVal == LAPSUS_FEAT_ON)
			return _synap->setState(true);

		return _synap->setState(false);
	}
	else if (hasFeature(id))
	{
		uint uVal = 0;

		if (nVal == LAPSUS_FEAT_ON) uVal = 1;
		else if (nVal == LAPSUS_FEAT_OFF) uVal = 0;
		else return false;

		uint oVal = readIdUInt(id);

		if (oVal == uVal) return false;

		bool res = writeIdUInt(id, uVal);

		if (res && _dbus)
		{
			_dbus->signalFeatureChanged(id, nVal);
		}

		return res;
	}

	QString disp;

	if (isDisplayFeature(id, disp))
	{
		uint uVal;
		uint offs = 0;

		if (nVal == LAPSUS_FEAT_ON) uVal = 1;
		else if (nVal == LAPSUS_FEAT_ON) uVal = 0;
		else return false;

		if (disp == LAPSUS_FEAT_DISPLAY_LCD) offs = 0;
		else if (disp == LAPSUS_FEAT_DISPLAY_CRT) offs = 1;
		else if (disp == LAPSUS_FEAT_DISPLAY_TV) offs = 2;
		else if (disp == LAPSUS_FEAT_DISPLAY_DVI) offs = 3;
		else return false;

		uint sVal, oVal = readPathUInt(ASUS_DISPLAY_PATH);

		// ON - set the bit
		if (uVal)
		{
			sVal = oVal | (1 << offs);
		}
		// OFF - clear the bit
		else
		{
			sVal = oVal & ~(1 << offs);
		}

		if (sVal == oVal) return false;

		bool res = writePathUInt(ASUS_DISPLAY_PATH, sVal);

		if (res && _dbus)
		{
			_dbus->signalFeatureChanged(id, nVal);
		}

		return res;
	}

	return false;
}
