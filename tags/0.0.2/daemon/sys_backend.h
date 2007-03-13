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

#include <qstring.h>
#include <qmap.h>

class SysBackend
{
	public:
		bool switches;
		bool cpufreq;
		bool display;
		bool backlight;

		QMap<QString, QString> switchPaths;
		QMap<QString, uint> displayBits;
		QString backlightSetPath;
		QString backlightGetPath;
		QString displayPath;
		uint maxBacklight;
		uint displayVal;
		uint displayLastBit;

		SysBackend();

		bool isValid();

		bool readSwitch(bool *ok, const QString &name);
		uint readUint(bool *ok, const QString &path);
		bool readWriteSwitch(const QString &name, bool *oldVal, bool *newVal);
		bool readWriteUint(const QString &path, uint *oldVal, uint *newVal);
		bool writeUint(const QString &path, uint newVal);

		bool setDisplay(const QString &name, bool value);
		bool getDisplay(const QString &name);

		uint getBacklight();
		bool changeBacklight(uint toVal, uint *oVal, uint *nVal);

		bool testR(const QString &path);
		bool testRW(const QString &path);
		void detect();

	private:
		int correctBuf(char *buf, int len);
};

#endif
