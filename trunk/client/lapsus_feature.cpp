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
#include "lapsus_dbus.h"
#include "lapsus_feature.h"

#define LAPSUS_CONF_FEATURE_TYPE	"feature_type"
#define LAPSUS_CONF_PANEL_FEAT_PREFIX	"panel_"
#define LAPSUS_CONF_MENU_FEAT_PREFIX	"menu_"

LapsusFeature::LapsusFeature(KConfig *cfg, const QString &dbusID,
			LapsusFeature::Place where, const char *featureType):
	_cfg(cfg), _featDBusID(dbusID), _place(where), _featureType(featureType),
	_validator(0), _confValid(false), _dbusValid(false), _dbusActive(false),
	_blockSendSet(false)
{
	if (_cfg != 0 && _featureType != 0)
	{
		QString confID = getFeatureConfID();
	
		if (confID.length() > 0)
		{
			_cfg->setGroup(confID);
			QString fType = _cfg->readEntry(LAPSUS_CONF_FEATURE_TYPE);
			
			if (fType == _featureType) _confValid = true;
		}
	}
	
	if (_featDBusID.length() > 0)
	{
		if ((LapsusDBus::get()->getFeatureValue(_featDBusID)).length() < 1) return;
		if ((LapsusDBus::get()->getFeatureName(_featDBusID)).length() < 1) return;
		
		QStringList featArgs = LapsusDBus::get()->getFeatureArgs(_featDBusID);
		
		if (featArgs.count() < 1) return;
		
		_validator = new LapsusValidator(featArgs);
		
		_dbusValid = true;
	}
}

LapsusFeature::~LapsusFeature()
{
	if (_validator != 0)
	{
		delete _validator;
		_validator = 0;
	}
}

bool LapsusFeature::isArgValid(const QString &arg)
{
	if (!_validator) return false;
	
	return _validator->isValid(arg);
}

bool LapsusFeature::dbusConnect()
{
	if (!_dbusValid) return false;
	
	connect(LapsusDBus::get(),
		SIGNAL(dbusStateUpdate(bool)),
		this,
		SLOT(dbusStateUpdate(bool)) );
	
	connect(LapsusDBus::get(),
		SIGNAL(dbusFeatureUpdate(const QString &, const QString &, bool)),
		this,
		SLOT(dbusFeatureUpdate(const QString &, const QString &, bool)));
	
	_dbusActive = LapsusDBus::get()->isActive();
	
	return true;
}

void LapsusFeature::dbusStateUpdate(bool state)
{
	_dbusActive = state;
}

void LapsusFeature::dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif)
{
	if (id != _featDBusID) return;
	
	if (isNotif)
	{
		emit featureUpdate(val);
	}
	else
	{
		emit featureUpdate(val);
	}
}

void LapsusFeature::updateFeatureValue()
{
	if (!_dbusValid) return;
	
	_blockSendSet = true;
	
	dbusFeatureUpdate(_featDBusID,
		LapsusDBus::get()->getFeatureValue(_featDBusID),
		false);
	
	_blockSendSet = false;
}

bool LapsusFeature::setFeatureValue(const QString &nVal)
{
	if (!_dbusValid || _blockSendSet) return false;
	
	return LapsusDBus::get()->setFeature(_featDBusID, nVal);
}

QString LapsusFeature::getFeatureValue()
{
	if (!_dbusValid) return QString();
	
	return LapsusDBus::get()->getFeatureValue(_featDBusID);
}

QString LapsusFeature::getFeatureName()
{
	if (!_dbusValid) return QString();
	
	return LapsusDBus::get()->getFeatureName(_featDBusID);
}

QStringList LapsusFeature::getFeatureArgs()
{
	if (!_dbusValid) return QStringList();
	
	return LapsusDBus::get()->getFeatureArgs(_featDBusID);
}

QString LapsusFeature::getFeatureDBusID()
{
	return _featDBusID;
}

QString LapsusFeature::getFeatureConfID()
{
	if (_place == LapsusFeature::PlacePanel)
		return QString(LAPSUS_CONF_PANEL_FEAT_PREFIX "%1").arg(_featDBusID);
	else if (_place == LapsusFeature::PlaceMenu)
		return QString(LAPSUS_CONF_MENU_FEAT_PREFIX "%1").arg(_featDBusID);
	
	return QString();
}

bool LapsusFeature::confValid()
{
	return _confValid;
}

bool LapsusFeature::dbusValid()
{
	return _dbusValid;
}

bool LapsusFeature::dbusActive()
{
	return _dbusActive;
}

bool LapsusFeature::saveFeature()
{
	if (!_cfg) return false;
	
	// OR - if at least one of them is valid it is enough to save the feature.
	if (!_confValid && !_dbusValid) return false;
	
	QString featConf = getFeatureConfID();
	
	if (featConf.length() < 1 || _featDBusID.length() < 1) return false;
	
	_cfg->deleteGroup(featConf);
	_cfg->setGroup(featConf);
	_cfg->writeEntry(LAPSUS_CONF_FEATURE_TYPE, _featureType);
	
	return true;
}

LapsusPanelWidget* LapsusFeature::createPanelWidget(Qt::Orientation, QWidget *)
{
	return 0;
}

bool LapsusFeature::createActionButton(KActionCollection *)
{
	return false;
}
