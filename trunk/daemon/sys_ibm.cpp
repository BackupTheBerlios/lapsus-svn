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
 *                                                                         *
 *   Parts of this code were taken from:                                   *
 *   ThinkPad Buttons - http://savannah.nongnu.org/projects/tpb/           *
 *   by Markus Braun <markus.braun@krawel.de>                              *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qfile.h>
#include <qstringlist.h>

#include "lapsus.h"
#include "sys_ibm.h"

// How often do we want to poll NVRAM for changes?
// For example 200 = every 200ms
#define NVRAM_POLL_INTERVAL_MS		200

#define qPrintable(str)			(str.ascii())

#define IBM_LED_PATH			"/proc/acpi/ibm/led"
#define IBM_LIGHT_PATH			"/proc/acpi/ibm/light"
#define IBM_DISPLAY_PATH		"/proc/acpi/ibm/video"
#define IBM_BACKLIGHT_PATH		"/proc/acpi/ibm/brightness"
#define IBM_BLUETOOTH_PATH		"/proc/acpi/ibm/bluetooth"
#define IBM_VOLUME_PATH			"/proc/acpi/ibm/volume"

#define IBM_LIGHT_ID			"thinklight"

// For reading/writing from/to /proc files
#define IBM_ON				"on"
#define IBM_OFF				"off"
#define IBM_BLINK			"blink"
#define IBM_ENABLE			"enable"
#define IBM_ENABLED			"enabled"
#define IBM_DISABLE			"disable"
#define IBM_DISABLED			"disabled"

// TODO - display auto_enable/disable, expand_toggle and video_switch

SysIBM::SysIBM():
	_hasLEDs(false), _hasBacklight(false), _hasDisplay(false),
	_hasBluetooth(false), _hasLight(false), _hasVolume(false),
	_hasNVRAM(false), _fdNVRAM(-1), _thinkpadNew(0), _thinkpadOld(0),
	_timerNVRAMId(0)
{
	detect();

	// We try to use NVRAM only if we know that it is IBM laptop
	if (hardwareDetected())
	{
		_fdNVRAM = open(IBM_NVRAM_DEVICE, O_RDONLY|O_NONBLOCK);

		if (_fdNVRAM >= 0)
		{
			printf("Opened NVRAM device\n\n");

			_thinkpadNew = (t_thinkpad_state *) malloc(sizeof(t_thinkpad_state));

			memset(_thinkpadNew, 0, sizeof(t_thinkpad_state));

			if (nvramRead(_thinkpadNew))
			{
				_thinkpadOld = (t_thinkpad_state *) malloc(sizeof(t_thinkpad_state));

				memcpy(_thinkpadOld, _thinkpadNew, sizeof(t_thinkpad_state));

				_hasNVRAM = true;

				_timerNVRAMId = startTimer(NVRAM_POLL_INTERVAL_MS);

				printf("NVRAM polling started\n\n");
			}
			else
			{
				printf("Failed to read NVRAM\n\n");

				free(_thinkpadNew);
				_thinkpadNew = 0;
			}
		}
		else
		{
			printf("Could not open NVRAM device...\n\n");
		}
	}
}

SysIBM::~SysIBM()
{
	if (_thinkpadNew) free(_thinkpadNew);
	if (_thinkpadOld) free(_thinkpadOld);

	if (_fdNVRAM >= 0) close(_fdNVRAM);
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
			ret.append(QString(LAPSUS_FEAT_LED_ID_PREFIX "%1").arg(QString::number(i)));
		}
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

	if (_hasBluetooth)
	{
		ret.append(LAPSUS_FEAT_BLUETOOTH_ID);
	}

	if (_hasLight)
	{
		ret.append(IBM_LIGHT_ID);
	}

	if (_hasVolume)
	{
		ret.append(LAPSUS_FEAT_VOLUME_ID);
	}

	return ret;
}

QString SysIBM::featureName(const QString &id)
{
	if (id == IBM_LIGHT_ID) return I18N_NOOP("ThinkLight");

	return SysBackend::featureName(id);
}

QStringList SysIBM::featureArgs(const QString &id)
{
	QStringList ret;

	if (id == LAPSUS_FEAT_BACKLIGHT_ID)
	{
		ret.append("0:7");
	}
	else if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		// TODO - add (un)mute command
		ret.append("0:15");
	}
	else if (id == LAPSUS_FEAT_BLUETOOTH_ID
		|| id == IBM_LIGHT_ID
		|| isDisplayFeature(id))
	{
		// TODO - maybe there should be also separate 'display' id
		// with commands like lcd_enable lcd_disable etc.

		ret.append(IBM_ON);
		ret.append(IBM_OFF);
	}
	else if (isLEDFeature(id))
	{
		ret.append(LAPSUS_FEAT_ON);
		ret.append(LAPSUS_FEAT_BLINK);
		ret.append(LAPSUS_FEAT_OFF);
	}

	return ret;
}

void SysIBM::acpiEvent(const QString &group, const QString &action,
	const QString &device, uint id, uint value)
{
	if (_dbus)
		_dbus->sendACPIEvent(group, action, device, id, value);
}

QString SysIBM::featureRead(const QString &id)
{
	printf("Feature Read: '%s'\n\n", id.ascii());

	if (id == LAPSUS_FEAT_BACKLIGHT_ID)
		return fieldValue("level", IBM_BACKLIGHT_PATH);

	if (id == IBM_LIGHT_ID)
	{
		if (fieldValue("status", IBM_LIGHT_PATH) == IBM_ON)
			return LAPSUS_FEAT_ON;

		return LAPSUS_FEAT_OFF;
	}

	// TODO - add (un)mute command
	if (id == LAPSUS_FEAT_VOLUME_ID)
		return fieldValue("level", IBM_VOLUME_PATH);

	if (id == LAPSUS_FEAT_BLUETOOTH_ID)
	{
		QString val = fieldValue("status", IBM_BLUETOOTH_PATH);

		// Enabled/disabled - lets 'translate' it
		if (val == IBM_ENABLED)
			return LAPSUS_FEAT_ON;
		else
			return LAPSUS_FEAT_OFF;
	}

	QString tmp;

	if (isDisplayFeature(id, tmp))
	{
		QString val = fieldValue(tmp, IBM_DISPLAY_PATH);

		// Enabled/disabled - lets 'translate' it
		if (val == IBM_ENABLED)
			return LAPSUS_FEAT_ON;
		else
			return LAPSUS_FEAT_OFF;
	}

	if (isLEDFeature(id, tmp))
	{
		// There is no way to READ LED status. So we return
		// what we wrote, or OFF it we haven't written anything
		QString val = _leds[tmp];

		if (val.length() < 1)
			return LAPSUS_FEAT_OFF;

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

	if (id == LAPSUS_FEAT_BACKLIGHT_ID)
	{
		bool ok;
		int n = nVal.toInt(&ok);

		if (!ok) return false;

		if (n < 0 || n > 7) return false;

		QString lvl = QString::number(n);

		if (oVal == lvl) return false;

		res = dbgWritePathString(IBM_BACKLIGHT_PATH, QString("level %1").arg(lvl));

		if (res && _dbus) _dbus->signalFeatureChanged(id, lvl);

		return res;
	}

	if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		bool ok;
		int n = nVal.toInt(&ok);

		if (!ok) return false;

		if (n < 0 || n > 15) return false;

		QString lvl = QString::number(n);

		if (oVal == lvl) return false;

		res = dbgWritePathString(IBM_VOLUME_PATH, QString("level %1").arg(lvl));

		if (res && _dbus) _dbus->signalFeatureChanged(id, lvl);

		return res;
	}

	if (id == IBM_LIGHT_ID)
	{
		bool val;

		if (nVal == LAPSUS_FEAT_ON) val = true;
		else val = false;

		if (val && oVal == LAPSUS_FEAT_ON) return false;
		if (!val && oVal == LAPSUS_FEAT_OFF) return false;

		res = dbgWritePathString(IBM_LIGHT_PATH, val?IBM_ON:IBM_OFF);

		if (res && _dbus)
			_dbus->signalFeatureChanged(id, val?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);

		return res;
	}

	if (id == LAPSUS_FEAT_BLUETOOTH_ID)
	{
		bool val;

		if (nVal == LAPSUS_FEAT_ON) val = true;
		else val = false;

		if (val && oVal == LAPSUS_FEAT_ON) return false;
		if (!val && oVal == LAPSUS_FEAT_OFF) return false;

		res = dbgWritePathString(IBM_BLUETOOTH_PATH, val?IBM_ENABLE:IBM_DISABLE);

		if (res && _dbus)
			_dbus->signalFeatureChanged(id, val?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);

		return res;
	}

	QString tmp;

	if (isDisplayFeature(id, tmp))
	{
		bool val;

		if (nVal == LAPSUS_FEAT_ON) val = true;
		else val = false;

		if (val && oVal == LAPSUS_FEAT_ON) return false;
		if (!val && oVal == LAPSUS_FEAT_OFF) return false;

		if (val) res = dbgWritePathString(IBM_DISPLAY_PATH, tmp.append("_" IBM_ENABLE));
		else res = dbgWritePathString(IBM_DISPLAY_PATH, tmp.append("_" IBM_DISABLE));

		if (res && _dbus)
			_dbus->signalFeatureChanged(id, val?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);

		return res;
	}

	if (isLEDFeature(id, tmp))
	{
		int val;

		if (nVal == LAPSUS_FEAT_ON) val = 2;
		else if (nVal == LAPSUS_FEAT_BLINK) val = 1;
		else val = 0;

		if (val == 2)
		{
			if (oVal == LAPSUS_FEAT_ON) return false;
			res = dbgWritePathString(IBM_LED_PATH, QString("%1 " IBM_ON).arg(tmp));

			if (res)
			{
				_leds[tmp] = LAPSUS_FEAT_ON;
				if (_dbus) _dbus->signalFeatureChanged(id, LAPSUS_FEAT_ON);
			}
		}
		else if (val == 1)
		{
			if (oVal == LAPSUS_FEAT_BLINK) return false;
			res = dbgWritePathString(IBM_LED_PATH, QString("%1 " IBM_BLINK).arg(tmp));

			if (res)
			{
				_leds[tmp] = LAPSUS_FEAT_BLINK;
				if (_dbus) _dbus->signalFeatureChanged(id, LAPSUS_FEAT_BLINK);
			}
		}
		else
		{
			if (oVal == LAPSUS_FEAT_OFF) return false;
			res = dbgWritePathString(IBM_LED_PATH, QString("%1 " IBM_OFF).arg(tmp));

			if (res)
			{
				_leds[tmp] = LAPSUS_FEAT_OFF;
				if (_dbus) _dbus->signalFeatureChanged(id, LAPSUS_FEAT_OFF);
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

#define CHECK_NVRAM_ARG(arg)	checkNVRAMPair(_thinkpadOld->arg, _thinkpadNew->arg, #arg)

bool SysIBM::checkNVRAMPair(unsigned char vOld, unsigned char vNew, char *desc)
{
	if (vOld != vNew)
	{
		printf("NVRAM EVENT: %s = %ud\n\n", desc, vNew);

		_dbus->sendACPIEvent("ibm", "nvram_event", desc, 1, vNew);
		return true;
	}

	return false;
}

void SysIBM::timerEvent( QTimerEvent * e)
{
	// If it is NVRAM timer, and we have dbus set.
	// If we don't have dbus, there is no point in polling NVRAM
	if (_timerNVRAMId && e->timerId() == _timerNVRAMId && _dbus)
	{
		t_thinkpad_state *tmp;

		tmp = _thinkpadOld;
		_thinkpadOld = _thinkpadNew;
		_thinkpadNew = tmp;

		if (!nvramRead(_thinkpadNew))
		{
			// nvram reading failed. We stop polling.
			// Just to be safe... for now.
			killTimer(_timerNVRAMId);
			_timerNVRAMId = 0;

			return;
		}

		// If nothing has changed...
		if (!memcmp(_thinkpadOld, _thinkpadNew, sizeof(t_thinkpad_state)))
			return;

		// Something has changed. Lets check what:
		CHECK_NVRAM_ARG(thinkpad_toggle);
		CHECK_NVRAM_ARG(zoom_toggle);
		CHECK_NVRAM_ARG(display_toggle);
		CHECK_NVRAM_ARG(home_toggle);
		CHECK_NVRAM_ARG(search_toggle);
		CHECK_NVRAM_ARG(mail_toggle);
		CHECK_NVRAM_ARG(wireless_toggle);
		CHECK_NVRAM_ARG(thinklight_toggle);
		CHECK_NVRAM_ARG(hibernate_toggle);
		CHECK_NVRAM_ARG(display_state);
		CHECK_NVRAM_ARG(expand_toggle);
		CHECK_NVRAM_ARG(brightness_level);
		CHECK_NVRAM_ARG(brightness_toggle);
		CHECK_NVRAM_ARG(volume_level);
		CHECK_NVRAM_ARG(volume_toggle);
		CHECK_NVRAM_ARG(mute_toggle);
		CHECK_NVRAM_ARG(powermgt_ac);
		CHECK_NVRAM_ARG(powermgt_battery);
	}
}

bool SysIBM::nvramReadBuf(unsigned char *buf, off_t pos, size_t len)
{
	if (lseek(_fdNVRAM, pos, SEEK_SET) != pos) return false;

	if (read(_fdNVRAM, buf, len) != (int) len) return false;

	return true;
}

/* get the current state from the nvram */
bool SysIBM::nvramRead(t_thinkpad_state *tState)
{
	unsigned char buf[4];

	// Read only the interesting bytes from nvram to reduce the CPU usage
	// The kernel nvram driver reads byte-by-byte from nvram, so just
	// reading interesting bytes reduces the amount of inb() calls */

	if (!nvramReadBuf(buf, 0x39, 0x01)) return false;

	tState->powermgt_ac		= (buf[0] & 0x07);
	tState->powermgt_battery	= (buf[0] & 0x38) >> 3;

	if (!nvramReadBuf(buf, 0x56, 0x04)) return false;

	tState->home_toggle		= (buf[0] & 0x01);
	tState->search_toggle		= (buf[0] & 0x02) >> 1;
	tState->mail_toggle		= (buf[0] & 0x04) >> 2;
	tState->wireless_toggle		= (buf[0] & 0x20) >> 5;
	tState->thinkpad_toggle		= (buf[1] & 0x08) >> 3;
	tState->zoom_toggle		= (buf[1] & 0x20) >> 5;
	tState->display_toggle		= (buf[1] & 0x40) >> 6;
	tState->hibernate_toggle	= (buf[2] & 0x01);
	tState->thinklight_toggle	= (buf[2] & 0x10) >> 4;
	tState->display_state		= (buf[3] & 0x03);
	tState->expand_toggle		= (buf[3] & 0x10) >> 4;

	if (!nvramReadBuf(buf, 0x5e, 0x01)) return false;

	tState->brightness_level	= (buf[0] & 0x07);
	tState->brightness_toggle	= (buf[0] & 0x20) >> 5;

	if (!nvramReadBuf(buf, 0x60, 0x01)) return false;

	tState->volume_level		= (buf[0] & 0x0f);
	tState->volume_toggle		= (buf[0] & 0x80) >> 7;
	tState->mute_toggle		= (buf[0] & 0x40) >> 6;

	return true;
}
