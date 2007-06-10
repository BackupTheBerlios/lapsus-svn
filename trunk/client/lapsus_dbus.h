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
#include <qmap.h>

// Qt DBUS includes
#include <dbus/qdbusdatalist.h>
#include <dbus/qdbuserror.h>
#include <dbus/qdbusmessage.h>
#include <dbus/qdbusconnection.h>
#include <dbus/qdbusobject.h>
#include <dbus/qdbusproxy.h>

class LapsusDBus : public QObject
{
	Q_OBJECT
	public:
		LapsusDBus();
		~LapsusDBus();

		bool isValid();

		QStringList listFeatures();
		QString getFeatureName(const QString &id);
		QStringList getFeatureArgs(const QString &id);
		QString getFeature(const QString &id);

		static LapsusDBus* get();
		static void create();
		static void remove();
		
	protected:
		void timerEvent( QTimerEvent * );

	private:
		QDBusConnection _conn;
		QDBusProxy *_proxy;
		bool _isValid;
		QStringList _features;
		QMap<QString, QString> _featureVal;
		QMap<QString, QString> _featureName;
		QMap<QString, QStringList> _featureArgs;
		int _timerId;
		
		static int dbusRefs;
		static LapsusDBus* globalDBusObject;
		
		void connError();
		bool restartDBus();
		void initParams();
		void checkFeature(const QString &id);

	public slots:
		void handleDBusSignal(const QDBusMessage &message);
		bool setFeature(const QString &id, const QString &val);

	signals:
		void featureChanged(const QString &id, const QString &val);
		void featureNotif(const QString &id, const QString &val);
		void stateChanged(bool state);
};

#endif
