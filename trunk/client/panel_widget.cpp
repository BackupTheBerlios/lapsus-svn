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
#include "panel_button.h"

LapsusPanelWidget::LapsusPanelWidget( const QString &id,
			Qt::Orientation orientation, QWidget *parent,
			LapsusDBus *dbus, KConfig *cfg):
	QWidget( parent, id ), _dbus(dbus), _cfg(cfg),
	_panelOrientation( orientation ), _id( id )
{
	setBackgroundMode(X11ParentRelative);
}

LapsusPanelWidget::~LapsusPanelWidget()
{
}

LapsusPanelWidget* LapsusPanelWidget::newAppletwidget(
	const QString &id, Qt::Orientation orientation,
	QWidget *parent, LapsusDBus *dbus, KConfig *cfg)
{
	if (id.length() < 1) return 0;

	cfg->setGroup(id.lower());

	if (!cfg->hasKey("widget_type"))
	{
		return 0;
	}

	QString wType = cfg->readEntry("widget_type");

	if (wType == "slider")
	{
		return new LapsusPanelSlider(id.lower(), orientation,
				parent, dbus, cfg);
	}
	else if (wType == "button")
	{
		return new LapsusPanelButton(id.lower(), orientation,
				parent, dbus, cfg);
	}

	return 0;
}

void LapsusPanelWidget::resizeEvent( QResizeEvent * )
{
}
