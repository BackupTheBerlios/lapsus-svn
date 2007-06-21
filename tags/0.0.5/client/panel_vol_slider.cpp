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

#include <qwidget.h>

#include "lapsus.h"
#include "lapsus_dbus.h"

#include "panel_vol_slider.h"
#include "lapsus_vol_slider.h"

LapsusPanelVolSlider::LapsusPanelVolSlider(Qt::Orientation orientation, QWidget *parent,
		LapsusVolSlider *sliderFeat):
		LapsusPanelSlider(orientation, parent, sliderFeat)
{
	if (!dbusValid()) return;
	
	connect(this, SIGNAL(toggleMute()),
		sliderFeat, SLOT(toggleMute()));

	connect(sliderFeat, SIGNAL(sliderMuteUpdate(bool)),
			_slider, SLOT(setGray(bool)));
}

LapsusPanelVolSlider::~LapsusPanelVolSlider()
{
}

bool LapsusPanelVolSlider::eventFilter( QObject* obj, QEvent* e )
{
	if (e->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent *qme = static_cast<QMouseEvent*>(e);

		if (qme->button() == Qt::MidButton)
		{
			emit toggleMute();
			
			return true;
		}
	}
	
	return LapsusPanelSlider::eventFilter(obj,e);
}
