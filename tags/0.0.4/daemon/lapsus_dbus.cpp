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
#include "lapsus_dbus.h"

/*
 * Little hack - qt3 backport of dbus bindings doesn't like when the
 * signal is emited before it receives a response to the message it
 * is waiting for. Iw we receive a message from a client, send a response
 * send a signal, and then return from a method serving the original
 * message, the signal is sent _before_ the response. The client will
 * see the signal after some time - this is bad. So we use qtimer to
 * emit the signal "just after" current function call stack ends.
 */
#include <qtimer.h>

#define qPrintable(str)         (str.ascii())

LapsusDBus::LapsusDBus(LapsusDaemon *daemon):
	_daemon(daemon), _isValid(false),
	_processingReply(false), _timerSet(false)
{
	doInit();
}

bool LapsusDBus::isValid()
{
	return _isValid;
}

LapsusDBus::~LapsusDBus()
{
	if (_isValid)
	{
		_connection.unregisterObject(LAPSUS_OBJECT_PATH);
	}
}

void LapsusDBus::doInit()
{
	_connection = QDBusConnection::addConnection(QDBusConnection::SystemBus);

	if (!_connection.isConnected())
	{
		fprintf(stderr, "Cannot connect to the D-BUS system bus!\n");
		return;
	}

	if (!_connection.requestName(LAPSUS_SERVICE_NAME))
	{
		fprintf(stderr, "Error registering D-BUS Lapsus Service Name '%s': %s\n",
			LAPSUS_SERVICE_NAME, qPrintable(_connection.lastError().message()));
		return;
	}

	if (!_connection.registerObject(LAPSUS_OBJECT_PATH, this))
	{
		fprintf(stderr, "Error registering D-BUS Lapsus Object Path '%s': %s\n",
			LAPSUS_OBJECT_PATH, qPrintable(_connection.lastError().message()));
		return;
	}

	_isValid = true;

	return;
}

void LapsusDBus::signalFeatureChanged(const QString &id, const QString &val)
{
	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(id));
	params.append(QDBusData::fromString(val));

	safeSendSignal(LAPSUS_DBUS_FEATURE_CHANGED, params);
}

void LapsusDBus::signalFeatureNotif(const QString &id, const QString &val)
{
	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(id));
	params.append(QDBusData::fromString(val));

	safeSendSignal(LAPSUS_DBUS_FEATURE_NOTIF, params);
}

void LapsusDBus::sendACPIEvent(const QString &group, const QString &action,
				const QString &device, uint id, uint value)
{
	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(group));
	params.append(QDBusData::fromString(action));
	params.append(QDBusData::fromString(device));
	params.append(QDBusData::fromUInt32(id));
	params.append(QDBusData::fromUInt32(value));

	safeSendSignal(LAPSUS_DBUS_ACPI_EVENT, params);
}

bool LapsusDBus::safeSendSignal(const QString &sigName,
				const QValueList<QDBusData>& params)
{
	/*
	// When ALSA is used it is possible that the reply is no longer
	// beeing processed, but it is probably too close to sending
	// the reply and the signal is not received anyway.
	// So lets just always use timer to send the signals
	if (!_processingReply)
	{
		return sendSignal(sigName, params);
	}
	*/

	signalsToSend.push_back(sigName);
	signalValsToSend.push_back(params);

	if (!_timerSet)
	{
		_timerSet = true;
		QTimer::singleShot( 10, this, SLOT(sendPendingSignals()) );
	}

	return true;
}

void LapsusDBus::sendPendingSignals()
{
	while(signalsToSend.size() > 0 && signalValsToSend.size() > 0)
	{
		QString name = signalsToSend.first();
		signalsToSend.pop_front();

		QValueList<QDBusData> params = signalValsToSend.first();
		signalValsToSend.pop_front();

		sendSignal(name, params);
	}

	signalsToSend.clear();
	signalValsToSend.clear();

	_timerSet = false;
}

bool LapsusDBus::sendSignal(const QString &sigName,
				const QValueList<QDBusData>& params)
{
	QDBusMessage msg = QDBusMessage::signal(LAPSUS_OBJECT_PATH,
						LAPSUS_INTERFACE, sigName);

	msg += params;

	return _connection.send(msg);
}

bool LapsusDBus::returnDBusError(const QString &str1, const QString &str2,
					const QDBusMessage& message)
{
	if (!_isValid || !_daemon) return false;

	QDBusError error(str1, str2);
	QDBusMessage reply = QDBusMessage::methodError(message, error);

	_connection.send(reply);

	return true;
}

// QT3 DBus message handler:
bool LapsusDBus::handleMethodCall(const QDBusMessage& message)
{
	if (!_isValid || !_daemon) return false;
	if (message.interface() != LAPSUS_INTERFACE) return false;
	if (message.type() != QDBusMessage::MethodCallMessage) return false;

	if (message.member() == LAPSUS_DBUS_LIST_FEATURES)
	{
		if (message.count() != 0)
		{
			return returnDBusError("org.freedesktop.DBus.Error"
				".InvalidSignature", "Expected no arguments",
				message);
		}

		QDBusMessage reply = QDBusMessage::methodReply(message);

		_processingReply = true;

		reply << QDBusData::fromList(_daemon->featureList());

		_processingReply = false;

		_connection.send(reply);

		return true;
	}
	else if (message.member() == LAPSUS_DBUS_GET_FEATURE
		|| message.member() == LAPSUS_DBUS_GET_FEATURE_INFO)
	{
		if (message.count() != 1 || message[0].type() != QDBusData::String)
		{
			return returnDBusError("org.freedesktop.DBus.Error"
				".InvalidSignature",
				"Expected one string argument",
				message);
		}

		QDBusMessage reply = QDBusMessage::methodReply(message);

		_processingReply = true;

		if (message.member() == LAPSUS_DBUS_GET_FEATURE_INFO)
		{
			QString id = message[0].toString();

			reply << QDBusData::fromString(_daemon->featureName(id));
			reply << QDBusData::fromList(_daemon->featureArgs(id));
			reply << QDBusData::fromList(_daemon->featureParams(id));
		}
		else
		{
			reply << QDBusData::fromString(_daemon->featureRead(message[0].toString()));
		}

		_processingReply = false;

		_connection.send(reply);

		return true;
	}
	else if (message.member() == LAPSUS_DBUS_SET_FEATURE)
	{
		if (message.count() != 2
			|| message[0].type() != QDBusData::String
			|| message[1].type() != QDBusData::String)
		{
			return returnDBusError("org.freedesktop.DBus.Error"
				".InvalidSignature",
				"Expected two string arguments",
				message);
		}

		QDBusMessage reply = QDBusMessage::methodReply(message);

		_processingReply = true;

		reply << QDBusData::fromBool(
				_daemon->featureWrite(
					message[0].toString(),
					message[1].toString()));

		_processingReply = false;

		_connection.send(reply);

		return true;
	}

	// TODO - cpufreq, maybe something more

	return false;
}
