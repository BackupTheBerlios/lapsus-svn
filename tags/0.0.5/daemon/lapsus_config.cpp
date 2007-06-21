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

#include "lapsus_config.h"
#include "lapsus_validator.h"
#include "lapsus.h"

#define LAPSUSD_CONFIG_GROUP			"/lapsusd/config"

LapsusConfig::LapsusConfig(QSettings *settingsFile):
	LapsusModule(LAPSUS_FEAT_CONFIG_PREFIX),
	_settings(settingsFile)
{
	readEntries();
}

LapsusConfig::~LapsusConfig()
{
	saveEntries();
	
	LapsusConfigEntry *ent;
	
	for (ent = _entries.first(); ent; ent = _entries.next())
	{
		delete ent;
	}
	
	_entries.clear();
	_ids.clear();
}

void LapsusConfig::readEntries()
{
	if (!_settings) return;
	
	QStringList keys = _settings->entryList(LAPSUSD_CONFIG_GROUP);
	
	_settings->beginGroup(LAPSUSD_CONFIG_GROUP);
	
	for (QStringList::Iterator it = keys.begin(); it != keys.end(); ++it )
	{
		LapsusConfigEntry *ent = new LapsusConfigEntry();
		
		ent->id = *it;
		ent->curValue = _settings->readEntry(ent->id);
		ent->entryRead = true;
		
		_entries.append(ent);
		_ids.replace(ent->id, ent);
	}
	
	_settings->endGroup();
}

void LapsusConfig::saveEntries()
{
	if (!_settings) return;
	
	_settings->resetGroup();
	
	QStringList keys = _settings->entryList(LAPSUSD_CONFIG_GROUP);
	
	_settings->beginGroup(LAPSUSD_CONFIG_GROUP);
	
	for (QStringList::Iterator it = keys.begin(); it != keys.end(); ++it )
	{
		_settings->removeEntry(*it);
	}
	
	LapsusConfigEntry *ent;
	
	for (ent = _entries.first(); ent; ent = _entries.next())
	{
		/*
		 * We save all entries that were not subscribed. Maybe some
		 * devices are not present so the appropriate backend was not loaded
		 * and couldn't subscribe its settings. We still want to preserve
		 * all custom settings of such backend.
		 *
		 * We also save all entries with current Value != default Value.
		 */
		if (!ent->entrySubscribed || ent->curValue != ent->defValue)
		{
			_settings->writeEntry(ent->id, ent->curValue);
		}
	}
	
	_settings->endGroup();
}

void LapsusConfig::subscribeEntry(const char *prefix, const QString &id, const char *name,
			const QStringList &args, const QString &defValue)
{
	QString str = QString("%1.%2").arg(prefix).arg(id);
	
	LapsusConfigEntry *ent = _ids.find(str);
	
	if (ent == 0)
	{
		ent = new LapsusConfigEntry();
		
		ent->id = str;
		ent->curValue = defValue;
		
		_entries.append(ent);
		_ids.replace(str, ent);
	}
	
	ent->entrySubscribed = true;
	
	/* Those are not saved in configuration file */
	ent->name = name;
	ent->args = args;
	ent->defValue = defValue;
}

QString LapsusConfig::getEntryValue(const char *prefix, const QString &id)
{
	QString str = QString("%1.%2").arg(prefix).arg(id);
	
	LapsusConfigEntry *ent = _ids.find(str);
	
	if (ent == 0) return QString();
	
	return ent->curValue;
}

QString LapsusConfig::featureRead(const QString &id)
{
	LapsusConfigEntry *ent = _ids.find(id);
	
	if (ent == 0) return QString();
	
	return ent->curValue;
}

QString LapsusConfig::featureName(const QString &id)
{
	LapsusConfigEntry *ent = _ids.find(id);
	
	if (ent == 0) return QString();
	
	return ent->name;
}

bool LapsusConfig::featureWrite(const QString &id, const QString &nVal)
{
	LapsusConfigEntry *ent = _ids.find(id);
	
	if (ent == 0) return false;
	
	if (nVal.length() == 0)
	{
		if (ent->defValue.length() < 1)
		{
			dbusSignalFeatureUpdate(id, nVal);
			
			_entries.removeRef(ent);
			_ids.remove(id);
			
			return true;
		}
		else if (ent->curValue != ent->defValue)
		{
			dbusSignalFeatureUpdate(id, ent->defValue);
			ent->curValue = ent->defValue;
			return true;
		}
		
		return false;
	}
	
	if (ent->args.count() < 1)
	{
		return false;
	}
	
	if (!LapsusValidator(ent->args).isValid(nVal))
	{
		return false;
	}
	
	if (ent->curValue != nVal)
	{
		dbusSignalFeatureUpdate(id, nVal);
		ent->curValue = nVal;
		return true;
	}
	
	return false;
}

QStringList LapsusConfig::featureArgs(const QString &id)
{
	LapsusConfigEntry *ent = _ids.find(id);
	
	if (ent == 0) return QStringList();
	
	return ent->args;
}

QStringList LapsusConfig::featureList()
{
	QStringList ret;
	
	LapsusConfigEntry *ent;
	
	for (ent = _entries.first(); ent; ent = _entries.next())
	{
		if (ent->entrySubscribed) ret.append(ent->id);
	}
	
	return ret;
}

bool LapsusConfig::hardwareDetected()
{
	return true;
}
