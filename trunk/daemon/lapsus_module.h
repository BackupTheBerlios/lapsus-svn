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
 ***************************************************************************/

#ifndef LAPSUS_MODULE_H
#define LAPSUS_MODULE_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

#include "lapsus_dbus.h"

// We don't need anything else from klocale.h
#define I18N_NOOP(x)			x

class LapsusModule: public QObject
{
	Q_OBJECT

	public:
		LapsusModule(const char *prefix);
		virtual ~LapsusModule();

		virtual bool hardwareDetected() = 0;

		virtual const char * modulePrefix();
		virtual QStringList featureList() = 0;
		virtual QStringList featureArgs(const QString &id) = 0;
		
		virtual QString featureRead(const QString &id) = 0;
		virtual bool featureWrite(const QString &id, const QString &nVal) = 0;
		
		virtual QString featureName(const QString &id);

		virtual void setDBus(LapsusDBus *dbus);
		
		virtual bool handleACPIEvent(const QString &group, const QString &action,
				const QString &device, uint id, uint value);

		virtual void dbusSignalFeatureUpdate(const QString &id, const char *val);
		virtual void dbusSignalFeatureUpdate(const QString &id, const QString &val);
		virtual void dbusSignalFeatureUpdate(const QString &id, const QStringList &vList);
		
		virtual void dbusSignalFeatureNotif(const QString &id, const char *val);
		virtual void dbusSignalFeatureNotif(const QString &id, const QString &val);
		virtual void dbusSignalFeatureNotif(const QString &id, const QStringList &vList);
		
	protected:
		LapsusDBus *_dbus;
		const char * _modulePrefix;
};

#endif
