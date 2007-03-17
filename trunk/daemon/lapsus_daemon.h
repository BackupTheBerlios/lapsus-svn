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

#ifndef LAPSUS_DAEMON_H
#define LAPSUS_DAEMON_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

class LapsusDaemon;

#include "acpi_event_parser.h"
#include "lapsus_dbus.h"
#include "sys_backend.h"

class LapsusDaemon : public QObject
{
	Q_OBJECT

	public:
		LapsusDaemon(uint acpiFd);
		~LapsusDaemon();
		bool isValid();

		QStringList featureList();
		QString featureName(const QString &id);
		QStringList featureArgs(const QString &id);
		QString featureRead(const QString &id);
		bool featureWrite(const QString &id, const QString &nVal);

	protected slots:
		void acpiEvent(const QString &group, const QString &action,
				const QString &device, uint id, uint value);

	private:
		uint _acpiFd;
		SysBackend *_backend;
		LapsusDBus *_dbus;
		ACPIEventParser *_acpiParser;
		bool _isValid;

		bool detectHardware();
		void doInit();
};

#endif
