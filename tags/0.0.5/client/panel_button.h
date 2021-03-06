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

#ifndef LAPSUS_PANEL_BUTTON_H
#define LAPSUS_PANEL_BUTTON_H

#include <qlabel.h>

#include "ksmallslider.h"
#include "panel_widget.h"

class LapsusSwitch;

class LapsusPanelButton : public LapsusPanelWidget
{
	Q_OBJECT

	public:
		LapsusPanelButton(Qt::Orientation orientation,
			QWidget *parent, LapsusSwitch *feat);
		~LapsusPanelButton();

		QSize sizeHint() const;
		QSize minimumSizeHint() const;
		QSizePolicy sizePolicy() const;

		bool eventFilter( QObject* obj, QEvent* e );

		static bool supportsArgs(const QStringList & args);
		
	signals:
		void rightButtonPressed();

	protected slots:
		void buttonUpdate(const QString &val);

	private:
		LapsusSwitch* _switchFeat;
		QBoxLayout* _layout;
		QLabel* _iconLabel;
		QMap<QString, int> _icons;
};

#endif
