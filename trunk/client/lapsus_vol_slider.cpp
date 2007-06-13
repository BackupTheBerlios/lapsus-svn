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

#define LAPSUS_CONF_WIDGET_VOL_SLIDER		"vol_slider"

LapsusVolSlider::LapsusVolSlider(KConfig *cfg, const QString &idConf, const char *idDBus):
	LapsusSlider(cfg, idConf, idDBus), _isMuted(false)
{
}

LapsusVolSlider::~LapsusVolSlider()
{
}

bool LapsusVolSlider::supportsArgs(const QStringList & args)
{
	if (!LapsusSlider::supportsArgs(args)) return false;
	
	bool hasMute = false;
	bool hasUnMute = false;
	
	for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
	{
		if (*it == LAPSUS_FEAT_MUTE) hasMute = true;
		else if (*it == LAPSUS_FEAT_UNMUTE) hasUnMute = true;
	}
	
	return (hasMute && hasUnMute);
}

void LapsusVolSlider::dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif)
{
	if (id != _featDBusID) return;
	
	QStringList args = QStringList::split(",", val);
	QStringList otherArgs;
	
	bool setGray = false;
	
	for (uint i = 0; i < args.size(); ++i)
	{
		if (args[i] == LAPSUS_FEAT_MUTE)
		{
			setGray = true;
		}
		else if (args[i] == LAPSUS_FEAT_UNMUTE)
		{
			setGray = false;
		}
		else
		{
			otherArgs.append(args[i]);
		}
	}
	
	if (setGray != _isMuted)
	{
		_isMuted = setGray;
		
		if (isNotif)
		{
			emit sliderMuteNotif(_isMuted);
		}
		else
		{
			emit sliderMuteUpdate(_isMuted);
		}
	}
	
	if (otherArgs.size() > 0)
	{
		LapsusSlider::dbusFeatureUpdate(id, otherArgs.join(","), isNotif);
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

bool LapsusVolSlider::saveFeature()
{
	if (!_cfg || !_isValid) return false;
	
	// TODO - save additional fields.
	return LapsusSlider::saveFeature();
}

bool LapsusVolSlider::addConfigEntry(const QString &confID, const QString &dbusID, KConfig *cfg)
{
	cfg->deleteGroup(confID);
	cfg->setGroup(confID);
	cfg->writeEntry(LAPSUS_CONF_WIDGET_TYPE, LAPSUS_CONF_WIDGET_VOL_SLIDER);
	cfg->writeEntry(LAPSUS_CONF_FEATURE_ID, dbusID);
	
	return true;
}

const char* LapsusVolSlider::featureType()
{
	return LAPSUS_CONF_WIDGET_VOL_SLIDER;
}
