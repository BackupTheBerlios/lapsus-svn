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

#ifndef LAPSUS_DBUS_H
#define LAPSUS_DBUS_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>

// Qt DBUS includes
#include <dbus/qdbusdatalist.h>
#include <dbus/qdbuserror.h>
#include <dbus/qdbusmessage.h>
#include <dbus/qdbusconnection.h>
#include <dbus/qdbusobject.h>

class DBUSFeatureManager
{
	public:
		virtual QStringList featureList() = 0;
		virtual QString featureName(const QString &id) = 0;
		virtual QStringList featureArgs(const QString &id) = 0;
		virtual QString featureRead(const QString &id) = 0;
		virtual bool featureWrite(const QString &id, const QString &nVal) = 0;
};

class SignalToSend
{
	public:
		const char *signalID;
		QValueList<QDBusData> signalParams;
		
		SignalToSend(const char *sID,
			const QValueList<QDBusData> & sList):
			signalID(sID), signalParams(sList)
		{
		}
};

class LapsusDBus : public QObject, QDBusObjectBase
{
	Q_OBJECT

	public:
		LapsusDBus(DBUSFeatureManager *fManager);
		~LapsusDBus();
		bool isValid();

		void signalFeatureChanged(const QString &id, const QString &val);
		void signalFeatureNotif(const QString &id, const QString &val);
		
		void sendACPIEvent(const QString &group, const QString &action,
				const QString &device, uint id, uint value);

	private:
		DBUSFeatureManager *_featManager;
		QDBusConnection _connection;
		bool _isValid;
		bool _timerSet;

		QPtrList< SignalToSend > signalsToSend;

		void doInit();
		bool safeSendSignal(const char *sigName,
				const QValueList<QDBusData>& params);
		bool sendSignal(const QString &sigName,
				const QValueList<QDBusData>& params);
		bool returnDBusError(const QString &str1, const QString &str2,
					const QDBusMessage& message);

	protected:
		virtual bool handleMethodCall(const QDBusMessage& message);

	protected slots:
		void sendPendingSignals();
};

#endif
