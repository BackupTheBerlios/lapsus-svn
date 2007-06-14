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

#include "lapsus.h"
#include "lapsus_switch.h"

#define LAPSUS_CONF_WIDGET_SWITCH		"switch"

LapsusSwitch::LapsusSwitch(KConfig *cfg, const QString &idConf, const char *idDBus):
	LapsusFeature(cfg, idConf, idDBus)
{
	if (!_isValid) return;

	if (!supportsArgs(getFeatureArgs()))
	{
		_isValid = false;
		return;
	}
	
	QString tmp = getFeatureValue();
	
	if (!isArgValid(tmp))
	{
		_isValid = false;
		return;
	}
	
	_val = tmp;
}

LapsusSwitch::~LapsusSwitch()
{
}

bool LapsusSwitch::supportsArgs(const QStringList & args)
{
	// At least two possible states
	if (args.size() > 1)
	{
		// None of the states is a range of values
		for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
		{
			if (QStringList::split(':', *it).size() > 1)
				return false;
		}

		return true;
	}

	return false;
}

void LapsusSwitch::dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif)
{
	if (id != _featDBusID) return;
	
	_val = val;
	
	if (isNotif)
	{
		emit featureUpdate(val);
	}
	else
	{
		emit featureUpdate(val);
	}
}

void LapsusSwitch::setSwitchValue(const QString &val)
{
	if (isArgValid(val)) setFeatureValue(val);
}

QString LapsusSwitch::getSwitchValue()
{
	return _val;
}

QStringList LapsusSwitch::getSwitchAllValues()
{
	// All possible arguments. Might be different in case of more
	// comples types.
	return getFeatureArgs();
}

bool LapsusSwitch::saveFeature()
{
	if (!_cfg || !_isValid) return false;
	
	addConfigEntry(_featConfID, _featDBusID, _cfg);
	return true;
}

void LapsusSwitch::addConfigEntry(const QString &confID, const QString &dbusID, KConfig *cfg)
{
	cfg->deleteGroup(confID);
	cfg->setGroup(confID);
	cfg->writeEntry(LAPSUS_CONF_WIDGET_TYPE, LAPSUS_CONF_WIDGET_SWITCH);
	cfg->writeEntry(LAPSUS_CONF_FEATURE_ID, dbusID);
}

const char* LapsusSwitch::featureType()
{
	return LAPSUS_CONF_WIDGET_SWITCH;
}
