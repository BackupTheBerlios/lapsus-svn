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

#include <qdir.h>
#include <qfile.h>
#include <qstringlist.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sys_backend.h"

#define qPrintable(str)         (str.ascii())

SysBackend::SysBackend()
{
	switches = false;
	cpufreq = false;
	display = false;
	backlight = false;

	maxBacklight = 0;
	displayVal = 0;
	displayLastBit = 0;

	detect();
}

bool SysBackend::isValid()
{
	return (switches || cpufreq || display || backlight);
}

bool SysBackend::readSwitch(bool *ok, const QString &name)
{
	*ok = false;

	if (!switchPaths.contains(name)) return 0;

	uint val = readUint(ok, switchPaths[name]);

	return (val != 0);
}

uint SysBackend::readUint(bool *ok, const QString &path)
{
	*ok = false;

	int fd = open(qPrintable(path), O_RDONLY);

	if (fd < 0) return 0;

	char buf[21];
	int c = read(fd, buf, 20);

	close(fd);

	c = correctBuf(buf, c);

	if (c < 1) return 0;

	buf[c] = '\0';

	return QString(buf).toUInt(ok);
}

bool SysBackend::writeUint(const QString &path, uint newVal)
{
	int fd = open(qPrintable(path), O_WRONLY);

	if (fd < 0) return false;

	char buf[21];

	snprintf(buf, 20, "%u", newVal);

	buf[20] = '\0';

	int c = write(fd, buf, strlen(buf));

	close(fd);

	if (c < 1) return false;

	return true;
}

bool SysBackend::readWriteSwitch(const QString &name, bool *oldVal, bool *newVal)
{
	if (!switchPaths.contains(name)) return false;

	uint nV = 0, oV = 0;
	bool ret;

	if (*newVal)
		nV = 1;

	ret = readWriteUint(switchPaths[name], &oV, &nV);

	*oldVal = (oV != 0);
	*newVal = (nV != 0);

	return ret;
}

bool SysBackend::readWriteUint(const QString &path, uint *oldVal, uint *newVal)
{
	int fd = open(qPrintable(path), O_RDONLY);

	if (fd < 0) return false;

	char buf[21];
	int c = read(fd, buf, 20);
	close(fd);

	c = correctBuf(buf, c);

	if (c < 1) return false;

	buf[c] = '\0';

	bool ok;

	*oldVal = QString(buf).toUInt(&ok);

	if (!ok) return false;

	fd = open(qPrintable(path), O_WRONLY);

	if (fd < 0) return false;

	snprintf(buf, 20, "%u", *newVal);

	buf[20] = '\0';

	c = write(fd, buf, strlen(buf));
	close(fd);

	if (c < 1) return false;

	fd = open(qPrintable(path), O_RDONLY);

	c = read(fd, buf, 20);
	close(fd);

	c = correctBuf(buf, c);

	if (c < 1) return false;

	buf[c] = '\0';

	*newVal = QString(buf).toUInt(&ok);

	return ok;
}

bool SysBackend::setDisplay(const QString &name, bool value)
{
	if (!displayBits.contains(name)) return false;

	uint dBit = displayBits[name];

	uint oVal, nVal;

	nVal = oVal = displayVal;

	if (value)
	{
		nVal |= (1 << dBit);
	}
	else
	{
		nVal &= ~(1 << dBit);
	}

	if (!readWriteUint(displayPath, &oVal, &nVal))
		return false;

	/* Well... something has changed ;) */
	if (nVal != displayVal || oVal != nVal || oVal != displayVal)
	{
		displayVal = nVal;
		return true;
	}

	return false;
}

bool SysBackend::getDisplay(const QString &name)
{
	if (!displayBits.contains(name)) return false;

	uint dBit = displayBits[name];

	return ((displayVal & (1 << dBit)) != 0);
}

uint SysBackend::getBacklight()
{
	if (!backlight) return 0;

	bool res = false;
	uint backl = readUint(&res, backlightGetPath);

	if (!res) return 0;

	return backl;
}

bool SysBackend::changeBacklight(uint toVal, uint *oVal, uint *nVal)
{
	bool res = false;

	*oVal = *nVal = 0;

	*oVal = readUint(&res, backlightGetPath);

	if (toVal > maxBacklight)
		toVal = maxBacklight;

	if (!writeUint(backlightSetPath, toVal))
		return false;

	*nVal = readUint(&res, backlightGetPath);

	return (*nVal != *oVal);
}

bool SysBackend::testRW(const QString &path)
{
	int c, fd = open(qPrintable(path), O_RDONLY);
	char buf[21];

	if (fd < 0) return false;

	c = read(fd, buf, 20);
	close(fd);

	buf[c] = '\0';

	c = correctBuf(buf, c);

	if (c < 1) return false;

	fd = open(qPrintable(path), O_WRONLY);

	if (fd < 0) return false;

	c = write(fd, buf, c);

	close(fd);

	return (c > 0);
}

bool SysBackend::testR(const QString &path)
{
	int fd = open(qPrintable(path), O_RDONLY);
	char buf[21];

	if (fd < 0) return false;

	if (read(fd, buf, 20) > 0)
	{
		close(fd);
		return true;
	}

	close(fd);
	return false;
}

void SysBackend::detect()
{
	QDir *dir;
	QString path;

	dir = new QDir("/sys/class/leds");

	if (dir->exists())
	{
		dir->setSorting(QDir::Name);
		dir->setFilter(QDir::Dirs);

		QStringList list = dir->entryList("asus:*");

		for (unsigned int i = 0; i < list.size(); ++i)
		{
			QString fName = list[i];

			if (fName.length() > 5)
			{
				path = QString("/sys/class/leds/%1/brightness").arg(fName);

				if (QFile::exists(path) && fName.startsWith("asus:") && testR(path))
				{
					QString name = QString("%1 LED").arg(fName.mid(5));

					name[0] = name[0].upper();

					switchPaths.insert(name, path);
					switches = true;
				}
			}
		}
	}

	delete dir;

	path = "/sys/devices/platform/asus-laptop/bluetooth";
	if (QFile::exists(path) && testR(path))
	{
		switchPaths.insert("Bluetooth", path);
		switches = true;
	}

	path = "/sys/devices/platform/asus-laptop/wlan";
	if (QFile::exists(path) && testR(path))
	{
		switchPaths.insert("Wireless", path);
		switches = true;
	}

	path = "/sys/devices/platform/asus-laptop/display";
	if (QFile::exists(path) && testR(path))
	{
		displayBits.insert("LCD", 0);
		displayBits.insert("CRT", 1);
		displayBits.insert( "TV", 2);
		displayBits.insert("DVI", 3);

		bool res = false;
		displayVal = readUint(&res, path);

		if (res)
		{
			displayLastBit = 3;
			displayPath = path;
			display = true;
		}
	}

	path = "/sys/class/backlight/asus-laptop/actual_brightness";
	if (QFile::exists(path) && testR(path))
	{
		backlightGetPath = path;

		path = "/sys/class/backlight/asus-laptop/max_brightness";
		if (QFile::exists(path) && testR(path))
		{
			bool res = false;
			maxBacklight = readUint(&res, path);

			path = "/sys/class/backlight/asus-laptop/brightness";

			if (res && QFile::exists(path))
			{
				backlightSetPath = path;
				backlight = true;
			}
			else
			{
				maxBacklight = 0;
				backlightGetPath = QString();
			}
		}
	}

	// TODO - cpufreq
}

int SysBackend::correctBuf(char *buf, int len)
{
	int i;

	for (i = 0; i < len; ++i)
	{
		if (buf[i]<'0')
		{
			buf[i] = '\0';
			return i;
		}
	}

	return len;
}
