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

#ifndef LAPSUS_CONF_H
#define LAPSUS_CONF_H

#include <kconfig.h>

#include "conf_base.h"
#include "osd.h"

#define LAPSUS_CONF_MAIN_GROUP		"applet"
#define LAPSUS_CONF_PANEL_LIST		"panel_entries"
#define LAPSUS_CONF_MENU_LIST		"menu_entries"
#define LAPSUS_CONF_AUTODETECT		"autodetect"

#define LAPSUS_CONF_PANEL_FEAT_PREFIX	"panel_"
#define LAPSUS_CONF_MENU_FEAT_PREFIX	"menu_"

#define LAPSUS_CONF_TRUE		"true"
#define LAPSUS_CONF_FALSE		"false"


class LapsusConf: public LapsusConfBase
{
	Q_OBJECT
	
	public:
		LapsusConf(QWidget *parent,
			KConfig *cfg);
		~LapsusConf();
	
	signals:
		void finished(bool dirty);
		
	protected slots:
		void tabChanged(QWidget *tab);
		void confOKClicked();
		void confCancelClicked();
	
	private:
		LapsusOSD* _osd;
		KConfig *_cfg;
		
		QPoint _osdPos;
};

#endif
