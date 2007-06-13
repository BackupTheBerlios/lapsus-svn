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

#include "panel_widget.h"
#include "panel_slider.h"
#include "panel_vol_slider.h"
#include "panel_button.h"
#include "lapsus_dbus.h"

LapsusPanelWidget::LapsusPanelWidget(Qt::Orientation orientation,
			QWidget *parent, LapsusFeature *feat):
	QWidget(parent), LapsusIcons(feat),
	_feature(feat), _panelOrientation( orientation )
{
	setBackgroundMode(X11ParentRelative);
}

LapsusPanelWidget::~LapsusPanelWidget()
{
}

bool LapsusPanelWidget::isValid()
{
	if (!_feature) return false;
	
	return _feature->isValid();
}

bool LapsusPanelWidget::hasDBus()
{
	if (!_feature) return false;
	
	return _feature->hasDBus();
}

// LapsusPanelWidget* LapsusPanelWidget::newAppletwidget(
// 	const QString &id, Qt::Orientation orientation,
// 	QWidget *parent, KConfig *cfg)
// {
// 	if (id.length() < 1) return 0;
// 
// 	cfg->setGroup(id.lower());
// 
// 	if (!cfg->hasKey("widget_type")
// 		|| !cfg->hasKey("feature_id"))
// 	{
// 		return 0;
// 	}
// 
// 	QString wType = cfg->readEntry("widget_type");
// 	QString fId = cfg->readEntry("feature_id");
// 
// 	if (fId.length() < 1
// 		|| LapsusDBus::get()->getFeatureName(fId).length() < 1
// 		|| LapsusDBus::get()->getFeatureArgs(fId).size() < 1)
// 	{
// 		return 0;
// 	}
// 
// 	if (wType == "vol_slider")
// 	{
// 		return new LapsusPanelVolSlider(id.lower(), orientation,
// 				parent, cfg);
// 	}
// 	else if (wType == "slider")
// 	{
// 		return new LapsusPanelSlider(id.lower(), orientation,
// 				parent, cfg);
// 	}
// 	else if (wType == "button")
// 	{
// 		return new LapsusPanelButton(id.lower(), orientation,
// 				parent, cfg);
// 	}
// 
// 	return 0;
// }

void LapsusPanelWidget::resizeEvent( QResizeEvent * )
{
}
