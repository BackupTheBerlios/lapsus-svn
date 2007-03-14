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
	_acpiFd(acpiFd), _backend(0), _dbus(0), _acpiParser(0),
	_isValid(false)
{
	doInit();
}

LapsusDaemon::~LapsusDaemon()
{
	if (_backend) delete _backend;
	if (_dbus) delete _dbus;
	if (_acpiParser) delete _acpiParser;

	_backend = 0;
	_dbus = 0;
	_acpiParser = 0;
}

bool LapsusDaemon::isValid()
{
	return _isValid;
}

bool LapsusDaemon::detectHardware()
{
	SysBackend *tmp;

	// Try Asus Backend
	tmp = new SysAsus();

	if (tmp->hardwareDetected())
	{
		_backend = tmp;
		return true;
	}

	delete tmp;

	// Try any other backends supported

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

		_backend = tmp;
		return true;
	}

	delete tmp;

	return false;
}

void LapsusDaemon::doInit()
{
	if (!detectHardware())
	{
		fprintf(stderr, "No supported hardware found. Is appropriate kernel module loaded?\n");
		return;
	}

	_dbus = new LapsusDBus(this);

	if (!_dbus->isValid())
	{
		return;
	}

	_acpiParser = new ACPIEventParser(_acpiFd);

	connect(_acpiParser,
		SIGNAL(acpiEvent(const QString &, const QString &,
					const QString &, uint, uint)),
		_dbus,
		SLOT(sendACPIEvent(const QString &, const QString &,
					const QString &, uint, uint)));

	_isValid = true;
}

QStringList LapsusDaemon::featureList()
{
	if (!_isValid) return QStringList();

	// TODO Add generic features - for example cpufreq control
	return _backend->featureList();
}

QString LapsusDaemon::featureName(const QString &id)
{
	if (!_isValid) return QString();

	// TODO Check other features - for example cpufreq control
	return _backend->featureName(id.lower());
}

QStringList LapsusDaemon::featureArgs(const QString &id)
{
	if (!_isValid) return QStringList();

	// TODO Add generic features - for example cpufreq control
	return _backend->featureArgs(id.lower());
}

QString LapsusDaemon::featureRead(const QString &id)
{
	if (!_isValid) return QString();

	// TODO Check other features
	return _backend->featureRead(id.lower());
}

bool LapsusDaemon::featureWrite(const QString &id, const QString &nVal)
{
	if (!_isValid) return false;

	// TODO Check other features
	return _backend->featureWrite(id.lower(), nVal.lower(), _dbus);
}

