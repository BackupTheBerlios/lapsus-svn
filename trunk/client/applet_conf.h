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


#ifndef LAPSUS_APPLET_CONF_H
#define LAPSUS_APPLET_CONF_H

#include <qwidget.h>
#include <kdialogbase.h>

#include "colorwidget.h"

class ColorWidget;

class AppletConfigDialog : public KDialogBase
{
	Q_OBJECT
	public:
		AppletConfigDialog( QWidget * parent=0, const char * name=0 );
		virtual ~AppletConfigDialog() {};

		void setActiveColors(const QColor& high, const QColor& low, const QColor& back);
		void activeColors(QColor& high, QColor& low, QColor& back) const;

	protected slots:
		virtual void slotOk();
		virtual void slotApply();

	signals:
		void applied();

	private:
		ColorWidget* colorWidget;
};

#endif
