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
		bool hasBacklight();
		bool hasSwitches();

		uint maxBacklight();
		uint getBacklight(bool force = false);

		bool getSwitch(const QString &name, bool force = false);

		QStringList listSwitches();

	private:
		QDBusConnection _conn;
		QDBusProxy *_proxy;
		bool _isValid;
		QStringList _features;
		QStringList _switches;
		uint _maxBacklight;
		uint _curBacklight;
		bool _hasBacklight;
		bool _hasSwitches;
		QMap<QString, bool> _switchVals;

		void initParams();


	public slots:
		void handleDBusSignal(const QDBusMessage &message);
		bool setSwitch(const QString &name, bool newVal, bool force = false);
		bool setBacklight(uint newVal, bool force = false);

	signals:
		void switchChanged(const QString &name, bool newVal);
		void backlightChanged(uint newVal);
};

#endif
