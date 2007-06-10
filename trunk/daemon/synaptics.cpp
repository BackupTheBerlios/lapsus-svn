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

#include "synaptics.h"
#include "lapsus.h"

LapsusSynaptics::LapsusSynaptics(): LapsusModule("synaptics"),
	 _procGet(0), _procSet(0),
	_isValid(false), _isOn(false), _notifyChange(false)
{
	_isValid = runGetProc();
}

LapsusSynaptics::~LapsusSynaptics()
{
}

bool LapsusSynaptics::hardwareDetected()
{
	return _isValid;
}

bool LapsusSynaptics::setState(bool nState, bool hardwareTrig)
{
	if (hardwareTrig) _notifyChange = true;
	
	return runSetProc(nState);
}

bool LapsusSynaptics::getState()
{
	return _isOn;
}

bool LapsusSynaptics::toggleState(bool hardwareTrig)
{
	if (hardwareTrig) _notifyChange = true;
	
	return setState(!_isOn);
}

void LapsusSynaptics::readFromStdout()
{
	if (!_procGet) return;

	QByteArray arr = _procGet->readStdout();

	if (arr.size() > 0) _dataRead.append(arr);
}

void LapsusSynaptics::getProcessExited()
{
	if (!_procGet) return;

	QByteArray arr = _procGet->readStdout();

	if (arr.size() > 0) _dataRead.append(arr);

	delete _procGet;
	_procGet = 0;

	int idx = _dataRead.find("TouchpadOff");
	if (idx < 0) return;

	int idxEq = _dataRead.find('=', idx);
	if (idxEq < 0) return;

	int idxN = _dataRead.find('\n', idxEq);
	if (idxN < 0 || idxN - idxEq < 2) return;

	bool ok = false;

	// Reversed. If we read '0' it means that touchpad is ON.
	// '1' means it's OFF.
	bool nVal = !(_dataRead.mid(idxEq+1, idxN - idxEq - 1).stripWhiteSpace().toInt(&ok));

	if (!ok) return;

	if (nVal != _isOn)
	{
		_isOn = nVal;

		stateChanged(_isOn);
	}
}

void LapsusSynaptics::setProcessExited()
{
	if (!_procSet) return;

	delete _procSet;
	_procSet = 0;

	runGetProc();
}

bool LapsusSynaptics::runGetProc()
{
	if (_procGet) return true;

	_procGet = new QProcess( this );

	_procGet->addArgument( "synclient" );
	_procGet->addArgument( "-l" );

	_dataRead.truncate(0);

	connect( _procGet, SIGNAL(readyReadStdout()),
		this, SLOT(readFromStdout()) );

	connect( _procGet, SIGNAL(processExited()),
		this, SLOT(getProcessExited()));

	if (!_procGet->start())
	{
		delete _procGet;
		_procGet = 0;
		return false;
	}

	return true;
}

bool LapsusSynaptics::runSetProc(bool nState)
{
	if (_procSet) return true;

	_procSet = new QProcess( this );

	_procSet->addArgument( "synclient" );

	if (nState) _procSet->addArgument( "TouchpadOff=0" );
	else _procSet->addArgument( "TouchpadOff=1" );

	connect( _procSet, SIGNAL(processExited()),
		this, SLOT(setProcessExited()));

	if (!_procSet->start())
	{
		delete _procSet;
		_procSet = 0;
		return false;
	}

	return true;
}

QStringList LapsusSynaptics::featureList()
{
	QStringList ret;
	ret.append(LAPSUS_FEAT_TOUCHPAD_ID);
	return ret;
}

QStringList LapsusSynaptics::featureArgs(const QString &id)
{
	QStringList ret;

	if (id == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		ret.append(LAPSUS_FEAT_ON);
		ret.append(LAPSUS_FEAT_OFF);
	}

	return ret;
}

QString LapsusSynaptics::featureRead(const QString &id)
{
	if (id == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		if (getState()) return LAPSUS_FEAT_ON;

		return LAPSUS_FEAT_OFF;
	}


	return "";
}

bool LapsusSynaptics::featureWrite(const QString &id, const QString &nVal)
{
	if (id == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		setState(nVal == LAPSUS_FEAT_ON);

		return true;
	}

	return false;
}

void LapsusSynaptics::stateChanged(bool nState)
{
	dbusSignalFeatureChanged(LAPSUS_FEAT_TOUCHPAD_ID,
		nState?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);

	if (_notifyChange)
	{
		dbusSignalFeatureNotif(LAPSUS_FEAT_TOUCHPAD_ID,
			nState?LAPSUS_FEAT_ON:LAPSUS_FEAT_OFF);
	}

	_notifyChange = false;
}
