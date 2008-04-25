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

#ifndef LAPSUS_PANEL_DEFAULT_H
#define LAPSUS_PANEL_DEFAULT_H

#include <qlabel.h>

#include "panel_widget.h"

class LapsusPanelDefault : public LapsusPanelWidget
{
	Q_OBJECT

	public:
		LapsusPanelDefault(Qt::Orientation orientation,	QWidget *parent);
		~LapsusPanelDefault();

		QSize sizeHint() const;
		QSize minimumSizeHint() const;
		QSizePolicy sizePolicy() const;

	signals:
		void rightButtonPressed();

	protected:
		void mousePressEvent( QMouseEvent * );

	private:
		QBoxLayout* _layout;
		QLabel* _label;
};

#endif
