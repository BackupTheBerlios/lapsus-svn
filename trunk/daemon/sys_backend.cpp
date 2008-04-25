/***************************************************************************
 *   Copyright (C) 2007, 2008 by Jakub Schmidtke                           *
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

#include "lapsus.h"
#include "sys_backend.h"

#define qPrintable(str)         (str.ascii())

SysBackend::SysBackend(const char *prefix): LapsusModule(prefix)
{
}

SysBackend::~SysBackend()
{
}

bool SysBackend::hasFeature(const QString &id)
{
	return _featurePaths.contains(id);
}

QString SysBackend::getFeaturePath(const QString &id)
{
	return _featurePaths[id];
}

QString SysBackend::getFeatureName(const QString &id)
{
	return _featureNames[id];
}

void SysBackend::setFeature(const QString &id, const QString &path, const QString &name)
{
	_featurePaths.insert(id, path);
	_featureNames.insert(id, name);
}

QString SysBackend::featureName(const QString &id)
{
	if (hasFeature(id))
	{
		QString name = getFeatureName(id);

		if (name.length() > 0)
			return name;
	}

	QString disp;

#if 0
	// Known Display names - only for translation purposes.
	I18N_NOOP("LCD Display");
	I18N_NOOP("CRT Display");
	I18N_NOOP("TV Display");
	I18N_NOOP("DVI Display");
#endif
	if (isDisplayFeature(id, disp))
	{
		return QString("%1 Display").arg(disp.upper());
	}

	return LapsusModule::featureName(id);
}

bool SysBackend::isDisplayFeature(const QString &id)
{
	if (id.length() > strlen(LAPSUS_FEAT_DISPLAY_ID_PREFIX)
		&& id.startsWith(LAPSUS_FEAT_DISPLAY_ID_PREFIX))
	{
		return true;
	}

	return false;
}

bool SysBackend::isDisplayFeature(const QString &id, QString &disp)
{
	if (!isDisplayFeature(id))
		return false;

	disp = id.mid(strlen(LAPSUS_FEAT_DISPLAY_ID_PREFIX));

	return true;
}

bool SysBackend::isLEDFeature(const QString &id)
{
	if (id.length() > strlen(LAPSUS_FEAT_LED_ID_PREFIX)
		&& id.startsWith(LAPSUS_FEAT_LED_ID_PREFIX))
	{
		return true;
	}

	return false;
}

bool SysBackend::isLEDFeature(const QString &id, QString &led)
{
	if (!isLEDFeature(id))
		return false;

	led = id.mid(strlen(LAPSUS_FEAT_LED_ID_PREFIX));

	return true;
}

QStringList SysBackend::getFeatures()
{
	return _featurePaths.keys();
}

QString SysBackend::readIdString(const QString &id)
{
	if (!_featurePaths.contains(id)) return QString();

	return readPathString(_featurePaths[id]);
}

QString SysBackend::readPathString(const QString &path)
{
	int fd = open(qPrintable(path), O_RDONLY);

	QString ret;

	if (fd < 0) return ret;

	char buf[201];
	int c;

	do
	{
		c = read(fd, buf, 200);

		if (c > 0)
		{
			buf[c] = '\0';

			ret = ret.append(buf);
		}
	}
	while(c == 200);

	close(fd);

	return ret;
}

bool SysBackend::writeIdString(const QString &id, const QString &val)
{
	if (!_featurePaths.contains(id)) return false;

	return writePathString(_featurePaths[id], val);
}

bool SysBackend::writePathString(const QString &path, const QString &val)
{
	if (val.length() < 1) return false;

	int fd = open(qPrintable(path), O_WRONLY);

	if (fd < 0) return false;

	int c = write(fd, val.ascii(), val.length());

	close(fd);

	if (c < 1) return false;

	return true;
}

uint SysBackend::readIdUInt(const QString &id)
{
	if (!_featurePaths.contains(id)) return 0;

	return readPathUInt(_featurePaths[id]);
}

uint SysBackend::readPathUInt(const QString &path)
{
	bool ok = false;

	int fd = open(qPrintable(path), O_RDONLY);

	if (fd < 0) return 0;

	char buf[21];
	int c = read(fd, buf, 20);

	close(fd);

	c = correctBuf(buf, c);

	if (c < 1) return 0;

	buf[c] = '\0';

	int ret = QString(buf).toUInt(&ok);

	if (!ok)
		return 0;

	return ret;
}

bool SysBackend::writeIdUInt(const QString &id, uint newVal)
{
	if (!_featurePaths.contains(id)) return false;

	return writePathUInt(_featurePaths[id], newVal);
}

bool SysBackend::writePathUInt(const QString &path, uint newVal)
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

uint SysBackend::correctBuf(char *buf, uint len)
{
	uint i;

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
