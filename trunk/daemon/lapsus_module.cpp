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

#include "lapsus.h"
#include "lapsus_module.h"

LapsusModule::LapsusModule(const char *prefix) : _dbus(0), _modulePrefix(prefix)
{
}

LapsusModule::~LapsusModule()
{
}

const char * LapsusModule::modulePrefix()
{
	return _modulePrefix;
}

void LapsusModule::setDBus(LapsusDBus *dbus)
{
	_dbus = dbus;
}

bool LapsusModule::handleACPIEvent(const QString &, const QString &,
	const QString &, uint, uint)
{
	return false;
}

QString LapsusModule::featureName(const QString &id)
{
	if (id == LAPSUS_FEAT_BLUETOOTH_ID) return I18N_NOOP("Bluetooth adapter");
	if (id == LAPSUS_FEAT_WIRELESS_ID) return I18N_NOOP("Wireless radio");
	if (id == LAPSUS_FEAT_BACKLIGHT_ID) return I18N_NOOP("LCD Backlight");
	if (id == LAPSUS_FEAT_VOLUME_ID) return I18N_NOOP("Volume");
	if (id == LAPSUS_FEAT_TOUCHPAD_ID) return I18N_NOOP("Touchpad");
	if (id == LAPSUS_FEAT_VOLUME_ID) return I18N_NOOP("Volume");
	
	return "";
}

void LapsusModule::dbusSignalFeatureUpdate(const QString &id, const char *val)
{
	if (!_dbus) return;
	
	dbusSignalFeatureUpdate(id, QString(val));
}

void LapsusModule::dbusSignalFeatureUpdate(const QString &id, const QStringList &vList)
{
	if (!_dbus) return;
	
	dbusSignalFeatureUpdate(id, vList.join(","));
}

void LapsusModule::dbusSignalFeatureUpdate(const QString &id, const QString &val)
{
	if (!_dbus) return;
	
	_dbus->signalFeatureUpdate(QString("%1.%2").arg(_modulePrefix).arg(id), val);
}

void LapsusModule::dbusSignalFeatureNotif(const QString &id, const char *val)
{
	if (!_dbus) return;
	
	dbusSignalFeatureNotif(id, QString(val));
}

void LapsusModule::dbusSignalFeatureNotif(const QString &id, const QStringList &vList)
{
	if (!_dbus) return;
	
	dbusSignalFeatureNotif(id, vList.join(","));
}

void LapsusModule::dbusSignalFeatureNotif(const QString &id, const QString &val)
{
	if (!_dbus) return;
	
	_dbus->signalFeatureNotif(QString("%1.%2").arg(_modulePrefix).arg(id), val);
}
