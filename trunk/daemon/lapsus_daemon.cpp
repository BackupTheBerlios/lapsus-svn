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
	_isValid(false)
{
	doInit();
}

LapsusDaemon::~LapsusDaemon()
{
#warning clean modules

	if (_dbus) delete _dbus;
	if (_acpiParser) delete _acpiParser;

	_dbus = 0;
	_acpiParser = 0;
}

bool LapsusDaemon::isValid()
{
	return _isValid;
}

void LapsusDaemon::addModule(LapsusModule *mod)
{
	if (!mod) return;
	
	modules.append(mod);
	prefixes.insert(mod->modulePrefix(), mod);
}

bool LapsusDaemon::detectHardware()
{
	LapsusMixer *mix = 0;
	LapsusSynaptics *syn = 0;
	
	syn = new LapsusSynaptics();
	
	if (syn->hardwareDetected())
	{
		addModule(syn);
	}
	else
	{
		delete syn;
		syn = 0;
	}
	
#ifdef HAVE_ALSA
	mix = new LapsusAlsaMixer();
	
	if (mix->hardwareDetected())
	{
		addModule(mix);
	}
	else
	{
		delete mix;
		mix = 0;
	}
#endif
	
	LapsusModule *tmp;

	// Try Asus Backend
	tmp = new SysAsus(mix, syn);

	if (tmp->hardwareDetected())
	{
		addModule(tmp);
	}
	else
	{
		delete tmp;
	}

	// Try IBM Backend

	tmp = new SysIBM();

	if (tmp->hardwareDetected())
	{
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

		addModule(tmp);
	}
	else
	{
		delete tmp;
	}

	return (modules.count() > 0);
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

	LapsusModule *mod;
	
	for (mod = modules.first(); mod; mod = modules.next())
	{
		mod->setDBus(_dbus);
	}
	
	_acpiParser = new ACPIEventParser(_acpiFd);
	
	connect(_acpiParser,
		SIGNAL(acpiEvent(const QString &, const QString &,
					const QString &, uint, uint)),
		this,
		SLOT(acpiEvent(const QString &, const QString &,
					const QString &, uint, uint)));
	
	_isValid = true;
}

void LapsusDaemon::acpiEvent(const QString &group, const QString &action,
	const QString &device, uint id, uint value)
{
	LapsusModule *mod;
	
	for (mod = modules.first(); mod; mod = modules.next())
	{
		if (mod->handleACPIEvent(group, action, device, id, value))
			return;
	}
	
	if (_dbus) _dbus->sendACPIEvent(group, action, device, id, value);
}

QStringList LapsusDaemon::featureList()
{
	if (!_isValid) return QStringList();
	
	QStringList features;
	LapsusModule *mod;
	
	for (mod = modules.first(); mod; mod = modules.next())
	{
		QStringList ft = mod->featureList();
		const char * prefix = mod->modulePrefix();
		
		for ( QStringList::Iterator it = ft.begin(); it != ft.end(); ++it )
		{
			features.append(QString("%1.%2").arg(prefix).arg(*it));
		}
	}
	
	return features;
}

bool LapsusDaemon::findModule(LapsusModule **mod, QString &id)
{
	int idx = id.find('.');
	
	if (idx < 1) return false;
	
	QString pref = id.left(idx);
	
	if ( (*mod = prefixes.find(pref.ascii())) == 0) return false;
	
	id = id.mid(idx+1);
	
	if (id.length() < 1) return false;
	
	return true;
}

QString LapsusDaemon::featureName(const QString &id)
{
	LapsusModule *mod;
	QString modId = id.lower();
	
	if (_isValid && findModule(&mod, modId) && mod)
	{
		return mod->featureName(modId);
	}
	
	return QString();
	
}

QStringList LapsusDaemon::featureArgs(const QString &id)
{
	LapsusModule *mod;
	QString modId = id.lower();
	
	if (_isValid && findModule(&mod, modId) && mod)
	{
		return mod->featureArgs(modId);
	}

	return QStringList();
}

QString LapsusDaemon::featureRead(const QString &id)
{
	LapsusModule *mod;
	QString modId = id.lower();
	
	if (_isValid && findModule(&mod, modId) && mod)
	{
		return mod->featureRead(modId);
	}
	
	return QString();
}

bool LapsusDaemon::featureWrite(const QString &id, const QString &nVal)
{
	LapsusModule *mod;
	QString modId = id.lower();
	
	if (_isValid && findModule(&mod, modId) && mod)
	{
		return mod->featureWrite(modId, nVal.lower());
	}
	
	return false;
}
