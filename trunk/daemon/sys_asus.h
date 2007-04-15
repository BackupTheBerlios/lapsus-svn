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

#ifndef SYS_ASUS_H
#define SYS_ASUS_H

#include <qobject.h>
#include <qstringlist.h>
#include <qstring.h>

#include "sys_backend.h"

#include "synaptics.h"

/**
 * Backend, which controls asus-laptop's kernel module /sys interface
 * files and "knows" what features can be supported.
 */
class SysAsus : public SysBackend
{
	Q_OBJECT

	public:
		SysAsus();
		~SysAsus();

		QStringList featureList();
		QStringList featureArgs(const QString &id);
		QStringList featureParams(const QString &id);
		QString featureName(const QString &id);
		QString featureRead(const QString &id);
		bool featureWrite(const QString &id, const QString &nVal);
		bool checkACPIEvent(const QString &group, const QString &action,
			const QString &device, uint id, uint value);

		bool hardwareDetected();

	protected slots:
#ifdef HAVE_ALSA
		void volumeChanged(int val);
		void muteChanged(bool muted);
#endif
		void touchpadChanged(bool nState);
		void acpiEvent(const QString &group, const QString &action,
				const QString &device, uint id, uint value);
	private:
		bool _hasSwitches;
		bool _hasBacklight;
		bool _hasDisplay;
		bool _hasTouchpad;
		bool _hasLightSensor;
		uint _maxBacklight;
		uint _maxLightSensor;

#ifdef HAVE_ALSA
		bool _hasVolume;
		LapsusAlsaMixer *_mix;
#endif
		LapsusSynaptics *_synap;
		bool _notifyTouchpadChange;

		void detect();
		bool setBacklight(uint nVal);
		bool setLightSensorLevel(uint nVal);
};

#endif
