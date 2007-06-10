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
#define ASUS_LS_SWITCH_PATH		"/sys/devices/platform/asus-laptop/ls_switch"
#define ASUS_LS_LEVEL_PATH		"/sys/devices/platform/asus-laptop/ls_level"
#define ASUS_GET_BACKLIGHT_PATH		"/sys/class/backlight/asus-laptop/actual_brightness"
#define ASUS_SET_BACKLIGHT_PATH		"/sys/class/backlight/asus-laptop/brightness"
#define ASUS_MAX_BACKLIGHT_PATH		"/sys/class/backlight/asus-laptop/max_brightness"

#define ASUS_CONF_BACKLIGHT_HK_CIRC	"backlight_hotkey_circular"
#define ASUS_CONF_LIGHT_SENSOR_HK_CIRC	"lightsensor_hotkey_circular"

SysAsus::SysAsus(LapsusModulesList *modList):
	SysBackend("asus"),
	_modList(modList),
	_hasSwitches(false), _hasBacklight(false), _hasDisplay(false),
	_hasLightSensor(false), _maxBacklight(0),
	_maxLightSensor(7), // maxLightSensor can't be larger than 15!
	_lastBacklightHotkeySet(-1)
{
	detect();
}

SysAsus::~SysAsus()
{
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

	if (QFile::exists(ASUS_LS_SWITCH_PATH) && testR(ASUS_LS_SWITCH_PATH)
		&& QFile::exists(ASUS_LS_LEVEL_PATH) && testR(ASUS_LS_LEVEL_PATH))
	{
		setFeature(LAPSUS_FEAT_LIGHT_SENSOR_ID, ASUS_LS_SWITCH_PATH, I18N_NOOP("Light Sensor"));
		
		_hasLightSensor = true;
		
		if (_modList && _modList->config)
		{
			QStringList list;
			
			list.append(LAPSUS_FEAT_ON);
			list.append(LAPSUS_FEAT_OFF);
			
			_modList->config->subscribeEntry(
				_modulePrefix,
				ASUS_CONF_LIGHT_SENSOR_HK_CIRC,
				I18N_NOOP("Controls if HotKey controlled LightSensor level list is circular"),
				list,
				LAPSUS_FEAT_ON);
				
		}
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
				// maxBacklight should not be larger than 15 - 
				// if it is we are running into problems while dealing
				// with brightness_up and down hotkeys.
				if (_maxBacklight > 15)
					_maxBacklight = 15;
				
				_hasBacklight = true;
				
				if (_modList && _modList->config)
				{
					QStringList list;
					
					list.append(LAPSUS_FEAT_ON);
					list.append(LAPSUS_FEAT_OFF);
					
					_modList->config->subscribeEntry(
						_modulePrefix,
						ASUS_CONF_BACKLIGHT_HK_CIRC,
						I18N_NOOP("Controls if HotKey controlled brightness level list is circular"),
						list,
						LAPSUS_FEAT_ON);
						
				}
			}
			else
			{
				_maxBacklight = 0;
			}
		}
	}

	if (!hardwareDetected()) return;
}

bool SysAsus::hardwareDetected()
{
	return _hasSwitches
		|| _hasBacklight
		|| _hasDisplay
		|| _hasLightSensor;
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

	if (_hasLightSensor)
	{
		ret.append(LAPSUS_FEAT_LIGHT_SENSOR_ID);
		ret.append(LAPSUS_FEAT_LIGHT_SENSOR_LEVEL_ID);
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
	else if (id == LAPSUS_FEAT_LIGHT_SENSOR_LEVEL_ID)
	{
		if (_maxLightSensor > 0)
		{
			ret.append(QString("0:%1").arg(QString::number(_maxLightSensor)));
		}
	}
	else if (hasFeature(id) || isDisplayFeature(id))
	{
		ret.append(LAPSUS_FEAT_ON);
		ret.append(LAPSUS_FEAT_OFF);
	}

	return ret;
}

QString SysAsus::featureName(const QString &id)
{
	if (id == LAPSUS_FEAT_LIGHT_SENSOR_LEVEL_ID)
	{
		return "Light Sensor Level";
	}

	return SysBackend::featureName(id);
}

bool SysAsus::handleACPIEvent(const QString &group, const QString &action,
		const QString &device, uint id, uint)
{
	if (group != "hotkey" || action != "hotkey" || device != "ATKD")
		return false;

	if (_modList->mixer)
	{
		if (id == 0x32)
		{
			_modList->mixer->mixerToggleMuted(true);
			
			return true;
		}
		else if (id == 0x31)
		{
			_modList->mixer->mixerVolumeDown(true);
			
			return true;
		}
		else if (id == 0x30)
		{
			_modList->mixer->mixerVolumeUp(true);
			
			return true;
		}
	}

	if (_modList->synaptics && id == 0x6b)
	{
		_modList->synaptics->toggleState(true);

		return true;
	}
	
	// if a LightSensor is available and enabled, the brightness button controls
	// its sensitivity level instead of directly controlling the backlight
	if (_hasLightSensor && featureRead(LAPSUS_FEAT_LIGHT_SENSOR_ID) == LAPSUS_FEAT_ON)
	{
		if (	(id >= 0x20 && id < (0x20 + _maxLightSensor) ) ||
			(id > 0x10 && id <= (0x10 + _maxLightSensor)))
		{
			int oVal = readPathString(ASUS_LS_LEVEL_PATH).toInt();
			int nVal;
	
			if (id < 0x20) nVal = id - 0x10;
			else nVal = id - 0x20;
	
			if (!_modList || !_modList->config
			|| _modList->config->getEntryValue(_modulePrefix,
					ASUS_CONF_LIGHT_SENSOR_HK_CIRC) == LAPSUS_FEAT_ON)
			{
				// UP: 0->1->...->max->0->1->...
				// DOWN: max->max-1->...->1->0->max->max-1->...
				// In case one of the buttons (up/down) doesn't work we
				// are still able to choose any of possible values
				if (nVal == 0 && oVal == nVal)
					nVal = _maxLightSensor;
				else if (nVal == (int) _maxLightSensor && oVal == nVal)
					nVal = 0;
			}
	
			// We force the method to send a signal with current value - the same reason
			// why we do that with brightness.
			setLightSensorLevel(nVal, true);

			// Same as with brightness - we wan't OSD even if it didn't modify the value.
			dbusSignalFeatureNotif(LAPSUS_FEAT_LIGHT_SENSOR_LEVEL_ID,
					QString::number(nVal));
	
			return true;
		}
	}
	// Works only when LightSensor is disabled
	else
	{
		// In case max_backlight is not 15 we are still running into problems,
		// because 0x10+max can't be >= 0x20... But maybe max_backlight is lower.
		// Just to be safe we limit max_backlight to value 15 during init.
		if (_hasBacklight
			&& (	(id >= 0x20 && id < (0x20 + _maxBacklight) ) ||
				(id > 0x10 && id <= (0x10 + _maxBacklight))))
		{
			int nVal;
	
			if (id < 0x20) nVal = id - 0x10;
			else nVal = id - 0x20;
	
			if (!_modList || !_modList->config
			|| _modList->config->getEntryValue(_modulePrefix,
					ASUS_CONF_BACKLIGHT_HK_CIRC) == LAPSUS_FEAT_ON)
			{
				// UP: 0->1->...->max->0->1->...
				// DOWN: max->max-1->...->1->0->max->max-1->...
				// In case one of the buttons (up/down) doesn't work we
				// are still able to choose any of possible values
				if (nVal == 0 && _lastBacklightHotkeySet == nVal)
					nVal = _maxBacklight;
				else if (nVal == (int) _maxBacklight && _lastBacklightHotkeySet == nVal)
					nVal = 0;
			}
	
			// Backlight is changed by a hotkey - no risk of infinite loop
			// of signals - we force setBacklight to emit a signal with
			// current brightness value. Needed if something else modified
			// backlight already.
			setBacklight(nVal, true);
			
			_lastBacklightHotkeySet = nVal;
			
			// We also want to show OSD even if we haven't modified the brightness
			// Hotkey has been pressed, so we want to show OSD.
			dbusSignalFeatureNotif(LAPSUS_FEAT_BACKLIGHT_ID,
					QString::number(nVal));
			
			return true;
		}
	}

	if (_hasLightSensor && id == 0x7a)
	{
		QString nVal = LAPSUS_FEAT_ON;

		if (featureRead(LAPSUS_FEAT_LIGHT_SENSOR_ID) == nVal)
			nVal = LAPSUS_FEAT_OFF;
		
		if (featureWrite(LAPSUS_FEAT_LIGHT_SENSOR_ID, nVal))
		{
			dbusSignalFeatureNotif(LAPSUS_FEAT_LIGHT_SENSOR_ID,
					nVal);
		}

		return true;
	}

	// Unknown hotkey. Just send is as a normal ACPI event.
	return false;
}

QString SysAsus::featureRead(const QString &id)
{
	if (id == LAPSUS_FEAT_BACKLIGHT_ID)
	{
		return readPathString(ASUS_GET_BACKLIGHT_PATH);
	}
	else if (id == LAPSUS_FEAT_LIGHT_SENSOR_LEVEL_ID)
	{
		return readPathString(ASUS_LS_LEVEL_PATH);
	}
	else if (hasFeature(id))
	{
		uint val = readIdUInt(id);

		if (val) return LAPSUS_FEAT_ON;

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

bool SysAsus::setBacklight(uint uVal, bool forceSignal)
{
	if (uVal > _maxBacklight)
		uVal = _maxBacklight;

	uint oVal = readPathUInt(ASUS_GET_BACKLIGHT_PATH);

	bool res = false;

	if (oVal != uVal) res = writePathUInt(ASUS_SET_BACKLIGHT_PATH, uVal);

	if (res || forceSignal)
	{
		dbusSignalFeatureChanged(LAPSUS_FEAT_BACKLIGHT_ID, QString::number(uVal));
	}
	
	return true;
}

bool SysAsus::setLightSensorLevel(uint uVal, bool forceSignal)
{
	if (uVal > _maxLightSensor)
		uVal = _maxLightSensor;

	uint oVal = readPathUInt(ASUS_LS_LEVEL_PATH);

	bool res = false;

	if (oVal != uVal) res = writePathUInt(ASUS_LS_LEVEL_PATH, uVal);

	if (res || forceSignal)
	{
		dbusSignalFeatureChanged(LAPSUS_FEAT_LIGHT_SENSOR_LEVEL_ID, QString::number(uVal));
	}

	return true;
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
	else if (id == LAPSUS_FEAT_LIGHT_SENSOR_LEVEL_ID)
	{
		bool res = false;
		uint uVal = nVal.toUInt(&res);

		if (!res) return false;
		
		return setLightSensorLevel(uVal);
	}
	else if (hasFeature(id))
	{
		uint uVal = 0;

		if (nVal == LAPSUS_FEAT_ON) uVal = 1;
		else if (nVal == LAPSUS_FEAT_OFF) uVal = 0;
		else return false;

		uint oVal = readIdUInt(id);

		if (writeIdUInt(id, uVal))
		{
			/*
			 * We perform write even if it is already set to desired value.
			 * Sometimes old status might not reflect real hardware state.
			 * This way we make sure it is set to correct state.
			 * But we don't want to send info if we don't think it has changed...
			 */
			if (oVal != uVal)
				dbusSignalFeatureChanged(id, nVal);
		}
		
		return true;
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

		if (sVal == oVal) return true;

		if (writePathUInt(ASUS_DISPLAY_PATH, sVal))
			dbusSignalFeatureChanged(id, nVal);

		return true;
	}

	return false;
}
