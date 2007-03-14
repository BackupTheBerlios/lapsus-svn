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

#ifndef SYS_IBM_H
#define SYS_IBM_H

#include <qmap.h>
#include <qstringlist.h>
#include <qstring.h>

#include "sys_backend.h"

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
		bool featureWrite(const QString &id, const QString &nVal, LapsusDBus *dbus);

		bool hardwareDetected();
		QString featurePrefix();

	private:
		QMap<QString, QString> _leds;
		bool _hasLEDs;
		bool _hasBacklight;
		bool _hasDisplay;
		bool _hasBluetooth;
		bool _hasLight;
		bool _hasVolume;

		void detect();

		QString fieldValue(const QString &fieldName, const QString &path);
		bool displayFeature(const QString &id);
		bool displayFeature(const QString &id, QString &disp);
		bool ledFeature(const QString &id);
		bool ledFeature(const QString &id, QString &led);
};

#endif
