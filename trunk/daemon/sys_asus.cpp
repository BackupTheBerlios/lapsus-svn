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

// We don't need anything else from klocale.h
#define I18N_NOOP(x)			x

#include "sys_asus.h"

#define qPrintable(str)			(str.ascii())

#define ASUS_DISPLAY_PATH		"/sys/devices/platform/asus-laptop/display"
#define ASUS_GET_BACKLIGHT_PATH		"/sys/class/backlight/asus-laptop/actual_brightness"
#define ASUS_SET_BACKLIGHT_PATH		"/sys/class/backlight/asus-laptop/brightness"
#define ASUS_MAX_BACKLIGHT_PATH		"/sys/class/backlight/asus-laptop/max_brightness"

// Following values have to be lower-case!
#define ASUS_PREFIX			"asus_"
#define ASUS_DISPLAY_ID			"asus_display_"
#define ASUS_LED_ID			"asus_led_"
#define ASUS_BACKLIGHT_ID		(ASUS_PREFIX "backlight")
#define ASUS_BLUETOOTH_ID		(ASUS_PREFIX "bluetooth")
#define ASUS_WIRELESS_ID		(ASUS_PREFIX "wireless")

// Following values have to be lower-case!
#define ASUS_ON				"on"
#define ASUS_OFF			"off"

// Following values have to be lower-case!
#define ASUS_LCD			"lcd"
#define ASUS_CRT			"crt"
#define ASUS_TV				"tv"
#define ASUS_DVI			"dvi"

SysAsus::SysAsus():
	_hasSwitches(false), _hasBacklight(false), _hasDisplay(false),
	maxBacklight(0)
{
	detect();
}

SysAsus::~SysAsus()
{
}

QString SysAsus::featurePrefix()
{
	return ASUS_PREFIX;
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
					QString id = QString(ASUS_LED_ID "%1").arg(fName.mid(5).lower());
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
		setFeature(ASUS_BLUETOOTH_ID, path, I18N_NOOP("Bluetooth adapter"));

		_hasSwitches = true;
	}

	path = "/sys/devices/platform/asus-laptop/wlan";

	if (QFile::exists(path) && testR(path))
	{
		setFeature(ASUS_WIRELESS_ID, path, I18N_NOOP("Wireless radio"));

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
			maxBacklight = readPathUInt(path);

			path = ASUS_SET_BACKLIGHT_PATH;

			if (maxBacklight > 0 && QFile::exists(path))
			{
				_hasBacklight = true;
			}
			else
			{
				maxBacklight = 0;
			}
		}
	}
}

bool SysAsus::hardwareDetected()
{
	return _hasSwitches || _hasBacklight || _hasDisplay;
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
		ret.append(ASUS_BACKLIGHT_ID);
	}

	if (_hasDisplay)
	{
		ret.append(ASUS_DISPLAY_ID ASUS_LCD);
		ret.append(ASUS_DISPLAY_ID ASUS_CRT);
		ret.append(ASUS_DISPLAY_ID ASUS_TV);
		ret.append(ASUS_DISPLAY_ID ASUS_DVI);
	}

	return ret;
}

bool SysAsus::displayFeature(const QString &id)
{
	if (id.length() > strlen(ASUS_DISPLAY_ID)
		&& id.startsWith(ASUS_DISPLAY_ID))
	{
		return true;
	}

	return false;
}

bool SysAsus::displayFeature(const QString &id, QString &disp)
{
	if (!displayFeature(id))
		return false;

	disp = id.mid(strlen(ASUS_DISPLAY_ID));

	return true;
}

QString SysAsus::featureName(const QString &id)
{

#if 0
	// Known feature names - only for translation purposes.
	I18N_NOOP("Mail LED");
	I18N_NOOP("Touchpad LED");
	I18N_NOOP("Recording LED");
	I18N_NOOP("Phone LED");
	I18N_NOOP("Gaming LED");
	I18N_NOOP("LCD Display");
	I18N_NOOP("CRT Display");
	I18N_NOOP("TV Display");
	I18N_NOOP("DVI Display");
#endif

	if (id == ASUS_BACKLIGHT_ID)
	{
		return I18N_NOOP("LCD Backlight");
	}
	else if (hasFeature(id))
	{
		return getFeatureName(id);
	}

	QString disp;

	if (displayFeature(id, disp))
	{
		return QString("%1 Display").arg(disp.upper());
	}

	return "";
}

QStringList SysAsus::featureArgs(const QString &id)
{
	QStringList ret;

	if (id == ASUS_BACKLIGHT_ID)
	{
		if (maxBacklight > 0)
		{
			ret.append(QString("0:%1").arg(QString::number(maxBacklight)));
		}
	}
	else if (hasFeature(id) || displayFeature(id))
	{
		ret.append(ASUS_ON);
		ret.append(ASUS_OFF);
	}

	return ret;
}

QString SysAsus::featureRead(const QString &id)
{
	if (id == ASUS_BACKLIGHT_ID)
	{
		return readPathString(ASUS_GET_BACKLIGHT_PATH);
	}
	else if (hasFeature(id))
	{
		uint val = readIdUInt(id);

		if (val)
			return ASUS_ON;

		return ASUS_OFF;
	}

	QString disp;

	if (displayFeature(id, disp))
	{
		uint val = readPathUInt(ASUS_DISPLAY_PATH);
		uint offs = 0;

		if (disp == ASUS_LCD) offs = 0;
		else if (disp == ASUS_CRT) offs = 1;
		else if (disp == ASUS_TV) offs = 2;
		else if (disp == ASUS_DVI) offs = 3;
		else return "";

		if (val & (1<<offs) )
		{
			return ASUS_ON;
		}

		return ASUS_OFF;
	}

	return "";
}

bool SysAsus::featureWrite(const QString &id, const QString &nVal, LapsusDBus *dbus)
{
	if (id == ASUS_BACKLIGHT_ID)
	{
		bool res = false;

		uint uVal = nVal.toUInt(&res);

		if (!res) return false;

		if (uVal > maxBacklight)
			uVal = maxBacklight;

		uint oVal = readPathUInt(ASUS_GET_BACKLIGHT_PATH);

		if (oVal == uVal) return false;

		res = writePathUInt(ASUS_SET_BACKLIGHT_PATH, uVal);

		if (res)
		{
			dbus->signalFeatureChanged(ASUS_BACKLIGHT_ID, QString::number(uVal));
		}

		return res;
	}
	else if (hasFeature(id))
	{
		uint uVal = 0;

		if (nVal == ASUS_ON) uVal = 1;
		else if (nVal == ASUS_OFF) uVal = 0;
		else return false;

		uint oVal = readIdUInt(id);

		if (oVal == uVal) return false;

		bool res = writeIdUInt(id, uVal);

		if (res)
		{
			dbus->signalFeatureChanged(id, nVal);
		}

		return res;
	}

	QString disp;

	if (displayFeature(id, disp))
	{
		uint uVal;
		uint offs = 0;

		if (nVal == ASUS_ON) uVal = 1;
		else if (nVal == ASUS_OFF) uVal = 0;
		else return false;

		if (disp == ASUS_LCD) offs = 0;
		else if (disp == ASUS_CRT) offs = 1;
		else if (disp == ASUS_TV) offs = 2;
		else if (disp == ASUS_DVI) offs = 3;
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

		if (res)
		{
			dbus->signalFeatureChanged(id, nVal);
		}

		return res;
	}

	return false;
}
