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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <qdir.h>
#include <qfile.h>
#include <qstringlist.h>

#include "lapsus.h"
#include "sys_ibm.h"

#define IBM_DEBUG_DUMP			0

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
	SysBackend("ibm"),
	_hasLEDs(false), _hasBacklight(false), _hasDisplay(false),
	_hasBluetooth(false), _hasLight(false), _hasVolume(false),
	_hasNVRAM(false), _fdNVRAM(-1), _thinkpadNew(0), _thinkpadOld(0),
	_timerNVRAMId(0), _lastVolume(15), _lastMute(false), _dbusTriggered(false)
{
	detect();

	// We try to use NVRAM only if we know that it is IBM laptop
	if (hardwareDetected())
	{
		_fdNVRAM = open(IBM_NVRAM_DEVICE, O_RDONLY|O_NONBLOCK);

		if (_fdNVRAM >= 0)
		{
			if (IBM_DEBUG_DUMP) printf("Opened NVRAM device\n\n");

			_thinkpadNew = (t_thinkpad_state *) malloc(sizeof(t_thinkpad_state));

			memset(_thinkpadNew, 0, sizeof(t_thinkpad_state));

			if (nvramRead(_thinkpadNew))
			{
				_thinkpadOld = (t_thinkpad_state *) malloc(sizeof(t_thinkpad_state));

				memcpy(_thinkpadOld, _thinkpadNew, sizeof(t_thinkpad_state));

				_hasNVRAM = true;

				_timerNVRAMId = startTimer(NVRAM_POLL_INTERVAL_MS);

				if (IBM_DEBUG_DUMP) printf("NVRAM polling started\n\n");
			}
			else
			{
				if (IBM_DEBUG_DUMP) printf("Failed to read NVRAM\n\n");

				free(_thinkpadNew);
				_thinkpadNew = 0;
			}
		}
		else
		{
			if (IBM_DEBUG_DUMP) printf("Could not open NVRAM device...\n\n");
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
			if (IBM_DEBUG_DUMP) printf("%s: Value for field '%s': '%s'\n",
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
		{
			_hasLEDs = true;

			setFeature(LAPSUS_FEAT_LED_ID_PREFIX "0", "",
					I18N_NOOP("Power LED"));
			setFeature(LAPSUS_FEAT_LED_ID_PREFIX "1", "",
					I18N_NOOP("Orange Battery LED"));
			setFeature(LAPSUS_FEAT_LED_ID_PREFIX "2", "",
					I18N_NOOP("Green Battery LED"));
			setFeature(LAPSUS_FEAT_LED_ID_PREFIX "3", "",
					I18N_NOOP("UltraBase LED"));
			setFeature(LAPSUS_FEAT_LED_ID_PREFIX "4", "",
					I18N_NOOP("UltraBay LED"));
			setFeature(LAPSUS_FEAT_LED_ID_PREFIX "5", "",
					I18N_NOOP("5th LED"));
			setFeature(LAPSUS_FEAT_LED_ID_PREFIX "6", "",
					I18N_NOOP("6th LED"));
			setFeature(LAPSUS_FEAT_LED_ID_PREFIX "7", "",
					I18N_NOOP("Standby LED"));
		}
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
	if (_hasLEDs || _hasBacklight || _hasDisplay
		|| _hasBluetooth || _hasLight || _hasVolume)
	{
		if (IBM_DEBUG_DUMP)
		{
			printf("Detected IBM hardware\nDetected features:\n\n");
	
			QStringList lines = this->featureList();
	
			for ( QStringList::ConstIterator it = lines.begin(); it != lines.end(); ++it )
			{
				QString line = (*it);
	
				printf("Feature ID: %s\n", line.ascii());
				printf("Feature Name: %s\n", this->featureName(line).ascii());
	
				QStringList args = this->featureArgs(line);
	
				for ( QStringList::ConstIterator iter = args.begin(); iter != args.end(); ++iter )
					printf("Feature Arg: %s\n", (*iter).ascii());
	
				printf("Feature Value: %s\n\n", this->featureRead(line).ascii());
			}
		}
		
		return true;
	}
	
	return false;
}

QStringList SysIBM::featureList()
{
	QStringList ret;

	if (_hasLEDs)
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
		ret.append(LAPSUS_FEAT_MUTE);
		ret.append(LAPSUS_FEAT_UNMUTE);
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

QString SysIBM::featureRead(const QString &id)
{
	if (IBM_DEBUG_DUMP) printf("Feature Read: '%s'\n\n", id.ascii());

	if (id == LAPSUS_FEAT_BACKLIGHT_ID)
		return fieldValue("level", IBM_BACKLIGHT_PATH);

	if (id == IBM_LIGHT_ID)
	{
		if (fieldValue("status", IBM_LIGHT_PATH) == IBM_ON)
			return LAPSUS_FEAT_ON;

		return LAPSUS_FEAT_OFF;
	}

	if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		QString volMute = fieldValue("mute", IBM_VOLUME_PATH);
		
		bool ok;
		QString vol = fieldValue("level", IBM_VOLUME_PATH);
		uint uI = vol.toUInt(&ok);
		
		if (ok) _lastVolume = uI;
		
		if (volMute == "on")
		{
			_lastMute = true;
			
			return QString("%1," LAPSUS_FEAT_MUTE).arg(vol);
		}
		
		_lastMute = false;
		
		return vol;
	}

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
	QString oVal = featureRead(id);

	if (IBM_DEBUG_DUMP) printf("Feature Write: '%s'\nOld value was: '%s'\nNew value is: '%s'\n\n",
		id.ascii(), oVal.ascii(), nVal.ascii());

	if (oVal.length() < 1) return false;

	if (id == LAPSUS_FEAT_BACKLIGHT_ID)
	{
		bool ok;
		int n = nVal.toInt(&ok);

		if (!ok) return false;

		if (n < 0 || n > 7) return false;

		QString lvl = QString::number(n);

		if (oVal != lvl
			&& dbgWritePathString(IBM_BACKLIGHT_PATH, QString("level %1").arg(lvl)))
		{
			if (!_hasNVRAM)
			{
				dbusSignalFeatureUpdate(id, lvl);
			}
			
			return true;
		}

		return false;
	}

	if (id == LAPSUS_FEAT_VOLUME_ID)
	{
		QStringList list = QStringList::split(",", nVal);
		bool setMute = false;
		bool setUnMute = false;
		uint setVal = _lastVolume;
		
		for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
		{
			QString val = *it;
			
			if (val == LAPSUS_FEAT_MUTE)
			{
				setMute = true;
				setUnMute = false;
			}
			else if (val == LAPSUS_FEAT_UNMUTE)
			{
				setMute = false;
				setUnMute = true;
			}
			else
			{
				bool res = false;
		
				uint iVal = val.toUInt(&res);
		
				if (res) setVal = iVal;
			}
		}
		
		if (setVal > 15) setVal = _lastVolume;
		
		QString vol = fieldValue("mute", IBM_VOLUME_PATH);
		bool isMuted = false;
		
		if (vol == "on") isMuted = true;
		
		bool ret = false;
		
		if (setVal > _lastVolume && isMuted)
		{
			setMute = false;
			setUnMute = true;
		}
		
		if (setMute)
		{
			ret |= dbgWritePathString(IBM_VOLUME_PATH, QString("mute"));
			if (ret) isMuted = true;
		}
		else if (setUnMute)
		{
			ret |= dbgWritePathString(IBM_VOLUME_PATH, QString("up"));
			if (ret) isMuted = false;
		}
		
		vol = QString::number(setVal);
		
		if (setVal != _lastVolume)
		{
			ret |= dbgWritePathString(IBM_VOLUME_PATH,
				QString("level %1").arg(vol));
		}
		
		if (ret)
		{
			if (!_hasNVRAM)
			{
				if (isMuted)
				{
					vol = QString("%1," LAPSUS_FEAT_MUTE).
						arg(vol);
				}
				else if (setUnMute && setVal == _lastVolume)
				{
					vol = LAPSUS_FEAT_UNMUTE;
				}
			
			 	dbusSignalFeatureUpdate(id, vol);
			}
			
			return true;
		}
		
		return false;
	}

	if (id == IBM_LIGHT_ID)
	{
		bool val;

		if (nVal == LAPSUS_FEAT_ON) val = true;
		else val = false;

		if (val && oVal == LAPSUS_FEAT_ON) return false;
		if (!val && oVal == LAPSUS_FEAT_OFF) return false;

		if (dbgWritePathString(IBM_LIGHT_PATH, val?IBM_ON:IBM_OFF))
		{
			if (!_hasNVRAM)
			{
				dbusSignalFeatureUpdate(id,
					val?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);
			}
			
			return true;
		}
		
		return false;
	}

	if (id == LAPSUS_FEAT_BLUETOOTH_ID)
	{
		bool val;

		if (nVal == LAPSUS_FEAT_ON) val = true;
		else val = false;

		if (val && oVal == LAPSUS_FEAT_ON) return false;
		if (!val && oVal == LAPSUS_FEAT_OFF) return false;

		if(dbgWritePathString(IBM_BLUETOOTH_PATH, val?IBM_ENABLE:IBM_DISABLE))
		{
			dbusSignalFeatureUpdate(id, val?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);
			return true;
		}
		
		return false;
	}

	QString tmp;

	if (isDisplayFeature(id, tmp))
	{
		bool val, res = false;

		if (nVal == LAPSUS_FEAT_ON) val = true;
		else val = false;

		if (val && oVal == LAPSUS_FEAT_ON) return false;
		if (!val && oVal == LAPSUS_FEAT_OFF) return false;

		if (val) res = dbgWritePathString(IBM_DISPLAY_PATH, tmp.append("_" IBM_ENABLE));
		else res = dbgWritePathString(IBM_DISPLAY_PATH, tmp.append("_" IBM_DISABLE));

		if (res)
		{
			dbusSignalFeatureUpdate(id, val?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);
			return true;
		}
		
		return false;
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
			
			if (dbgWritePathString(IBM_LED_PATH, QString("%1 " IBM_ON).arg(tmp)))
			{
				_leds[tmp] = LAPSUS_FEAT_ON;
				dbusSignalFeatureUpdate(id, LAPSUS_FEAT_ON);
				return true;
			}
		}
		else if (val == 1)
		{
			if (oVal == LAPSUS_FEAT_BLINK) return false;
			
			if (dbgWritePathString(IBM_LED_PATH, QString("%1 " IBM_BLINK).arg(tmp)))
			{
				_leds[tmp] = LAPSUS_FEAT_BLINK;
				dbusSignalFeatureUpdate(id, LAPSUS_FEAT_BLINK);
				return true;
			}
		}
		else
		{
			if (oVal == LAPSUS_FEAT_OFF) return false;
			
			if (dbgWritePathString(IBM_LED_PATH, QString("%1 " IBM_OFF).arg(tmp)))
			{
				_leds[tmp] = LAPSUS_FEAT_OFF;
				dbusSignalFeatureUpdate(id, LAPSUS_FEAT_OFF);
				return true;
			}
		}

		return false;
	}

	return false;
}

QString SysIBM::dbgReadPathString(const QString &path)
{
	QString ret = readPathString(path);

	if (IBM_DEBUG_DUMP) printf("READ '%s':\n%s(END)\n\n", path.ascii(), ret.ascii());

	return ret;
}

bool SysIBM::dbgWritePathString(const QString &path, const QString &val)
{
	_dbusTriggered = true;
	
	bool ret = writePathString(path, val);

	if (IBM_DEBUG_DUMP) printf("WRITE [%d] '%s' <- '%s'\n\n", ret, path.ascii(), val.ascii());

	return ret;
}

#define CHECK_NVRAM_ARG(arg)	checkNVRAMPair(_thinkpadOld->arg, _thinkpadNew->arg, #arg)

bool SysIBM::checkNVRAMPair(unsigned char vOld, unsigned char vNew, const char *desc)
{
	if (vOld != vNew)
	{
		if (IBM_DEBUG_DUMP) printf("NVRAM EVENT: %s = 0x%02x (%u)\n\n", desc, vNew, vNew);

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

		if (!_dbus) return;

		QString muteChange;
		
		if (_thinkpadOld->mute_toggle != _thinkpadNew->mute_toggle)
		{
			if (_thinkpadNew->mute_toggle)
			{
				_lastMute = true;
				muteChange = LAPSUS_FEAT_MUTE;
			}
			else
			{
				_lastMute = false;
				muteChange = LAPSUS_FEAT_UNMUTE;
			}
		}
		
		if (_thinkpadOld->volume_level != _thinkpadNew->volume_level)
		{
			_lastVolume = _thinkpadNew->volume_level;
			
			if (!_lastMute)
			{
				signalNVRAMNumChange(LAPSUS_FEAT_VOLUME_ID,
					_thinkpadNew->volume_level);
			}
			else
			{
				signalNVRAMStrChange(LAPSUS_FEAT_VOLUME_ID,
					QString("%1," LAPSUS_FEAT_MUTE).
						arg(QString::number(_thinkpadNew->volume_level)));
			}
		}
		else if (muteChange.length() > 0)
		{
			signalNVRAMStrChange(LAPSUS_FEAT_VOLUME_ID, muteChange);
		}

		if (_thinkpadOld->brightness_level != _thinkpadNew->brightness_level)
		{
			signalNVRAMNumChange(LAPSUS_FEAT_BACKLIGHT_ID,
					_thinkpadNew->brightness_level);
		}
		
		if (_thinkpadOld->thinklight_toggle != _thinkpadNew->thinklight_toggle)
		{
			if (_thinkpadNew->thinklight_toggle)
			{
				signalNVRAMStrChange(IBM_LIGHT_ID,
					LAPSUS_FEAT_ON);
			}
			else
			{
				signalNVRAMStrChange(IBM_LIGHT_ID,
					LAPSUS_FEAT_OFF);
			}
		}
		
		// Something has changed. Lets generate ACPI events,
		// but only if it wasn't triggered by dbus write:
		if (!_dbusTriggered)
		{
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
	
	_dbusTriggered = false;
}

void SysIBM::signalNVRAMStrChange(const QString &id, const QString &nVal)
{
	dbusSignalFeatureUpdate(id, nVal);
	
	if (!_dbusTriggered)
		dbusSignalFeatureNotif(id, nVal);
}

void SysIBM::signalNVRAMNumChange(const QString &id, unsigned char nVal)
{
	QString val = QString::number(nVal);

	signalNVRAMStrChange(id, val);
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
