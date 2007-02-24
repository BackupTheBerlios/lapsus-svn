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

// Qt DBUS includes
#include <dbus/qdbusdatalist.h>
#include <dbus/qdbuserror.h>
#include <dbus/qdbusmessage.h>
#include <dbus/qdbusconnection.h>
#include <dbus/qdbusobject.h>

#include "acpi_event_parser.h"
#include "sys_backend.h"

class LapsusDaemon : public QObject, QDBusObjectBase
{
	Q_OBJECT

	public:
		LapsusDaemon();
		~LapsusDaemon();
		bool isValid();
		void addACPIParser(ACPIEventParser *parser);
		bool registerObject(QDBusConnection *conn);

	private:
		SysBackend sysBack;
		QDBusConnection *myConnection;
		uint backlightToEmit;
		QStringList switchesToEmit;
		QStringList displaysToEmit;

		bool sendSignal(const QString &sigName,
				const QValueList<QDBusData>& params);
		bool sendReply(const QDBusMessage& orgMessage,
				const QValueList<QDBusData>& params);
		bool returnDBusError(const QString &str1, const QString &str2,
				const QDBusMessage& message);

	protected:
		virtual bool handleMethodCall(const QDBusMessage& message);

	// QT4: public Q_SLOTS: // METHODS
	public slots:
		void emitBacklight();
		void emitSwitch();
		void emitDisplay();
		void ACPIEvent(const QString &group, const QString &action,
				const QString &device, uint id, uint value);
		uint getBacklight();
		bool getCpufreqParams(QString &governor,
				QStringList &freqSteps, QStringList &governors,
				QString &min, QString &max);
		bool getDisplay(const QString &name);
		uint getMaxBacklight();
		bool getSwitch(const QString &name);
		QStringList listDisplayTypes();
		QStringList listFeatures();
		QStringList listSwitches();
		bool setBacklight(uint value);
		bool setCpufreqParams(const QString &governor,
				const QString &min, const QString &max);
		bool setDisplay(const QString &name, bool value);
		bool setSwitch(const QString &name, bool value);
};

#endif
