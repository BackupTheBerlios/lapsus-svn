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

#ifndef SYS_ASUS_H
#define SYS_ASUS_H

#include <qstringlist.h>
#include <qstring.h>

#include "sys_backend.h"

/**
 * Backend, which controls asus-laptop's kernel module /sys interface
 * files and "knows" what features can be supported.
 */
class SysAsus : public SysBackend
{
	public:
		SysAsus();
		~SysAsus();

		QStringList featureList();
		QString featureName(const QString &id);
		QStringList featureArgs(const QString &id);
		QString featureRead(const QString &id);
		bool featureWrite(const QString &id, const QString &nVal, LapsusDBus *dbus);

		bool hardwareDetected();
		QString featurePrefix();

	private:
		bool _hasSwitches;
		bool _hasBacklight;
		bool _hasDisplay;
		uint maxBacklight;

		void detect();
		bool displayFeature(const QString &id);
		bool displayFeature(const QString &id, QString &disp);
};

#endif
