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

#ifndef LAPSUS_PANEL_MAIN_H
#define LAPSUS_PANEL_MAIN_H

#include <qwidget.h>
#include <kconfig.h>
#include <kactioncollection.h>
#include <kpopupmenu.h>

#include "lapsus_dbus.h"
#include "flowlayout.h"

class LapsusPanelMain : public QWidget
{
	Q_OBJECT

	public:
		LapsusPanelMain(QWidget *parent, LapsusDBus *dbus,
			Qt::Orientation orientation);

		virtual ~LapsusPanelMain();

		int widthForHeight(int) const;
		int heightForWidth(int) const;

		QSize sizeHint() const;
		QSize minimumSize() const;

		void appletPreferences();

	protected:
		LapsusDBus *_dbus;

		void resizeEvent( QResizeEvent * );
		void mousePressEvent( QMouseEvent * );

	public slots:
		void showContextMenu();

	private:
		KConfig _cfg;
		QStringList _panelEntries;
		QStringList _menuEntries;
		FlowLayout* _layout;
		KPopupMenu* _popMenu;
		KActionCollection* _actions;
		Qt::Orientation _orientation;

		void saveConfig();
		void loadConfig();
};

#endif
