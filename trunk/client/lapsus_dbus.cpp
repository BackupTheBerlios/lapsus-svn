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

#include <klocale.h>

#include "lapsus.h"
#include "lapsus_dbus.h"

#define TIMER_INTERVAL				2000

LapsusDBus* LapsusDBus::globalDBusObject = 0;
int LapsusDBus::dbusRefs = 0;

LapsusDBus::LapsusDBus() : _proxy(0), _isValid(false), _timerId(0)
{
	printf("DBus constructed\n");
	
	restartDBus();

	if (!_isValid)
	{
		connError();
	}
}

LapsusDBus::~LapsusDBus()
{
	printf("DBus destructed\n");
	
	if (_proxy)
	{
		delete _proxy;
	}
}

LapsusDBus* LapsusDBus::get()
{
	if (!globalDBusObject)
	{
		create();
	}
	
	return globalDBusObject;
}

void LapsusDBus::create()
{
	if (!globalDBusObject)
	{
		globalDBusObject = new LapsusDBus();
	}
	
	++dbusRefs;
}

void LapsusDBus::remove()
{
	--dbusRefs;
	
	if (dbusRefs < 1 && globalDBusObject)
	{
		delete globalDBusObject;
		globalDBusObject = 0;
	}
}

bool LapsusDBus::isValid()
{
	return _isValid;
}

bool LapsusDBus::restartDBus()
{
	if (!_conn.isConnected())
	{
		// TODO - Check why this doesn't work if dbus system is restarted...
		// Probably QDBusConnection doesn't let for that.
		// Is that a problem if it does not go up after dbus restart?
		// Does other KDE application are able to reconnect after dbus restart?

		_conn = QDBusConnection::addConnection(QDBusConnection::SystemBus);
	}

	if (_conn.isConnected())
	{
		_proxy = new QDBusProxy(_conn);
		_proxy->setService(LAPSUS_SERVICE_NAME);
		_proxy->setPath(LAPSUS_OBJECT_PATH);
		_proxy->setInterface(LAPSUS_INTERFACE);

		connect(_proxy, SIGNAL(dbusSignal(const QDBusMessage&)),
			this, SLOT(handleDBusSignal(const QDBusMessage&)));

		initParams();

		if (_isValid) return true;
	}

	return false;
}

void LapsusDBus::connError()
{
	if (_timerId) return;

	_timerId = startTimer( TIMER_INTERVAL );

	if (_timerId)
	{
		if (_isValid)
		{
			// We want _isValid to be false when the signal is emited,
			// but we want the signal to be emited only when _isValid
			// was true
			_isValid = false;
			emit stateChanged(false);
		}
		else
		{
			_isValid = false;
		}
	}
}

void LapsusDBus::timerEvent( QTimerEvent * e)
{
	if (_timerId && e->timerId() == _timerId)
	{
		if (restartDBus())
		{
			killTimer(_timerId);
			_timerId = 0;

			emit stateChanged(true);
		}
	}
}

void LapsusDBus::initParams()
{
	QValueList<QDBusData> params;
	QDBusMessage reply = _proxy->sendWithReply(LAPSUS_DBUS_LIST_FEATURES, params);

	_features.clear();
	_featureArgs.clear();
	_featureName.clear();

	if (reply.type() != QDBusMessage::ReplyMessage)
		return;

	if (reply.count() != 1 || reply[0].type() != QDBusData::List)
		return;

	bool ok = false;

	QDBusDataList list = reply[0].toList(&ok);

	if (!ok) return;

	ok = false;

	QStringList tmpFeatures = list.toStringList(&ok);

	if (!ok) return;

	QStringList argList, paramList;

	for (QStringList::iterator it = tmpFeatures.begin(); it != tmpFeatures.end(); ++it)
	{
		QString id = *it;

		params.clear();

		params.append(QDBusData::fromString(id));

		reply = _proxy->sendWithReply(LAPSUS_DBUS_GET_FEATURE_INFO, params);

		if (reply.type() == QDBusMessage::ReplyMessage)
		{
			if (reply.count() == 2
				&& reply[0].type() == QDBusData::String
				&& reply[1].type() == QDBusData::List )
			{
				ok = false;

				list = reply[1].toList(&ok);

				if (!ok) continue;

				ok = false;

				argList = list.toStringList(&ok);

				if (!ok) continue;

				_features.append(id);
				_featureArgs.insert(id, argList);
				_featureName.insert(id, reply[0].toString());

				getFeature(id);
			}
		}
	}

	_isValid = true;
}

QStringList LapsusDBus::listFeatures()
{
	if (!_isValid) return QStringList();

	return _features;
}

QString LapsusDBus::getFeatureName(const QString &id)
{
	if (!_isValid) return QString();

	QString str = _featureName[id.lower()];

	if (str.length() > 0) return i18n(str);

	return str;
}

QStringList LapsusDBus::getFeatureArgs(const QString &id)
{
	if (!_isValid) return QStringList();

	return _featureArgs[id.lower()];
}

QString LapsusDBus::getFeature(const QString &id)
{
	if (!_isValid) return QString();

	QString lId = id.lower();

	if (!_features.contains(lId))
		return QString();

	if (!_featureVal.contains(lId))
	{
		QValueList<QDBusData> params;

		params.append(QDBusData::fromString(lId));

		QDBusMessage reply = _proxy->sendWithReply(LAPSUS_DBUS_GET_FEATURE, params);

		if (reply.type() != QDBusMessage::ReplyMessage)
			return QString();

		if (reply.count() != 1 || reply[0].type() != QDBusData::String)
			return QString();

		QString str = reply[0].toString();

		_featureVal.insert(lId, str);

		return str;
	}

	return _featureVal[lId];
}

void LapsusDBus::checkFeature(const QString &id)
{
	if (!_isValid) return;

	QString lId = id.lower();

	if (!_features.contains(lId))
		return;

	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(lId));

	QDBusMessage reply = _proxy->sendWithReply(LAPSUS_DBUS_GET_FEATURE, params);

	if (reply.type() != QDBusMessage::ReplyMessage)
		return;

	if (reply.count() != 1 || reply[0].type() != QDBusData::String)
		return;

	QString str = reply[0].toString();

	if (!_featureVal.contains(lId) || _featureVal[lId] != str)
	{
		_featureVal.insert(lId, str);
		emit featureChanged(lId, str);
	}
}

bool LapsusDBus::setFeature(const QString &id, const QString &val)
{
	if (!_isValid) return false;

	QString lId = id.lower();

	if (!_features.contains(lId))
		return false;

	if (_featureVal.contains(lId) && _featureVal[lId] == val)
		return true;

	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(lId));
	params.append(QDBusData::fromString(val));

	QDBusMessage reply = _proxy->sendWithReply(LAPSUS_DBUS_SET_FEATURE, params);

	if (reply.type() != QDBusMessage::ReplyMessage)
	{
		if (_conn.isConnected())
		{
			QDBusError err = _proxy->lastError();
			QDBusError::ErrorType type = err.type();

			// TODO Check other types of errors that may happen.
			// Which of them should lead to dbus restart?
			switch (type)
			{
				case (QDBusError::ServiceUnknown):
					connError();
				break;

				default:
				break;
			}
		}
		else
		{
			connError();
		}

		return false;
	}

	if (reply.count() != 1 || reply[0].type() != QDBusData::Bool)
		return false;

	bool res = reply[0].toBool();

	// If res = false it is possible that the value written
	// was already set - check this.
	if (!res) checkFeature(id);

	return res;
}

void LapsusDBus::handleDBusSignal(const QDBusMessage &message)
{
	if (message.interface() != LAPSUS_INTERFACE) return;

	if (message.type() != QDBusMessage::SignalMessage) return;

	if (message.member() == LAPSUS_DBUS_FEATURE_CHANGED
		|| message.member() == LAPSUS_DBUS_FEATURE_NOTIF)
	{
		if (message.count() != 2
			|| message[0].type() != QDBusData::String
			|| message[1].type() != QDBusData::String)
		{
			return;
		}

		QString sName = message[0].toString();
		QString sVal = message[1].toString();

		if (message.member() == LAPSUS_DBUS_FEATURE_CHANGED)
		{
			_featureVal.insert(sName, sVal);
			emit featureChanged(sName, sVal);
		}
		else
		{
			emit featureNotif(sName, sVal);
		}

		return;
	}

	// TODO - ACPI Events
}
