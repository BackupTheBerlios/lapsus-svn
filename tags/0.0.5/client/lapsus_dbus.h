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
#include <qdict.h>

// Qt DBUS includes
#include <dbus/qdbusdatalist.h>
#include <dbus/qdbuserror.h>
#include <dbus/qdbusmessage.h>
#include <dbus/qdbusconnection.h>
#include <dbus/qdbusobject.h>
#include <dbus/qdbusproxy.h>

class DBusFeature
{
	public:
		
		DBusFeature(const QString &fName, const QStringList &fArgList);
		~DBusFeature();
		
		QString name;
		QString value;
		QStringList argList;
		bool blockSet;
};

class LapsusDBus : public QObject
{
	Q_OBJECT
	public:
		LapsusDBus();
		~LapsusDBus();

		bool isActive();

		QStringList listFeatures();
		QString getFeatureName(const QString &id);
		QStringList getFeatureArgs(const QString &id);
		QString getFeatureValue(const QString &id);
		bool updateFeatureInfo(const QString &id);
		bool updateFeatureValue(const QString &id);

		static LapsusDBus* get();
		static void create();
		static void remove();
		
	signals:
		void dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif);
		void dbusStateUpdate(bool state);
	
	public slots:
		bool setFeature(const QString &id, const QString &val);
	
	protected:
		void timerEvent( QTimerEvent * );

	protected slots:
		void handleDBusSignal(const QDBusMessage &message);
	
	private:
		QDBusConnection _connDBus;
		QDBusProxy* _proxyDBus;
		QStringList _featureList;
		QDict<DBusFeature> _features;
		int _timerId;
		bool _isActive;
		
		static int dbusRefs;
		static LapsusDBus* globalDBusObject;
		
		void connError();
		bool restartDBus();
		void initParams();

		DBusFeature *getDBusFeature(const QString &id);
};

#endif
