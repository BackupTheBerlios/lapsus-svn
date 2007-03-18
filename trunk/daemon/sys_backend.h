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

#ifndef SYS_BACKEND_H
#define SYS_BACKEND_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

class SysBackend;

#include "lapsus_dbus.h"

#include "../config.h"

#ifdef HAVE_ALSA
#include "alsa_mixer.h"
#endif

// We don't need anything else from klocale.h
#define I18N_NOOP(x)			x

/**
 * Generic backend for dealing with /sys interface files.
 * Provides methods for reading and writing values from/to those
 * files and dealing with id:path mappings.
 */
class SysBackend: public QObject
{
	Q_OBJECT

	public:
		SysBackend();
		virtual ~SysBackend();

		virtual bool hardwareDetected() = 0;

		virtual QStringList featureList() = 0;
		virtual QStringList featureArgs(const QString &id) = 0;
		virtual QString featureRead(const QString &id) = 0;
		virtual bool featureWrite(const QString &id, const QString &nVal) = 0;
		virtual QStringList featureParams(const QString &id);
		virtual QString featureName(const QString &id);

		virtual void setDBus(LapsusDBus *dbus);

	protected slots:
		virtual void acpiEvent(const QString &group, const QString &action,
				const QString &device, uint id, uint value);

	protected:
		LapsusDBus *_dbus;

		bool hasFeature(const QString &id);
		QString getFeaturePath(const QString &id);
		QString getFeatureName(const QString &id);
		void setFeature(const QString &id, const QString &path, const QString &name = "");
		QStringList getFeatures();

		bool isDisplayFeature(const QString &id);
		bool isDisplayFeature(const QString &id, QString &disp);
		bool isLEDFeature(const QString &id);
		bool isLEDFeature(const QString &id, QString &led);

		QString readIdString(const QString &id);
		bool writeIdString(const QString &id, const QString &val);
		uint readIdUInt(const QString &id);
		bool writeIdUInt(const QString &id, uint newVal);

		static QString readPathString(const QString &path);
		static bool writePathString(const QString &path, const QString &val);
		static uint readPathUInt(const QString &path);
		static bool writePathUInt(const QString &path, uint newVal);
		static bool testR(const QString &path);

	private:
		QMap<QString, QString> _featurePaths;
		QMap<QString, QString> _featureNames;

		static uint correctBuf(char *buf, uint len);
};

#endif
