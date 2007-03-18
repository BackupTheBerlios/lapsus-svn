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

#ifndef SYS_GENERIC_H
#define SYS_GENERIC_H

#include <qobject.h>
#include <qstringlist.h>
#include <qstring.h>

#include "sys_backend.h"

/**
 * Generic backend, which can be used if all other backends fail to detect
 * specific hardware. For now it only tries to use AlsaMixer.
 */
class SysGeneric : public SysBackend
{
	Q_OBJECT

	public:
		SysGeneric();
		~SysGeneric();

		QStringList featureList();
		QStringList featureArgs(const QString &id);
		QString featureRead(const QString &id);
		bool featureWrite(const QString &id, const QString &nVal);
		bool checkACPIEvent(const QString &group, const QString &action,
			const QString &device, uint id, uint value);

		bool hardwareDetected();

#ifdef HAVE_ALSA
	protected slots:
		void volumeChanged(int val);
		void muteChanged(bool muted);
#endif

	private:
#ifdef HAVE_ALSA
		bool _hasVolume;
		LapsusAlsaMixer *_mix;
#endif
		void detect();
};

#endif
