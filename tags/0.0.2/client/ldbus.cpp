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

#include "ldbus.h"
#include "lapsus.h"

LapsusDBus::LapsusDBus()
{
	_isValid = false;
	_maxBacklight = 0;
	_hasBacklight = false;
	_hasSwitches = false;

	_proxy = 0;
	_conn = QDBusConnection::addConnection(QDBusConnection::SystemBus);

	if (_conn.isConnected())
	{
		_proxy = new QDBusProxy(_conn);
		_proxy->setService(LAPSUS_SERVICE_NAME);
		_proxy->setPath(LAPSUS_OBJECT_PATH);
		_proxy->setInterface(LAPSUS_INTERFACE);

		connect(_proxy, SIGNAL(dbusSignal(const QDBusMessage&)),
			this, SLOT(handleDBusSignal(const QDBusMessage&)));

		initParams();
	}
}

LapsusDBus::~LapsusDBus()
{
	if (_proxy)
	{
		delete _proxy;
	}
}

bool LapsusDBus::isValid()
{
	return _isValid;
}

bool LapsusDBus::hasBacklight()
{
	return _hasBacklight;
}

bool LapsusDBus::hasSwitches()
{
	return _hasSwitches;
}

void LapsusDBus::initParams()
{
	QValueList<QDBusData> params;
	QDBusMessage reply = _proxy->sendWithReply("listFeatures", params);

	if (reply.type() != QDBusMessage::ReplyMessage)
		return;

	if (reply.count() != 1 || reply[0].type() != QDBusData::List)
		return;

	bool ok = false;

	QDBusDataList list = reply[0].toList(&ok);

	if (!ok) return;

	ok = false;

	_features = list.toStringList(&ok);

	if (!ok) return;

	for (QStringList::iterator it = _features.begin(); it != _features.end(); ++it)
	{
		if ((*it) == "backlight")
			_hasBacklight = true;
		else if ((*it) == "switches")
			_hasSwitches = true;
	}

	if (_hasBacklight)
	{
		_hasBacklight = false;

		reply = _proxy->sendWithReply("getMaxBacklight", params);

		if (reply.type() == QDBusMessage::ReplyMessage)
		{
			if (reply.count() == 1 && reply[0].type() == QDBusData::UInt32)
			{
				_maxBacklight = reply[0].toUInt32();

				if (_maxBacklight > 0)
				{
					_hasBacklight = true;

					getBacklight(true);
				}
			}
		}
	}

	if (_hasSwitches)
	{
		_hasSwitches = false;

		reply = _proxy->sendWithReply("listSwitches", params);

		if (reply.type() == QDBusMessage::ReplyMessage)
		{
			if (reply.count() == 1 && reply[0].type() == QDBusData::List)
			{
				ok = false;

				list = reply[0].toList(&ok);

				if (ok)
				{
					ok = false;
					_switches = list.toStringList(&ok);

					if (_switches.count() > 0)
					{
						_hasSwitches = true;

						for (QStringList::iterator it = _switches.begin();
							it != _switches.end(); ++it)
						{
							getSwitch(*it, true);
						}
					}
				}


			}
		}
	}

	_isValid = _hasBacklight || _hasSwitches;
}

uint LapsusDBus::maxBacklight()
{
	return _maxBacklight;
}

uint LapsusDBus::getBacklight(bool force)
{
	if (!force && !_hasBacklight)
		return 0;

	if (force)
	{
		QValueList<QDBusData> params;
		QDBusMessage reply = _proxy->sendWithReply("getBacklight", params);

		if (reply.type() != QDBusMessage::ReplyMessage)
			return 0;

		if (reply.count() != 1 || reply[0].type() != QDBusData::UInt32)
			return 0;

		_curBacklight = reply[0].toUInt32();
	}

	return _curBacklight;
}

QStringList LapsusDBus::listSwitches()
{
	return _switches;
}

bool LapsusDBus::setBacklight(uint newVal, bool force)
{
	if (!force)
	{
		if (!_hasBacklight)
			return false;

		if (newVal == _curBacklight)
			return true;
	}

	QValueList<QDBusData> params;

	params.append(QDBusData::fromUInt32(newVal));

	QDBusMessage reply = _proxy->sendWithReply("setBacklight", params);

	if (reply.type() != QDBusMessage::ReplyMessage)
		return false;

	if (reply.count() != 1 || reply[0].type() != QDBusData::Bool)
		return false;

	return reply[0].toBool();
}

bool LapsusDBus::getSwitch(const QString &name, bool force)
{
	if (!force && !_hasSwitches)
		return false;

	if (force || !_switchVals.contains(name))
	{
		QValueList<QDBusData> params;

		params.append(QDBusData::fromString(name));

		QDBusMessage reply = _proxy->sendWithReply("getSwitch", params);

		if (reply.type() != QDBusMessage::ReplyMessage)
			return false;

		if (reply.count() != 1 || reply[0].type() != QDBusData::Bool)
			return false;

		bool val = reply[0].toBool();

		_switchVals.insert(name, val);

		return val;
	}

	return _switchVals[name];
}

bool LapsusDBus::setSwitch(const QString &name, bool newVal, bool force)
{
	if (!force)
	{
		if (!_hasSwitches)
			return false;

		if (_switchVals.contains(name) && _switchVals[name] == newVal)
			return true;
	}

	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(name));
	params.append(QDBusData::fromBool(newVal));

	QDBusMessage reply = _proxy->sendWithReply("setSwitch", params);

	if (reply.type() != QDBusMessage::ReplyMessage)
		return false;

	if (reply.count() != 1 || reply[0].type() != QDBusData::Bool)
		return false;

	return reply[0].toBool();
}


void LapsusDBus::handleDBusSignal(const QDBusMessage &message)
{
	if (message.interface() != LAPSUS_INTERFACE) return;

	if (message.type() != QDBusMessage::SignalMessage) return;

	if (message.member() == "switchChanged")
	{
		if (message.count() != 2
			|| message[0].type() != QDBusData::String
			|| message[1].type() != QDBusData::Bool)
		{
			return;
		}

		QString sName = message[0].toString();
		bool sVal = message[1].toBool();

		_switchVals.insert(sName, sVal);

		emit switchChanged(sName, sVal);

		return;
	}
	else if (message.member() == "backlightChanged")
	{
		if (message.count() != 1
			|| message[0].type() != QDBusData::UInt32)
		{
			return;
		}

		uint bVal = message[0].toUInt32();

		_curBacklight = bVal;

		emit backlightChanged(bVal);

		return;
	}
}
