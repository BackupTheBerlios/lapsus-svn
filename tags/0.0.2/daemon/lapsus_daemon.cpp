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

LapsusDaemon::LapsusDaemon()
{
	myConnection = 0;
}

LapsusDaemon::~LapsusDaemon()
{
	if (myConnection)
	{
		myConnection->unregisterObject(LAPSUS_OBJECT_PATH);
	}
}

bool LapsusDaemon::registerObject(QDBusConnection *conn)
{
	if (conn->registerObject(LAPSUS_OBJECT_PATH, this))
	{
		myConnection = conn;

		return true;
	}

	return false;
}

void LapsusDaemon::addACPIParser(ACPIEventParser *parser)
{
	connect(parser,
		SIGNAL(acpiEvent(const QString &, const QString &,
					const QString &, uint, uint)),
		this,
		SLOT(ACPIEvent(const QString &, const QString &,
					const QString &, uint, uint)));
}

bool LapsusDaemon::isValid()
{
	return sysBack.isValid();
}

uint LapsusDaemon::getBacklight()
{
	if (!sysBack.backlight) return 0;

	return sysBack.getBacklight();
}

bool LapsusDaemon::getSwitch(const QString &name)
{
	if (!sysBack.switches) return false;

	bool ok;
	bool val = sysBack.readSwitch(&ok, name);

	return (ok && val);
}

bool LapsusDaemon::getCpufreqParams(QString &governor, QStringList &freqSteps,
	QStringList &governors, QString &min, QString &max)
{
	if (!sysBack.cpufreq) return false;

	governor = QString();
	freqSteps = QStringList();
	governors = QStringList();
	min = QString();
	max = QString();

	// TODO
	return false;
}

bool LapsusDaemon::getDisplay(const QString &name)
{
	if (!sysBack.display) return false;

	return sysBack.getDisplay(name);
}

uint LapsusDaemon::getMaxBacklight()
{
	if (!sysBack.backlight) return 0;

	return sysBack.maxBacklight;
}

QStringList LapsusDaemon::listSwitches()
{
	if (!sysBack.switches) return QStringList();

	return sysBack.switchPaths.keys();
}

QStringList LapsusDaemon::listDisplayTypes()
{
	if (!sysBack.display) return QStringList();

	return sysBack.displayBits.keys();
}

QStringList LapsusDaemon::listFeatures()
{
	QStringList list;

	if (sysBack.backlight)
		list.append("backlight");

	if (sysBack.switches)
		list.append("switches");

	if (sysBack.cpufreq)
		list.append("cpufreq");

	if (sysBack.display)
		list.append("display");

	return list;
}

bool LapsusDaemon::setBacklight(uint value)
{
	if (!sysBack.backlight) return false;

	uint oVal = 0;
	uint nVal = 0;

	if (sysBack.changeBacklight(value, &oVal, &nVal))
	{
		backlightToEmit = nVal;
		QTimer::singleShot( 10, this, SLOT(emitBacklight()) );

		return true;
	}

	return false;
}

void LapsusDaemon::emitBacklight()
{
	QValueList<QDBusData> params;

	params.append(QDBusData::fromUInt32(backlightToEmit));

	sendSignal("backlightChanged", params);
}

bool LapsusDaemon::setSwitch(const QString &name, bool value)
{
	if (!sysBack.switches) return false;

	bool oldVal = false;
	bool newVal = value;

	bool ok = sysBack.readWriteSwitch(name, &oldVal, &newVal);

	if (!ok) return false;

	if (oldVal != newVal)
	{
		switchesToEmit.push_back(name);
		QTimer::singleShot( 10, this, SLOT(emitSwitch()) );

		// QT4: emit switchChanged(name, newVal);
	}

	return (newVal == value);
}

void LapsusDaemon::emitSwitch()
{
	while(switchesToEmit.size() > 0)
	{
		QValueList<QDBusData> params;
		QString name = switchesToEmit.first();
		switchesToEmit.pop_front();

		params.append(QDBusData::fromString(name));
		params.append(QDBusData::fromBool(getSwitch(name)));

		sendSignal("switchChanged", params);
	}
}

bool LapsusDaemon::setCpufreqParams(const QString &governor,
				const QString &min, const QString &max)
{
	if (!sysBack.cpufreq) return false;

	QString a = governor;
	a = min;
	a = max;

	// TODO
	return false;
}

bool LapsusDaemon::setDisplay(const QString &name, bool value)
{
	if (!sysBack.display) return false;

	QMap<QString, bool> disp;
	QStringList list = sysBack.displayBits.keys();

	for (unsigned int i = 0; i < list.size(); ++i)
	{
		QString dN = list[i];

		disp.insert(dN, sysBack.getDisplay(dN));
	}

	bool ret = sysBack.setDisplay(name, value);

	if (ret)
	{
		for (unsigned int i = 0; i < list.size(); ++i)
		{
			QString dN = list[i];
			bool nVal = sysBack.getDisplay(dN);

			if (nVal != disp[dN])
			{
				displaysToEmit.push_back(dN);
				QTimer::singleShot( 10, this, SLOT(emitDisplay()) );

				// QT4: emit displayChanged(dN, nVal);
			}
		}
	}

	return ret;
}

void LapsusDaemon::emitDisplay()
{
	while(displaysToEmit.size() > 0)
	{
		QValueList<QDBusData> params;
		QString name = displaysToEmit.first();
		displaysToEmit.pop_front();

		params.append(QDBusData::fromString(name));
		params.append(QDBusData::fromBool(getDisplay(name)));

		sendSignal("displayChanged", params);
	}
}

void LapsusDaemon::ACPIEvent(const QString &group, const QString &action,
				const QString &device, uint id, uint value)
{
	QValueList<QDBusData> params;

	params.append(QDBusData::fromString(group));
	params.append(QDBusData::fromString(action));
	params.append(QDBusData::fromString(device));
	params.append(QDBusData::fromUInt32(id));
	params.append(QDBusData::fromUInt32(value));

	sendSignal("ACPIEvent", params);
}

bool LapsusDaemon::sendSignal(const QString &sigName,
				const QValueList<QDBusData>& params)
{
	QDBusMessage msg = QDBusMessage::signal(LAPSUS_OBJECT_PATH,
						LAPSUS_INTERFACE, sigName);

	msg += params;

	return myConnection->send(msg);
}

bool LapsusDaemon::sendReply(const QDBusMessage& orgMessage,
				const QValueList<QDBusData>& params)
{
	QDBusMessage reply = QDBusMessage::methodReply(orgMessage);

	reply << QDBusData::fromList(QDBusDataList(params));

	return myConnection->send(reply);
}

bool LapsusDaemon::returnDBusError(const QString &str1, const QString &str2,
					const QDBusMessage& message)
{
	QDBusError error(str1, str2);
	QDBusMessage reply = QDBusMessage::methodError(message, error);

	myConnection->send(reply);

	return true;
}

// QT3 DBus message handler:
bool LapsusDaemon::handleMethodCall(const QDBusMessage& message)
{
	if (message.interface() != LAPSUS_INTERFACE) return false;

	if (message.type() != QDBusMessage::MethodCallMessage) return false;

	if (message.member() == "listFeatures"
		|| message.member() == "listSwitches"
		|| message.member() == "listDisplayTypes"
		|| message.member() == "getMaxBacklight"
		|| message.member() == "getBacklight")
	{
		if (message.count() != 0)
		{
			return returnDBusError("org.freedesktop.DBus.Error"
				".InvalidSignature", "Expected no arguments",
				message);
		}

		QDBusMessage reply = QDBusMessage::methodReply(message);

		if (message.member() == "listFeatures")
		{
			reply << QDBusData::fromList(listFeatures());
		}
		else if (message.member() == "listSwitches")
		{
			reply << QDBusData::fromList(listSwitches());
		}
		else if (message.member() == "listDisplayTypes")
		{
			reply << QDBusData::fromList(listDisplayTypes());
		}
		else if (message.member() == "getMaxBacklight")
		{
			reply << QDBusData::fromUInt32(getMaxBacklight());
		}
		else if (message.member() == "getBacklight")
		{
			reply << QDBusData::fromUInt32(getBacklight());
		}
		else
		{
			// Should not happen...
			// TODO - some kind of error? to syslog? using dbus?
			reply << QDBusData::fromUInt32(0);
		}

		myConnection->send(reply);

		return true;
	}
	else if (message.member() == "getSwitch"
		|| message.member() == "getDisplay")
	{
		if (message.count() != 1 || message[0].type() != QDBusData::String)
		{
			return returnDBusError("org.freedesktop.DBus.Error"
				".InvalidSignature",
				"Expected one string argument",
				message);
		}

		QDBusMessage reply = QDBusMessage::methodReply(message);

		if (message.member() == "getSwitch")
		{
			reply << QDBusData::fromBool(
					getSwitch(message[0].toString()));
		}
		else if (message.member() == "getDisplay")
		{
			reply << QDBusData::fromBool(
					getDisplay(message[0].toString()));
		}
		else
		{
			// Should not happen...
			// TODO - some kind of error? to syslog? using dbus?
			reply << QDBusData::fromBool(false);
		}

		myConnection->send(reply);

		return true;
	}
	else if (message.member() == "setSwitch"
		|| message.member() == "setDisplay")
	{
		if (message.count() != 2
			|| message[0].type() != QDBusData::String
			|| message[1].type() != QDBusData::Bool)
		{
			return returnDBusError("org.freedesktop.DBus.Error"
				".InvalidSignature",
				"Expected two arguments: string and bool",
				message);
		}

		QDBusMessage reply = QDBusMessage::methodReply(message);

		if (message.member() == "setSwitch")
		{
			reply << QDBusData::fromBool(
					setSwitch(message[0].toString(),
						message[1].toBool()));
		}
		else if (message.member() == "setDisplay")
		{
			reply << QDBusData::fromBool(
					setDisplay(message[0].toString(),
						message[1].toBool()));
		}
		else
		{
			// Should not happen...
			// TODO - some kind of error? to syslog? using dbus?
			reply << QDBusData::fromBool(false);
		}

		myConnection->send(reply);

		return true;
	}
	else if (message.member() == "setBacklight")
	{
		if (message.count() != 1
			|| message[0].type() != QDBusData::UInt32)
		{
			return returnDBusError("org.freedesktop.DBus.Error"
				".InvalidSignature",
				"Expected one uint32 argument",
				message);
		}

		QDBusMessage reply = QDBusMessage::methodReply(message);

		if (message.member() == "setBacklight")
		{
			reply << QDBusData::fromBool(
					setBacklight(message[0].toUInt32()));
		}
		else
		{
			// Should not happen...
			// TODO - some kind of error? to syslog? using dbus?
			reply << QDBusData::fromBool(false);
		}

		myConnection->send(reply);

		return true;
	}

	// TODO - cpufreq

	return false;
}
