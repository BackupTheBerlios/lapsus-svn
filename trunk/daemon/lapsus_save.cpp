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

#include "lapsus_save.h"
#include "lapsus_validator.h"
#include "lapsus.h"

#define LAPSUSD_SAVE_GROUP			"/lapsusd/save"
#define LAPSUSD_SAVE_LIST			"list"

LapsusSave::LapsusSave(QSettings *settingsFile, LapsusModulesList *modList):
	LapsusModule(LAPSUS_FEAT_SAVE_PREFIX),
	_settings(settingsFile), _modList(modList)
{
	readConfig();
}

LapsusSave::~LapsusSave()
{
}

void LapsusSave::readConfig()
{
	if (!_settings) return;
	
	_savedFeatures.clear();
	
	_settings->beginGroup(LAPSUSD_SAVE_GROUP);
	
	_savedFeatures = _settings->readListEntry(LAPSUSD_SAVE_LIST);

	_settings->endGroup();
}

void LapsusSave::saveValues()
{
	if (!_settings) return;
	
	_settings->resetGroup();
	_settings->beginGroup(LAPSUSD_SAVE_GROUP);
	
	_settings->removeEntry(LAPSUSD_SAVE_LIST);
	
	if (!_modList)
	{
		_settings->endGroup();
		return;
	}
	
	QStringList features;
	
	for (QStringList::Iterator it = _savedFeatures.begin(); it != _savedFeatures.end(); ++it )
	{
		QString modId = *it;
		LapsusModule *mod;
		
		if (_modList->findModule(&mod, modId) && mod)
		{
			QString val = mod->featureRead(modId);
			
			if (val.length() > 0)
			{
				_settings->writeEntry(*it, val);
				features.append(*it);
			}
		}
	}
	
	_settings->writeEntry(LAPSUSD_SAVE_LIST, features);
	
	_settings->endGroup();
}

void LapsusSave::setSavedValues()
{
	if (!_modList || !_settings) return;
	
	_settings->resetGroup();
	_settings->beginGroup(LAPSUSD_SAVE_GROUP);
	
	for (QStringList::Iterator it = _savedFeatures.begin(); it != _savedFeatures.end(); ++it )
	{
		QString modId = *it;
		LapsusModule *mod;
		
		if (_modList->findModule(&mod, modId) && mod)
		{
			mod->featureWrite(modId, _settings->readEntry(*it));
		}
	}
	
	_settings->endGroup();
}

QString LapsusSave::featureRead(const QString &id)
{
	if (_savedFeatures.contains(id))
		return LAPSUS_FEAT_ON;
	
	return LAPSUS_FEAT_OFF;
}

QString LapsusSave::featureName(const QString &id)
{
	return QString("Autosave value of %1").arg(id);
}

bool LapsusSave::featureWrite(const QString &id, const QString &nVal)
{
	if (!_modList) return false;
	
	if (nVal == LAPSUS_FEAT_OFF)
	{
		if (_savedFeatures.contains(id))
		{
			_savedFeatures.remove(id);
			dbusSignalFeatureUpdate(id, LAPSUS_FEAT_OFF);
		}
		
		return true;
	}
	else if (nVal == LAPSUS_FEAT_ON)
	{
		QString modId = id;
		LapsusModule *mod;
		
		if (_modList->findModule(&mod, modId) && mod && mod->featureArgs(modId).count()>0)
		{
			if (!_savedFeatures.contains(id))
			{
				_savedFeatures.append(id);
				dbusSignalFeatureUpdate(id, LAPSUS_FEAT_ON);
			}
		
			return true;
		}
	}
	
	return false;
}

QStringList LapsusSave::featureArgs(const QString &)
{
	QStringList list;
	
	list.append(LAPSUS_FEAT_ON);
	list.append(LAPSUS_FEAT_OFF);
	
	return list;
}

QStringList LapsusSave::featureList()
{
	return _savedFeatures;
}

bool LapsusSave::hardwareDetected()
{
	return true;
}
