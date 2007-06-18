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
#include "lapsus_vol_slider.h"
#include "panel_vol_slider.h"

LapsusVolSlider::LapsusVolSlider(KConfig *cfg, const QString &dbusID,
		LapsusFeature::Place where, const char *featureType):
	LapsusSlider(cfg, dbusID, where, featureType),
	_isMuted(false)
{
	if (_dbusValid)
	{
		bool hasMute = false;
		bool hasUnMute = false;
		QStringList args = getFeatureArgs();
	
		for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
		{
			if (*it == LAPSUS_FEAT_MUTE) hasMute = true;
			else if (*it == LAPSUS_FEAT_UNMUTE) hasUnMute = true;
		}
	
		if (!hasMute || !hasUnMute)
		{
			_dbusValid = false;
			return;
		}
		
		args = QStringList::split(",", getFeatureValue());
		
		for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
		{
			if (*it == LAPSUS_FEAT_MUTE) _isMuted = true;
			else if (*it == LAPSUS_FEAT_UNMUTE) _isMuted = false;
			
			bool ok = false;
			int tmp = (*it).toInt(&ok);
			if (ok) _val = tmp;
		}
	}
}

LapsusVolSlider::~LapsusVolSlider()
{
}

void LapsusVolSlider::dbusSliderUpdate(const QString &val, bool isNotif)
{
	QStringList args = QStringList::split(",", val);
	QStringList otherArgs;
	
	bool setGray = false;
	
	for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
	{
		if (*it == LAPSUS_FEAT_MUTE)
		{
			setGray = true;
		}
		else if (*it == LAPSUS_FEAT_UNMUTE)
		{
			setGray = false;
		}
		else
		{
			otherArgs.append(*it);
		}
	}
	
	if (setGray != _isMuted)
	{
		if (isNotif)
		{
			emit sliderMuteNotif(_isMuted);
		}
		else
		{
			_isMuted = setGray;
			emit sliderMuteUpdate(_isMuted);
		}
	}
	
	if (otherArgs.size() > 0)
	{
		LapsusSlider::dbusSliderUpdate(otherArgs.join(","), isNotif);
	}
}

void LapsusVolSlider::toggleMute()
{
	setSliderMute(!_isMuted);
}

void LapsusVolSlider::setSliderMute(bool val)
{
	setFeatureValue(val ? LAPSUS_FEAT_MUTE : LAPSUS_FEAT_UNMUTE);
}

LapsusPanelWidget* LapsusVolSlider::createPanelWidget(Qt::Orientation orientation, QWidget *parent)
{
	return new LapsusPanelVolSlider(orientation, parent, this);
}

bool LapsusVolSlider::saveFeature()
{
	if (!LapsusSlider::saveFeature()) return false;
	
	// TODO - here all options specific for VolSlider Feature should be saved.
	
	return true;
}
