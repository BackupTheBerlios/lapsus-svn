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

#include <qwidget.h>
#include <qlabel.h>
#include <qtooltip.h>

#include "lapsus.h"
#include "lapsus_dbus.h"
#include "lapsus_slider.h"

LapsusFeature::LapsusFeature(KConfig *cfg, const QString &idConf, const char *idDBus):
		_cfg(cfg), _validator(0), _featConfID(idConf),
		_hasDBus(false), _isValid(false), _blockSendSet(false)
{
	if (_cfg)
	{
		_cfg->setGroup(_featConfID);
	}
	
	if (idDBus)
	{
		_featDBusID = QString(idDBus);
	}
	else if (_cfg && _cfg->hasKey(LAPSUS_CONF_FEATURE_ID))
	{
		_featDBusID = _cfg->readEntry(LAPSUS_CONF_FEATURE_ID);
	}
	
	if (_featDBusID.length() > 0)
	{
		_isValid = true;
		
		QString val = LapsusDBus::get()->getFeatureValue(_featDBusID);
		QString featName = LapsusDBus::get()->getFeatureName(_featDBusID);
		QStringList featArgs = LapsusDBus::get()->getFeatureArgs(_featDBusID);
		_validator = new LapsusValidator(featArgs);
		
		if (val.length() > 0 && featName.length() > 0 && featArgs.count() > 0)
		{
			_hasDBus = true;
			
			connect ( LapsusDBus::get(), SIGNAL(dbusStateUpdate(bool)),
				this, SLOT(dbusStateUpdate(bool)) );

			connect(LapsusDBus::get(),
				SIGNAL(dbusFeatureUpdate(const QString &, const QString &, bool)),
				this,
				SLOT(dbusFeatureUpdate(const QString &, const QString &, bool)));
		}
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
	if (!_isValid) return;
	
	_blockSendSet = true;
	
	dbusFeatureUpdate(_featDBusID,
		LapsusDBus::get()->getFeatureValue(_featDBusID),
		false);
	
	_blockSendSet = false;
}

bool LapsusFeature::setFeatureValue(const QString &nVal)
{
	if (!_isValid || !_hasDBus || _blockSendSet) return false;
	
	return LapsusDBus::get()->setFeature(_featDBusID, nVal);
}

QString LapsusFeature::getFeatureValue()
{
	if (!_isValid) return QString();
	
	return LapsusDBus::get()->getFeatureValue(_featDBusID);
}

QString LapsusFeature::getFeatureName()
{
	if (!_isValid) return QString();
	
	return LapsusDBus::get()->getFeatureName(_featDBusID);
}

QStringList LapsusFeature::getFeatureArgs()
{
	if (!_isValid) return QStringList();
	
	return LapsusDBus::get()->getFeatureArgs(_featDBusID);
}

QString LapsusFeature::getFeatureDBusID()
{
	if (!_isValid) return QString();
	
	return _featDBusID;
}

QString LapsusFeature::getFeatureConfID()
{
	if (!_isValid) return QString();
	
	return _featConfID;
}

bool LapsusFeature::isValid()
{
	return _isValid;
}

bool LapsusFeature::hasDBus()
{
	return _hasDBus;
}

bool LapsusFeature::saveFeature()
{
	if (!_cfg) return false;
	
	if (_featConfID.length() < 1 || _featDBusID.length() < 1) return false;
	
	_cfg->setGroup(_featConfID);
	_cfg->writeEntry(LAPSUS_CONF_FEATURE_ID, _featDBusID);
	
	return true;
}

void LapsusFeature::dbusStateUpdate(bool state)
{
	_hasDBus = state;
}

QString LapsusFeature::readFeatureType(const QString &confID, KConfig *cfg)
{
	if (!cfg) return QString();
	
	cfg->setGroup(confID);
	return cfg->readEntry(LAPSUS_CONF_WIDGET_TYPE);
}

QString LapsusFeature::readFeatureDBusID(const QString &confID, KConfig *cfg)
{
	if (!cfg) return QString();
	
	cfg->setGroup(confID);
	return cfg->readEntry(LAPSUS_CONF_FEATURE_ID);
}
