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

#ifndef LAPSUS_PANEL_WIDGET_H
#define LAPSUS_PANEL_WIDGET_H

#include <kconfig.h>

#include <qlayout.h>
#include <qwidget.h>

#include "lapsus_icons.h"

class LapsusFeature;

class LapsusPanelWidget : public QWidget, protected LapsusIcons
{
	Q_OBJECT

	public:
		LapsusPanelWidget(Qt::Orientation orientation,
			QWidget *parent, LapsusFeature *feat);
		virtual ~LapsusPanelWidget();
		
		virtual bool dbusValid();
		virtual bool dbusActive();
		
	protected:
		LapsusFeature* _feature;
		Qt::Orientation _panelOrientation;
		
		virtual void resizeEvent( QResizeEvent * );
};

#endif
