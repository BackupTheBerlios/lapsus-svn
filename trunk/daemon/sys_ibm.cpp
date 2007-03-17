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

#include "sys_ibm.h"

#define qPrintable(str)			(str.ascii())

#define IBM_LED_PATH			"/proc/acpi/ibm/led"
#define IBM_LIGHT_PATH			"/proc/acpi/ibm/light"
#define IBM_DISPLAY_PATH		"/proc/acpi/ibm/video"
#define IBM_BACKLIGHT_PATH		"/proc/acpi/ibm/brightness"
#define IBM_BLUETOOTH_PATH		"/proc/acpi/ibm/bluetooth"
#define IBM_VOLUME_PATH			"/proc/acpi/ibm/volume"

// Following values have to be lower-case!
#define IBM_PREFIX			"ibm_"
#define IBM_DISPLAY_ID			"ibm_display_"
#define IBM_LED_ID			"ibm_led_"
#define IBM_LIGHT_ID			(IBM_PREFIX "light")
#define IBM_VOLUME_ID			(IBM_PREFIX "volume")
#define IBM_BACKLIGHT_ID		(IBM_PREFIX "backlight")
#define IBM_BLUETOOTH_ID		(IBM_PREFIX "bluetooth")

// Following values have to be lower-case!
#define IBM_ON				"on"
#define IBM_OFF				"off"
#define IBM_BLINK			"blink"
#define IBM_ENABLE			"enable"
#define IBM_ENABLED			"enabled"
#define IBM_DISABLE			"disable"
#define IBM_DISABLED			"disabled"

// Following values have to be lower-case!
// TODO - auto_enable/disable, expand_toggle and video_switch
#define IBM_LCD				"lcd"
#define IBM_CRT				"crt"
#define IBM_TV				"tv"
#define IBM_DVI				"dvi"

SysIBM::SysIBM():
	_hasLEDs(false), _hasBacklight(false), _hasDisplay(false),
	_hasBluetooth(false), _hasLight(false), _hasVolume(false)
{
	detect();
}

SysIBM::~SysIBM()
{
}

QString SysIBM::featurePrefix()
{
	return IBM_PREFIX;
}

QString SysIBM::fieldValue(const QString &fieldName, const QString &path)
{
	int len = fieldName.length();

	QString val;

	if (len < 1) return val;

	val = dbgReadPathString(path);

	if (val.length() < 1) return val;

	QStringList lines = QStringList::split("\n", val);

	for ( QStringList::ConstIterator it = lines.begin(); it != lines.end(); ++it )
	{
		QString line = (*it);

		if ((int) line.length() > len + 1
			&& line[len] == ':'
			&& line.startsWith(fieldName))
		{
			printf("%s: Value for field '%s': '%s'\n",
				path.ascii(), fieldName.ascii(),
				line.mid(len+2).stripWhiteSpace().ascii());
			return line.mid(len+2).stripWhiteSpace();
		}
	}

	return QString();
}

void SysIBM::detect()
{
	QString path, val;

	path = IBM_BACKLIGHT_PATH;

	if (QFile::exists(path) && testR(path))
	{
		val = fieldValue("level", path);

		if (val.length() > 0)
		{
			bool ok;

			val.toInt(&ok);

			if (ok) _hasBacklight = true;
		}
	}

	path = IBM_VOLUME_PATH;

	if (QFile::exists(path) && testR(path))
	{
		val = fieldValue("level", path);

		if (val.length() > 0)
		{
			bool ok;

			val.toInt(&ok);

			if (ok) _hasVolume = true;
		}
	}

	path = IBM_BLUETOOTH_PATH;

	if (QFile::exists(path) && testR(path))
	{
		val = fieldValue("status", path);

		if (val == "enabled" || val == "disabled")
			_hasBluetooth = true;
	}

	path = IBM_LIGHT_PATH;

	if (QFile::exists(path) && testR(path))
	{
		val = fieldValue("status", path);

		if (val == "on" || val == "off")
			_hasLight = true;
	}

	path = IBM_LED_PATH;

	if (QFile::exists(path) && testR(path))
	{
		val = fieldValue("status", path);

		if (val == "supported")
			_hasLEDs = true;
	}

	path = IBM_DISPLAY_PATH;

	if (QFile::exists(path) && testR(path))
	{
		val = fieldValue("status", path);

		if (val == "supported")
			_hasDisplay = true;
	}
}

bool SysIBM::hardwareDetected()
{
	return (_hasLEDs || _hasBacklight || _hasDisplay
		|| _hasBluetooth || _hasLight || _hasVolume);
}

QStringList SysIBM::featureList()
{
	QStringList ret;

	if (_hasLEDs)
	{
		for (int i = 0; i <= 7; i++)
		{
			ret.append(QString(IBM_LED_ID "%1").arg(QString::number(i)));
		}
	}

	if (_hasBacklight)
	{
		ret.append(IBM_BACKLIGHT_ID);
	}

	if (_hasDisplay)
	{
		ret.append(IBM_DISPLAY_ID IBM_LCD);
		ret.append(IBM_DISPLAY_ID IBM_CRT);
		ret.append(IBM_DISPLAY_ID IBM_TV);
		ret.append(IBM_DISPLAY_ID IBM_DVI);
	}

	if (_hasBluetooth)
	{
		ret.append(IBM_BLUETOOTH_ID);
	}

	if (_hasLight)
	{
		ret.append(IBM_LIGHT_ID);
	}

	if (_hasVolume)
	{
		ret.append(IBM_VOLUME_ID);
	}

	return ret;
}

bool SysIBM::displayFeature(const QString &id)
{
	if (id.length() > strlen(IBM_DISPLAY_ID)
		&& id.startsWith(IBM_DISPLAY_ID))
	{
		return true;
	}

	return false;
}

bool SysIBM::displayFeature(const QString &id, QString &disp)
{
	if (!displayFeature(id))
		return false;

	disp = id.mid(strlen(IBM_DISPLAY_ID));

	return true;
}

bool SysIBM::ledFeature(const QString &id)
{
	if (id.length() > strlen(IBM_LED_ID)
		&& id.startsWith(IBM_LED_ID))
	{
		return true;
	}

	return false;
}

bool SysIBM::ledFeature(const QString &id, QString &led)
{
	if (!ledFeature(id))
		return false;

	led = id.mid(strlen(IBM_LED_ID));

	return true;
}

QString SysIBM::featureName(const QString &id)
{
	if (id == IBM_BACKLIGHT_ID) return I18N_NOOP("LCD Backlight");
	if (id == IBM_BLUETOOTH_ID) return I18N_NOOP("Bluetooth adapter");
	if (id == IBM_LIGHT_ID) return I18N_NOOP("ThinkLight");
	if (id == IBM_VOLUME_ID) return I18N_NOOP("Volume");

	QString tmp;

	if (displayFeature(id, tmp))
		return QString("%1 Display").arg(tmp.upper());

	if (ledFeature(id, tmp))
		return QString("LED %1").arg(tmp);

	printf("Unknown name for: '%s'\n", id.ascii());

	return "";
}

QStringList SysIBM::featureArgs(const QString &id)
{
	QStringList ret;

	if (id == IBM_BACKLIGHT_ID)
	{
		ret.append("0:7");
	}
	else if (id == IBM_VOLUME_ID)
	{
		// TODO - add (un)mute command
		ret.append("0:15");
	}
	else if (id == IBM_BLUETOOTH_ID
		|| id == IBM_LIGHT_ID
		|| displayFeature(id))
	{
		// TODO - maybe there should be also separate 'display' id
		// with commands like lcd_enable lcd_disable etc.

		ret.append(IBM_ON);
		ret.append(IBM_OFF);
	}
	else if (ledFeature(id))
	{
		ret.append(IBM_ON);
		ret.append(IBM_BLINK);
		ret.append(IBM_OFF);
	}

	return ret;
}

bool SysIBM::checkACPIEvent(const QString &group, const QString &action,
	const QString &device, uint id, uint value)
{
	return false;
}

QString SysIBM::featureRead(const QString &id)
{
	printf("Feature Read: '%s'\n\n", id.ascii());

	if (id == IBM_BACKLIGHT_ID)
		return fieldValue("level", IBM_BACKLIGHT_PATH);

	if (id == IBM_LIGHT_ID)
		return fieldValue("status", IBM_LIGHT_PATH);

	// TODO - add (un)mute command
	if (id == IBM_VOLUME_ID)
		return fieldValue("level", IBM_VOLUME_PATH);

	if (id == IBM_BLUETOOTH_ID)
	{
		QString val = fieldValue("status", IBM_BLUETOOTH_PATH);

		// Enabled/disabled - lets 'translate' it
		if (val == IBM_ENABLED)
			return IBM_ON;
		else
			return IBM_OFF;
	}

	QString tmp;

	if (displayFeature(id, tmp))
	{
		QString val = fieldValue(tmp, IBM_DISPLAY_PATH);

		// Enabled/disabled - lets 'translate' it
		if (val == IBM_ENABLED)
			return IBM_ON;
		else
			return IBM_OFF;
	}

	if (ledFeature(id, tmp))
	{
		// There is no way to READ LED status. So we return
		// what we wrote, or OFF it we haven't written anything
		QString val = _leds[tmp];

		if (val.length() < 1)
			return IBM_OFF;

		return val;
	}

	return "";
}

bool SysIBM::featureWrite(const QString &id, const QString &nVal)
{
	bool res = false;
	QString oVal = featureRead(id);

	printf("Feature Write: '%s'\nOld value was: '%s'\nNew value is: '%s'\n\n",
		id.ascii(), oVal.ascii(), nVal.ascii());

	if (oVal.length() < 1) return false;

	if (id == IBM_BACKLIGHT_ID)
	{
		bool ok;
		int n = nVal.toInt(&ok);

		if (!ok) return false;

		if (n < 0 || n > 7) return false;

		QString lvl = QString::number(n);

		if (oVal == lvl) return false;

		res = dbgWritePathString(IBM_BACKLIGHT_PATH, QString("level %1").arg(lvl));

		if (res) _dbus->signalFeatureChanged(id, lvl);

		return res;
	}

	if (id == IBM_VOLUME_ID)
	{
		bool ok;
		int n = nVal.toInt(&ok);

		if (!ok) return false;

		if (n < 0 || n > 15) return false;

		QString lvl = QString::number(n);

		if (oVal == lvl) return false;

		res = dbgWritePathString(IBM_VOLUME_PATH, QString("level %1").arg(lvl));

		if (res) _dbus->signalFeatureChanged(id, lvl);

		return res;
	}

	if (id == IBM_LIGHT_ID)
	{
		bool val;

		if (nVal == IBM_ON) val = true;
		else val = false;

		if (val && oVal == IBM_ON) return false;
		if (!val && oVal == IBM_OFF) return false;

		res = dbgWritePathString(IBM_LIGHT_PATH, val?IBM_ON:IBM_OFF);

		if (res) _dbus->signalFeatureChanged(id, val?IBM_ON:IBM_OFF);

		return res;
	}

	if (id == IBM_BLUETOOTH_ID)
	{
		bool val;

		if (nVal == IBM_ON) val = true;
		else val = false;

		if (val && oVal == IBM_ON) return false;
		if (!val && oVal == IBM_OFF) return false;

		res = dbgWritePathString(IBM_BLUETOOTH_PATH, val?IBM_ENABLE:IBM_DISABLE);

		if (res) _dbus->signalFeatureChanged(id, val?IBM_ON:IBM_OFF);

		return res;
	}

	QString tmp;

	if (displayFeature(id, tmp))
	{
		bool val;

		if (nVal == IBM_ON) val = true;
		else val = false;

		if (val && oVal == IBM_ON) return false;
		if (!val && oVal == IBM_OFF) return false;

		if (val) res = dbgWritePathString(IBM_DISPLAY_PATH, tmp.append("_" IBM_ENABLE));
		else res = dbgWritePathString(IBM_DISPLAY_PATH, tmp.append("_" IBM_DISABLE));

		if (res) _dbus->signalFeatureChanged(id, val?IBM_ON:IBM_OFF);

		return res;
	}

	if (ledFeature(id, tmp))
	{
		int val;

		if (nVal == IBM_ON) val = 2;
		else if (nVal == IBM_BLINK) val = 1;
		else val = 0;

		if (val == 2)
		{
			if (oVal == IBM_ON) return false;
			res = dbgWritePathString(IBM_LED_PATH, QString("%1 " IBM_ON).arg(tmp));

			if (res)
			{
				_leds[tmp] = IBM_ON;
				_dbus->signalFeatureChanged(id, IBM_ON);
			}
		}
		else if (val == 1)
		{
			if (oVal == IBM_BLINK) return false;
			res = dbgWritePathString(IBM_LED_PATH, QString("%1 " IBM_BLINK).arg(tmp));

			if (res)
			{
				_leds[tmp] = IBM_BLINK;
				_dbus->signalFeatureChanged(id, IBM_BLINK);
			}
		}
		else
		{
			if (oVal == IBM_OFF) return false;
			res = dbgWritePathString(IBM_LED_PATH, QString("%1 " IBM_OFF).arg(tmp));

			if (res)
			{
				_leds[tmp] = IBM_OFF;
				_dbus->signalFeatureChanged(id, IBM_OFF);
			}
		}

		return res;
	}

	return false;
}

QString SysIBM::dbgReadPathString(const QString &path)
{
	QString ret = readPathString(path);

	printf("READ '%s':\n%s(END)\n\n", path.ascii(), ret.ascii());

	return ret;
}

bool SysIBM::dbgWritePathString(const QString &path, const QString &val)
{
	bool ret = writePathString(path, val);

	printf("WRITE [%d] '%s' <- '%s'\n\n", ret, path.ascii(), val.ascii());

	return ret;
}
