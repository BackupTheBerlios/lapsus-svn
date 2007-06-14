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

#include "lapsus.h"
#include "lapsus_slider.h"

#define LAPSUS_CONF_WIDGET_SLIDER		"slider"

LapsusSlider::LapsusSlider(KConfig *cfg, const QString &idConf, const char *idDBus):
	LapsusFeature(cfg, idConf, idDBus),
	_valMin(0), _valMax(0), _val(0)
{
	if (!_isValid) return;

	if (!getMinMaxArgs(getFeatureArgs(), &_valMin, &_valMax))
	{
		_isValid = false;
		return;
	}
	
	bool ok = false;
	int tmp;
	
	tmp = getFeatureValue().toInt(&ok);
	
	if (!ok)
	{
		_isValid = false;
		return;
	}
	
	_val = tmp;
}

LapsusSlider::~LapsusSlider()
{
}

bool LapsusSlider::getMinMaxArgs(const QStringList & args, int *minV, int *maxV)
{
	bool ret = false;
	int tmpMinV, tmpMaxV;
	
	for (uint i = 0; i < args.size(); ++i)
	{
		QStringList list = QStringList::split(':', args[i]);

		if (list.size() == 2)
		{
			bool ok = false;
			
			tmpMinV = list[0].toInt(&ok);
			
			if (ok) tmpMaxV = list[1].toInt(&ok);

			if (ok && tmpMinV < tmpMaxV )
			{
				*minV = tmpMinV;
				*maxV = tmpMaxV;
				ret = true;
			}
		}
	}
	
	return ret;
}

bool LapsusSlider::supportsArgs(const QStringList & args)
{
	int minV, maxV;

	return getMinMaxArgs(args, &minV, &maxV);
}

void LapsusSlider::dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif)
{
	if (id != _featDBusID) return;
	
	bool ok = false;
	int iVal = val.toInt(&ok);
	
	if (!ok) return;
	
	_val = iVal;
	
	if (isNotif)
	{
		emit sliderNotif(iVal);
	}
	else
	{
		emit sliderUpdate(iVal);
	}
}

void LapsusSlider::setSliderValue(int val)
{
	setFeatureValue(QString::number(val));
}

int LapsusSlider::getSliderValue()
{
	return _val;
}

int LapsusSlider::getSliderMin()
{
	return _valMin;
}


int LapsusSlider::getSliderMax()
{
	return _valMax;
}

bool LapsusSlider::saveFeature()
{
	if (!_cfg || !_isValid) return false;
	
	addConfigEntry(_featConfID, _featDBusID, _cfg);
	return true;
}

void LapsusSlider::addConfigEntry(const QString &confID, const QString &dbusID, KConfig *cfg)
{
	cfg->deleteGroup(confID);
	cfg->setGroup(confID);
	cfg->writeEntry(LAPSUS_CONF_WIDGET_TYPE, LAPSUS_CONF_WIDGET_SLIDER);
	cfg->writeEntry(LAPSUS_CONF_FEATURE_ID, dbusID);
}

const char* LapsusSlider::featureType()
{
	return LAPSUS_CONF_WIDGET_SLIDER;
}
