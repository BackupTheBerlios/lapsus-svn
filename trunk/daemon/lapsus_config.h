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

#ifndef LAPSUS_CONFIG_H
#define LAPSUS_CONFIG_H

#include <qsettings.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qdict.h>

#include "lapsus_module.h"

class LapsusConfigEntry
{
	public:
		LapsusConfigEntry()
		{
			entryRead = false;
			entrySubscribed = false;
		}
		
		~LapsusConfigEntry()
		{
		}
		
		QString id;
		QStringList args;
		QString defValue;
		QString curValue;
		bool entryRead;
		bool entrySubscribed;
};

class LapsusConfig : public LapsusModule
{
	Q_OBJECT

	public:
		LapsusConfig(QSettings *settings);
		virtual ~LapsusConfig();
		
		void subscribeEntry(const char *prefix, const QString &id, const QStringList &args, const QString &defValue);
		QString getEntryValue(const char *prefix, const QString &id);
		
		bool hardwareDetected();
		QStringList featureList();
		QStringList featureArgs(const QString &id);
		
		QString featureRead(const QString &id);
		bool featureWrite(const QString &id, const QString &nVal, bool testWrite = false);
		
	private:
		QSettings *_settings;
		QPtrList<LapsusConfigEntry> _entries;
		QDict<LapsusConfigEntry> _ids;
		
		void readEntries();
		void saveEntries();
};

#endif
