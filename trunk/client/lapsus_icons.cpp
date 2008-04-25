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

#include <qpainter.h>
#include <qimage.h>
#include <kiconloader.h>

#include "lapsus.h"
#include "lapsus_icons.h"

LapsusIcons::LapsusIcons(LapsusFeature* feat): _feature(feat)
{
	if (!feat) return;
	
	QString fId = feat->getFeatureDBusID();
	
	int idx = fId.findRev('.');
	
	if (idx > 0)
	{
		_featureType = fId.mid(idx+1);
	}
}

LapsusIcons::~LapsusIcons()
{
}

int LapsusIcons::loadNewAutoIcon(int size)
{
	if (_featureType == LAPSUS_FEAT_BACKLIGHT_ID)
		return loadNewIcon("light_bulb", "", size);
	else if (_featureType == LAPSUS_FEAT_VOLUME_ID)
		return loadNewIcon("speaker", "", size);
	else if (_featureType == LAPSUS_FEAT_LIGHT_SENSOR_LEVEL_ID)
		return loadNewIcon("moon", "", size);

	return -1;
}

int LapsusIcons::loadNewAutoIcon(const QString &val, int size)
{
	QString img;
	QString desc;

	if (_featureType == LAPSUS_FEAT_BLUETOOTH_ID)
	{
		if (val == LAPSUS_FEAT_ON) img = "bluetooth";
		else if (val == LAPSUS_FEAT_OFF) img = "bluetooth_gray";
	}
	else if (_featureType == LAPSUS_FEAT_WIRELESS_ID)
	{
		if (val == LAPSUS_FEAT_ON) img = "wifi";
		else if (val == LAPSUS_FEAT_OFF) img = "wifi_gray";
	}
	else if (_featureType.startsWith(LAPSUS_FEAT_LED_ID_PREFIX))
	{
		if (val == LAPSUS_FEAT_ON) img = "green";
		else if (val == LAPSUS_FEAT_OFF) img = "gray";
		else if (val == LAPSUS_FEAT_BLINK) img = "orange";

		int len = strlen(LAPSUS_FEAT_LED_ID_PREFIX);

		if ((int) _featureType.length() > len)
		{
			desc = _featureType.mid(len, 1).upper();
		}
	}
	else if (_featureType == LAPSUS_FEAT_TOUCHPAD_ID)
	{
		if (val == LAPSUS_FEAT_ON) img = "touchpad";
		else if (val == LAPSUS_FEAT_OFF) img = "touchpad_gray";
	}
	else
	{
		// TODO - what default values?
		if (val == LAPSUS_FEAT_ON) img = "yellow";
		else if (val == LAPSUS_FEAT_OFF) img = "gray";
	}

	return loadNewIcon(img, desc, size);
}

int LapsusIcons::loadNewIcon(const QString &img, int size)
{
	return loadNewIcon(img, "", size);
}

int LapsusIcons::loadNewIcon(const QString &img, const QString &desc, int size)
{
	if (img.length() < 1) return -1;

	QPixmap imgBase = UserIcon(img);

	if (imgBase.isNull()) return -1;

	if (desc.length() > 0)
	{
		QPainter p;
		int iH = imgBase.height();
		int iW = imgBase.width();
		QFont f;

		int maxH = iH/2;
		int maxW = iW/2;

		int nSize = QMIN(imgBase.height(), imgBase.width())*6/10;
		QSize size;

		do
		{
			f.setPointSize(nSize);
			QFontMetrics fm(f);
			size = fm.size(0, desc, 1);
			nSize--;
		}
		while(nSize > 5 && size.width() > maxW && size.height() > maxH);

		p.begin( &imgBase);
		p.setFont(f);

		p.drawText((iW - size.width())/2, (iH + (size.height()*5)/6) / 2, desc, 1);
	}

	int idx = _cachedIcons.count();

	_cachedIcons.push_back(
		QPixmap(imgBase.convertToImage().smoothScale(size, size)));

	return idx;
}

QPixmap LapsusIcons::getIcon(int id) const
{
	if (id < 0 || id >= (int) _cachedIcons.count())
		return QPixmap();

	return _cachedIcons[id];
}
