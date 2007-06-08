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
#include "lapsus_daemon.h"

#include "sys_asus.h"
#include "sys_ibm.h"

LapsusDaemon::LapsusDaemon(uint acpiFd):
	_acpiFd(acpiFd), _dbus(0), _acpiParser(0),
	_init(0), _isValid(false)
{
	_modList = new LapsusModulesList();
	
	_settings = new QSettings();
	_settings->insertSearchPath(QSettings::Unix, LAPSUSD_LIB_PATH);

	doInit();
}

LapsusDaemon::~LapsusDaemon()
{
	if (_modList) delete _modList;
	_modList = 0;
	
	if (_dbus) delete _dbus;
	_dbus = 0;
	
	if (_acpiParser) delete _acpiParser;
	_acpiParser = 0;
	
	if (_settings) delete _settings;
	_settings = 0;
}

bool LapsusDaemon::isValid()
{
	return _isValid;
}

bool LapsusDaemon::detectHardware()
{
	if (!_modList) return false;
	
	int hardwareMods = 0;
	
	LapsusConfig *conf = new LapsusConfig(_settings);
	
	_modList->addConfig(conf);
	
	LapsusSynaptics *syn = new LapsusSynaptics();
	
	if (syn->hardwareDetected())
	{
		_modList->addSynaptics(syn);
		++hardwareMods;
	}
	else
	{
		delete syn;
	}
	
#ifdef HAVE_ALSA
	LapsusMixer *mix = new LapsusAlsaMixer();
	
	if (mix->hardwareDetected())
	{
		_modList->addMixer(mix);
		++hardwareMods;
	}
	else
	{
		delete mix;
	}
#endif
	
	LapsusModule *tmp;

	// Try Asus Backend
	tmp = new SysAsus(_modList);

	if (tmp->hardwareDetected())
	{
		_modList->addModule(tmp);
		++hardwareMods;
	}
	else
	{
		delete tmp;
	}

	// Try IBM Backend

	tmp = new SysIBM();

	if (tmp->hardwareDetected())
	{
		++hardwareMods;
		
		printf("Detected IBM hardware\nDetected features:\n\n");

		QStringList lines = tmp->featureList();

		for ( QStringList::ConstIterator it = lines.begin(); it != lines.end(); ++it )
		{
			QString line = (*it);

			printf("Feature ID: %s\n", line.ascii());
			printf("Feature Name: %s\n", tmp->featureName(line).ascii());

			QStringList args = tmp->featureArgs(line);

			for ( QStringList::ConstIterator iter = args.begin(); iter != args.end(); ++iter )
				printf("Feature Arg: %s\n", (*iter).ascii());

			printf("Feature Value: %s\n\n", tmp->featureRead(line).ascii());
		}

		_modList->addModule(tmp);
	}
	else
	{
		delete tmp;
	}

	if (hardwareMods < 1) return false;
	
	_init = new LapsusInit(_settings, _modList);
	
	_modList->addModule(_init);
	
	return true;
}

void LapsusDaemon::doInit()
{
	if (!detectHardware())
	{
		fprintf(stderr, "No supported hardware found. Is appropriate kernel module loaded?\n");
		return;
	}

	_dbus = new LapsusDBus(this);

	if (!_dbus->isValid()) return;

	LapsusModulesIterator it = _modList->modulesIterator();
	
	while (it.current() != 0)
	{
		it.current()->setDBus(_dbus);
		++it;
	}
	
	_acpiParser = new ACPIEventParser(_acpiFd);
	
	connect(_acpiParser,
		SIGNAL(acpiEvent(const QString &, const QString &,
					const QString &, uint, uint)),
		this,
		SLOT(acpiEvent(const QString &, const QString &,
					const QString &, uint, uint)));
	
	_isValid = true;
	
	if (_init)
	{
		_init->setInitValues();
	}
}

void LapsusDaemon::acpiEvent(const QString &group, const QString &action,
	const QString &device, uint id, uint value)
{
	LapsusModulesIterator it = _modList->modulesIterator();
	
	while (it.current() != 0)
	{
		if (it.current()->handleACPIEvent(group, action, device, id, value))
			return;
		++it;
	}
	
	if (_dbus) _dbus->sendACPIEvent(group, action, device, id, value);
}

QStringList LapsusDaemon::featureList()
{
	if (!_isValid) return QStringList();
	
	QStringList features;
	
	LapsusModulesIterator it = _modList->modulesIterator();
	
	while (it.current() != 0)
	{
		QStringList ft = it.current()->featureList();
		const char * prefix = it.current()->modulePrefix();
		
		for ( QStringList::Iterator fIt = ft.begin(); fIt != ft.end(); ++fIt )
		{
			features.append(QString("%1.%2").arg(prefix).arg(*fIt));
		}
		
		++it;
	}
	
	return features;
}

QString LapsusDaemon::featureName(const QString &id)
{
	LapsusModule *mod;
	QString modId = id.lower();
	
	if (_isValid && _modList->findModule(&mod, modId) && mod)
	{
		return mod->featureName(modId);
	}
	
	return QString();
}

QStringList LapsusDaemon::featureArgs(const QString &id)
{
	LapsusModule *mod;
	QString modId = id.lower();
	
	if (_isValid && _modList->findModule(&mod, modId) && mod)
	{
		return mod->featureArgs(modId);
	}

	return QStringList();
}

QString LapsusDaemon::featureRead(const QString &id)
{
	LapsusModule *mod;
	QString modId = id.lower();
	
	if (_isValid && _modList->findModule(&mod, modId) && mod)
	{
		return mod->featureRead(modId);
	}
	
	return QString();
}

bool LapsusDaemon::featureWrite(const QString &id, const QString &nVal)
{
	LapsusModule *mod;
	QString modId = id.lower();
	
	if (_isValid && _modList->findModule(&mod, modId) && mod)
	{
		return mod->featureWrite(modId, nVal.lower());
	}
	
	return false;
}
