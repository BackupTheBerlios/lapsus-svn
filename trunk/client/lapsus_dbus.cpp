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

DBusFeature::DBusFeature(const QString &fName, const QStringList &fArgList):
	name(fName), argList(fArgList), blockSet(false)
{
}

DBusFeature::~DBusFeature()
{
}

LapsusDBus::LapsusDBus(): _proxyDBus(0), _timerId(0), _isValid(false)
{
	_features.setAutoDelete(true);
	
	restartDBus();

	if (!_isValid)
	{
		connError();
	}
}

LapsusDBus::~LapsusDBus()
{
	if (_proxyDBus)
	{
		delete _proxyDBus;
		_proxyDBus = 0;
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
	if (!_connDBus.isConnected())
	{
		// TODO - Check why this doesn't work if dbus system is restarted...
		// Probably QDBusConnection doesn't let for that.
		// Is that a problem if it does not go up after dbus restart?
		// Does other KDE application are able to reconnect after dbus restart?

		_connDBus = QDBusConnection::addConnection(QDBusConnection::SystemBus);
	}

	if (_connDBus.isConnected())
	{
		if (_proxyDBus) delete _proxyDBus;
		
		_proxyDBus = new QDBusProxy(_connDBus);
		_proxyDBus->setService(LAPSUS_SERVICE_NAME);
		_proxyDBus->setPath(LAPSUS_OBJECT_PATH);
		_proxyDBus->setInterface(LAPSUS_INTERFACE);

		connect(_proxyDBus, SIGNAL(dbusSignal(const QDBusMessage&)),
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
		// We want the signal to be emited only when _isValid is true,
		// but during the signal it should be already false
		if (_isValid)
		{
			_isValid = false;
			
			emit dbusStateUpdate(false);
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

			emit dbusStateUpdate(true);
		}
	}
}

bool LapsusDBus::updateFeatureInfo(const QString &fId)
{
	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(fId));

	QDBusMessage reply = _proxyDBus->sendWithReply(LAPSUS_DBUS_GET_FEATURE_INFO, params);

	if (reply.type() == QDBusMessage::ReplyMessage
		&& reply.count() == 2
		&& reply[0].type() == QDBusData::String
		&& reply[1].type() == QDBusData::List )
	{
		bool ok = false;

		QDBusDataList list = reply[1].toList(&ok);

		if (!ok) return false;

		ok = false;

		QStringList argList = list.toStringList(&ok);

		if (!ok) return false;

		_featureList.remove(fId);
		_featureList.append(fId);
		_features.replace(fId, new DBusFeature(reply[0].toString(), argList));
		
		return true;
	}
	
	return false;
}

void LapsusDBus::initParams()
{
	QValueList<QDBusData> params;
	QDBusMessage reply = _proxyDBus->sendWithReply(LAPSUS_DBUS_LIST_FEATURES, params);

	_featureList.clear();
	_features.clear();

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

	_isValid = true;

	for (QStringList::iterator it = tmpFeatures.begin(); it != tmpFeatures.end(); ++it)
	{
		if (updateFeatureInfo(*it))
			updateFeatureValue(*it);
	}
}

QStringList LapsusDBus::listFeatures()
{
	if (!_isValid) return QStringList();

	return _featureList;
}

DBusFeature* LapsusDBus::getDBusFeature(const QString &id)
{
	if (!_isValid) return 0;
	
	return _features.find(id.lower());
}

QString LapsusDBus::getFeatureName(const QString &id)
{
	DBusFeature *feat = getDBusFeature(id);
	
	if (!feat || feat->name.length() < 1) return QString();
	
	return i18n(feat->name);
}

QStringList LapsusDBus::getFeatureArgs(const QString &id)
{
	DBusFeature *feat = getDBusFeature(id);
	
	if (!feat || feat->name.length() < 1) return QStringList();
	
	return feat->argList;
}

QString LapsusDBus::getFeatureValue(const QString &id)
{
	DBusFeature *feat = getDBusFeature(id);
	
	if (!feat) return QString();
	
	return feat->value;
}

bool LapsusDBus::updateFeatureValue(const QString &id)
{
	if (!_isValid) return false;

	QString lId = id.lower();

	DBusFeature *feat = getDBusFeature(lId);

	if (!feat) return false;
	
	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(lId));

	QDBusMessage reply = _proxyDBus->sendWithReply(LAPSUS_DBUS_GET_FEATURE, params);

	if (reply.type() != QDBusMessage::ReplyMessage)
		return false;

	if (reply.count() != 1 || reply[0].type() != QDBusData::String)
		return false;

	QString str = reply[0].toString();

	if (feat->value != str)
	{
		feat->value = str;
		feat->blockSet = true;
		
		emit dbusFeatureUpdate(lId, str, false);
		
		feat->blockSet = false;
	}
	
	return true;
}

void LapsusDBus::handleDBusSignal(const QDBusMessage &message)
{
	if (message.interface() != LAPSUS_INTERFACE) return;

	if (message.type() != QDBusMessage::SignalMessage) return;

	if (message.member() == LAPSUS_DBUS_FEATURE_UPDATE
		|| message.member() == LAPSUS_DBUS_FEATURE_NOTIF)
	{
		if (message.count() != 2
			|| message[0].type() != QDBusData::String
			|| message[1].type() != QDBusData::String)
		{
			return;
		}

		QString lId = message[0].toString().lower();
		QString sVal = message[1].toString();
		
		DBusFeature *feat = getDBusFeature(lId);
		
		if (!feat)
		{
			updateFeatureInfo(lId);
			feat = getDBusFeature(lId);
			
			if (!feat) return;
		}
		
		if (message.member() == LAPSUS_DBUS_FEATURE_UPDATE)
		{
			feat->value = sVal;
			feat->blockSet = true;
			
			emit dbusFeatureUpdate(lId, sVal, false);
			
			feat->blockSet = false;
		}
		else
		{
			emit dbusFeatureUpdate(lId, sVal, true);
		}

		return;
	}

	// TODO - ACPI Events
}

bool LapsusDBus::setFeature(const QString &id, const QString &val)
{
	if (!_isValid) return false;

	QString lId = id.lower();

	DBusFeature *feat = getDBusFeature(lId);

	if (!feat || feat->blockSet) return false;

	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(lId));
	params.append(QDBusData::fromString(val));

	QDBusMessage reply = _proxyDBus->sendWithReply(LAPSUS_DBUS_SET_FEATURE, params);

	if (reply.type() != QDBusMessage::ReplyMessage)
	{
		if (_connDBus.isConnected())
		{
			QDBusError err = _proxyDBus->lastError();
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
	if (!res) updateFeatureValue(id);

	return res;
}
