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

#ifndef LAPSUS_INIT_H
#define LAPSUS_INIT_H

#include <qsettings.h>
#include <qstring.h>

#include "lapsus_module.h"
#include "modules_list.h"

class LapsusInit : public LapsusModule
{
	Q_OBJECT

	public:
		LapsusInit(QSettings *settings, LapsusModulesList *modList);
		virtual ~LapsusInit();
		
		void setInitValues();
		
		bool hardwareDetected();
		QStringList featureList();
		QString featureName(const QString &id);
		QStringList featureArgs(const QString &id);
		
		QString featureRead(const QString &id);
		bool featureWrite(const QString &id, const QString &nVal);
		
	private:
		QSettings *_settings;
		LapsusModulesList *_modList;
		QMap<QString, QString> _initVals;
		
		void readEntries();
		void saveEntries();
};

#endif
