/***************************************************************************
 *   Copyright (C) 2007, 2008 by Jakub Schmidtke                           *
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

#ifndef SYS_IBM_H
#define SYS_IBM_H

#include <qmap.h>
#include <qstringlist.h>
#include <qstring.h>

#include "sys_backend.h"

#define IBM_NVRAM_DEVICE	"/dev/nvram"

typedef struct {
	unsigned char thinkpad_toggle;		/* ThinkPad button */
	unsigned char zoom_toggle;		/* Zoom toggle */
	unsigned char display_toggle;		/* Display toggle */
	unsigned char home_toggle;		/* Home button */
	unsigned char search_toggle;		/* Search button */
	unsigned char mail_toggle;		/* Mail button */
	unsigned char wireless_toggle;		/* Wireless button */
	unsigned char thinklight_toggle;	/* ThinkLight */
	unsigned char hibernate_toggle;		/* Hibernation/suspend toggle */
	unsigned char display_state;		/* Display state */
	unsigned char expand_toggle;		/* HV expansion state */
	unsigned char brightness_level;		/* Brightness level */
	unsigned char brightness_toggle;	/* Brightness toggle */
	unsigned char volume_level;		/* Volume level */
	unsigned char volume_toggle;		/* Volume toggle */
	unsigned char mute_toggle;		/* Mute toggle */
	unsigned char powermgt_ac;		/* Power management mode ac */
	unsigned char powermgt_battery;		/* Power management mode battery */
} t_thinkpad_state;

/**
 * Backend, which controls asus-laptop's kernel module /sys interface
 * files and "knows" what features can be supported.
 */
class SysIBM : public SysBackend
{
	public:
		SysIBM();
		~SysIBM();

		QStringList featureList();
		QString featureName(const QString &id);
		QStringList featureArgs(const QString &id);
		QString featureRead(const QString &id);
		bool featureWrite(const QString &id, const QString &nVal);
		bool checkACPIEvent(const QString &group, const QString &action,
			const QString &device, uint id, uint value);

		bool hardwareDetected();

	protected:
		QString dbgReadPathString(const QString &path);
		bool dbgWritePathString(const QString &path, const QString &val);
		void timerEvent( QTimerEvent * );

	private:
		QMap<QString, QString> _leds;
		bool _hasLEDs;
		bool _hasBacklight;
		bool _hasDisplay;
		bool _hasBluetooth;
		bool _hasLight;
		bool _hasVolume;
		bool _hasNVRAM;
		int _fdNVRAM;
		t_thinkpad_state *_thinkpadNew;
		t_thinkpad_state *_thinkpadOld;
		int _timerNVRAMId;
		uint _lastVolume;
		bool _lastMute;
		bool _dbusTriggered;

		void detect();

		QString fieldValue(const QString &fieldName, const QString &path);
		bool nvramReadBuf(unsigned char *buf, off_t pos, size_t len);
		bool nvramRead(t_thinkpad_state *tState);
		bool checkNVRAMPair(unsigned char vOld, unsigned char VNew, const char *desc);
		void signalNVRAMNumChange(const QString &id, unsigned char nVal);
		void signalNVRAMStrChange(const QString &id, const QString &nVal);
};

#endif
