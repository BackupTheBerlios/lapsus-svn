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

#include <qpainter.h>
#include <qimage.h>
#include <kiconloader.h>

#include "lapsus_icons.h"

LapsusIcons::LapsusIcons(const QString &id, KConfig *cfg):
	_cfg(cfg)
{
	_cfg->setGroup(id);

	if (_cfg->hasKey("feature_id"))
	{
		_featureId = _cfg->readEntry("feature_id");
	}
}

LapsusIcons::~LapsusIcons()
{
}

int LapsusIcons::loadNewAutoIcon(int size)
{
	if (_featureId.find("_backlight") >= 0)
		return loadNewIcon("light_bulb", "", size);
	else if (_featureId.find("_volume") >= 0)
		return loadNewIcon("speaker", "", size);

	return -1;
}

int LapsusIcons::loadNewAutoIcon(const QString &val, int size)
{
	QString first;
	QString second;
	int i;

	if (_featureId.find("bluetooth") >= 0)
	{
		if (val == "on") first = "bluetooth";
		else if (val == "off") first = "bluetooth_gray";
	}
	else if (_featureId.find("wireless") >= 0)
	{
		if (val == "on") first = "wifi";
		else if (val == "off") first = "wifi_gray";
	}
	else if ((i = _featureId.find("_led_")) >= 0 )
	{
		if (val == "on") first = "green";
		else if (val == "off") first = "gray";
		else if (val == "blink") first = "orange";

		if ((int) _featureId.length() > i+5)
		{
			second = _featureId.mid(i+5, 1);
		}
	}
	else
	{
		// TODO - what default values?
		if (val == "on") first = "yellow";
		else if (val == "off") first = "gray";
	}

	return loadNewIcon(first, second, size);
}

int LapsusIcons::loadNewIcon(const QString &first, int size)
{
	return loadNewIcon(first, QString(), size);
}

int LapsusIcons::loadNewIcon(const QString &first, const QString &second, int size)
{
	if (first.length() < 1) return -1;

	QPixmap imgFirst = UserIcon(first);

	if (imgFirst.isNull()) return -1;

	if (second.length() > 0)
	{
		QPixmap imgSecond = UserIcon(second);

		if (!imgSecond.isNull())
		{
			QPainter p;

			p.begin( &imgFirst);
			p.drawPixmap (0, 0, imgSecond);
		}
	}

	int idx = _cachedIcons.count();

	_cachedIcons.push_back(
		QPixmap(imgFirst.convertToImage().smoothScale(size, size)));

	return idx;
}

QPixmap LapsusIcons::getIcon(int id) const
{
	if (id < 0 || id >= (int) _cachedIcons.count())
		return QPixmap();

	return _cachedIcons[id];
}
