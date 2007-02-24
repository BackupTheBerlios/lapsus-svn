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

// Qt
#include <qlayout.h>
#include <qwidget.h>

// KDE
#include <kpanelapplet.h>
#include <kactioncollection.h>
#include <kpopupmenu.h>

#include "ksmallslider.h"
#include "colorwidget.h"
#include "applet_conf.h"
#include "ldbus.h"

class LapsusApplet : public KPanelApplet
{
	Q_OBJECT

	public:
		LapsusApplet(const QString& configFile, Type t = Normal,
			QWidget *parent = 0, const char *name = 0 );
		virtual ~LapsusApplet();

		struct Colors {
			QColor high, low, back;
		};

		void about();
		void help();
		void preferences();
		void reportBug();

		QSize sizeHint() const;
		bool eventFilter( QObject* obj, QEvent* e );
		QSizePolicy sizePolicy() const;
		int widthForHeight(int) const;
		int heightForWidth(int) const;

	protected slots:
		void applyPreferences();
		void preferencesDone();
		void updateGeometrySlot();
		void toggleSwitch(const QString &name, bool nValue);
		void sliderValueChanged(int nValue);
		void dbusSwitchChanged(const QString &name, bool nValue);
		void dbusBacklightChanged(uint nValue);

	protected:
		void resizeEvent( QResizeEvent * );
		void saveConfig();
		void loadConfig();

	private:
		void positionChange(Position);
		void setColors();
		void setColors( const Colors &color );
		void showContextMenu();

		LapsusDBus _dbus;
		KPopupMenu* _popMenu;
		KActionCollection* _actions;
		LapsusApplet::Colors _colors;

		AppletConfigDialog* _pref;
		KSmallSlider* _slider;
		QBoxLayout* _layout;
		QLabel* _iconLabel;
};

#endif
