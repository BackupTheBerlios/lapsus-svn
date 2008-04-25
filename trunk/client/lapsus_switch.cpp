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
#include "lapsus_switch.h"
#include "panel_button.h"
#include "action_button.h"

LapsusSwitch::LapsusSwitch(KConfig *cfg, const QString &dbusID,
			LapsusFeature::Place where, const char *featureType):
	LapsusFeature(cfg, dbusID, where, featureType)
{
	if (_dbusValid)
	{
		QStringList args = getFeatureArgs();
		
		// At least two possible states
		if (args.size() < 2)
		{
			_dbusValid = false;
			return;
		}
		
		// None of the states is a range of values
		for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
		{
			if (QStringList::split(':', *it).size() > 1)
			{
				_dbusValid = false;
				return;
			}
		}
		
		QString tmp = getFeatureValue();
	
		if (!isArgValid(tmp))
		{
			_dbusValid = false;
			return;
		}
		
		_val = tmp;
	}
}

LapsusSwitch::~LapsusSwitch()
{
}

void LapsusSwitch::dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif)
{
	if (id != _featDBusID) return;
	
	_val = val;
	
	LapsusFeature::dbusFeatureUpdate(id, val, isNotif);
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

LapsusPanelWidget* LapsusSwitch::createPanelWidget(Qt::Orientation orientation, QWidget *parent)
{
	return new LapsusPanelButton(orientation, parent, this);
}

bool LapsusSwitch::createActionButton(KActionCollection *parent)
{
	new LapsusActionButton(getFeatureConfID(), parent, this);
	
	return true;
}

bool LapsusSwitch::saveFeature()
{
	if (!LapsusFeature::saveFeature()) return false;
	
	// TODO - here all options specific for Switch Feature should be saved.
	
	return true;
}
