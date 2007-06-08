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

#include "lapsus_init.h"
#include "lapsus.h"

#define LAPSUSD_INIT_GROUP			"/lapsusd/init"

LapsusInit::LapsusInit(QSettings *settingsFile, LapsusModulesList *modList):
	LapsusModule(LAPSUS_FEAT_INIT_PREFIX),
	_settings(settingsFile), _modList(modList)
{
	readEntries();
}

LapsusInit::~LapsusInit()
{
	saveEntries();
	
	_initVals.clear();
}

void LapsusInit::readEntries()
{
	if (!_settings) return;
	
	_initVals.clear();
	
	QStringList keys = _settings->entryList(LAPSUSD_INIT_GROUP);
	
	_settings->beginGroup(LAPSUSD_INIT_GROUP);
	
	for (QStringList::Iterator it = keys.begin(); it != keys.end(); ++it )
	{
		_initVals.insert(*it, _settings->readEntry(*it));
	}
	
	_settings->endGroup();
}

void LapsusInit::saveEntries()
{
	if (!_settings) return;
	
	_settings->resetGroup();
	
	QStringList keys = _settings->entryList(LAPSUSD_INIT_GROUP);
	
	_settings->beginGroup(LAPSUSD_INIT_GROUP);
	
	for (QStringList::Iterator it = keys.begin(); it != keys.end(); ++it )
	{
		_settings->removeEntry(*it);
	}
	
	QMap<QString, QString>::Iterator initIt;
	
	for (initIt = _initVals.begin(); initIt != _initVals.end(); ++initIt)
	{
		_settings->writeEntry(initIt.key(), initIt.data());
	}
	
	_settings->endGroup();
}

void LapsusInit::setInitValues()
{
	if (!_modList) return;
	
	QMap<QString, QString>::Iterator initIt;
	
	for (initIt = _initVals.begin(); initIt != _initVals.end(); ++initIt)
	{
		QString modId = initIt.key();
		LapsusModule *mod;
		
		if (_modList->findModule(&mod, modId) && mod)
		{
			/* Real write. */
			mod->featureWrite(modId, initIt.data());
		}
	}
}

QString LapsusInit::featureRead(const QString &id)
{
	return _initVals[id];
}

QString LapsusInit::featureName(const QString &id)
{
	if (_modList)
	{
		QString modId = id;
		LapsusModule *mod;
		
		if (_modList->findModule(&mod, modId) && mod)
		{
			return mod->featureName(modId);
		}
	}
	
	return QString();
}

bool LapsusInit::featureWrite(const QString &id, const QString &nVal, bool testWrite)
{
	if (nVal.length() < 1)
	{
		_initVals.remove(id);
		return true;
	}
	else
	{
		if (_modList)
		{
			QString modId = id;
			LapsusModule *mod;
			
			if (_modList->findModule(&mod, modId) && mod)
			{
				/* Test write. Some modules may respect test flag, some may not ;) */
				if (mod->featureWrite(modId, nVal, true))
				{
					_initVals[id] = nVal;
					return true;
				}
			}
		}
	}
	
	return false;
}

QStringList LapsusInit::featureArgs(const QString &id)
{
	if (_modList)
	{
		QString modId = id;
		LapsusModule *mod;
		
		if (_modList->findModule(&mod, modId) && mod)
		{
			return mod->featureArgs(modId);
		}
	}
	
	return QStringList();
}

QStringList LapsusInit::featureList()
{
	QStringList ret;
	
	QMap<QString, QString>::Iterator initIt;
	
	for (initIt = _initVals.begin(); initIt != _initVals.end(); ++initIt)
	{
		ret.append(initIt.key());
	}
	
	return ret;
}

bool LapsusInit::hardwareDetected()
{
	return true;
}
