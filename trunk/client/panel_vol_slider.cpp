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
#include "panel_vol_slider.h"

LapsusPanelVolSlider::LapsusPanelVolSlider( const QString &id,
	Qt::Orientation orient, QWidget *parent, LapsusDBus *dbus, KConfig *cfg) :
		LapsusPanelSlider(id, orient, parent, dbus, cfg)
{
}

LapsusPanelVolSlider::~LapsusPanelVolSlider()
{
}

bool LapsusPanelVolSlider::supportsArgs(const QStringList & args)
{
	bool hasMute = false;
	bool hasUnMute = false;
	bool okMinMax = false;
	
	for (uint i = 0; i < args.size(); ++i)
	{
		QStringList list = QStringList::split(':', args[i]);

		if (list.size() == 2)
		{
			int minV = list[0].toInt();
			int maxV = list[1].toInt();

			if ( minV < maxV )
				okMinMax = true;
		}
		else if (list.size() < 2)
		{
			if (args[i] == LAPSUS_FEAT_MUTE) hasMute = true;
			else if (args[i] == LAPSUS_FEAT_UNMUTE) hasUnMute = true;
		}
	}
	
	if (hasMute && hasUnMute && okMinMax) return true;
	
	return false;
}

void LapsusPanelVolSlider::featureChanged(const QString &id, const QString &val)
{
	if (id == _featureId)
	{
		QStringList args = QStringList::split(",", val);
			
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
		}
		
		_slider->setGray(setGray);
		
		LapsusPanelSlider::featureChanged(id, val);
	}
}

bool LapsusPanelVolSlider::eventFilter( QObject* obj, QEvent* e )
{
	if (e->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent *qme = static_cast<QMouseEvent*>(e);

		if (qme->button() == Qt::MidButton)
		{
			if (_dbus && _hasDBus)
			{
				_slider->setGray(!_slider->gray());
				
				_dbus->setFeature(_featureId, (_slider->gray())?LAPSUS_FEAT_MUTE:LAPSUS_FEAT_UNMUTE);
			}
			
			return true;
		}
	}
	
	return LapsusPanelSlider::eventFilter(obj,e);
}
