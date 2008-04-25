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

#include "lapsus.h"
#include "lapsus_slider.h"
#include "panel_slider.h"

LapsusSlider::LapsusSlider(KConfig *cfg, const QString &dbusID,
		LapsusFeature::Place where, const char *featureType):
	LapsusFeature(cfg, dbusID, where, featureType),
	_valMin(0), _valMax(0), _val(0)
{
	if (_dbusValid)
	{
		if (!getMinMaxArgs(getFeatureArgs(), &_valMin, &_valMax))
		{
			_dbusValid = false;
			return;
		}
	}
}

LapsusSlider::~LapsusSlider()
{
}

bool LapsusSlider::getMinMaxArgs(const QStringList & args, int *minV, int *maxV)
{
	bool ret = false;
	int tmpMinV, tmpMaxV;
	
	for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
	{
		QStringList list = QStringList::split(':', *it);
		
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

void LapsusSlider::dbusSliderUpdate(const QString &val, bool isNotif)
{
	bool ok = false;
	int iVal = val.toInt(&ok);
	
	if (!ok) return;
	
	if (isNotif)
	{
		emit sliderNotif(iVal);
	}
	else
	{
		_val = iVal;
		emit sliderUpdate(iVal);
	}
}

void LapsusSlider::dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif)
{
	if (id != _featDBusID) return;
	
	dbusSliderUpdate(val, isNotif);
	LapsusFeature::dbusFeatureUpdate(id, val, isNotif);
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

LapsusPanelWidget* LapsusSlider::createPanelWidget(Qt::Orientation orientation, QWidget *parent)
{
	return new LapsusPanelSlider(orientation, parent, this);
}

bool LapsusSlider::saveFeature()
{
	if (!LapsusFeature::saveFeature()) return false;
	
	// TODO - here all options specific for Slider Feature should be saved.
	
	return true;
}
