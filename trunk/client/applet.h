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


#ifndef LAPSUS_APPLET_H
#define LAPSUS_APPLET_H

// KDE
#include <kpanelapplet.h>
#include <kactioncollection.h>
#include <kpopupmenu.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <kbugreport.h>

#include "panel_main.h"
#include "conf_dialog.h"

class LapsusApplet : public KPanelApplet
{
	Q_OBJECT

	public:
		LapsusApplet(const QString& configFile, Type t = Normal,
			QWidget *parent = 0, const char *name = 0 );
		virtual ~LapsusApplet();

		void about();
		void help();
		void preferences();
		void reportBug();

		int widthForHeight(int) const;
		int heightForWidth(int) const;

	protected:
		void mousePressEvent( QMouseEvent *e );
		void resizeEvent( QResizeEvent * );
		
	private:
		KConfig _cfg;
		QBoxLayout *_layout;
		LapsusPanelMain *_mainWidget;
		Qt::Orientation _orientation;
		KAboutApplication *_aboutDlg;
		KBugReport *_bugDlg;
		LapsusConfDialog *_confDlg;
		KAboutData _aboutData;
		
		void changeOrientation(Qt::Orientation orientation);
};

#endif
